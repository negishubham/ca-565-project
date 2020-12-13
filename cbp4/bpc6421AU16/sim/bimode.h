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

#ifndef __BI_MODE_PRED_HH__
#define __BI_MODE_PRED_HH__

#include "utils.h"
#include "tracer.h"
#include <vector>
#include "predictor.h"

class BiMode : public virtual PREDICTOR
{
  public:
    BiMode(void);
    virtual bool    GetPrediction(UINT32 PC);  
    virtual void    UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget);
    virtual void    TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget);

  private:
    void updateGlobalHistReg(bool taken);

    unsigned instShiftAmt;

    unsigned globalHistoryReg;
    // was the taken array's prediction used?
    // true: takenPred used
    // false: notPred used
    bool takenUsed;
    // prediction of the taken array
    // true: predict taken
    // false: predict not-taken
    bool takenPred;
    // prediction of the not-taken array
    // true: predict taken
    // false: predict not-taken
    bool notTakenPred;
    // the final taken/not-taken prediction
    // true: predict taken
    // false: predict not-taken

    unsigned globalHistoryBits;
    unsigned historyRegisterMask;

    unsigned choicePredictorSize;
    unsigned choiceCtrBits;
    unsigned choiceHistoryMask;
    unsigned globalPredictorSize;
    unsigned globalCtrBits;
    unsigned globalHistoryMask;

    // choice predictors
    std::vector<unsigned> choiceCounters;
    // taken direction predictors
    std::vector<unsigned> takenCounters;
    // not-taken direction predictors
    std::vector<unsigned> notTakenCounters;

    unsigned choiceCtrMax;
    unsigned globalCtrMax;
    unsigned choiceThreshold;
    unsigned takenThreshold;
    unsigned notTakenThreshold;
};

#endif // __BI_MODE_PRED_HH__
