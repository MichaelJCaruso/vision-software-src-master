#ifndef V_VAtomicOperations_Interface
#define V_VAtomicOperations_Interface

/************************
 *****  Components  *****
 ************************/

/**************************
 *****  Declarations  *****
 **************************/

#include "V_VAtomicMemoryOperations_.h"

/***********************
 *****  Templates  *****
 ***********************/

namespace V {
    /*---------------------------*
     *----  No Interlocking  ----*
     *---------------------------*/

    template <typename T> class VAtomicNilOperations_ : public VAtomicMemoryNilOperations_<sizeof (T)> {
    //  Family Values
    public:
	typedef T value_t;
	typedef value_t storage_t;

    //  Constants
    public:
	static value_t zero () {
	    return 0;
	}

    //  Operations
    public:
	static void increment (value_t volatile &rValue) {
	    rValue++;
	}
	static void decrement (value_t volatile &rValue) {
	    rValue--;
	}
	static bool decrementIsZero (value_t volatile &rValue) {
	    return 0 == --rValue;
	}
	static bool isZero (value_t iValue) {
	    return 0 == iValue;
	}

	static value_t fetchAndIncrement (value_t volatile &rValue) {
	    return rValue++;
	}
	static value_t fetchAndDecrement (value_t volatile &rValue) {
	    return rValue++;
	}

	static value_t incrementAndFetch (value_t volatile &rValue) {
	    return ++rValue;
	}
	static value_t decrementAndFetch (value_t volatile &rValue) {
	    return --rValue;
	}
    };


#if defined(V_VAtomicMemoryOperations_Disabled)
    /*---------------------------*
     *----  No Interlocking  ----*
     *---------------------------*/

    template <typename T> class VAtomicOperations_ : public VAtomicNilOperations_<T> {
    };


#elif defined(__VMS)
    /*-------------------------*
     *----  OpenVMS/Alpha  ----*
     *-------------------------*/

    template <typename T> class VAtomicOperations_ : public VAtomicMemoryOperations_<sizeof (T)> {
    //  Family Values
    public:
	typedef T value_t;
	typedef value_t storage_t;
    };

    template<>
    class VAtomicOperations_<unsigned __int32> : public VAtomicMemoryOperations_<sizeof (unsigned __int32)> {
    //  Family Values
    public:
	typedef unsigned __int32 value_t;
	typedef value_t storage_t;

    //  Constants
    public:
	static value_t zero () {
	    return 0;
	}

    //  Operations
    public:
	static void increment (value_t volatile &rValue) {
	    fetchAndIncrement (rValue);
	}
	static void decrement (value_t volatile &rValue) {
	    fetchAndDecrement (rValue);
	}
	static bool decrementIsZero (value_t volatile &rValue) {
	    return isZero (decrementAndFetch (rValue));
	}
	static bool isZero (value_t iValue) {
	    return 0 == iValue;		//  ... memory barrier not needed until we act on this info.
	}

	static value_t fetchAndIncrement (value_t volatile &rValue) {
	    return __ATOMIC_INCREMENT_LONG ((void volatile*)&rValue);
	}
	static value_t fetchAndDecrement (value_t volatile &rValue) {
	    MemoryBarrierStoreStore ();	//  ... see comments in "boost_detail_atomic_count_...h"
	    return __ATOMIC_DECREMENT_LONG ((void volatile*)&rValue);
	}

	static value_t incrementAndFetch (value_t volatile &rValue) {
	    return fetchAndIncrement (rValue) + 1;
	}
	static value_t decrementAndFetch (value_t volatile &rValue) {
	    return fetchAndDecrement (rValue) - 1;
	}
    };

    template<>
    class VAtomicOperations_<unsigned __int64> : public VAtomicMemoryOperations_<sizeof (unsigned __int64)> {
    //  Family Values
    public:
	typedef unsigned __int64 value_t;
	typedef value_t storage_t;

    //  Constants
    public:
	static value_t zero () {
	    return 0;
	}

