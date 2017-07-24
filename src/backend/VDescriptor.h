#ifndef VDescriptor_Interface
#define VDescriptor_Interface

/************************
 *****  Components  *****
 ************************/

#include "VTransient.h"

#include "m.h"
#include "vdsc.h"

#include "VFragmentation.h"

/**************************
 *****  Declarations  *****
 **************************/

#include "RTlink.h"
#include "RTrefuv.h"
#include "RTvector.h"

/*************************
 *****  Definitions  *****
 *************************/


class VDescriptor : public VTransient {
//  Aliases
public:
    typedef Vdd::Store Store;

//  Types & Enumerations...
public:
/*---------------------------------------------------------------------------*
 * Extended descriptors are a polymorphic generalization of descriptors as
 * defined in 'DSC'.  The following enumerated type defines the
 *
 * Virtual machine extended descriptors can have one of the following types:
 *
 *	Empty			- a descriptor referencing no objects.
 *	Standard		- a descriptor for a homomorphic collection of
 *				  objects in the form of a standard descriptor
 *				  (see "vdsc.h").
 *	Vector			- a descriptor for a polymorphic collection of
 *				  objects in the form of a vector.
 *	VectorC			- a descriptor for a polymorphic collection of
 *				  objects in the form of a vector contructor.
 *	Fragmentation		- a descriptor for a polymorphic collection of
 *				  of objects in the form of a fragmentation.
 *---------------------------------------------------------------------------*/
    enum Type {
	Type_Empty,
	Type_Std,
	Type_Vector,
	Type_VectorC,
	Type_Fragmentation
    };

//  Counters, Parameters, and Switches
public:
    static unsigned int HomogenizeVCCount;
    static unsigned int HomogenizeVectorCount;

    static bool DefragmentationAttemptEnabled;

//  Construction/Initialization
public:
    VDescriptor ();

protected:
    void construct () {
	m_xType = Type_Empty;
	m_pDistribution = NilOf (M_CPD*);
    }

private:
    VDescriptor& operator= (VDescriptor const& rSource) {
	m_xType		= rSource.m_xType;
	m_pDistribution	= rSource.m_pDistribution;
	m_uContent	= rSource.m_uContent;
	return *this;
    }

//  Destruction
public:
    ~VDescriptor ();

//  Query
public:
    bool isEmpty () const {
	return m_xType == Type_Empty;
    }
    bool isStandard () const {
	return m_xType == Type_Std;
    }
    bool isAVector () const {
	return m_xType == Type_Vector;
    }
    bool isAVectorC () const {
	return m_xType == Type_VectorC;
    }
    unsigned isAFragmentation () const {
	return m_xType == Type_Fragmentation;
    }

//  Access
public:
    void assignTo (DSC_Pointer &rSubscript, Vdd::Store *pTarget);
    void assignTo (DSC_Scalar &rSubscript, Vdd::Store *pTarget);
    void assignTo (rtLINK_CType *pSubscript, Vdd::Store *pTarget);

    M_CPD *distribution	() const { return m_pDistribution; }

    rtLINK_CType *subsetInStore (Store *pStore, VDescriptor *pValueReturn);
    rtLINK_CType *subsetInStore (Store *pStore) {
	return subsetInStore (pStore, 0);
    }
    rtLINK_CType *subsetOfType (
	M_ASD *pSubsetSpace, M_KOTM pKOTM, VDescriptor *pValueReturn
    );
    rtLINK_CType *subsetOfType (M_ASD *pSubsetSpace, M_KOTM pKOTM) {
	return subsetOfType (pSubsetSpace, pKOTM, 0);
    }

    Type type () const { return m_xType; }
    char const *typeName () const;

public: //protected:
    DSC_Descriptor const& contentAsMonotype () const {
	return m_uContent.asDescriptor;
    }
    VFragmentation const& contentAsFragmentation () const {
	return m_uContent.asFragmentation;
    }
    rtVECTOR_Handle *contentAsVector () const {
	return m_uContent.asVector;
    }
    rtVECTOR_CType *contentAsVectorC () const {
	return m_uContent.asVectorC;
    }

