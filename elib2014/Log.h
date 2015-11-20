#ifndef LOG_H_RFT
#define LOG_H_RFT

#include "libbase.h"
#include <string>
#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <set>
#include <condition_variable>
#include <initializer_list>
#include <tuple>
#include "ObjectPool.h"
#include "ClassAttr.h"
#include <cassert>
#include <algorithm>
#include <sstream>
#include "Singleton.h"

__LIB_NAME_SPACE_BEGIN__

//Ч��
//�ļ��Զ�rolling

enum class LogType : unsigned int { DBG = 1, INFO = 2, NOTIFY = 4, WARN = 8, ERR = 16 };
//
//class LogLevel
//{
//public:
//    LogLevel(std::initializer_list<LogType>);
//    void Add(LogType l)
//    {
//        l_ |= static_cast<unsigned int>(l);
//    }
//    void Del(LogType l)
//    {
//        l_ &= ~static_cast<unsigned int>(l);
//    }
//    bool Contain(LogType lt)const
//    {
//        return (static_cast<unsigned int>(lt)& l_) != 0;
//    }
//private:
//    unsigned int l_;
//};

using LogLevel = LogType;

class LogFileBase :public elib::nocopyable
{
public:
	LogFileBase(const char *name, LogLevel lv) :loglevel_(lv), name_(name)
	{}
	virtual ~LogFileBase() = 0
	{}
	virtual void WriteLog(LogType, const char *, size_t len) = 0;
	void SetLevel(const LogLevel & ll)
	{
		loglevel_ = ll;
	}
	const LogLevel &Level()const
	{
		return loglevel_;
	}
	const char *Name()const
	{
		return name_.c_str();
	}
private:
	LogLevel loglevel_;
	std::string name_;
};

class Fmt : public elib::nocopyable
{
public:
	template<typename T>
	Fmt(const char* fmt, T val);

	const char* data() const { return buf_; }
	int length() const { return length_; }

private:
	char buf_[32];
	int length_;
};

namespace detail{
	class LogStream;
}
class Logger : public nocopyable
{
public:
	Logger();
	~Logger();
	void AddLogFile(std::unique_ptr<LogFileBase> lf);
	void DeleteLogFileByName(const char *);
	LogFileBase *FindLogFileByName(const char *);

	LogType LowerLogLevel()const
	{
		return log_type_;
	}
	void ReflushLowerLogLevel();
	friend class detail::LogStream;
	typedef  std::vector < std::unique_ptr<LogFileBase> >::iterator iterator;

private:
	void BufferIn(LogType, std::string);
	std::vector < std::unique_ptr<LogFileBase> > logfiles_;
	std::vector < std::tuple<LogType, std::string>> buffer_;
	std::vector<std::tuple<LogType, std::string>> buffer1_;

	LogType log_type_;
	void Process();
	bool run_;
	std::mutex mutex_;
	std::condition_variable cv_;
	std::thread thread_;
};

namespace detail
{
	class LogStream : nocopyable
	{
		typedef LogStream self;
	public:
		LogStream(LogType t, Logger & l) : log_type_(t), logger_(l)
		{}
		~LogStream()
		{
			logger_.BufferIn(log_type_, ss_.str());
		}
		template<typename T>
		self & operator<<(const T& x)
		{
			ss_ << x;
			return *this;
		}
		self & operator<<(const Fmt& fmt)
		{
			ss_ << fmt.data();
			return *this;
		}
		self& operator<< (std::ostream& (*pf)(std::ostream&))
		{
			ss_ << pf;
			return *this;
		}
		self& operator<< (std::ios& (*pf)(std::ios&))
		{
			ss_ << pf;
			return *this;
		}
		self& operator<< (std::ios_base& (*pf)(std::ios_base&))
		{
			ss_ << pf;
			return *this;
		}
	private:
		std::ostringstream ss_;
		LogType log_type_;
		Logger & logger_;
	};
}

class LogFile :public  LogFileBase  //�����ļ���־
{
public:
	LogFile(const char *Path, const char *FileBaseName, LogLevel LogLevel);
	~LogFile();
private:
	virtual void WriteLog(LogType, const char *, size_t);
	FILE *fp_;
	std::string path_;
	std::string file_base_name_;
	static FILE *CreateLogFile(const char *Path, const char *FileBaseName);
};

#define LOG_INFO(x) if((x).LowerLogLevel()<=elib::LogType::INFO)elib::detail::LogStream(elib::LogType::INFO,(x))
#define LOG_NOTIFY(x) if((x).LowerLogLevel()<=elib::LogType::NOTIFY)elib::detail::LogStream(elib::LogType::NOTIFY,(x))
#define LOG_DEBUG(x) if((x).LowerLogLevel()<=elib::LogType::DBG)elib::detail::LogStream(elib::LogType::DBG,(x))
#define LOG_WARN(x) if((x).LowerLogLevel()<=elib::LogType::WARN)elib::detail::LogStream(elib::LogType::WARN,(x))
#define LOG_ERROR(x) if((x).LowerLogLevel()<=elib::LogType::ERR)elib::detail::LogStream(elib::LogType::ERR,(x))

__LIB_NAME_SPACE_END__

#endif