    //  Operations
    public:
	static void increment (value_t volatile &rValue) {
	    fetchAndIncrement (rValue);
	}
	static void decrement (value_t volatile &rValue) {
	    fetchAndDecrement (rValue);
	}
	static bool decrementIsZero (value_t volatile &rValue) {
	    return isZero (decrementAndFetch (rValue));
	}
	static bool isZero (value_t iValue) {
	    return 0 == iValue;		//  ... memory barrier not needed until we act on this info.
	}

	static value_t fetchAndIncrement (value_t volatile &rValue) {
	    return __ATOMIC_INCREMENT_QUAD ((void volatile*)&rValue);
	}
	static value_t fetchAndDecrement (value_t volatile &rValue) {
	    MemoryBarrierStoreStore ();	//  ... see comments in "boost_detail_atomic_count_...h"
	    return __ATOMIC_DECREMENT_QUAD ((void volatile*)&rValue);
	}
	static value_t incrementAndFetch (value_t volatile &rValue) {
	    return fetchAndIncrement (rValue) + 1;
	}
	static value_t decrementAndFetch (value_t volatile &rValue) {
	    return fetchAndDecrement (rValue) - 1;
	}
    };


#elif defined(_WIN32)
#pragma warning (push)
#pragma warning (disable:4035)

    /*-----------------------*
     *----  Windows/x86  ----*
     *-----------------------*/

    template <typename T> class VAtomicOperations_ : public VAtomicMemoryOperations_<sizeof (T)> {
    //  Family Values
    public:
	typedef T value_t;
	typedef value_t storage_t;
    };

    template<>
    class VAtomicOperations_<unsigned __int32> : public VAtomicMemoryOperations_<sizeof (unsigned __int32)> {
    //  Family Values
    public:
	typedef unsigned __int32 value_t;
	typedef value_t storage_t;

    //  Constants
    public:
	static value_t zero () {
	    return 0;
	}

    //  Operations
    public:
	static void increment (value_t volatile &rValue) {
	    InterlockedIncrement ((LONG volatile*)&rValue);
	}
	static void decrement (value_t volatile &rValue) {
	    InterlockedDecrement ((LONG volatile*)&rValue);
	}
	static bool decrementIsZero (value_t volatile &rValue) {
	    return isZero (decrementAndFetch (rValue));
	}
	static bool isZero (value_t iValue) {
	    return 0 == iValue;
	}

	static value_t fetchAndIncrement (value_t volatile &rValue) {
	    return InterlockedIncrement ((LONG volatile*)&rValue) - 1;
	}

	static value_t fetchAndDecrement (value_t volatile &rValue) {
	    return InterlockedDecrement ((LONG volatile*)&rValue) + 1;
	}

	static value_t incrementAndFetch (value_t volatile &rValue) {
	    return InterlockedIncrement ((LONG volatile*)&rValue);
	}
	static value_t decrementAndFetch (value_t volatile &rValue) {
	    return InterlockedDecrement ((LONG volatile*)&rValue);
	}
    };

    template<>
    class VAtomicOperations_<unsigned __int64> : public VAtomicMemoryOperations_<sizeof (unsigned __int64)> {
    //  Family Values
    public:
	typedef unsigned __int64 value_t;
	typedef value_t storage_t;

    //  Constants
    public:
	static value_t zero () {
	    return 0;
	}

    //  Operations
    public:
	static void increment (value_t volatile &rValue) {
	    InterlockedIncrement64 ((LONGLONG volatile*)&rValue);
	}
	static void decrement (value_t volatile &rValue) {
	    InterlockedDecrement64 ((LONGLONG volatile*)&rValue);
	}
	static bool decrementIsZero (value_t volatile &rValue) {
	    return isZero (decrementAndFetch (rValue));
	}
	static bool isZero (value_t iValue) {
	    return 0 == iValue;
	}

