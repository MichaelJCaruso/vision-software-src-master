#ifndef Vxa_VCollectableCollection_Interface
#define Vxa_VCollectableCollection_Interface

/************************
 *****  Components  *****
 ************************/

#include "Vxa_VCollection.h"

/**************************
 *****  Declarations  *****
 **************************/

#include "Vxa_VClass.h"

/*************************
 *****  Definitions  *****
 *************************/

namespace Vxa {
    class VCollectableObject;

    /*----------------------------------------------------------------------*
     *---- template <typename collectable_t> struct VCollectableTraits  ----*
     *----------------------------------------------------------------------*/

    template <typename collectable_t> struct VCollectableTraits {
	typedef collectable_t* val_t;
	typedef typename collectable_t::Reference var_t;
	typedef typename collectable_t::template ClusterOf<typename collectable_t::Reference>::type cluster_t;
    };
    template <typename collectable_t> struct VCollectableTraits<VReference<collectable_t> > : public VCollectableTraits<collectable_t> {
    };

    /*----------------------------------------*
     *----  class VCollectableCollection  ----*
     *----------------------------------------*/

    class Vxa_API VCollectableCollection : public VCollection {
	DECLARE_ABSTRACT_RTTLITE (VCollectableCollection, VCollection);

    //  Aliases
    public:
	typedef cardinality_t collection_index_t;

    //  Construction
    protected:
	VCollectableCollection (VClass *pClass);

    //  Destruction
    protected:
	~VCollectableCollection ();

    //  Access
    private:
	virtual cardinality_t cardinality_() const {
	    return cardinality ();
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
    protected: //  Override of VCollection::invokeMethod_ ...
	virtual bool invokeMethod_(VString const &rMethodName, VCallHandle const &rCallHandle) {
	    return m_pClass->invokeMethod (rMethodName, rCallHandle, this);
	}

    //  Transmission
    protected:
	bool transmitUsing_(VCallType2Exporter *pExporter, VMonotypeMapMaker *pMapMaker, object_reference_array_t const &rInjection);
	bool transmitUsing_(VCallType2Exporter *pExporter, VMonotypeMapMaker *pMapMaker);

    //  Update
    protected:
	bool attach (VCollectableObject *pObject);
	bool detach (VCollectableObject *pObject);

    //  State
    private:
	VClass::Pointer const m_pClass;
	cardinality_t m_sCollection;
    };


    /*--------------------------------------*
     *----  class VCollectableIdentity  ----*
     *--------------------------------------*/

    class Vxa_API VCollectableIdentity {
    //  Friends
        friend class VCollectableObject;

    //  Aliases
    public:
        typedef VCollectableCollection cluster_t;
        typedef VCollectableCollection::collection_index_t cluster_index_t;

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
        bool getIdentity (VCollectableIdentity &rResult) const {
            if (isntSet ())
                return false;
            rResult.m_xObject  = m_xObject;
            rResult.m_pCluster = m_pCluster;
            return true;
        }

    //  Query
    public:
        bool isSet () const {
            return m_pCluster.isntNil ();
        }
        bool isntSet () const {
            return m_pCluster.isNil ();
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

    typedef VCollectableIdentity::cluster_t       cluster_t;
    typedef VCollectableIdentity::cluster_index_t cluster_index_t;
}


#endif
