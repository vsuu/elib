#ifndef _LIB_BASE_H_RFT
#define _LIB_BASE_H_RFT

#define __LIB_NAME__ elib
#define __LIB_NAME_SPACE_BEGIN__  namespace __LIB_NAME__ {
#define __LIB_NAME_SPACE_END__    }

#define ELIB_STRINGLINE(x) #x
#define ELIB_LINETOSTRING(x) ELIB_STRINGLINE(x)
#define ERR_WHERE  "error at " __FILE__  ", line " ELIB_LINETOSTRING(__LINE__) " : "

#define elif else if

#define C2RNG(x) x.begin(),x.end()

#endif // _LIB_BASE_H_RFT
