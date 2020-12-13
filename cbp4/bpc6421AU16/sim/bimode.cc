#include "bimode.h"

BiMode::BiMode(void){
    
    instShiftAmt = 0;

    takenUsed = false;
    takenPred = false;
    notTakenPred = false;

    globalHistoryReg = 0;
    globalHistoryBits = 17; 
    choicePredictorSize = 1 << globalHistoryBits;
    choiceCtrBits = 2;
    globalPredictorSize = choicePredictorSize;
    globalCtrBits = 2;

    
    historyRegisterMask = choicePredictorSize - 1;
    choiceHistoryMask = choicePredictorSize - 1;
    globalHistoryMask = globalPredictorSize - 1;

    choiceCtrMax = (1 << choiceCtrBits ) - 1;
    globalCtrMax = (1 << globalCtrBits) - 1;
    choiceThreshold = (1 << (choiceCtrBits - 1)) - 1;
    takenThreshold = (1 << (globalCtrBits - 1)) - 1; 
    notTakenThreshold = (1 << (globalCtrBits - 1)) - 1; 

    choiceCounters.resize(choicePredictorSize, choiceThreshold+1);
    takenCounters.resize(globalPredictorSize, takenThreshold + 1);
    notTakenCounters.resize(globalPredictorSize, notTakenThreshold);
}


bool
BiMode::GetPrediction(UINT32 PC){
    unsigned choiceHistoryIdx = ((PC >> instShiftAmt)
                                & choiceHistoryMask);
    unsigned globalHistoryIdx = (((PC >> instShiftAmt)
                                ^ globalHistoryReg) 
                                & globalHistoryMask);


    bool choicePrediction = choiceCounters[choiceHistoryIdx]
                            > choiceThreshold;
    bool takenGHBPrediction = takenCounters[globalHistoryIdx]
                              > takenThreshold;
    bool notTakenGHBPrediction = notTakenCounters[globalHistoryIdx]
                                 > notTakenThreshold;
    bool finalPrediction = false;

    takenUsed = choicePrediction;
    takenPred = takenGHBPrediction;
    notTakenPred = notTakenGHBPrediction;

    if (choicePrediction) {
        finalPrediction = takenGHBPrediction;
    } else {
        finalPrediction = notTakenGHBPrediction;
    }
    //updateGlobalHistReg(finalPrediction);
    return finalPrediction;
}

void
BiMode::UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget){
    // We do not update the counters speculatively on a squash.
    // We just restore the global history register.


    unsigned choiceHistoryIdx = ((PC >> instShiftAmt)
                                & choiceHistoryMask);
    unsigned globalHistoryIdx = (((PC >> instShiftAmt)
                                ^ globalHistoryReg)
                                & globalHistoryMask);


    if (takenUsed) {
        // if the taken array's prediction was used, update it
        if (resolveDir) {
            takenCounters[globalHistoryIdx] = SatIncrement(takenCounters[globalHistoryIdx], globalCtrMax);
        } else {
            takenCounters[globalHistoryIdx] = SatDecrement(takenCounters[globalHistoryIdx]);
        }
    } else {
        // if the not-taken array's prediction was used, update it
        if (resolveDir) {
            notTakenCounters[globalHistoryIdx] = SatIncrement(takenCounters[globalHistoryIdx], globalCtrMax);
        } else {
            notTakenCounters[globalHistoryIdx] = SatDecrement(takenCounters[globalHistoryIdx]);
        }
    }

    if (predDir == resolveDir) {
        if (takenUsed) {
            if (resolveDir) {
                choiceCounters[choiceHistoryIdx] = SatIncrement(choiceCounters[choiceHistoryIdx], choiceCtrMax);
            }
        }
        else {
            if (!resolveDir) {
                choiceCounters[choiceHistoryIdx] = SatDecrement(choiceCounters[choiceHistoryIdx]);
            }
        }
    } else {
        // always update the choice predictor on an incorrect prediction
        if (resolveDir) {
            choiceCounters[choiceHistoryIdx] = SatIncrement(choiceCounters[choiceHistoryIdx], choiceCtrMax);
        } else {
            choiceCounters[choiceHistoryIdx] = SatDecrement(choiceCounters[choiceHistoryIdx]);
        }
    }
    updateGlobalHistReg(resolveDir);
}

void BiMode::TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget){

  // This function is called for instructions which are not
  // conditional branches, just in case someone decides to design
  // a predictor that uses information from such instructions.
  // We expect most contestants to leave this function untouched.

  return;
}

void
BiMode::updateGlobalHistReg(bool taken)
{
    globalHistoryReg = taken ? (globalHistoryReg << 1) | 1 :
                               (globalHistoryReg << 1);
    //globalHistoryReg &= historyRegisterMask;
}
