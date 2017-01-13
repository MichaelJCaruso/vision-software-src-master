/*****  Global Option Access Facility Exported Declarations  *****/
#ifndef GOPT_Interface
#define GOPT_Interface

/*****  Global Option Description Declaration Utilities  *****/
#include "gopt.d"

/*****  Function Declarations  *****/
PublicFnDecl void GOPT_AcquireOptions (int argc, char *argv[]);

PublicFnDecl char const *GOPT_GetValueOption (char const *optionName);
PublicFnDecl int GOPT_GetSwitchOption (char const *optionName);

PublicFnDecl void GOPT_SeekExtraArgument (
    int				offset,
    int				whence
);

PublicFnDecl char const *GOPT_GetExtraArgument ();

PublicFnDecl int GOPT_GetExtraArgCnt ();

PublicFnDecl char const *GOPT_ProgramName ();


#endif
