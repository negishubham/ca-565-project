/*
 * Copyright (c) 2014 The Regents of The University of Michigan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* @file
 * Implementation of a bi-mode branch predictor
 */

#include "cpu/pred/yags.hh"

#include "base/bitfield.hh"
#include "base/intmath.hh"

YAGSBP::YAGSBP(const YAGSBPParams *params)
    : BPredUnit(params),
      setAssociativity(params->setAssociativity),
      globalHistoryReg(params->numThreads, 0),
      globalHistoryBits(ceilLog2(params->choicePredictorSize)),         // 13
      choicePredictorSize(params->choicePredictorSize),                 // 8192
      choiceCtrBits(params->choiceCtrBits),                             // 2   
      globalPredictorSize(params->globalPredictorSize/setAssociativity),   // 8192
      globalCtrBits(params->globalCtrBits),                                 // 2
      tagBits(params->tagBits),
      choiceCounters(choicePredictorSize, SatCounter(choiceCtrBits, (ULL(1) << (choiceCtrBits - 1)) - 1)),       // 8192
      takenCache(globalPredictorSize),
      notTakenCache(globalPredictorSize)
      
{
    if (!isPowerOf2(choicePredictorSize))
        fatal("Invalid choice predictor size.\n");
    if (!isPowerOf2(globalPredictorSize))
        fatal("Invalid global history predictor size.\n");

    historyRegisterMask = mask(globalHistoryBits);
    choiceHistoryMask = choicePredictorSize - 1;
    globalPredictorMask = globalPredictorSize - 1;

    tagMask = mask(tagBits);

    choiceThreshold = (ULL(1) << (choiceCtrBits - 1)) - 1;
    takenThreshold = (ULL(1) << (globalCtrBits - 1)) - 1;
    notTakenThreshold = (ULL(1) << (globalCtrBits - 1)) - 1;

    for(int size=0; size < globalPredictorSize; size++){
        takenCache[size].tag.resize(setAssociativity);
        takenCache[size].recentUsedRank.resize(setAssociativity);
        takenCache[size].LRU = setAssociativity-1;
        notTakenCache[size].tag.resize(setAssociativity);
        notTakenCache[size].recentUsedRank.resize(setAssociativity);
        notTakenCache[size].LRU = setAssociativity-1;
        for(int set=0; set < setAssociativity; set++){
            takenCache[size].ctr.push_back(SatCounter(globalCtrBits, takenThreshold+1));
            takenCache[size].tag[set] = 0;
            takenCache[size].recentUsedRank[set] = set;
            notTakenCache[size].ctr.push_back(SatCounter(globalCtrBits, notTakenThreshold));
            notTakenCache[size].tag[set] = 0;
            notTakenCache[size].recentUsedRank[set] = set;
        }
    }
}


/*
 * For an unconditional branch we set its history such that
 * everything is set to taken. I.e., its choice predictor
 * chooses the taken array and the taken array predicts taken.
 */
void
YAGSBP::uncondBranch(ThreadID tid, Addr pc, void * &bpHistory)
{
    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];
    history->cacheUsed = 1;
    history->takenPred = true;
    history->notTakenPred = true;
    history->finalPred = true;
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, true);
}

void
YAGSBP::squash(ThreadID tid, void *bpHistory)
{
    BPHistory *history = static_cast<BPHistory*>(bpHistory);
    globalHistoryReg[tid] = history->globalHistoryReg;

    delete history;
}

/*
 * Here we lookup the actual branch prediction. We use the PC to
 * identify the bias of a particular branch, which is based on the
 * prediction in the choice array. A hash of the global history
 * register and a branch's PC is used to index into both the taken
 * and not-taken predictors, which both present a prediction. The
 * choice array's prediction is used to select between the two
 * direction predictors for the final branch prediction.
 */

void
YAGSBP::updateTakenCacheLRU(unsigned idx, unsigned set){
    unsigned oldRank = takenCache[idx].recentUsedRank[set];
    for(int i=0; i< setAssociativity; i++){
        if(takenCache[idx].recentUsedRank[i] < oldRank){
            takenCache[idx].recentUsedRank[i]++;
        }
        if(takenCache[idx].recentUsedRank[i] == setAssociativity-1){
            takenCache[idx].LRU = i;
        }
    }
    takenCache[idx].recentUsedRank[set] = 0;
}

