#ifndef SYMKEY_MANAGER_H_RFT
#define SYMKEY_MANAGER_H_RFT
#include "libbase.h"
#include <list>
#include "Singleton.h"
#include "BinData.h"
#include "ClassAttr.h"

__LIB_NAME_SPACE_BEGIN__

enum class SymkeyCategory :unsigned int { AC, ENC, MAC, IDN };
enum class SymkeyAlgoType :unsigned int { TDES, SM4 };

const std::string &SymkeyCategoryToString(SymkeyCategory);
SymkeyCategory StringToSymkeyCategory(const std::string &);

const std::string &SymkeyAlgoTypeToString(SymkeyAlgoType);
SymkeyAlgoType StringToSymkeyAlgoType(const std::string &);

struct SymKey
{
	BinData keyvalue_;
	SymkeyCategory category_;
	std::string bin_;
	SymkeyAlgoType algotype_;
};

class SymkeyManager :public nocopyable
{
public:
	void Init(const char *path);
	SymKey &GetKey(const std::string &Pan, SymkeyCategory, SymkeyAlgoType);
	void AddKey(const SymKey &);
	void AddKey(SymKey &&);
	void Del(const std::string &Pan, SymkeyCategory, SymkeyAlgoType);
	typedef std::list<SymKey>::iterator iterator;
	void save();
	iterator begin()
	{
		return buff_.begin();
	}
	iterator end()
	{
		return buff_.end();
	}
private:
	std::list < SymKey > buff_;
	SymkeyManager() = default;
	friend class Singleton < SymkeyManager > ;
	std::string  path_;
};

__LIB_NAME_SPACE_END__
#endif