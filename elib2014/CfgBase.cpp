#include "CfgBase.h"

#include <string>
#include <fstream>
#include <vector>
//#define RAPIDXML_STATIC_POOL_SIZE ( 64*1024)
#include "rapidxml\rapidxml.hpp"
#include "rapidxml\rapidxml_utils.hpp"
#include "rapidxml\rapidxml_print.hpp"

using namespace std;
using namespace rapidxml;

namespace std {
    string to_string(bool v)
    {
        return v ? "true" : "false";
    }
}

__LIB_NAME_SPACE_BEGIN__

class _XMLCFGFile_impl
{
public:
    void Load(const char *xml_file_name)
    {
        file_name_.clear();
        if (nullptr != xml_file_)
        {
            delete xml_file_;
        }

        xml_file_ = new file<char>(xml_file_name);
        xml_.parse<0>(xml_file_->data());

        file_name_.assign(xml_file_name);
    }
    void Save()
    {
        ofstream ofs(file_name_.c_str());
        if (ofs.is_open())
        {
            print<char>(ofs, xml_);
        }
        else
        {
            throw "保存xml文件失败：打开文件失败!";
        }
    }

    void Add(const char *path, const char *value)
    {
        auto tmp = Parse(path);
        xml_node<char> *node_tmp = nullptr;
        char *tmp_str = nullptr;
        xml_node<char> *node = xml_.first_node("root");
        if (nullptr == node)
        {
            tmp_str = xml_.allocate_string("root");
            node = xml_.allocate_node(node_element, tmp_str);
            xml_.append_node(node);
        }

        size_t i = 0;
        for (; i < tmp.size() - 1; ++i)
        {
            node_tmp = node->first_node(tmp[i].c_str());
            if (nullptr != node_tmp)
            {
                node = node_tmp;
            }
            else
            {
                break;
            }
        }
        for (; i < tmp.size(); ++i)
        {
            tmp_str = xml_.allocate_string(tmp[i].c_str(), tmp[i].size());
            node_tmp = xml_.allocate_node(node_element, tmp_str);
            node->append_node(node_tmp);
            node = node_tmp;
        }

        tmp_str = xml_.allocate_string(value);
        node->value(tmp_str);
    }
    void Del(const char *path, const char *value)
    {
        auto tmp = Parse(path);
        xml_node<char> *node = xml_.first_node("root");
        if (nullptr == node)return;
        for (auto &x : tmp)
        {
            node = node->first_node(x.c_str());
            if (nullptr == node)
            {
                return;
            }
        }
        string nodename(node->name());
        decltype(node) node1 = nullptr;
        decltype(node) parnet = node->parent();
        while (node)
        {
            node1 = node->next_sibling(node->name());
            if (strcmp(value, node->value()) == 0)
            {
                node->parent()->remove_node(node);
            }
            node = node1;
        }
    }
    void Del(const char *path)
    {
        auto tmp = Parse(path);
        xml_node<char> *node = xml_.first_node("root");
        if (nullptr == node)return;
        for (auto &x : tmp)
        {
            node = node->first_node(x.c_str());
            if (nullptr == node)
            {
                return;
            }
        }

        xml_node<char> *node1 = nullptr;
        do
        {
            node1 = node->next_sibling(node->name());
            node->parent()->remove_node(node);
            node = node1;
        } while (nullptr != node);
    }

    const char * Get(const char *path)
    {
        return GetNode(path)->value();
    }

    std::vector<const char *> GetList(const char *path)
    {
        xml_node<char> *node = GetNode(path);
        vector<const char *> ret;

        while (node)
        {
            ret.push_back(node->value());
            node = node->next_sibling(node->name());
        }
        return ret;
    }

    void Set(const char *path, const char *value)
    {
        auto tmp = GetNode(path);
        tmp->value(xml_.allocate_string(value));
        tmp->first_node()->value(tmp->value());
    }

    void Set(const char *path, const char * old_value, const char * new_value)
    {
        auto node = GetNode(path);
        while (node)
        {
            if (strcmp(node->value(), old_value) == 0)
            {
                node->value(xml_.allocate_string(new_value));
                node->first_node()->value(node->value());
                return;
            }
        }
        throw logic_error(string("未找到相应结点，path=") + path + string("old_value=") + old_value);
    }

