#ifndef _YAGS_H_
#define _YAGS_H_

#include "utils.h"
#include "tracer.h"
#include <vector>
#include "predictor.h"

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class YAGS : public virtual PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:

  void updateGlobalHistReg(bool taken);

  struct cacheEntry{
    std::vector<UINT32> ctr;
    std::vector<UINT32> tag;
    UINT32 LRU; // idx of oldest 
    std::vector<UINT32> recentUsedRank; // Most recently used : 0
  };


  UINT32 cacheUsed;
  // prediction of the taken array
  // 0 : choice PHT used
  // 1 : taken cache used
  // 2 : not-taken cache used
  bool takenPred;
  // prediction of the not-taken array
  // true: predict taken
  // false: predict not-taken
  bool notTakenPred;
  // the final taken/not-taken prediction
  // true: predict taken
  // false: predict not-taken      

  UINT32 setAssociativity;

  UINT32 globalHistoryReg;
  UINT32 globalHistoryBits;
  UINT32 historyRegisterMask;

  UINT32 choicePredictorSize;
  UINT32 choiceCtrBits;
  UINT32 choiceCtrMax;
  UINT32 choiceHistoryMask;
  UINT32 globalPredictorSize;
  UINT32 globalCtrBits;
  UINT32 globalCtrMax;
  UINT32 globalPredictorMask;
  
  UINT32 tagBits;
  UINT32 tagMask;
  UINT32 instShiftAmt;
  // choice predictors
  std::vector<UINT32> choiceCounters;
  // taken direction predictors
  std::vector<struct cacheEntry> takenCache;
  // not-taken direction predictors
  std::vector<struct cacheEntry> notTakenCache;

  UINT32 choiceThreshold;
  UINT32 takenThreshold;
  UINT32 notTakenThreshold;

  void updateTakenCache(UINT32 idx, UINT32 tag, bool taken);
  void updateNotTakenCache(UINT32 idx, UINT32 tag, bool taken);
  void updateTakenCacheLRU(UINT32 idx, UINT32 set);
  void updateNotTakenCacheLRU(UINT32 idx, UINT32 set); 

public:

// The interface to the four functions below CAN NOT be changed

  YAGS(void);
  virtual bool    GetPrediction(UINT32 PC);  
  virtual void    UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget);
  virtual void    TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget);

  // Contestants can define their own functions below

};



/***********************************************************/
#endif

