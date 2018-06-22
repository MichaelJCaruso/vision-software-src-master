#ifndef Vxa_VCollection_Interface
#define Vxa_VCollection_Interface

/************************
 *****  Components  *****
 ************************/

#include "Vxa_VSet.h"

#include "Vxa_ICollection.h"

/**************************
 *****  Declarations  *****
 **************************/

/*************************
 *****  Definitions  *****
 *************************/

namespace Vxa {
    class VCallHandle;
    class VClass;
    class VCollectableObject;
    class VCollectableCollection;
    class VTask;

    class Vxa_API VCollection : public VSet {
	DECLARE_ABSTRACT_RTTLITE (VCollection, VSet);

        friend class VCallData;

    //  Aliases
    public:
	typedef cardinality_t collection_index_t;

    //  Construction
    protected:
	VCollection ();

    //  Destruction
    protected:
	~VCollection ();


    //  Access
    private:
        virtual VCollectableObject *object_ (collection_index_t xElement) const {
            return 0;
        }
        virtual VClass *type_() const {
            return 0;
        }
    public:
        VCollectableObject *object (collection_index_t xElement) const {
            return object_(xElement);
        }
        VClass *type () const {
            return type_();
        }

    //  Roles
    public:
	using BaseClass::getRole;

    //  ICollection
    private:
	Vca::VRole<ThisClass,ICollection> m_pICollection;
    public:
	void getRole (ICollection::Reference &rpRole) {
	    m_pICollection.getRole (rpRole);
	}

    //  ICollection Methods
    public:
	void Bind (
	    ICollection *pRole, ICaller *pCaller, VString const &rMessageName, cardinality_t cParameters, cardinality_t cTask
	);
	void Invoke (
	    ICollection *pRole, ICaller *pCaller, VString const &rMessageName, cardinality_t cParameters, cardinality_t cTask
	);
	void QueryCardinality (ICollection *pRole, IVReceiver<cardinality_t> *pResultReceiver);

    //  ISingleton Methods
    public:
	void ExternalImplementation (
	    ISingleton *pRole, IVSNFTaskHolder *pCaller, VString const &rMessageName, cardinality_t cParameters, cardinality_t cTask
	);

    //  Use
    private:
	/*-----------------------------
	 *  External calls start here with the invocation of this virtual class member.
	 *  Override in a subclass to provide the actual implementation.  There is no
	 *  default implementation - the override must take full responsibility for
	 *  implementing the message or returning an error to the caller.  Successful
	 *  overrides should indicate that by returning true - errors and SNF's should
	 *  return false.  (This convention may change in the future).
	 *-----------------------------
	 */
	virtual bool invokeCall (VCallHandle const &rCallHandle) = 0;

        virtual bool launchTask (VTask *pTask) = 0;
    };
}


#endif
