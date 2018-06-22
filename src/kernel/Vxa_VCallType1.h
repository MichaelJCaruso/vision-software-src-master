#ifndef Vxa_VCallType1_Interface
#define Vxa_VCallType1_Interface

/************************
 *****  Components  *****
 ************************/

#include "Vxa_VCallHandle.h"

/**************************
 *****  Declarations  *****
 **************************/

#include "Vxa_IVSNFTaskHolder.h"

#include "Vxa_VScalar.h"

/*************************
 *****  Definitions  *****
 *************************/

namespace Vxa {
    template <class Handle_T> class VCallAgentFor;
    class VCallAgent;

    class VCallType1Exporter;
    class VCallType1Importer;

    class VImportableType;

    class VTask;

    /******************************
     *----  class VCallType1  ----*
     ******************************/

    class Vxa_API VCallType1 : public VCallHandle {
	DECLARE_FAMILY_MEMBERS (VCallType1, VCallHandle);

	friend class VCallAgentFor<ThisClass>;

    //  Aliases
    public:
	typedef VCallType1Exporter Exporter;
	typedef VCallType1Importer Importer;

    //****************************************************************
    //  Construction
    public:
	VCallType1 (
            VCollection *pCluster, VString const &rMethodName, cardinality_t cParameters, cardinality_t ocTask, IVSNFTaskHolder *pTask
        );
	VCallType1 (ThisClass const &rOther);

    //  Destruction
    public:
	~VCallType1 ();

    //  Access
    private:
	virtual IVUnknown *caller () const OVERRIDE;

    //  Invocation
    public:
	virtual bool invokeMethod (VMethod *pMethod) const OVERRIDE;

    //  Parameter Acquisition
    protected:
	void returnImplementationHandle (IVSNFTaskImplementation *pHandle) const;
	bool onParameterRequest (VTask *pTask, unsigned int xParameter) const;
	bool onParameterReceipt (VTask *pTask, unsigned int xParameter) const;
    public:
	template <typename provider_t> bool getSelfProviderFor (
            provider_t &rpProvider, VTask *pTask
	) const {
            typedef typename provider_t::ReferencedClass::value_t val_t;
	    rpProvider.setTo (
		new VScalarInstance<val_t>(
		    TaskObjectClass (pTask), dynamic_cast<val_t>(TaskObject (pTask, 0))
		)
	    );
	    return true;
	}
    private:
        static VClass *TaskObjectClass (VTask *pTask);
        static VCollectableObject *TaskObject (VTask *pTask, object_reference_t xObject);

    //  Result Return
    public:
	template <typename T> bool returnConstant (T iT) const {
	    return raiseResultTypeException (typeid(*this), typeid (T));
	}
	bool returnConstant (bool iConstant) const;
	bool returnConstant (short iConstant) const;
	bool returnConstant (unsigned short iConstant) const;
	bool returnConstant (int iConstant) const;
	bool returnConstant (unsigned int iConstant) const;
	bool returnConstant (float iConstant) const;
	bool returnConstant (double iConstant) const;
	bool returnConstant (char const *pConstant) const;
	bool returnConstant (VString const &rConstant) const;

	bool returnVector (VkDynamicArrayOf<bool> const &rVector) const;
	bool returnVector (VkDynamicArrayOf<int> const &rVector) const;
	bool returnVector (VkDynamicArrayOf<float> const &rVector) const;
	bool returnVector (VkDynamicArrayOf<double> const &rVector) const;
	bool returnVector (VkDynamicArrayOf<VString> const &rVector) const;
	bool returnVector (VkDynamicArrayOf<ISingleton::Reference> const &rVector) const;

	virtual bool returnError (VString const &rMessage) const OVERRIDE;
	virtual bool returnSNF () const OVERRIDE;

    //  Task Management
    public:
	bool start (VTask *pTask) const;

    //  State
    private:
	IVSNFTaskHolder::Reference const m_pCaller;
    };
}


#endif
