#include "TLVDescription.h"
#include "rapidxml\rapidxml.hpp"
#include "rapidxml\rapidxml_utils.hpp"
#include <sstream>
#include "BinData.h"
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

std::array<std::unordered_map<int, TLV_VALUE_FORMAT>, static_cast<size_t>(STANDARD::SIZE)> TLVDescription::dict_value_format;

std::unordered_map<int, TLV_VALUE_FORMAT> TLVDescription::common_vt_dict;

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

	return std::string("未找到对应的说明数据");
}

void init_dict(const std::string &filename, TLVDescription::maptype &dict, std::unordered_map<int, std::string> &python_map)
{
	std::string file_path(filename);
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
				ss << filename << " 存在错误配置，无Id 项 ";
				throw std::runtime_error(ss.str().c_str());
			}
			int tag = std::stoi(node_id->value(), nullptr, 16);
			//std::string desc;
			vector<pair<string, string>> desc;
			if (node_name)
			{
				/*desc.append("名称 : ");
				desc.append(node_name->value());
				desc.push_back('\n');*/
				desc.push_back(make_pair("名称", node_name->value()));
			}
			else
			{
				throw std::runtime_error((filename + "配置错误 id=" + node_id->value()).c_str());
			}
			if (node_len)
			{
				/*desc.append("长度 : ");
				desc.append(node_len->value());
				desc.push_back('\n');*/
				desc.push_back(make_pair("长度", node_len->value()));
			}
			if (node_prensence)
			{
				/*desc.append("需求程度 : ");
				desc.append(node_prensence->value());
				desc.push_back('\n');*/
				desc.push_back(make_pair("需求程度", node_prensence->value()));
			}
			if (node_from)
			{
				/*desc.append("来源 : ");
				desc.append(node_from->value());
				desc.push_back('\n');*/
				desc.push_back(make_pair("来源", node_from->value()));
			}
			if (node_description)
			{
				///*desc.append("描述 : ");
				//desc.append(node_description->value());
				//desc.push_back('\n');*/
				desc.push_back(make_pair("描述", node_description->value()));
			}
			if (node_format)
			{
				/*desc.append("格式 : ");
				desc.append(node_format->value());
				desc.push_back('\n');*/
				desc.push_back(make_pair("格式", node_format->value()));
			}
			if (node_value)
			{
				/*desc.append("值组织逻辑 : ");
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
				desc.push_back(make_pair("值组织逻辑", tmp));
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

void init_value_format_dict(const TLVDescription::maptype &map_all, std::unordered_map<int, TLV_VALUE_FORMAT> &vt_map)
{
	TLV_VALUE_FORMAT vt;
	for (auto it = map_all.begin(); it != map_all.end(); ++it)
	{
		for (auto &x : it->second)
		{
			if (x.first == "格式")
			{
				if (x.second == "n" || x.second == "数字")
				{
					vt = TLV_VALUE_FORMAT::n;
				}
				else if (x.second == "cn" || x.second == "压缩数字")
				{
					vt = TLV_VALUE_FORMAT::cn;
				}
				else if (x.second == "b" || x.second == "二进制")
				{
					vt = TLV_VALUE_FORMAT::b;
				}
				else if (x.second == "an" || x.second == "字母数字")
				{
					vt = TLV_VALUE_FORMAT::an;
				}
				else if (x.second == "ans" || x.second == "特殊字母数字")
				{
					vt = TLV_VALUE_FORMAT::ans;
				}
				vt_map.emplace(it->first, vt);//tag, value type
				break;
			}
		}
	}
}

void  TLVDescription::init_impl(const char *path)
{
	init_dict(std::string(path) + "\\pboc_tag_info.xml", TLVDescription::dict_array[static_cast<size_t>(STANDARD::PBOC)], TLVDescription::python[static_cast<size_t>(STANDARD::PBOC)]);
	init_dict(std::string(path) + "\\visa_tag_info.xml", TLVDescription::dict_array[static_cast<size_t>(STANDARD::VISA)], TLVDescription::python[static_cast<size_t>(STANDARD::VISA)]);
	init_dict(std::string(path) + "\\jcb_tag_info.xml", TLVDescription::dict_array[static_cast<size_t>(STANDARD::JCB)], TLVDescription::python[static_cast<size_t>(STANDARD::JCB)]);
	init_dict(std::string(path) + "\\mastercard_tag_info.xml", TLVDescription::dict_array[static_cast<size_t>(STANDARD::MASTERCARD)], TLVDescription::python[static_cast<size_t>(STANDARD::MASTERCARD)]);
	init_dict(std::string(path) + "\\common_tag_info.xml", TLVDescription::common_dict, TLVDescription::common_python);
	init_value_format_dict(TLVDescription::dict_array[static_cast<size_t>(STANDARD::PBOC)], dict_value_format[static_cast<size_t>(STANDARD::PBOC)]);
	init_value_format_dict(TLVDescription::dict_array[static_cast<size_t>(STANDARD::VISA)], dict_value_format[static_cast<size_t>(STANDARD::PBOC)]);
	init_value_format_dict(TLVDescription::dict_array[static_cast<size_t>(STANDARD::JCB)], dict_value_format[static_cast<size_t>(STANDARD::PBOC)]);
	init_value_format_dict(TLVDescription::dict_array[static_cast<size_t>(STANDARD::MASTERCARD)], dict_value_format[static_cast<size_t>(STANDARD::PBOC)]);
	init_value_format_dict(common_dict, common_vt_dict);

	g_var_no_description.push_back(std::make_pair(string("error"), string("未找到本TAG的解释!")));
}

void TLVDescription::init(const char *path)
{
	std::call_once(init_flag, &TLVDescription::init_impl, path);
}

const std::string TLVDescription::GetTagInfo(int tag, const std::string &value, STANDARD s)
{
	std::string ret;
	auto dict = python[static_cast<int>(s)];
	if (dict.find(tag) != dict.end())
	{
		ret = dict[tag];
	}
	else if (common_python.find(tag) != common_python.end())
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
			break;//throw DataParseException(_T("python 导入模块失败"));
		}
		pFun = PyObject_GetAttrString(pModule, "transform");
		if (NULL == pFun)
		{
			break;// throw DataParseException(_T("python 导入函数失败"));
		}
		pRet = PyEval_CallFunction(pFun, "(s)", value.c_str());
		if (NULL == pRet)
		{
			break;// throw DataParseException(_T("返回值为NULL"));
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
		throw std::runtime_error("执行Python 脚本出现错误!");
	}

	return ret;
}

TLV_VALUE_FORMAT TLVDescription::GetValueFomat(int tag, STANDARD s)
{
	auto it = dict_value_format[static_cast<size_t>(s)].find(tag);
	if (it != dict_value_format[static_cast<size_t>(s)].end())
	{
		return it->second;
	}
	it = common_vt_dict.find(tag);
	if (it != common_vt_dict.end())
	{
		return it->second;
	}
	return TLV_VALUE_FORMAT::err;
}

__LIB_NAME_SPACE_END__