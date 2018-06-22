#ifndef Vxa_VCollectableCollection_Interface
#define Vxa_VCollectableCollection_Interface

/************************
 *****  Components  *****
 ************************/

#include "Vxa_VCollection.h"
#include "Vxa_VTicketProducer.h"

/**************************
 *****  Declarations  *****
 **************************/

#include "Vxa_VClass.h"

/*************************
 *****  Definitions  *****
 *************************/

namespace Vxa {
    class VCollectableIdentity;
    class VCollectableObject;

    typedef VTicketProducer::cluster_t       cluster_t;
    typedef VTicketProducer::cluster_index_t cluster_index_t;

/*----------------------------------------*
 *----  class VCollectableCollection  ----*
 *----------------------------------------*/
    class Vxa_API VCollectableCollection : public VCollection {
	DECLARE_ABSTRACT_RTTLITE (VCollectableCollection, VCollection);

        friend class VCollectableIdentity;

    //  Construction
    protected:
	VCollectableCollection (VClass *pClass);

    //  Destruction
    protected:
	~VCollectableCollection ();

    //  Access
    private:
	virtual cardinality_t cardinality_() const OVERRIDE {
	    return cardinality ();
	}
        virtual VClass *type_ () const OVERRIDE {
            return type ();
        }
    public:
	cardinality_t cardinality () const {
	    return m_sCollection;
	}
	VClass *type () const {
	    return m_pClass;
	}

    //  Accounting
    protected:
	void onCollectionCreation (cardinality_t sCollection) const {
	    m_pClass->onCollectionCreation (sCollection);
	}
	void onCollectionDeletion (cardinality_t sCollection) const {
	    m_pClass->onCollectionDeletion (sCollection);
	}

    //  Method Invocation
    protected: //  Override of VCollection::invokeCall ...
	virtual bool invokeCall (VCallHandle const &rCallHandle) OVERRIDE {
	    return m_pClass->invokeCall (rCallHandle);
	}

    //  Transmission
    protected:
	virtual bool transmitUsing_(VCallType2Exporter *pExporter, VMonotypeMapMaker *pMapMaker, object_reference_array_t const &rInjection) OVERRIDE;
	virtual bool transmitUsing_(VCallType2Exporter *pExporter, VMonotypeMapMaker *pMapMaker) OVERRIDE;

    //  Ticket Generation
    public:
        bool getTicketKey (VString &rKey) const {
            return m_iTicketProducer.getTicketKey (rKey);
        }
        bool getTicket (VString &rTicket, collection_index_t xObject, bool bSingleUse) {
            return m_iTicketProducer.getTicket (rTicket, this, xObject, bSingleUse);
        }

    //  Update
    protected:
	bool attach (VCollectableObject *pObject);
	bool detach (VCollectableObject *pObject);

    //  State
    private:
	VClass::Pointer const m_pClass;
        VTicketProducer const m_iTicketProducer;
	cardinality_t         m_sCollection;
    };


/*--------------------------------------*
 *----  class VCollectableIdentity  ----*
 *--------------------------------------*/
    class Vxa_API VCollectableIdentity {
    //  Friends
        friend class VCollectableObject;

    //  Construction / Destruction
    public:
        VCollectableIdentity () : m_xObject (0) {
        }
        ~VCollectableIdentity () {
        }

    //  Access
    public:
        cluster_t *cluster () const {
            return m_pCluster;
        }
        cluster_index_t clusterIndex () const {
            return m_xObject;
        }

    //  Query
    public:
        bool isSet () const {
            return m_pCluster.isntNil ();
        }
        bool isntSet () const {
            return m_pCluster.isNil ();
        }

    //  Ticket Generation
    public:
        bool getTicket (VString &rTicket, bool bSingleUse) const {
            return m_pCluster && m_pCluster->getTicket (rTicket, m_xObject, bSingleUse);
        }

    //  Update
    private:
        bool attach (cluster_t *pCluster, cluster_index_t xObject);
        bool detach (cluster_t *pCluster);

    //  State
    private:
        cluster_t::Pointer m_pCluster;
        cluster_index_t    m_xObject;
    };


/*----------------------------------------------------------------------*
 *---- template <typename collectable_t> struct VCollectableTraits  ----*
 *----------------------------------------------------------------------*/

    template <typename collectable_t> struct VCollectableTraits {
	typedef collectable_t* val_t;
	typedef typename collectable_t::Reference var_t;
	typedef typename collectable_t::template ClusterOf<typename collectable_t::Reference>::type cluster_t;

        static void CreateIdentity (
            VCollectableCollection::Reference &rpCluster,
            collectable_t *pObject, VClass *pClass, VCardinalityHints *pTailHints = 0
        ) {
	    if (pObject->hasAnIdentity ()) {
            } else if (rpCluster) {
                static_cast<cluster_t*>(rpCluster.referent ())->append (pObject);
            } else {
                rpCluster.setTo (new cluster_t (pClass, pTailHints, pObject));
            }
        }
    };

    template <typename collectable_t>
    struct VCollectableTraits<VReference<collectable_t> > : public VCollectableTraits<collectable_t> {
    };
}


#endif
