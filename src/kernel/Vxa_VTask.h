#ifndef Vxa_VTask_Interface
#define Vxa_VTask_Interface

/************************
 *****  Components  *****
 ************************/

#include "Vca_VActivity.h"

#include "Vxa_VRolePlayer.h"
#include "Vxa_VCallData.h"
#include "Vxa_VTaskCursor.h"

/**************************
 *****  Declarations  *****
 **************************/

/*************************
 *****  Definitions  *****
 *************************/

namespace Vxa {
    class VCallType1;
    class VCallType2;

    class VCallType1Importer;
    class VCallType2Importer;
    class VError;

    class Vxa_API VTask : public VRolePlayer, public Vca::VActivity {
	DECLARE_ABSTRACT_RTTLITE (VTask, VRolePlayer);

	friend class VCallType1;
	friend class VCallType2;

    //  LaunchRequest
    public:
	class LaunchRequest;
	friend class LaunchRequest;

    //  Construction
    protected:
	VTask (VCallData const &rCallData);

    //  Destruction
    protected:
	~VTask ();

    //  Access
    public:
	cardinality_t cardinality () const {
	    return m_iCallData.cardinality ();
	}
	VTaskCursor *cursor () const {
	    return m_pCursor;
	}

    //  Query
    public:
	bool isAlive () const {
	    return m_pCursor->isAlive ();
	}
	bool moreToDo () const {
	    return m_pCursor->moreToDo ();
	}

    //  Execution
    protected:
	virtual bool startUsing (VCallType1Importer &rImporter) = 0;
	virtual bool startUsing (VCallType2Importer &rImporter) = 0;

    private:
	bool startWithMonitorUsing (VCallType1Importer &rImporter);
	bool startWithMonitorUsing (VCallType2Importer &rImporter);

    public:
	void onErrorEvent (VError *pError);

    public:
	void suspend () {
	    m_cSuspensions++;
	}
	bool resume () {
	    return --m_cSuspensions > 0 || launch ();
	}
    private:
	bool launch ();
    public:
	void kill () {
	    m_pCursor->kill ();
	}
    private:
	virtual bool run () = 0;
	bool runWithMonitor ();

    //  Iteration
    public:
	void first () {
	    m_pCursor->first ();
	}
	void next () {
	    m_pCursor->next ();
	}

    //  State
    private:
	VCallData const m_iCallData;
	VTaskCursor::Reference m_pCursor;
	unsigned int m_cSuspensions;
    };
}


#endif