	static value_t fetchAndIncrement (value_t volatile &rValue) {
	    return InterlockedIncrement64 ((LONGLONG volatile*)&rValue) - 1;
	}
	static value_t fetchAndDecrement (value_t volatile &rValue) {
	    return InterlockedDecrement64 ((LONGLONG volatile*)&rValue) + 1;
	}
	static value_t incrementAndFetch (value_t volatile &rValue) {
	    return InterlockedIncrement64 ((LONGLONG volatile*)&rValue);
	}
	static value_t decrementAndFetch (value_t volatile &rValue) {
	    return InterlockedDecrement64 ((LONGLONG volatile*)&rValue);
	}
    };
#pragma warning (pop)


#elif defined(__linux__)
    /*---------------------*
     *----  Linux/x86  ----*
     *---------------------*/

    template <typename T> class VAtomicOperations_ : public VAtomicMemoryOperations_<sizeof (T)> {
    //  Family Values
    public:
	typedef T value_t;
	typedef value_t storage_t;
    };

    template<>
    class VAtomicOperations_<unsigned __int32> : public VAtomicMemoryOperations_<sizeof (unsigned __int32)> {
    //  Family Values
    public:
	typedef unsigned __int32 value_t;
	typedef value_t storage_t;

    //  Constants
    public:
	static value_t zero () {
	    return 0;
	}

    //  Operations
    private:
	static value_t interlockedAdd (value_t volatile &rValue, value_t iAddend) {
	    register value_t result;
	    __asm__ __volatile__ (
		"lock; xaddl %0,%2"
		: "=r" (result) : "0" (iAddend), "m" (rValue) : "memory"
	    );
	    return result;
	}
    public:
	static void increment (value_t volatile &rValue) {
	    __asm__ __volatile__(
		"lock; incl %0" : "=m" (rValue) : "m" (rValue) : "memory"
	    );
	}
	static void decrement (value_t volatile &rValue) {
	    __asm__ __volatile__(
		"lock; decl %0" : "=m" (rValue) : "m" (rValue) : "memory"
	    );
	}
	static bool decrementIsZero (value_t volatile &rValue) {
	    return isZero (decrementAndFetch (rValue));
	}
	static bool isZero (value_t iValue) {
	    return 0 == iValue;
	}

	static value_t fetchAndIncrement (value_t volatile &rValue) {
	    return interlockedAdd (rValue, 1);
	}
	static value_t fetchAndDecrement (value_t volatile &rValue) {
	    return interlockedAdd (rValue, (value_t)-1);
	}
	static value_t incrementAndFetch (value_t volatile &rValue) {
	    return fetchAndIncrement (rValue) + 1;
	}
	static value_t decrementAndFetch (value_t volatile &rValue) {
	    return fetchAndDecrement (rValue) - 1;
	}
    };

    template<>
    class VAtomicOperations_<unsigned __int64> : public VAtomicMemoryOperations_<sizeof (unsigned __int64)> {
    //  Family Values
    public:
	typedef unsigned __int64 value_t;
	typedef value_t storage_t;

    //  Constants
    public:
	static value_t zero () {
	    return 0;
	}

    //  Operations
    private:
	static value_t interlockedAdd (value_t volatile &rValue, value_t iAddend) {
	    register value_t result;
	    __asm__ __volatile__ (
		"lock; xaddq %0,%2"
		: "=r" (result) : "0" (iAddend), "m" (rValue) : "memory"
	    );
	    return result;
	}
    public:
	static void increment (value_t volatile &rValue) {
	    __asm__ __volatile__(
		"lock; incq %0" : "=m" (rValue) : "m" (rValue) : "memory"
	    );
	}
	static void decrement (value_t volatile &rValue) {
	    __asm__ __volatile__(
		"lock; decq %0" : "=m" (rValue) : "m" (rValue) : "memory"
	    );
	}
	static bool decrementIsZero (value_t volatile &rValue) {
	    return isZero (decrementAndFetch (rValue));
	}
	static bool isZero (value_t iValue) {
	    return 0 == iValue;
	}

