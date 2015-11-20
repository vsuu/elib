#include "TerminalTag.h"
#include "CfgBase.h"

__LIB_NAME_SPACE_BEGIN__
void TerminalTagSet::Load(const char *name)
{
	XMLCfgFile cfg;
	cfg.Load(name);
	auto tmp = cfg.GetList<const char *>("TLV");

	map_.clear();
	for (auto &x : tmp)
	{
		Add(BerTLV(Hex2Bin(x)));
	}
}

void _Save(TerminalTagSet *obj, XMLCfgFile &cfg)
{
	for (auto it = obj->begin(); it != obj->end(); ++it)
	{
		cfg.Add("TLV", it->ToBin().ToHex().c_str());
	}
	cfg.Save();
}

void TerminalTagSet::Save(const char *name)
{
	XMLCfgFile cfg;
	cfg.Load(name);
	_Save(this, cfg);
}
void TerminalTagSet::SaveAs(const char *name)
{
	XMLCfgFile cfg;
	cfg.Create(name);
	_Save(this, cfg);
}

void TerminalTagSet::Add(const BerTLV &tlv)
{
	map_.emplace(tlv.Tag(), tlv);
}
void TerminalTagSet::Del(TagType tag)
{
	auto it = map_.find(tag);
	if (it != map_.end())
	{
		map_.erase(it);
	}
}

__LIB_NAME_SPACE_END__