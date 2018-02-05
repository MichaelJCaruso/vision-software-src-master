#ifndef Vca_VBSConsumerDevice__Interface
#define Vca_VBSConsumerDevice__Interface

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
    template <class Implementation> class VBSConsumerDevice_
	: public VDevice_<Implementation>, private VDevice::BSWriteFace
    {
	DECLARE_CONCRETE_RTTLITE (VBSConsumerDevice_<Implementation>, VDevice_<Implementation>);

    //  Aliases
    public:
	typedef typename Implementation::Data ImplementationData;
	typedef typename BaseClass::Put Put;
	typedef typename BaseClass::WritePoll WritePoll;

    //  Construction
    public:
	VBSConsumerDevice_(ImplementationData &rDeviceData) : BaseClass (rDeviceData), VDevice::BSWriteFace (this) {
	}

    //  Destruction
    private:
	~VBSConsumerDevice_() {
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
	    VkStatus &rStatus, VDeviceBSWriter *pUser, VDeviceBSWriteArea const &rArea
	) OVERRIDE {
	    VReference<Put> pUse (new Put (this));
	    return pUse->start (rStatus, pUser, rArea);
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

    //  User Accounting
    private:
	virtual void onFirstUser_(VDevice::BSWriteFace *pFace) OVERRIDE {
	    static_cast<Implementation*>(this)->onFirstWriter ();
	}
	virtual void onFinalUser_(VDevice::BSWriteFace *pFace) OVERRIDE {
	    static_cast<Implementation*>(this)->onFinalWriter ();
	}

    //  User Creation
    private:
	virtual bool supplyConnection_(VReference<VConnection>&rpUser) OVERRIDE {
	    return static_cast<VDevice::BSWriteFace*>(this)->supply (rpUser);
	}
	virtual bool supplyBSConsumer_(VReference<VBSConsumer>&rpUser) OVERRIDE {
	    return static_cast<VDevice::BSWriteFace*>(this)->supply (rpUser);
	}
    };
}


#endif
