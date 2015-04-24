#include "TLVDescription.h"
#include "rapidxml\rapidxml.hpp"
#include "rapidxml\rapidxml_utils.hpp"
#include <sstream>
#include "strop.h"
#include <memory>
#include <Python.h>
#include "OnScopeExit.h"

#include <mutex>
using namespace std;

namespace {
    std::once_flag init_flag;
}

__LIB_NAME_SPACE_BEGIN__

std::array<TLVDescription::maptype, static_cast<size_t>(STANDARD::SIZE)> TLVDescription::dict_array;
TLVDescription::maptype TLVDescription::common_dict;

std::array<std::unordered_map<int, std::string>, static_cast<size_t>(STANDARD::SIZE)> TLVDescription::python;
std::unordered_map<int, std::string> TLVDescription::common_python;

std::vector<std::pair<std::string, std::string>> g_var_no_description;

const std::vector<std::pair<std::string, std::string>> & TLVDescription::GetTagDescriptionVec(int tag, STANDARD standard)
{
    const maptype & dict = dict_array[static_cast<size_t>(standard)];
    maptype::const_iterator it = dict.find(tag);
    if (it != dict.end())
    {
        return it->second;
    }
    it = common_dict.find(tag);
    if (it != common_dict.end())
    {
        return it->second;
    }

    return g_var_no_description;
}

std::string TLVDescription::GetTagDescription(int tag, STANDARD standard)
{
    const maptype & dict = dict_array[static_cast<size_t>(standard)];
    maptype::const_iterator it = dict.find(tag);
    if (it != dict.end())
    {
        string ret;
        for (auto & x : it->second)
        {
            ret += x.first;
            ret += " : ";
            ret += x.second;
            ret += "\n";
        }
        return ret;
    }
    it = common_dict.find(tag);
    if (it != common_dict.end())
    {
        string ret;
        for (auto & x : it->second)
        {
            ret += x.first;
            ret += " : ";
            ret += x.second;
            ret += "\n";
        }
        return ret;
    }

    return std::string("δ�ҵ���Ӧ��˵������");
}

void init_dict(const std::string &filename, TLVDescription::maptype &dict, std::unordered_map<int, std::string> &python_map)
{
    std::string file_path(std::string("xml\\") + filename);
    try
    {
        rapidxml::file<char> xml_file(file_path.c_str());
        std::unique_ptr<rapidxml::xml_document<char>> xml_doc(new rapidxml::xml_document<char>);
        xml_doc->parse<0>(xml_file.data());
        rapidxml::xml_node<char> *root_node = xml_doc->first_node();
        auto node = root_node->first_node();

        while (nullptr != node)
        {
            auto node_id = node->first_node("Id");
            auto node_name = node->first_node("Name");
            auto node_len = node->first_node("Len");
            auto node_prensence = node->first_node("Presence");
            auto node_from = node->first_node("From");
            auto node_description = node->first_node("Description");
            auto node_value = node->first_node("Value");
            auto node_format = node->first_node("Format");
            auto node_python = node->first_node("Python");

            if (nullptr == node_id)
            {
                std::ostringstream ss;
                ss << filename << " ���ڴ������ã���Id �� ";
                throw std::runtime_error(ss.str().c_str());
            }
            int tag = std::stoi(node_id->value(), nullptr, 16);
            //std::string desc;
            vector<pair<string, string>> desc;
            if (node_name)
            {
                /*desc.append("���� : ");
                desc.append(node_name->value());
                desc.push_back('\n');*/
                desc.push_back(make_pair("����", node_name->value()));
            }
            else
            {
                throw std::runtime_error((filename + "���ô��� id=" + node_id->value()).c_str());
            }
            if (node_len)
            {
                /*desc.append("���� : ");
                desc.append(node_len->value());
                desc.push_back('\n');*/
                desc.push_back(make_pair("����", node_len->value()));
            }
            if (node_prensence)
            {
                /*desc.append("����̶� : ");
                desc.append(node_prensence->value());
                desc.push_back('\n');*/
                desc.push_back(make_pair("����̶�", node_prensence->value()));
            }
            if (node_from)
            {
                /*desc.append("��Դ : ");
                desc.append(node_from->value());
                desc.push_back('\n');*/
                desc.push_back(make_pair("��Դ", node_from->value()));
            }
            if (node_description)
            {
                ///*desc.append("���� : ");
                //desc.append(node_description->value());
                //desc.push_back('\n');*/
                desc.push_back(make_pair("����", node_description->value()));
            }
            if (node_format)
            {
                /*desc.append("��ʽ : ");
                desc.append(node_format->value());
                desc.push_back('\n');*/
                desc.push_back(make_pair("��ʽ", node_format->value()));
            }
            if (node_value)
            {
                /*desc.append("ֵ��֯�߼� : ");
                desc.append(node_value->value());
                desc.push_back('\n');*/
                string tmp(node_value->value());
                if (!tmp.empty())
                {
                    if (tmp.substr(0, 2) == "\r\n")
                    {
                        tmp.erase(0, 2);
                    }
                }
                if (!tmp.empty())
                {
                    if (tmp.back() == '\n')tmp.pop_back();
                    if (tmp.substr(tmp.size() - 2, 2) == "\r\n")
                    {
                        tmp.erase(tmp.size() - 2, 2);
                    }
                }
                desc.push_back(make_pair("ֵ��֯�߼�", tmp));
            }

            dict[tag] = std::move(desc);

            if (node_python)
            {
                python_map[tag] = node_python->value();
            }

            //loop
            node = node->next_sibling();
        }
    }
    catch (const runtime_error &)
    {
        throw;
    }
    catch (std::exception &e)
    {
        throw std::runtime_error(e.what());
    }
}

