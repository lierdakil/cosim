#ifndef COSIMPROTO_GLOBAL_H
#define COSIMPROTO_GLOBAL_H

#  if defined(Q_OS_WIN) || defined(Q_CC_NOKIAX86) || defined(Q_CC_RVCT)
#    define Q_DECL_IMPORT __declspec(dllimport)
#  else
#    define Q_DECL_IMPORT
#  endif

#if defined(COSIMPROTO_LIBRARY)
#  define COSIMPROTOSHARED_EXPORT
#else
#  define COSIMPROTOSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // COSIMPROTO_GLOBAL_H
