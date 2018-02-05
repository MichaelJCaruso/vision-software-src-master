#ifndef Vsa_VPoolApplication_Interface
#define Vsa_VPoolApplication_Interface

/************************
 *****  Components  *****
 ************************/

#include "Vca_VServerApplication.h"

#include "Vsa_IEvaluator.h"
#include "Vsa_IEvaluatorPool_Ex5.h"

#include "Vsa_IPoolApplication.h" 
#include "Vsa_IPoolApplication_Ex2.h"

#include "Vsa_IUpDownPublisher.h" 

/**************************
 *****  Declarations  *****
 **************************/

#include "Vsa_VEvaluatorPool.h"

namespace Vca {
   class VTimer;
}

/*************************
 *****  Definitions  *****2
 *************************/

namespace Vsa {
    class VEvaluatorPool;
    class IUpDownSubscriber;
    class ISubscription;

    class VPoolApplication : public Vca::VServerApplication {
	DECLARE_CONCRETE_RTTLITE (VPoolApplication, Vca::VServerApplication);
	DECLARE_ROLEPLAYER_CLASSINFO ();

    //  Construction
    public:
	VPoolApplication (Context *pContext);

    //  Destruction
    private:
	~VPoolApplication ();

    //  Base Class Roles
    public:
	using BaseClass::getRole;

    //  IPoolApplication Role 
    /********************************************************************
     ****  Supported solely for the purpose of maintaining          *****
     ****  compatibility with the older pools	                    ***** 
     ********************************************************************/	
    private:
	Vca::VRole<ThisClass,IPoolApplication> m_pIPoolApplication;
    public:
	void getRole (VReference<IPoolApplication>&rpRole);

	void Stop   (IPoolApplication *pRole);
	void GetPoolWorkerProcessSource (
	    IPoolApplication *pRole, IVReceiver<VString const &> *pClient
	);
	void SetPoolWorkerProcessSource (
	     IPoolApplication *pRole, VString const &iValue
	);

    //  IApplication  Role
    public:
	void GetStatistics (
	    Vca::IApplication *pRole, IVReceiver<VString const &> *pClient
	);

    //  IPoolApplication_Ex1 Role
    public:
	void Stop   (IPoolApplication_Ex1 *pRole);
	void GetPoolWorkerProcessSource (
	    IPoolApplication_Ex1 *pRole, IVReceiver<VString const &> *pClient
	);
	void SetPoolWorkerProcessSource (
	     IPoolApplication_Ex1 *pRole, VString const &iValue
	);

    //  IPoolApplication_Ex2 Role
    public:
	void getRole (VReference<IPoolApplication_Ex2>&rpRole);
	void HardStop   (IPoolApplication_Ex2 *pRole);
    private:
	Vca::VRole<ThisClass,IPoolApplication_Ex2> m_pIPoolApplication_Ex;


    //  IEvaluator Role
    private:
	Vca::VRole<ThisClass,IEvaluator_Ex1> m_pIEvaluator;
    public:
	void getRole (VReference<IEvaluator_Ex1>&rpRole);

	void EvaluateExpression (
	    IEvaluator*		pRole,
	    IEvaluatorClient*	pClient,
	    VString const&	rPath,
	    VString const&	rExpression
	);

	void EvaluateURL (
	    IEvaluator*		pRole,
	    IEvaluatorClient*	pClient,
	    VString const&	rPath,
	    VString const&	rQuery,
	    VString const&	rEnvironment
	);

    //  IEvaluator_Ex1 Role
	void EvaluateExpression_Ex (
	    IEvaluator*		pRole,
	    IEvaluatorClient*	pClient,
	    VString const&	rPath,
	    VString const&	rExpression,
	    VString const&	rID,
	    VString const&	rCID
	);

