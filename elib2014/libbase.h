#ifndef _LIB_BASE_H_RFT
#define _LIB_BASE_H_RFT

#define __LIB_NAME__ elib
#define __LIB_NAME_SPACE_BEGIN__  namespace __LIB_NAME__ {
#define __LIB_NAME_SPACE_END__    }

#define STR(x) #x
#define XSTR(x) STR(x)

#define ERR_WHERE  "error at " __FILE__  ", line " XSTR(__LINE__) " : "

#define elif else if

#define RNG(x) std::begin(x),std::end(x)



#endif // _LIB_BASE_H_RFT