    DSC_Descriptor& contentAsMonotype () {
	return m_uContent.asDescriptor;
    }
    VFragmentation &contentAsFragmentation () {
	return m_uContent.asFragmentation;
    }
    rtVECTOR_Handle *&contentAsVector () {
	return m_uContent.asVector;
    }
    rtVECTOR_CType *&contentAsVectorC () {
	return m_uContent.asVectorC;
    }

//  Distribution
public:
    M_CPD *claimedDistribution () {
	M_CPD *pDistribution = m_pDistribution;
	m_pDistribution = NilOf (M_CPD*);
	return pDistribution;
    }

    void distribute (M_CPD *pDistribution) {
    /*****
     *	pDistribution		- a standard CPD for a reference u-vector
     *				  containing the distribution to apply to
     *				  this descriptor.  This method will duplicate
     *				  a pointer to this CPD if that will most
     *				  efficiently implement its algorithm.
     *****/
	if (IsntNil (m_pDistribution)) {
	    M_CPD *pNewDistribution = rtREFUV_UVExtract (
		pDistribution, m_pDistribution
	    );
	    m_pDistribution->release ();
	    m_pDistribution = pNewDistribution;
	}
	else {
	    checkDistribution (pDistribution);
	    pDistribution->retain ();
	    m_pDistribution = pDistribution;
	}
    }

    void reorder (M_CPD *pReordering) {
    /*****
     *	pReordering		- a standard CPD for a reference u-vector
     *				  containing the reordering to apply to
     *				  this descriptor.  This method will duplicate
     *				  a pointer to this CPD if that will most
     *				  efficiently implement its algorithm.
     *****/
	M_CPD *pDistribution = rtREFUV_Flip (pReordering);
	distribute (pDistribution);
	pDistribution->release ();
    }

    void normalize ();

//  Update
protected:
    void clearContent ();

public:
    void clear ();

    bool convertMonotypeToFragmentation (unsigned int fragmentSize);

    bool convertVectorsToMonotypeIfPossible ();

    VFragmentation &convertToFragmentation (bool fCoalescing = DefragmentationAttemptEnabled);

    rtVECTOR_Handle *convertToVector ();

    rtVECTOR_CType *convertToVectorC ();

protected:
    void setContentToMoved (DSC_Descriptor& rSource) {
	clearContent ();
	contentAsMonotype () = rSource;
	rSource.construct ();
	m_xType = Type_Std;
    }
    void setContentToVector (rtVECTOR_Handle *pSource) {
	pSource->retain ();
	clearContent ();
	contentAsVector () = pSource;
	m_xType = Type_Vector;
    }
    void setContentToVector (rtVECTOR_CType *pSource) {
	pSource->retain ();
	clearContent ();
	contentAsVectorC () = pSource;
	m_xType = Type_VectorC;
    }

public:
    void setToCopied (VDescriptor const& rSource);
    void setToCopied (DSC_Descriptor const& rSource) {
	clear ();
	contentAsMonotype ().construct (rSource);
	m_xType = Type_Std;
    }

    void setToMoved (VDescriptor& rSource) {
	clear ();
	*this = rSource;
	rSource.construct ();
    }
    void setToMoved (DSC_Descriptor& rSource) {
	clear ();
	contentAsMonotype () = rSource;
	rSource.construct ();
	m_xType = Type_Std;
    }

    void setToReorderedSubset (
	M_CPD *pReordering, rtLINK_CType *pSubset, VDescriptor& rSource
    );

    void setToSubset (rtLINK_CType *pSubset, VDescriptor& rSource);
    void setToSubset (DSC_Pointer & rSubset, VDescriptor& rSource);

    void setToSubset (rtLINK_CType *pSubset, DSC_Descriptor& rSource) {
	clear ();
	contentAsMonotype ().constructComposition (pSubset, rSource);
	m_xType = Type_Std;
    }
    void setToSubset (DSC_Pointer& rSubset, DSC_Descriptor& rSource) {
	clear ();
	contentAsMonotype ().constructComposition (rSubset, rSource);
	m_xType = Type_Std;
    }

