#ifndef __TLVDESCRIPTION_H_RFT
#define __TLVDESCRIPTION_H_RFT

#include <unordered_map>
#include <string>
#include <vector>
#include <utility>
#include <array>

#include "libbase.h"

__LIB_NAME_SPACE_BEGIN__

enum class STANDARD :unsigned int { VISA = 0, MASTERCARD, JCB, PBOC, SIZE };
enum class TLV_VALUE_FORMAT : unsigned int { n, cn, b, an, ans, err };

class TLVDescription
{
public:
	//using maptype = std::unordered_map<int, std::string>;
	using maptype = std::unordered_map < int, std::vector<std::pair<std::string, std::string>> > ;
	static std::string GetTagDescription(int tag, STANDARD);
	static const std::vector<std::pair<std::string, std::string>> &GetTagDescriptionVec(int tag, STANDARD);
	static const std::string GetTagInfo(int tag, const std::string &, STANDARD);
	static TLV_VALUE_FORMAT GetValueFomat(int tag, STANDARD);
	static void init(const char *path);
private:
	static std::array<std::unordered_map<int, TLV_VALUE_FORMAT>, static_cast<size_t>(STANDARD::SIZE)> dict_value_format;
	static std::array<maptype, static_cast<size_t>(STANDARD::SIZE)> dict_array;
	static std::array<std::unordered_map<int, std::string>, static_cast<size_t>(STANDARD::SIZE)> python;
	static maptype common_dict;
	static std::unordered_map<int, TLV_VALUE_FORMAT> common_vt_dict;
	static std::unordered_map<int, std::string> common_python;
	static void init_impl(const char *path);
};

__LIB_NAME_SPACE_END__

#endif