#ifndef VTaskDomainedStore_Interface
#define VTaskDomainedStore_Interface

/************************
 *****  Components  *****
 ************************/

#include "VInternalGroundStore.h"

/**************************
 *****  Declarations  *****
 **************************/

#include "VTaskDomain.h"

/*************************
 *****  Definitions  *****
 *************************/

class ABSTRACT VTaskDomainedStore : public VInternalGroundStore {
//  Run Time Type
    DECLARE_ABSTRACT_RTT (VTaskDomainedStore, VInternalGroundStore);

//  Meta Maker
protected:

//  Construction
protected:
    VTaskDomainedStore (VTaskDomain* pDomain) : m_pDomain (pDomain) {
    }

//  Access
public:
    unsigned int cardinality () const {
	return m_pDomain->cardinality ();
    }
    virtual unsigned int cardinality_() const OVERRIDE {
	return cardinality ();
    }

    VTaskDomain* domain () const {
	return m_pDomain;
    }

    rtPTOKEN_Handle *ptoken () const {
	return m_pDomain->ptoken ();
    }
    virtual rtPTOKEN_Handle *ptoken_() const OVERRIDE {
	return ptoken ();
    }

//  Query
public:
    bool isGround () const {
	return m_pDomain->isGround ();
    }
    bool isScalar () const {
	return m_pDomain->isScalar ();
    }
    bool isntScalar () const {
	return m_pDomain->isntScalar ();
    }

//  State
protected:
    VReference<VTaskDomain> const m_pDomain;
};


#endif