    void setToCorrespondence (rtPTOKEN_Handle *pPPT, Store *pStore) {
	clear ();
	contentAsMonotype ().constructCorrespondence (pPPT, pStore);
	m_xType = Type_Std;
    }

    void setToIdentity (Store *pStore, rtPTOKEN_Handle *pPToken) {
	clear ();
	contentAsMonotype ().constructIdentity (pStore, pPToken);
	m_xType = Type_Std;
    }

    /************************
     *  setToNA/True/False  *
     ************************/

    void setToNA (rtPTOKEN_Handle *pPPT, M_KOT *pKOT) {
	clear ();
	contentAsMonotype ().constructNA (pPPT, pKOT);
	m_xType = Type_Std;
    }
    void setToNA (M_KOT *pKOT) {
	clear ();
	contentAsMonotype ().constructNA (pKOT);
	m_xType = Type_Std;
    }

    void setToTrue (rtPTOKEN_Handle *pPPT, M_KOT *pKOT) {
	clear ();
	contentAsMonotype ().constructTrue (pPPT, pKOT);
	m_xType = Type_Std;
    }

    void setToFalse (rtPTOKEN_Handle *pPPT, M_KOT *pKOT) {
	clear ();
	contentAsMonotype ().constructFalse (pPPT, pKOT);
	m_xType = Type_Std;
    }

    void setToBoolean (rtPTOKEN_Handle *pPPT, M_KOT *pKOT, bool iBoolean) {
	clear ();
	contentAsMonotype ().constructBoolean (pPPT, pKOT, iBoolean);
	m_xType = Type_Std;
    }

    /*******************
     *  setToConstant  *
     *******************/

    void setToConstant (rtPTOKEN_Handle *pPPT, Store *pStore, rtPTOKEN_Handle *pRPT) {
	clear ();
	contentAsMonotype ().constructConstant (pPPT, pStore, pRPT);
	m_xType = Type_Std;
    }
    void setToConstant (rtPTOKEN_Handle *pPPT, M_KOTE const &rKOTE) {
	clear ();
	contentAsMonotype ().constructConstant (pPPT, rKOTE);
	m_xType = Type_Std;
    }

    void setToConstant (rtPTOKEN_Handle *pPPT, Store *pStore, rtPTOKEN_Handle *pRPT, char value) {
	clear ();
	contentAsMonotype ().constructConstant (pPPT, pStore, pRPT, value);
	m_xType = Type_Std;
    }
    void setToConstant (rtPTOKEN_Handle *pPPT, M_KOTE const &rKOTE, char value) {
	clear ();
	contentAsMonotype ().constructConstant (pPPT, rKOTE, value);
	m_xType = Type_Std;
    }
    void setToConstant (rtPTOKEN_Handle *pPPT, M_KOT *pKOT, char value) {
	setToConstant (pPPT, pKOT->TheCharacterClass, value);
    }

    void setToConstant (rtPTOKEN_Handle *pPPT, Store *pStore, rtPTOKEN_Handle *pRPT, double value) {
	clear ();
	contentAsMonotype ().constructConstant (pPPT, pStore, pRPT, value);
	m_xType = Type_Std;
    }
    void setToConstant (rtPTOKEN_Handle *pPPT, M_KOTE const &rKOTE, double value) {
	clear ();
	contentAsMonotype ().constructConstant (pPPT, rKOTE, value);
	m_xType = Type_Std;
    }
    void setToConstant (rtPTOKEN_Handle *pPPT, M_KOT *pKOT, double value) {
	setToConstant (pPPT, pKOT->TheDoubleClass, value);
    }

    void setToConstant (rtPTOKEN_Handle *pPPT, Store *pStore, rtPTOKEN_Handle *pRPT, float value) {
	clear ();
	contentAsMonotype ().constructConstant (pPPT, pStore, pRPT, value);
	m_xType = Type_Std;
    }
    void setToConstant (rtPTOKEN_Handle *pPPT, M_KOTE const &rKOTE, float value) {
	clear ();
	contentAsMonotype ().constructConstant (pPPT, rKOTE, value);
	m_xType = Type_Std;
    }
    void setToConstant (rtPTOKEN_Handle *pPPT, M_KOT *pKOT, float value) {
	setToConstant (pPPT, pKOT->TheFloatClass, value);
    }

