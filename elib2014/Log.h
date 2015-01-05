#ifndef LOG_H_RFT
#define LOG_H_RFT

#include "libbase.h"


__LIB_NAME_SPACE_BEGIN__

//效率
//文件自动rolling

enum class LOGLEVEL {TRACE,DEBUG,INFO,WARN,ERR,FATAL};
const unsigned long long rotate_size = 1024 * 1024 * 1024;
const LOGLEVEL GLOBAL_LOG_LEVEL = DEBUG;

class elog
{
public:
	elog();
private:

};

#if GLOBAL_LOG_LEVEL > LOGLEVEL::ERR
#define LOG_ERR
#else
#define LOG_ERR 1
#endif



__LIB_NAME_SPACE_END__



#endif