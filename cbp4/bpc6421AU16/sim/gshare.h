#ifndef _GSHARE_H_
#define _GSHARE_H_

#include "utils.h"
#include "tracer.h"
#include "predictor.h"

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class GSHARE : public virtual PREDICTOR{

  // The state is defined for Gshare, change for your design

 private:
  UINT32  ghr;           // global history register
  UINT32  *pht;          // pattern history table
  UINT32  historyLength; // history length
  UINT32  numPhtEntries; // entries in pht 

 public:

  // The interface to the four functions below CAN NOT be changed

  GSHARE(void);
  
    virtual bool    GetPrediction(UINT32 PC);  
    virtual void    UpdatePredictor(UINT32 PC, bool resolveDir, bool predDir, UINT32 branchTarget);
    virtual void    TrackOtherInst(UINT32 PC, OpType opType, UINT32 branchTarget);

  // Contestants can define their own functions below

};



/***********************************************************/
#endif

