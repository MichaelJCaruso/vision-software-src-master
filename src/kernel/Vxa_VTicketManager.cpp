/*****  Vxa_VTicketManager Implementation  *****/

/************************
 ************************
 *****  Interfaces  *****
 ************************
 ************************/

/********************
 *****  System  *****
 ********************/

#include "Vk.h"

#include <iostream>

/******************
 *****  Self  *****
 ******************/

#include "Vxa_VTicketProducer.h"
#include "Vxa_VTicketConsumer.h"

#include "VkMapOf.h"

/************************
 *****  Supporting  *****
 ************************/

#include "VkMapOf.h"
#include "VkUUID.h"

#include "V_VSpinlock.h"

#include "Vxa_VCollectableCollection.h"
#include "Vxa_VCollectableObject.h"


/**********************************
 **********************************
 *****                        *****
 *****  Vxa::VTicketRegistry  *****
 *****                        *****
 **********************************
 **********************************/

namespace {
    using namespace Vxa;

/**********************************
 *---- class VTicketRegistry  ----*
 **********************************/
    class VTicketRegistry {
    //  Aliases
    public:
        typedef V::VSpinlock Mutex;
        typedef VTicketRegistry Registry;

    //  class Entry
    public:
        class Entry : public VReferenceable {
            DECLARE_CONCRETE_RTTLITE (Entry, VReferenceable);

        //  Construction
        public:
            Entry (Registry *pRegistry, VString const &rKey, cluster_t *pCluster);

        //  Destruction
        private:
            ~Entry () {
            }
            bool onDeleteThis ();

        //  Access
        public:
            bool getTicket (VString &rTicket, cluster_index_t xObject, bool bSingleUse);
            bool getObject (
                VCollectableObject::Reference &rpObject, cluster_index_t xObject, bool bSingleUse
            );

        //  State
        private:
            Registry*            const m_pRegistry;
            VString              const m_iKey;
            cluster_t::Reference const m_pCluster;
        };
        typedef VkMapOf<VString,VString const&,VString const&,Entry::Pointer> Map;
        friend class Entry;

    //  Construction
    public:
        VTicketRegistry () {
        }

    //  Destruction
    public:
        ~VTicketRegistry () {
        }

    //  Producer Support
    public:
        bool getTicket (
            VString &rTicket, cluster_t *pCluster, cluster_index_t xObject, bool bSingleUse
        );

    //  Consumer Support
    public:
        bool getObject (VCollectableObject::Reference &rpObject, VString const &rTicket);

    //  Map Management
    private:
        bool locateEntry (Entry::Reference &rpEntry, VString const &rkEntry, cluster_t *pCluster);
        bool locateEntry (Entry::Reference &rpEntry, VString const &rkEntry) const;
        bool deleteEntry (VString const &rkEntry);