void
YAGSBP::updateNotTakenCacheLRU(unsigned idx, unsigned set){
    unsigned oldRank = notTakenCache[idx].recentUsedRank[set];
    for(int i=0; i< setAssociativity; i++){
        if(notTakenCache[idx].recentUsedRank[i] < oldRank){
            notTakenCache[idx].recentUsedRank[i]++;
        }
        if(notTakenCache[idx].recentUsedRank[i] == setAssociativity-1){
            notTakenCache[idx].LRU = i;
        }
    }
    notTakenCache[idx].recentUsedRank[set] = 0;
}

bool
YAGSBP::lookup(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    unsigned choiceHistoryIdx = ((branchAddr >> instShiftAmt)
                                & choiceHistoryMask);
    unsigned globalPredictorIdx = (((branchAddr >> instShiftAmt)
                                ^ globalHistoryReg[tid])
                                & globalPredictorMask);

    assert(choiceHistoryIdx < choicePredictorSize);
    assert(globalPredictorIdx < globalPredictorSize);

    unsigned tagMatch = ((branchAddr >> instShiftAmt) & tagMask);
   
    bool choicePrediction = choiceCounters[choiceHistoryIdx]
                            > choiceThreshold;


    bool takenCachePrediction = false;
    bool notTakenCachePrediction = false;
    bool finalPrediction = false;
    bool takenCacheTagFound = false;
    bool notTakenCacheTagFound = false;

    BPHistory *history = new BPHistory;
    history->globalHistoryReg = globalHistoryReg[tid];

    // Lookup taken cache //
    for(int i=0; i < setAssociativity; i++){
        //updateTakenCacheLRU(globalPredictorIdx, i);
        if(takenCache[globalPredictorIdx].tag[i] == tagMatch){
            updateTakenCacheLRU(globalPredictorIdx, i);
            if(takenCache[globalPredictorIdx].ctr[i] > takenThreshold){
                takenCachePrediction = true;
            } else {
                takenCachePrediction = false;
            }
            takenCacheTagFound = true;
            break;
        }
    }

    // Lookup not-taken cache //
    for(int i=0; i < setAssociativity; i++){
        //updateNotTakenCacheLRU(globalPredictorIdx, i);
        if(notTakenCache[globalPredictorIdx].tag[i] == tagMatch){
            updateNotTakenCacheLRU(globalPredictorIdx, i);
            if(notTakenCache[globalPredictorIdx].ctr[i] > notTakenThreshold){
                notTakenCachePrediction = true;
            } else {
                notTakenCachePrediction = false;
            }
            notTakenCacheTagFound = true;
            break;
        }
    }

    if(takenCacheTagFound){
        history->takenPred = takenCachePrediction;
    }    
    if(notTakenCacheTagFound){
        history->notTakenPred = notTakenCachePrediction;
    }

    // Get prediction with choice PHT //
    if(choicePrediction){
        if(takenCacheTagFound){
            history->cacheUsed = 1;
            finalPrediction = takenCachePrediction;
        }
        else if(notTakenCacheTagFound){
            history->cacheUsed = 2;
            finalPrediction = notTakenCachePrediction;
        }
        else{
            history->cacheUsed = 0;
            finalPrediction = choicePrediction;
        }
    }
    else{
        if(notTakenCacheTagFound){
            history->cacheUsed = 2;
            finalPrediction = notTakenCachePrediction;
        }
        else if(takenCacheTagFound){
            history->cacheUsed = 1;
            finalPrediction = takenCachePrediction;            
        }
        else{
            history->cacheUsed = 0;
            finalPrediction = choicePrediction;
        }
    }
    history->finalPred = finalPrediction;
    bpHistory = static_cast<void*>(history);
    updateGlobalHistReg(tid, finalPrediction);

    return finalPrediction;
}



void
YAGSBP::btbUpdate(ThreadID tid, Addr branchAddr, void * &bpHistory)
{
    globalHistoryReg[tid] &= (historyRegisterMask & ~ULL(1));
}

