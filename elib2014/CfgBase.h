#ifndef CFGBASE_H_RFT
#define CFGBASE_H_RFT

#include <vector>
#include <stdint.h>
#include "ClassAttr.h"
#include "libbase.h"
#include <memory>

__LIB_NAME_SPACE_BEGIN__
/*

本类用于做配置读取、更新、增加、删除

*/
class _XMLCFGFile_impl;

class XMLCfgFile : __LIB_NAME__::nocopyable
{
public:
    XMLCfgFile();
    ~XMLCfgFile();
    void Load(const char *file_name);//加载xml文件
    void Save()const;//保存到xml文件

    template<class T>
    void Add(const char *path, T);
    template<class T>
    void Del(const char *path, T);
    void Del(const char *path);
    template<class T>
    T Get(const char *path)const;
    template<class T>
    std::vector<T> GetList(const char *path)const;
    template<class T>
    void Set(const char *path, T);
    template<class T>
    void Set(const char *path, T old_value, T new_value);
    template<class T>
    void Set(const char *path, const std::vector<T> &);
private:
    _XMLCFGFile_impl *impl_;
};

__LIB_NAME_SPACE_END__
#endif