    //  State
    private:
        Mutex mutable m_iMutex;
        Map           m_iMap;
    };
/*----------------*/
/*----------------*/
    VTicketRegistry::Entry::Entry (
        Registry *pRegistry, VString const &rKey, cluster_t *pCluster
    ) : m_pRegistry (pRegistry), m_iKey (rKey), m_pCluster (pCluster) {
    }
/*----------------*/
    bool VTicketRegistry::Entry::onDeleteThis () {
    /*
        std::cerr
            << this
            << ": onDelete : "
            << referenceCount ()
            << ", "
            << m_iKey
            << ", "
            << m_pCluster.referent ()
            << std::endl;
    */
        m_pRegistry->deleteEntry (m_iKey);
        return true;
    }
/*----------------*/
    bool VTicketRegistry::Entry::getTicket (
        VString &rTicket, cluster_index_t xObject, bool bSingleUse
    ) {
        retain ();
        rTicket << "<!*:" << m_iKey << ":" << xObject << (bSingleUse ? ":T" : ":H") << ":*!>";
    /*
        std::cerr
            << this
            << ": getTicket: "
            << referenceCount ()
            << ", "
            << m_iKey
            << ", "
            << m_pCluster.referent ()
            << ", "
            << xObject
            << (bSingleUse ? ", T: " : ", H: ")
            << m_pCluster->object (xObject)
            << ": "
            << rTicket
            << std::endl;
    */
        return true;
    }
/*----------------*/
    bool VTicketRegistry::Entry::getObject (
        VCollectableObject::Reference &rpObject, cluster_index_t xObject, bool bSingleUse
    ) {
        rpObject.setTo (m_pCluster->object (xObject));
    /*
        std::cerr
            << this
            << ": getObject: "
            << referenceCount () - 1
            << ", "
            << m_iKey
            << ", "
            << m_pCluster.referent ()
            << ", "
            << xObject
            << (bSingleUse ? ", T: " : ", H: ")
            << rpObject.referent ()
            << std::endl;
    */
        if (bSingleUse)
            release ();
        return rpObject.isntNil ();
    }
/*----------------*/
/*----------------*/
    bool VTicketRegistry::getTicket (
        VString &rTicket, cluster_t *pCluster, cluster_index_t xObject, bool bSingleUse
    ) {
        VString kEntry; Entry::Reference pEntry;
        return pCluster->getTicketKey (kEntry)
            && locateEntry (pEntry, kEntry, pCluster)
            && pEntry->getTicket (rTicket, xObject, bSingleUse);
    }
/*----------------*/
    bool VTicketRegistry::getObject (VCollectableObject::Reference &rpObject, VString const &rTicket) {
    //  Parse the ticket...
        static unsigned int const Expecting_Opening = 0;
        static unsigned int const Expecting_Key     = 1;
        static unsigned int const Expecting_Index   = 2;
        static unsigned int const Expecting_Kind    = 3;
        static unsigned int const Expecting_Closing = 4;
        VString iKey; unsigned int xObject = 0; bool bSingleUse = false;
        VString iNext (rTicket), iHead, iRest;
        for (unsigned int xWhatsNext = Expecting_Opening ; xWhatsNext < Expecting_Closing; xWhatsNext++) {
            if (!iNext.getPrefix (':', iHead, iRest))
                return false;
            switch (xWhatsNext) {
            case Expecting_Opening:
                if (iHead != "<!*")
                    return false;
                break;
            case Expecting_Key:
                iKey.setTo (iHead);
                break;
            case Expecting_Index:
                if (1 != sscanf (iHead.content (), "%u", &xObject))
                    return false;
                break;
            case Expecting_Kind:
                if (iHead == "T")
                    bSingleUse = true;
                else if (iHead == "H")
                    bSingleUse = false;
                else
                    return false;
                break;
            }
            iNext.setTo (iRest);
        }
        if (iRest != "*!>")
            return false;

    //  ... and get the object:
        Entry::Reference pEntry;
        return locateEntry (pEntry, iKey) && pEntry->getObject (rpObject, xObject, bSingleUse);
    }
/*----------------*/
    bool VTicketRegistry::locateEntry (Entry::Reference &rpEntry, VString const &rkEntry, cluster_t *pCluster) {
        Mutex::Claim iMutexClaim (m_iMutex);

        unsigned int xEntry = 0;
        if (m_iMap.Insert (rkEntry, xEntry)) {
            m_iMap[xEntry].setTo (new Entry (this, rkEntry, pCluster));
        }
        return (rpEntry = m_iMap[xEntry]).isntNil ();
    }
    bool VTicketRegistry::locateEntry (Entry::Reference &rpEntry, VString const &rkEntry) const {
        Mutex::Claim iMutexClaim (m_iMutex);

        unsigned int xEntry;
        return m_iMap.Locate (rkEntry, xEntry) && (rpEntry = m_iMap[xEntry]).isntNil ();
    }
/*----------------*/
    bool VTicketRegistry::deleteEntry (VString const &rkEntry) {
        Mutex::Claim iMutexClaim (m_iMutex);
        return m_iMap.Delete(rkEntry);
    }
/*----------------*/
/*----------------*/
    VTicketRegistry &Registry () {
        static VTicketRegistry g_iTicketRegistry;
        return g_iTicketRegistry;
    }
/*----------------*/
/*----------------*/
}


/**********************************
 **********************************
 *****                        *****
 *****  Vxa::VTicketProducer  *****
 *****                        *****
 **********************************
 **********************************/

/*******************************
 *******************************
 *****  Ticket Generation  *****
 *******************************
 *******************************/

bool Vxa::VTicketProducer::getTicketKey (VString &rKey) const {
    if (m_iKey.isntEmpty () || VkUUID::GetString (m_iKey, VkUUID::ReturnUUID ())) {
        rKey.setTo (m_iKey);
        return true;
    }
    return false;
}

bool Vxa::VTicketProducer::getTicket (
    VString &rTicket, cluster_t *pCluster, cluster_index_t xObject, bool bSingleUse
) const {
    return Registry ().getTicket (rTicket, pCluster, xObject, bSingleUse);
}


/**********************************
 **********************************
 *****                        *****
 *****  Vxa::VTicketConsumer  *****
 *****                        *****
 **********************************
 **********************************/

/***************************
 ***************************
 *****  Instantiation  *****
 ***************************
 ***************************/

namespace {
    using namespace Vxa;

    class Consumer : public VTicketConsumer {
        DECLARE_CONCRETE_RTTLITE (Consumer, VTicketConsumer);

    //  Construction
    public:
        Consumer () {
        }

    //  Destruction
    private:
        ~Consumer () {
        }

    //  Use
    public:
        virtual bool getObject (VCollectableObject* &rpObject, VString const &rTicket) OVERRIDE {
            return Registry().getObject (m_pObject, rTicket) && getCachedObject (rpObject);
        }
        virtual bool getObject (VCollectableObject* &rpObject) OVERRIDE {
            return getCachedObject (rpObject);
        }
    private:
        bool getCachedObject (VCollectableObject* &rpObject) {
            rpObject = m_pObject;
            return m_pObject.isntNil ();
        }

    //  State
    private:
        VCollectableObject::Reference m_pObject;
    };
}

bool Vxa::VTicketConsumer::Get (Reference &rpTicketConsumer) {
    if (rpTicketConsumer.isNil ())
        rpTicketConsumer.setTo (new Consumer ());
    return rpTicketConsumer.isntNil ();
}