	void EvaluateURL_Ex (
	    IEvaluator*		pRole,
	    IEvaluatorClient*	pClient,
	    VString const&	rPath,
	    VString const&	rQuery,
	    VString const&	rEnvironment,
	    VString const&	rID,
	    VString const&	rCID
	);

      //  IUpDownPublisher Role
    private:
	Vca::VRole<ThisClass,IUpDownPublisher> m_pIUpDownPublisher;
    public:
	void getRole (VReference<IUpDownPublisher>&rpRole);

        void Subscribe (IUpDownPublisher *pRole, IUpDownSubscriber *pSub, IVReceiver<ISubscription*>*pRec);

    //  IEvaluatorPool Role
    public:
	void GetSettings (IEvaluatorPool *pRole, IVReceiver<IEvaluatorPoolSettings*>*pClient);
	void AddEvaluator (IEvaluatorPool *pRole, IEvaluator *pEvaluator);
	void FlushWorkers (IEvaluatorPool *pRole, IVReceiver<bool>*pClient);

    //  IEvaluatorPool_Ex1 Role
    public:
	void HardRestart (IEvaluatorPool_Ex1*, IVReceiver<bool>*);
	void IsWorkerCreationFailureLmtReached (IEvaluatorPool_Ex1*, IVReceiver<bool>*);
	void GetCurrentGenerationTotalWorkers (IEvaluatorPool_Ex1*, IVReceiver<Vca::U32>*);
	void GetWorkerInterface  (IEvaluatorPool_Ex1*, IVReceiver<IVUnknown*>*, Vca::U32);
	void GetTotalWorkers (IEvaluatorPool_Ex1*, IVReceiver<Vca::U32>*);
	void HardStop (IEvaluatorPool_Ex1*, IVReceiver<bool>*);

    // IEvaluatorPool_Ex2 Role
    public:
	void BroadcastExpression (IEvaluatorPool_Ex2*, IEvaluatorClient*, VString const &, VString const &);
	void BroadcastURL        (IEvaluatorPool_Ex2*, IEvaluatorClient*, VString const &, VString const &, VString const &);

    // IEvaluatorPool_Ex3 Role
    public:
        void TakeWorkerOffline (IEvaluatorPool_Ex3*, VString const &, bool, IVReceiver<VString const &>*);
        void MakeWorkerOnline (IEvaluatorPool_Ex3*, VString const &, bool, IVReceiver<VString const &>*);
        void EvaluateUsingOfflineWorker (IEvaluatorPool_Ex3*, VString const &rQuery, VString const &, bool, IEvaluatorClient*);
        void RetireOfflineWorker (IEvaluatorPool_Ex3*, VString const &, bool, IVReceiver<VString const &>*);

        void DumpWorker (IEvaluatorPool_Ex3*, VString const &, bool, VString const &, IVReceiver<VString const &>*);
        void DumpPoolQueue (IEvaluatorPool_Ex3*, VString const &, Vca::U32, IVReceiver<VString const &>*);
        void DumpWorkingWorkers (IEvaluatorPool_Ex3*, VString const &, IVReceiver<VString const &>*);
	void GetWorker_Ex  (IEvaluatorPool_Ex3*, VString const &, bool, IVReceiver<IVUnknown*>*);
        void GetWorkersStatistics (IEvaluatorPool_Ex3*, IVReceiver<VString const &>*);
        void GetGenerationStatistics (IEvaluatorPool_Ex3*, Vca::U32, IVReceiver<VString const &>*);

    // IEvaluatorPool_Ex4 Role
    public:
        void QueryDetails (Vsa::IEvaluatorPool_Ex4 *pRole, Vca::U32 iID, IVReceiver<const VString&>* pClient);
        void CancelQuery (Vsa::IEvaluatorPool_Ex4 *pRole, Vca::U32 iID, IVReceiver<bool>* pClient);

	// IEvaluatorPool_Ex5 Role
	private:
		Vca::VRole<ThisClass,IEvaluatorPool_Ex5> m_pIEvaluatorPool;
	public:
        void getRole (VReference<IEvaluatorPool_Ex5>&rpRole);

