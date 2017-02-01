#ifndef Vxa_VCollectableCollectionOf_Interface
#define Vxa_VCollectableCollectionOf_Interface

/************************
 *****  Components  *****
 ************************/

#include "Vxa_VCollectableCollection.h"

#include "VkDynamicArrayOf.h"

/**************************
 *****  Declarations  *****
 **************************/

#include "Vxa_VCardinalityHints.h"

/*************************
 *****  Definitions  *****
 *************************/

namespace Vxa {
    template <typename Val_T, typename Var_T>
    class VCollectableCollectionOf : public VCollectableCollection {
	typedef VCollectableCollectionOf<Val_T,Var_T> this_t;
	DECLARE_CONCRETE_RTTLITE (this_t, VCollectableCollection);

    //  Aliases
    public:
	typedef ThisClass collection_t;
	typedef Reference collection_reference_t;
	typedef VkDynamicArrayOf<Var_T> container_t;

	typedef Val_T val_t;
	typedef Var_T var_t;

    //  Construction
    public:
	VCollectableCollectionOf (
	    VClass *pClass, VCardinalityHints *pTailHints, Val_T pInstance
	) : BaseClass (pClass), m_iContainer (pTailHints ? pTailHints->span () : 1) {
//################################################################
//  Calling attach causes a nasty stack dump which is very
//  useful for testing error detection and recovery...
//	    attach (pInstance);
	    append (pInstance);
	    onCollectionCreation (1);
	}

    //  Destruction
    protected:
	~VCollectableCollectionOf () {
	    for (collection_index_t xObject = 0; xObject < m_iContainer.cardinality (); xObject++)
		BaseClass::detach (m_iContainer[xObject]);
	    onCollectionDeletion (cardinality ());
	}

    //  Access
    public:
	Val_T operator[] (unsigned int xElement) const {
	    return element (xElement);
	}
	Val_T element (unsigned int xElement) const {
	    return xElement < cardinality() ? m_iContainer[xElement] : static_cast<Val_T>(0);
	}

    //  Update
    public:
	void append (Val_T pInstance) {
	    collection_index_t const xObject = cardinality ();
	    if (xObject >= m_iContainer.cardinality ())
		m_iContainer.Append (xObject - m_iContainer.cardinality () + 1);
	    m_iContainer[xObject].setTo (pInstance);
	    attach (pInstance);
	}

    //  State
    private:
	container_t m_iContainer;
    };
}


#endif