void
YAGSBP::updateTakenCache(unsigned idx, unsigned tag, bool taken){

    bool takenCacheTagFound = false;

    // Lookup taken cache //
    for(int i=0; i < setAssociativity; i++){
        if(takenCache[idx].tag[i] == tag){
            updateTakenCacheLRU(idx, i);
            takenCacheTagFound = true;
            if(taken){
                takenCache[idx].ctr[i]++;
            }
            else{
                takenCache[idx].ctr[i]--;
            }
            break;
        }
    }
    
    if(!takenCacheTagFound){
        unsigned oldest = takenCache[idx].LRU;
        takenCache[idx].tag[oldest] = tag;
        takenCache[idx].ctr[oldest].reset();    // reset to '10'
        if(taken){
            takenCache[idx].ctr[oldest]++;
        }
        else{ //(!taken){
            takenCache[idx].ctr[oldest]--;      // if final outcome was not-taken --> '01'
        }
        updateTakenCacheLRU(idx, oldest);
    }
}
void
YAGSBP::updateNotTakenCache(unsigned idx, unsigned tag, bool taken){
    
    bool notTakenCacheTagFound = false;

    // Lookup taken cache //
    for(int i=0; i < setAssociativity; i++){
        if(notTakenCache[idx].tag[i] == tag){
            updateNotTakenCacheLRU(idx, i);
            notTakenCacheTagFound = true;
            if(taken){
                notTakenCache[idx].ctr[i]++;
            }
            else{
                notTakenCache[idx].ctr[i]--;
            }
            break;
        }
    }
    
    if(!notTakenCacheTagFound){
        unsigned oldest = notTakenCache[idx].LRU;
        notTakenCache[idx].tag[oldest] = tag;
        notTakenCache[idx].ctr[oldest].reset(); 
        if(taken){
            notTakenCache[idx].ctr[oldest]++; 
        }
        else{
            notTakenCache[idx].ctr[oldest]--;
        }
        updateNotTakenCacheLRU(idx, oldest);
    }
}

/* Only the selected direction predictor will be updated with the final
 * outcome; the status of the unselected one will not be altered. The choice
 * predictor is always updated with the branch outcome, except when the
 * choice is opposite to the branch outcome but the selected counter of
 * the direction predictors makes a correct final prediction.
 */
void
YAGSBP::update(ThreadID tid, Addr branchAddr, bool taken, void *bpHistory,
                 bool squashed, const StaticInstPtr & inst, Addr corrTarget)
{
    assert(bpHistory);

    BPHistory *history = static_cast<BPHistory*>(bpHistory);

    // We do not update the counters speculatively on a squash.
    // We just restore the global history register.
    if (squashed) {
        globalHistoryReg[tid] = (history->globalHistoryReg << 1) | taken;
        globalHistoryReg[tid] &= historyRegisterMask;
        return;
    }

    unsigned choiceHistoryIdx = ((branchAddr >> instShiftAmt)
                                & choiceHistoryMask);
    unsigned globalHistoryIdx = (((branchAddr >> instShiftAmt)
                                ^ history->globalHistoryReg)
                                & globalPredictorMask);

    assert(choiceHistoryIdx < choicePredictorSize);
    assert(globalHistoryIdx < globalPredictorSize);

    unsigned tag = ((branchAddr >> instShiftAmt) & tagMask);

    if (history->cacheUsed == 0) {
        // if both taken & not-taken cache were not used == choice PHT determined the prediction
        if (history->finalPred == taken) {
            // prediction correct
            if (taken) {
                choiceCounters[choiceHistoryIdx]++;
            } else {
                choiceCounters[choiceHistoryIdx]--;
            }
        }
        else if (taken){
            // prediction wrong : prediction was not-taken but truth is taken
            choiceCounters[choiceHistoryIdx]++;
            updateTakenCache(globalHistoryIdx, tag, taken);
        }
        else{
            // prediction wrong : prediction was taken but truth is not-taken
            choiceCounters[choiceHistoryIdx]--;
            updateNotTakenCache(globalHistoryIdx, tag, taken);
        }
    } else if (history->cacheUsed == 1){
        // if taken cache was used, update choice PHT
        // when prediction was wrong as well as the final outcome was not-taken
        if (history->finalPred != taken || taken) {
            if (taken) {
                choiceCounters[choiceHistoryIdx]++;
            } else {
                choiceCounters[choiceHistoryIdx]--;
            }
        }
        updateTakenCache(globalHistoryIdx, tag, taken);

    } else if (history->cacheUsed == 2){
        // if not-taken cache was used, update choice PHT
        // when prediction was wrong as well as the final outcome was taken
        if (history->finalPred != taken || !taken) {
            if (taken) {
                choiceCounters[choiceHistoryIdx]++;
            } else {
                choiceCounters[choiceHistoryIdx]--;
            }
        }
        updateNotTakenCache(globalHistoryIdx, tag, taken);
    }

    delete history;
}

void
YAGSBP::updateGlobalHistReg(ThreadID tid, bool taken)
{
    globalHistoryReg[tid] = taken ? (globalHistoryReg[tid] << 1) | 1 :
                               (globalHistoryReg[tid] << 1);
    globalHistoryReg[tid] &= historyRegisterMask;
}


YAGSBP*
YAGSBPParams::create()
{
    return new YAGSBP(this);
}