	static value_t fetchAndIncrement (value_t volatile &rValue) {
	    return interlockedAdd (rValue, 1);
	}
	static value_t fetchAndDecrement (value_t volatile &rValue) {
	    return interlockedAdd (rValue, (value_t)-1);
	}
	static value_t incrementAndFetch (value_t volatile &rValue) {
	    return fetchAndIncrement (rValue) + 1;
	}
	static value_t decrementAndFetch (value_t volatile &rValue) {
	    return fetchAndDecrement (rValue) - 1;
	}
    };


#elif defined(sun)
    /*-------------------------*
     *----  Solaris/Sparc  ----*
     *-------------------------*/

    template <typename T> class VAtomicOperations_ : public VAtomicMemoryOperations_<sizeof (T)> {
    //  Family Values
    public:
	typedef T value_t;
	typedef value_t storage_t;
    };

    template<>
    class VAtomicOperations_<unsigned __int32> : public VAtomicMemoryOperations_<sizeof (unsigned __int32)> {
    //  Family Values
    public:
	typedef unsigned __int32 value_t;
	typedef value_t storage_t;

    //  Constants
    public:
	static value_t zero () {
	    return 0;
	}

    //  Operations
    private:
	static value_t fetchAndAdd (value_t volatile &rValue, value_t iAddend) {
	    return fetchAndAdd32 (rValue, iAddend);
	}
    public:
	static void increment (value_t volatile &rValue) {
	    fetchAndIncrement (rValue);
	}
	static void decrement (value_t volatile &rValue) {
	    fetchAndDecrement (rValue);
	}
	static bool decrementIsZero (value_t volatile &rValue) {
	    return isZero (decrementAndFetch (rValue));
	}
	static bool isZero (value_t iValue) {
	    return 0 == iValue;
	}

	static value_t fetchAndIncrement (value_t volatile &rValue) {
	    return fetchAndAdd (rValue, 1);
	}
	static value_t fetchAndDecrement (value_t volatile &rValue) {
	    MemoryBarrierStoreStore ();
	    return fetchAndAdd (rValue, (value_t)-1);
	}
	static value_t incrementAndFetch (value_t volatile &rValue) {
	    return fetchAndIncrement (rValue) + 1;
	}
	static value_t decrementAndFetch (value_t volatile &rValue) {
	    return fetchAndDecrement (rValue) - 1;
	}
    };

    template<>
    class VAtomicOperations_<unsigned __int64> : public VAtomicMemoryOperations_<sizeof (unsigned __int64)> {
    //  Family Values
    public:
	typedef unsigned __int64 value_t;
	typedef value_t storage_t;

    //  Constants
    public:
	static value_t zero () {
	    return 0;
	}

    //  Operations
    private:
	static value_t fetchAndAdd (value_t volatile &rValue, value_t iAddend) {
	    return fetchAndAdd64 (rValue, iAddend);
	}
    public:
	static void increment (value_t volatile &rValue) {
	    fetchAndIncrement (rValue);
	}
	static void decrement (value_t volatile &rValue) {
	    fetchAndDecrement (rValue);
	}
	static bool decrementIsZero (value_t volatile &rValue) {
	    return isZero (decrementAndFetch (rValue));
	}
	static bool isZero (value_t iValue) {
	    return 0 == iValue;
	}

	static value_t fetchAndIncrement (value_t volatile &rValue) {
	    return fetchAndAdd (rValue, 1);
	}
	static value_t fetchAndDecrement (value_t volatile &rValue) {
	    MemoryBarrierStoreStore ();
	    return fetchAndAdd (rValue, (value_t)-1);
	}
	static value_t incrementAndFetch (value_t volatile &rValue) {
	    return fetchAndIncrement (rValue) + 1;
	}
	static value_t decrementAndFetch (value_t volatile &rValue) {
	    return fetchAndDecrement (rValue) - 1;
	}
    };
#endif
}


#endif