void  TLVDescription::init_impl()
{
    init_dict("pboc_tag_info.xml", TLVDescription::dict_array[static_cast<size_t>(STANDARD::PBOC)], TLVDescription::python[static_cast<size_t>(STANDARD::PBOC)]);
    init_dict("visa_tag_info.xml", TLVDescription::dict_array[static_cast<size_t>(STANDARD::VISA)], TLVDescription::python[static_cast<size_t>(STANDARD::VISA)]);
    init_dict("jcb_tag_info.xml", TLVDescription::dict_array[static_cast<size_t>(STANDARD::JCB)], TLVDescription::python[static_cast<size_t>(STANDARD::JCB)]);
    init_dict("mastercard_tag_info.xml", TLVDescription::dict_array[static_cast<size_t>(STANDARD::MASTERCARD)], TLVDescription::python[static_cast<size_t>(STANDARD::MASTERCARD)]);
    init_dict("common_tag_info.xml", TLVDescription::common_dict, TLVDescription::common_python);
    g_var_no_description.push_back(std::make_pair(string("error"), string("δ�ҵ���TAG�Ľ���!")));

    Py_SetPath(L"Python34.zip");

    Py_Initialize();

    if (!Py_IsInitialized())
    {
        //AfxMessageBox(_T("��ʼ��Pythonʧ��!"));
    }
    else
    {
        std::string python_path;//= GetPath();
        python_path += "py";

        PySys_SetPath(elib::str2wstr(python_path).c_str());
    }
}

void TLVDescription::init()
{
    std::call_once(init_flag, &TLVDescription::init_impl);
}

const std::string TLVDescription::GetTagInfo(int tag, const std::string &value, STANDARD s)
{
    std::string ret;
    auto dict = python[static_cast<int>(s)];
    if (dict.find(tag) != dict.end())
    {
        ret = dict[tag];
    }
    else
        if (common_python.find(tag) != common_python.end())
        {
            ret = common_python[tag];
        }
        else
        {
            return ret;
        }
    PyObject * pModule = NULL;
    PyObject * pFun = NULL;
    PyObject * pRet = NULL;

    bool flag = false;
    do
    {
        pModule = PyImport_ImportModule(ret.c_str());
        if (NULL == pModule)
        {
            break;//throw DataParseException(_T("python ����ģ��ʧ��"));
        }
        pFun = PyObject_GetAttrString(pModule, "transform");
        if (NULL == pFun)
        {
            break;// throw DataParseException(_T("python ���뺯��ʧ��"));
        }
        pRet = PyEval_CallFunction(pFun, "(s)", elib::Bin2Hex(value.data(), value.size()).c_str());
        if (NULL == pRet)
        {
            break;// throw DataParseException(_T("����ֵΪNULL"));
        }
        flag = true;
    } while (false);

    if (flag)
    {
        wchar_t * desc = NULL;
        int len = 0;
        PyArg_Parse(pRet, "u#", &desc, &len);
        ret = elib::wstr2str(wstring(desc, len));
    }
    if (pRet)Py_DECREF(pRet);
    if (pFun)Py_DECREF(pFun);
    if (pModule)Py_DECREF(pModule);

    if ((!flag) && (PyErr_Occurred() != NULL))
    {
        throw std::runtime_error("ִ��Python �ű����ִ���!");
    }

    return ret;
}

__LIB_NAME_SPACE_END__