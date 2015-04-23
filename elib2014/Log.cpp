#include "Log.h"
#include <cstdarg>
#include <stdexcept>
#include <sstream>
#include "TimeCost.h"
#include <cstdio>
#include <type_traits>
#include <algorithm>

using namespace std;

__LIB_NAME_SPACE_BEGIN__

Logger::Logger()
:thread_(&Logger::Process, this), run_(true)
{
}
Logger::~Logger()
{
    {
        lock_guard<mutex> locker(mutex_);
        run_ = false;
    }
    cv_.notify_one();
    thread_.join();
}
void Logger::AddLogFile(std::unique_ptr<LogFileBase> lf)
{
    if (logfiles_.empty())
    {
        log_type_ = lf->Level();
    }
    else if (static_cast<unsigned int>(lf->Level()) < static_cast<unsigned int>(log_type_))
    {
        log_type_ = lf->Level();
    }

    logfiles_.push_back(std::move(lf));
}
void Logger::DeleteLogFileByName(const char *name)
{
    for (auto it = logfiles_.begin(); it != logfiles_.end(); ++it)
    {
        if (strcmp((*it)->Name(), name) == 0)
        {
            logfiles_.erase(it);
            break;
        }
    }
    auto it = min_element(begin(logfiles_), end(logfiles_),
        [](const unique_ptr<LogFileBase> &l, const unique_ptr<LogFileBase> &r)
    {
        return static_cast<unsigned int>(l->Level()) < static_cast<unsigned int>(r->Level());
    }
    );
    log_type_ = (*it)->Level();
}

void Logger::BufferIn(LogType t, std::string  s)
{
    {
        lock_guard<mutex> locker(mutex_);
        if (run_)
        {
            buffer_.emplace_back(t, std::move(s));
        }
    }
    cv_.notify_one();
}

void Logger::Process()
{
    unique_lock<mutex> locker(mutex_);
    while (true)
    {
        cv_.wait(locker, [&]{return !run_ || !buffer_.empty(); });
        if (buffer_.empty())break;
        buffer_.swap(buffer1_);
        locker.unlock();
        for (auto &tmp : buffer1_)
        {
            for (auto &x : logfiles_)
            {
                //if (x->Level().Contain(get<0>(tmp)))
                if (static_cast<unsigned int>(get<0>(tmp)) >= static_cast<unsigned int>(x->Level()))
                {
                    x->WriteLog(get<0>(tmp), get<1>(tmp).c_str(), get<1>(tmp).size());
                }
            }
        }
        buffer1_.clear();
        locker.lock();
    }
}

LogFile::LogFile(const char *Path, const char *FileBaseName, LogLevel LogLevel)
    :LogFileBase(FileBaseName, LogLevel), path_(Path), file_base_name_(FileBaseName)
    , fp_(CreateLogFile(Path, FileBaseName))
{
    if (nullptr == fp_)
    {
        //        throw std::system_error(errno);
    }
}
LogFile::~LogFile()
{
    if (nullptr != fp_)
    {
        fclose(fp_);
    }
}
void LogFile::WriteLog(LogType t, const char *s, size_t)
{
    fputs(s, fp_);
    if (ftell(fp_) > 100 * 1024 * 1024)
    {
        fclose(fp_);
        fp_ = CreateLogFile(path_.c_str(), file_base_name_.c_str());
    }
}

FILE *LogFile::CreateLogFile(const char *Path, const char *FileBaseName)
{
    std::stringstream ss;
    ss << Path << FileBaseName << '_' << this_thread::get_id() << '_';
    time_t t = time(0);
    struct tm*		pstTM = 0;
    pstTM = localtime(&t);
    char tmp[1024];
    sprintf(tmp,
        "%04d%02d%02d_%02d%02d%02d",
        pstTM->tm_year + 1900,
        pstTM->tm_mon + 1,
        pstTM->tm_mday,
        pstTM->tm_hour,
        pstTM->tm_min,
        pstTM->tm_sec);
    ss << tmp << ".log";

    FILE * ret = fopen(ss.str().c_str(), "w");
    if (nullptr == ret)
    {
        throw runtime_error("创建日志文件失败!");
    }
    return ret;
}

template<typename T>
Fmt::Fmt(const char* fmt, T val)
{
    static_assert(std::is_arithmetic<T>::value, "Fmt Not Support this type");

    int length_ = _snprintf(buf_, sizeof(buf_), fmt, val);
    assert(length_ > 0 && static_cast<size_t>(length_) < sizeof(buf_));
}

// Explicit instantiations

template Fmt::Fmt(const char* fmt, char);

template Fmt::Fmt(const char* fmt, short);
template Fmt::Fmt(const char* fmt, unsigned short);
template Fmt::Fmt(const char* fmt, int);
template Fmt::Fmt(const char* fmt, unsigned int);
template Fmt::Fmt(const char* fmt, long);
template Fmt::Fmt(const char* fmt, unsigned long);
template Fmt::Fmt(const char* fmt, long long);
template Fmt::Fmt(const char* fmt, unsigned long long);

template Fmt::Fmt(const char* fmt, float);
template Fmt::Fmt(const char* fmt, double);

__LIB_NAME_SPACE_END__