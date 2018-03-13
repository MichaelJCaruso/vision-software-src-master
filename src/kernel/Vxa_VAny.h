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

/*************************
 *****  Definitions  *****
 *************************/

namespace Vxa {
    class VCollectableObject;

/************************
 *----  class VAny  ----*
 ************************/
    class Vxa_API VAny {
    //  Aliases
    public:
        typedef VAny const& value_t;

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

/********************************
 *----  class VAny::Client  ----*
 ********************************/
    class Vxa_API VAny::Client {
    //  Construction
    protected:
        Client () {}

    //  Destruction
    protected:
        ~Client () {}

    //  Delivery
    public:
        template <typename Datum> bool consume (Datum iDatum) {
            return on (iDatum);
        }

    //  Callbacks
    protected:
        virtual bool on (int iValue) = 0;
        virtual bool on (double iValue) = 0;
        virtual bool on (VString const &rString) = 0;
        virtual bool on (VCollectableObject *pObject) = 0;
    };
} //  namespace Vxa


#endif