    void Set(const char *path, std::vector<const char *> &values)
    {
        auto node = GetNode(path);
        auto parent = node->parent();
        string node_name = node->name();
        decltype(node) node1 = nullptr;
        while (node)
        {
            node1 = node->next_sibling(node->name());
            node->parent()->remove_node(node);
            node = node1;
        }
        char *tmpstr = nullptr;
        for (auto &x : values)
        {
            tmpstr = xml_.allocate_string(node_name.c_str());
            node = xml_.allocate_node(node_element, tmpstr);
            tmpstr = xml_.allocate_string(x);
            node->value(tmpstr);
            parent->append_node(node);
        }
    }
private:
    vector<string> Parse(const char *path)
    {
        vector<string> ret;
        string tmp(path);
        int pos = 0;
        int pos1;
        while (true)
        {
            pos1 = tmp.find('/', pos);
            if (string::npos != pos1)
            {
                ret.push_back(tmp.substr(pos, pos1 - pos));
                pos = pos1 + 1;
            }
            else
            {
                ret.push_back(tmp.substr(pos));
                break;
            }
        }
        return ret;
    }
    xml_node<char> *GetNode(const char *path)
    {
        auto tmp = Parse(path);
        xml_node<char> *node = xml_.first_node("root");
        if (nullptr == node)logic_error("xml文件不存在root结点");

        for (auto &x : tmp)
        {
            node = node->first_node(x.c_str());
            if (nullptr == node)
            {
                throw logic_error(("不存在相应的xml结点:" + x).c_str());
            }
        }
        return node;
    }
    string file_name_;
    rapidxml::xml_document<char> xml_;
    rapidxml::file<char>   *xml_file_;
};

XMLCfgFile::XMLCfgFile() :impl_(new _XMLCFGFile_impl)
{
}
XMLCfgFile::~XMLCfgFile()
{
    delete impl_;
}
void XMLCfgFile::Load(const char *file_name)
{
    impl_->Load(file_name);
}
void XMLCfgFile::Save()
{
    impl_->Save();
}

void XMLCfgFile::Del(const char *path)
{
    impl_->Del(path);
}

//字符串
template<>
void XMLCfgFile::Add(const char *path, const char *value)
{
    impl_->Add(path, value);
}
template<>
void XMLCfgFile::Del(const char *path, const char *value)
{
    impl_->Del(path, value);
}
template<>
const char*  XMLCfgFile::Get(const char *path)
{
    return impl_->Get(path);
}
template<>
std::vector<const char *> XMLCfgFile::GetList(const char *path)
{
    return impl_->GetList(path);
}
template<>
void XMLCfgFile::Set(const char *path, const  char *value)
{
    impl_->Set(path, value);
}
template<>
void XMLCfgFile::Set(const char *path, const char * old_value, const char * new_value)
{
    impl_->Set(path, old_value, new_value);
}
template<>
void XMLCfgFile::Set(const char *path, std::vector<const char *> &vec)
{
    impl_->Set(path, vec);
}

template<class T>
T stoT(const char *);

template<>
int stoT(const char *value)
{
    return stoi(value);
}

template<>
short stoT(const char *value)
{
    return stoi(value);
}

template<>
long stoT(const char *value)
{
    return stol(value);
}

template<>
long long stoT(const char *value)
{
    return stoll(value);
}

template<>
float stoT(const char *value)
{
    return stof(value);
}

template<>
double stoT(const char *value)
{
    return stod(value);
}

template<>
long double stoT(const char *value)
{
    return stold(value);
}

template<>
bool stoT(const char *value)
{
    return (strcmp("true", value) == 0) ? true : false;
}

template<class T>
void XMLCfgFile::Add(const char *path, T value)
{
    impl_->Add(path, to_string(value).c_str());
}
template<class T>
void XMLCfgFile::Del(const char *path, T value)
{
    impl_->Del(path, to_string(value).c_str());
}
template<class T>
T XMLCfgFile::Get(const char *path)
{
    return stoT<T>(impl_->Get(path));
}
template<class T>
vector<T> XMLCfgFile::GetList(const char *path)
{
    auto vec = impl_->GetList(path);
    vector<T> ret;
    for (auto x : vec)
    {
        ret.push_back(stoT<T>(x));
    }
    return ret;
}
template<class T>
void XMLCfgFile::Set(const char *path, T value)
{
    impl_->Set(path, to_string(value).c_str());
}
template<class T>
void XMLCfgFile::Set(const char *path, T old_value, T new_value)
{
    impl_->Set(path, to_string(old_value).c_str(), to_string(new_value).c_str());
}
template<class T>
void XMLCfgFile::Set(const char *path, vector<T> &vec)
{
    vector<string> tmp1;
    for (auto &x : vec)
    {
        tmp1.push_back(to_string(x));
    }
    vector<const char *> tmp2;
    for (auto &x : tmp1)
    {
        tmp2.push_back(x.c_str());
    }
    impl_->Set(path, tmp2);
}

#define DEFINE_IMPL(T) \
    template\
    void XMLCfgFile::Add(const char *path, T);\
    template\
    void XMLCfgFile::Del(const char *path, T);\
    template\
    T XMLCfgFile::Get(const char *path);\
    template\
    std::vector<T> XMLCfgFile::GetList(const char *path);\
    template\
    void XMLCfgFile::Set(const char *path, T);\
    template\
    void XMLCfgFile::Set(const char *path, T old_value, T new_value);\
    template\
    void XMLCfgFile::Set(const char *path, std::vector<T> &);

DEFINE_IMPL(short)
DEFINE_IMPL(int)
DEFINE_IMPL(long)
DEFINE_IMPL(long long)
DEFINE_IMPL(float)
DEFINE_IMPL(double)
DEFINE_IMPL(long double)
DEFINE_IMPL(bool)

__LIB_NAME_SPACE_END__