#ifndef VSecondaryCall_Interface
#define VSecondaryCall_Interface

/************************
 *****  Components  *****
 ************************/

#include "VCall.h"

/**************************
 *****  Declarations  *****
 **************************/

/*************************
 *****  Definitions  *****
 *************************/

class VSecondaryCall : public VCall {
//  Run Time Type
    DECLARE_CONCRETE_RTT (VSecondaryCall, VCall);

//  Meta Maker
protected:
    static void MetaMaker ();

//  Construction
public:
    VSecondaryCall (VTask* pCaller, rtLINK_CType* pCallerSubset);

//  Access
public:
    virtual rtBLOCK_Handle *boundBlock () const OVERRIDE;

    virtual VSelector const &selector_() const OVERRIDE;

//  State
protected:
};


#endif
