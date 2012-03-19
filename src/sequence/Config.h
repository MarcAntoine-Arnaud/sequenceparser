/*
 * Author: Guillaume Chatelet
 * adapted from http://gcc.gnu.org/wiki/Visibility
 */

#ifndef CONFIG_H_
#define CONFIG_H_

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
  #define SEQUENCEPARSER_HELPER_DLL_IMPORT __declspec(dllimport)
  #define SEQUENCEPARSER_HELPER_DLL_EXPORT __declspec(dllexport)
  #define SEQUENCEPARSER_HELPER_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define SEQUENCEPARSER_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define SEQUENCEPARSER_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define SEQUENCEPARSER_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define SEQUENCEPARSER_HELPER_DLL_IMPORT
    #define SEQUENCEPARSER_HELPER_DLL_EXPORT
    #define SEQUENCEPARSER_HELPER_DLL_LOCAL
  #endif
#endif

// Now we use the generic helper definitions above to define SP_API and SP_LOCAL.
// SP_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// SP_LOCAL is used for non-api symbols.

#ifdef SEQUENCEPARSER_DLL // defined if SP is compiled as a DLL
  #ifdef SEQUENCEPARSER_DLL_EXPORTS // defined if we are building the SP DLL (instead of using it)
    #define SEQUENCEPARSER_API SEQUENCEPARSER_HELPER_DLL_EXPORT
  #else
    #define SEQUENCEPARSER_API SEQUENCEPARSER_HELPER_DLL_IMPORT
  #endif // SEQUENCEPARSER_DLL_EXPORTS
  #define SEQUENCEPARSER_LOCAL SEQUENCEPARSER_HELPER_DLL_LOCAL
#else // SEQUENCEPARSER_DLL is not defined: this means SP is a static lib.
  #define SEQUENCEPARSER_API
  #define SEQUENCEPARSER_LOCAL
#endif // SEQUENCEPARSER_DLL

#endif /* CONFIG_H_ */
