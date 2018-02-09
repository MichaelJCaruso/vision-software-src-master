#ifndef Vxa_VAny_Interface
#define Vxa_VAny_Interface

/*********************
 *****  Library  *****
 *********************/

#include "Vxa.h"

/************************
 *****  Components  *****
 ************************/

/**************************
 *****  Declarations  *****
 **************************/

#include "V_VFamilyValues.h"

/*************************
 *****  Definitions  *****
 *************************/

namespace Vxa {
    class Vxa_API VAny {
    //  Client
    public:
        class Client;

    //  Construction
    protected:
        VAny () {}

    //  Destruction
    protected:
        ~VAny () {}

    //  Use
    public:
        virtual void supply (Client &rClient) const = 0;
    };

    class Vxa_API VAny::Client {
    //  Construction
    protected:
        Client () {}

    //  Destruction
    protected:
        ~Client () {}

    //  Callbacks
    public:
        virtual void on (int iValue) = 0;
        virtual void on (double iValue) = 0;
        virtual void on (VString const &rString) = 0;
    };
} //  namespace Vxa


#endif