        void StatSum (IEvaluatorPool_Ex5 *pRole, VString, Vca::U64, Vca::U64, IVReceiver<VString const &>*);
        void StatDivide (IEvaluatorPool_Ex5 *pRole, VString, VString, Vca::U64, Vca::U64, IVReceiver<Vca::F64>*);
        void StatMax (IEvaluatorPool_Ex5 *pRole, VString, Vca::U64, Vca::U64, IVReceiver<Vca::U32>*, IVReceiver<V::VTime const &>*);
        void StatMin (IEvaluatorPool_Ex5 *pRole, VString, Vca::U64, Vca::U64, IVReceiver<Vca::U32>*, IVReceiver<V::VTime const &>*);
        void StatMaxString (IEvaluatorPool_Ex5 *pRole, VString, Vca::U64, Vca::U64, IVReceiver<VString const &>*);
        void StatMinString (IEvaluatorPool_Ex5 *pRole, VString, Vca::U64, Vca::U64, IVReceiver<VString const &>*);
        void AllStatValues (IEvaluatorPool_Ex5 *pRole, VString, IVReceiver<VkDynamicArrayOf<Vca::U64> const &>*, 
                            IVReceiver<VkDynamicArrayOf<V::VTime> const &>*, IVReceiver<VkDynamicArrayOf<Vca::U64> const &>*, 
                            IVReceiver<VkDynamicArrayOf<Vca::U64>  const &>*, IVReceiver<VkDynamicArrayOf<Vca::F64> const &>*);


    //  IEvaluatorControl Methods:
    public:
	void GetEvaluator (IEvaluatorControl* pRole, IEvaluatorSink*);
	void Suspend (IEvaluatorControl*, IVReceiver<bool>*);
	void Resume  (IEvaluatorControl*, IVReceiver<bool>*);
	void Restart (IEvaluatorControl*, IVReceiver<bool>*);
	void GetStatistics (IEvaluatorControl*, IVReceiver<VString const &>*);
	void GetWorker     (IEvaluatorControl*, IVReceiver<IVUnknown*>*, Vca::U32);
	void Stop    (IEvaluatorControl*, IVReceiver<bool>*);

    //  IGetter Methods:
    protected:
	virtual bool GetU32Value_(IVReceiver<Vca::U32>*, VString const &) OVERRIDE;
	virtual bool GetStringValue_(IVReceiver<VString const &>*, VString const &) OVERRIDE;

    //  Control
    private:
	virtual bool start_() OVERRIDE;
	virtual bool stop_(bool bHardStop) OVERRIDE;
	virtual bool pause_() OVERRIDE;
	virtual bool resume_() OVERRIDE;

    //  Call Backs 
    private:
	void onPoolStopped  (Vca::VTrigger<ThisClass> *pTrigger);

    //  Update
    public:
	void setPoolWorkerProcessSourceTo (VString const &rValue);

    //  Query
    public:
	VEvaluatorPool::trackable_count_t const &queueLengthTrackable () {
	    return m_pEvaluatorPool->queueLengthTrackable ();
	}
	
	VEvaluatorPool::trackable_count_t const &busynessTrackable () {
		return m_pEvaluatorPool->busynessTrackable ();
	}

	Vca::U32 stopTimeout () const {
	    return m_pEvaluatorPool.isNil () ? 0 : m_pEvaluatorPool->getStopTimeOut ();
	}
    private:
	virtual Vca::U32 stopTimeout_() const OVERRIDE {
	    return stopTimeout ();
	}
   
    //  State
    protected:
	VEvaluatorPool::Reference  m_pEvaluatorPool;
	bool			   m_bHardStopPending;	// to maintain compatibility with old vers
	bool const		   m_bPlumbingWorkers;
    };
}


#endif