    void setToConstant (rtPTOKEN_Handle *pPPT, Store *pStore, rtPTOKEN_Handle *pRPT, int value) {
	clear ();
	contentAsMonotype ().constructConstant (pPPT, pStore, pRPT, value);
	m_xType = Type_Std;
    }
    void setToConstant (rtPTOKEN_Handle *pPPT, M_KOTE const &rKOTE, int value) {
	clear ();
	contentAsMonotype ().constructConstant (pPPT, rKOTE, value);
	m_xType = Type_Std;
    }
    void setToConstant (rtPTOKEN_Handle *pPPT, M_KOT *pKOT, int value) {
	setToConstant (pPPT, pKOT->TheIntegerClass, value);
    }

    void setToReferenceConstant (
	rtPTOKEN_Handle *pPPT, Store *pStore, rtPTOKEN_Handle *pRPT, unsigned int value
    ) {
	clear ();
	contentAsMonotype ().constructReferenceConstant (pPPT, pStore, pRPT, value);
	m_xType = Type_Std;
    }

    void setToMonotype (M_CPD *pMonotype) {
	clear ();
	contentAsMonotype ().constructMonotype (pMonotype);
	m_xType = Type_Std;
    }
    void setToMonotype (Store *pStore, M_CPD *pMonotype) {
	clear ();
	if (pStore)
	    contentAsMonotype ().constructMonotype (pStore, pMonotype);
	else
	    contentAsMonotype ().constructMonotype (pMonotype);
	m_xType = Type_Std;
    }

    void setToMonotype (Store *pStore, rtLINK_CType *pMonotype) {
	clear ();
	contentAsMonotype ().constructMonotype (pStore, pMonotype);
	m_xType = Type_Std;
    }

    void setToVector (rtVECTOR_Handle *pSource) {
	pSource->retain ();
	clear ();
	contentAsVector () = pSource;
	m_xType = Type_Vector;
    }
    void setToVector (rtVECTOR_CType *pSource) {
	pSource->retain ();
	clear ();
	contentAsVectorC () = pSource;
	m_xType = Type_VectorC;
    }

    VFragmentation &setToFragmentation (rtPTOKEN_Handle *ptoken) {
	clear ();
	contentAsFragmentation ().construct (ptoken);
	m_xType = Type_Fragmentation;

	return contentAsFragmentation ();
    }

//  Consistency Checking
protected:
    void checkDistribution (M_CPD *pDistribution) const;

//  Description
public:
    void describe (unsigned int xLevel);

//  Error Reporting
public:
    void raiseTypeException (char const* pWhere = 0) const;

protected:
    void raiseAssignmentTargetException (Vdd::Store *pStore) const;

//  State
/*---------------------------------------------------------------------------*
 *	m_xType			- the descriptor type.
 *	m_pDistribution		- an optional ('Nil' if absent) standard CPD
 *				  for a reference u-vector.  When present, this
 *				  u-vector specifies the positions to which the
 *				  values contained in the 'content' component
 *				  of this descriptor must be 'distributed' when
 *				  viewed as an ordered collection.
 *	m_uContent		- a union of representation formats for a
 *				  descriptor.  The following describes those
 *				  formats:
 *	.asDescriptor		... a standard descriptor.  Used to represent
 *				    'Descriptor' descriptors.  May have its
 *				    value store overriden by 'context' defined
 *				    below.
 *	.asVector		... a standard CPD for a vector.  Used to
 *				    represent 'Vector' descriptors.
 *	.asVectorC		... a vector constructor.  Used to represent
 *				    'VectorC' descriptors.
 *	.asFragmentation	... a 'Fragmentation'.  Used to represent
 *				    fragmentation descriptors.
 *---------------------------------------------------------------------------*/
protected:
    Type		m_xType;
    M_CPD*		m_pDistribution;
    union content_t {
	DSC_Descriptor	    asDescriptor;
	rtVECTOR_Handle*    asVector;
	rtVECTOR_CType*	    asVectorC;
	VFragmentation	    asFragmentation;
    }			m_uContent;
};


#endif
