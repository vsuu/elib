#include "SymKeyManager.h"
#include "rapidxml\rapidxml.hpp"
#include "rapidxml\rapidxml_utils.hpp"
#include "rapidxml\rapidxml_print.hpp"
#include <stdexcept>

using namespace std;

__LIB_NAME_SPACE_BEGIN__
void SymkeyManager::Init(const char *path)
{
	buff_.clear();

	rapidxml::file<char> file(path);

	rapidxml::xml_document<char>  doc;

	doc.parse<0>(file.data());

	using node_type = rapidxml::xml_node < char > ;
	node_type *root = doc.first_node("root");
	if (nullptr == root)
	{
		throw runtime_error(ERR_WHERE "对称密钥配置文件格式错误");
	}
	node_type *key_node = root->first_node("symkey");
	while (nullptr != key_node)
	{
		node_type *category = key_node->first_node("category");
		node_type *algotype = key_node->first_node("algotype");
		node_type *bin = key_node->first_node("bin");
		node_type *keyvalue = key_node->first_node("value");

		if ((nullptr == category)
			|| (nullptr == algotype)
			|| (nullptr == bin)
			|| (nullptr == keyvalue)
			)
		{
			throw runtime_error(ERR_WHERE "对称密钥配置文件格式错误");
		}
		SymKey key;
		key.bin_ = bin->value();
		key.keyvalue_ = Hex2Bin(keyvalue->value());
		key.category_ = StringToSymkeyCategory(category->value());
		key.algotype_ = StringToSymkeyAlgoType(algotype->value());
		buff_.push_back(std::move(key));

		key_node = key_node->next_sibling("symkey");
	}

	path_ = path;
}

SymKey &SymkeyManager::GetKey(const string &Pan, SymkeyCategory category, SymkeyAlgoType algotype)
{
	for (auto &x : buff_)
	{
		if ((x.category_ == category)
			&& (x.algotype_ == algotype)
			&& (Pan.substr(0, x.bin_.size()) == x.bin_)
			)
		{
			return x;
		}
	}
	throw runtime_error(ERR_WHERE "未查询到相关密钥信息,卡号=" + Pan);
}
void SymkeyManager::AddKey(const SymKey &key)
{
	buff_.push_back(key);
}

void SymkeyManager::AddKey(SymKey &&key)
{
	buff_.push_back(std::move(key));
}
void SymkeyManager::Del(const string &Pan, SymkeyCategory category, SymkeyAlgoType algotype)
{
	for (auto it = buff_.begin(); it != buff_.end(); ++it)
	{
		if ((it->category_ == category)
			&& (it->algotype_ == algotype)
			&& (Pan.substr(0, it->bin_.size()) == it->bin_)
			)
		{
			buff_.erase(it);
			return;
		}
	}
}
void SymkeyManager::save()
{
	if (!path_.empty())
	{
		ofstream ofs(path_.c_str(), ios_base::trunc | ios_base::out);
		if (ofs)
		{
			rapidxml::xml_document<char> xml_;
			using node_type = rapidxml::xml_node < char > ;
			node_type *root = xml_.allocate_node(rapidxml::node_element, "root");
			xml_.append_node(root);

			for (auto &x : buff_)
			{
				node_type *key_node = xml_.allocate_node(rapidxml::node_element, "symkey");
				node_type *category_node = xml_.allocate_node(rapidxml::node_element, "category", SymkeyCategoryToString(x.category_).c_str());
				node_type *algotype_node = xml_.allocate_node(rapidxml::node_element, "algotype", SymkeyAlgoTypeToString(x.algotype_).c_str());
				node_type *bin_node = xml_.allocate_node(rapidxml::node_element, "bin", xml_.allocate_string(x.bin_.c_str()));
				node_type *value_node = xml_.allocate_node(rapidxml::node_element, "value", xml_.allocate_string(x.keyvalue_.ToHex().c_str()));
				key_node->append_node(category_node);
				key_node->append_node(algotype_node);
				key_node->append_node(bin_node);
				key_node->append_node(value_node);
				root->append_node(key_node);
			}
			rapidxml::print<char>(ofs, xml_);
		}
		else
		{
			throw runtime_error(ERR_WHERE "创建文件失败,filename=" + path_);
		}
	}
}

namespace
{
	const string strSM4("SM4");
	const string strTDES("TDES");
	const string strAC("AC");
	const string strENC("ENC");
	const string strMAC("MAC");
	const string strIDN("IDN");
}

const std::string &SymkeyCategoryToString(SymkeyCategory category)
{
	switch (category)
	{
	case SymkeyCategory::AC:
		return strAC;
	case SymkeyCategory::ENC:
		return strENC;
	case SymkeyCategory::MAC:
		return strMAC;
	case SymkeyCategory::IDN:
		return strIDN;
	default:
		throw invalid_argument(ERR_WHERE "非法的参数");
	}
}

SymkeyCategory StringToSymkeyCategory(const std::string &str)
{
	if (str == strAC)
	{
		return SymkeyCategory::AC;
	}
	else if (str == strENC)
	{
		return SymkeyCategory::ENC;
	}
	else if (str == strMAC)
	{
		return SymkeyCategory::MAC;
	}
	else if (str == strIDN)
	{
		return SymkeyCategory::IDN;
	}
	else
	{
		throw invalid_argument(ERR_WHERE "非法的参数");
	}
}

const std::string &SymkeyAlgoTypeToString(SymkeyAlgoType type)
{
	switch (type)
	{
	case SymkeyAlgoType::TDES:
		return strTDES;
	case SymkeyAlgoType::SM4:
		return strSM4;
	default:
		throw invalid_argument(ERR_WHERE "非法的参数");
	}
}
SymkeyAlgoType StringToSymkeyAlgoType(const std::string & str)
{
	if (str == strTDES)
	{
		return SymkeyAlgoType::TDES;
	}
	else if (str == strSM4)
	{
		return SymkeyAlgoType::SM4;
	}
	else
	{
		throw invalid_argument(ERR_WHERE "非法的参数");
	}
}

__LIB_NAME_SPACE_END__