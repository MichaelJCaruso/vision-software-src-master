#ifndef Vca_VBSProducerConsumerDevice__Interface
#define Vca_VBSProducerConsumerDevice__Interface

/*********************
 *****  Library  *****
 *********************/

#include "Vca.h"

/************************
 *****  Components  *****
 ************************/

#include "Vca_VDevice_.h"

/**************************
 *****  Declarations  *****
 **************************/

/*************************
 *****  Definitions  *****
 *************************/

namespace Vca {
    /**
     * Abstract device providing both read and write capabilities.
     *
     * @tparam Implementation delegate class used to provide device-specific implementation details.
     */
    template <class Implementation> class VBSProducerConsumerDevice_
	: public VDevice_<Implementation>
	, private VDevice::BSReadFace
	, private VDevice::BSWriteFace
    {
	DECLARE_CONCRETE_RTTLITE (VBSProducerConsumerDevice_<Implementation>, VDevice_<Implementation>);

    //  Aliases
    public:
	typedef typename Implementation::Data ImplementationData;
	typedef typename BaseClass::Get Get;
	typedef typename BaseClass::Put Put;
	typedef typename BaseClass::ReadPoll ReadPoll;
	typedef typename BaseClass::WritePoll WritePoll;

    //  Construction
    public:
	VBSProducerConsumerDevice_(ImplementationData &rDeviceData)
	    : BaseClass (rDeviceData), VDevice::BSReadFace (this), VDevice::BSWriteFace (this)
	{
	}

    //  Destruction
    private:
	~VBSProducerConsumerDevice_() {
	}

    //  Face Implementation
    private:
	virtual VDevice *device_() OVERRIDE {
	    return this;
	}
	virtual bool getName_(VkStatus &rStatus, VString &rName) OVERRIDE {
	    return static_cast<BaseClass*>(this)->getName (rStatus, rName);
	}
	virtual bool start_(
	    VkStatus &rStatus, VDeviceBSReader *pUser, VDeviceBSReadArea const &rArea
	) OVERRIDE {
	    VReference<Get> pUse (new Get (this));
	    return pUse->start (rStatus, pUser, rArea);
	}
	virtual bool start_(
	    VkStatus &rStatus, VDeviceBSWriter *pUser, VDeviceBSWriteArea const &rArea
	) OVERRIDE {
	    VReference<Put> pUse (new Put (this));
	    return pUse->start (rStatus, pUser, rArea);
	}
        virtual bool start_(
	    VkStatus &rStatus, VDeviceBSReader *pUser
        ) OVERRIDE {
	    VReference<ReadPoll> pUse (new ReadPoll (this));
	    return pUse->start (rStatus, pUser);
        }
        virtual bool start_(
	    VkStatus &rStatus, VDeviceBSWriter *pUser
        ) OVERRIDE {
	    VReference<WritePoll> pUse (new WritePoll (this));
	    return pUse->start (rStatus, pUser);
        }

    //  Face Access
    public:
        virtual operator BSWriteFace* () OVERRIDE { return this; }
        virtual operator BSReadFace* () OVERRIDE { return this; }

    //  User Accounting
    private:
	virtual void onFirstUser_(VDevice::BSReadFace *pFace) OVERRIDE {
	    static_cast<Implementation*>(this)->onFirstReader ();
	}
	virtual void onFinalUser_(VDevice::BSReadFace *pFace) OVERRIDE {
	    static_cast<Implementation*>(this)->onFinalReader ();
	}

	virtual void onFirstUser_(VDevice::BSWriteFace *pFace) OVERRIDE {
	    static_cast<Implementation*>(this)->onFirstWriter ();
	}
	virtual void onFinalUser_(VDevice::BSWriteFace *pFace) OVERRIDE {
	    static_cast<Implementation*>(this)->onFinalWriter ();
	}

    //  User Creation
    private:
	virtual bool supplyConnection_(VReference<VConnection>&rpUser) OVERRIDE {
	    return static_cast<VDevice::BSReadFace*>(this)->supply (rpUser);
	}
	virtual bool supplyBSProducer_(VReference<VBSProducer>&rpUser) OVERRIDE {
	    return static_cast<VDevice::BSReadFace*>(this)->supply (rpUser);
	}
	virtual bool supplyBSConsumer_(VReference<VBSConsumer>&rpUser) OVERRIDE {
	    return static_cast<VDevice::BSWriteFace*>(this)->supply (rpUser);
	}
    };
}


#endif
