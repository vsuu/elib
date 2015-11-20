#ifndef TERMINALTAG_H_RFT
#define TERMINALTAG_H_RFT

#include "libbase.h"
#include "Singleton.h"
#include <map>
#include <iterator>
#include "BerTLV.h"

__LIB_NAME_SPACE_BEGIN__

class TerminalTagSet
{
public:
	//±È¿˙ tlv∂‘œÛ
	class iterator :public std::iterator < std::bidirectional_iterator_tag, BerTLV >
	{
		typedef iterator self;
		std::map <TagType, BerTLV>::iterator it_;
		iterator(std::map <TagType, BerTLV>::iterator it) :it_(it)
		{}
		friend class TerminalTagSet;
	public:
		~iterator() = default;
		bool operator==(const iterator& r)const
		{
			return it_ == r.it_;
		}
		bool operator!=(const iterator& r)const
		{
			return it_ != r.it_;
		}
		BerTLV &operator*()const
		{
			return it_->second;
		}
		BerTLV *operator->()const
		{
			return &(it_->second);
		}
		self &operator++()
		{
			it_++;
			return *this;
		}
		self operator++(int)
		{
			self ret(*this);
			it_++;
			return ret;
		}
		self &operator--()
		{
			it_--;
			return *this;
		}
		self operator--(int)
		{
			self ret(*this);
			it_--;
			return ret;
		}
	};
	typedef const iterator const_iterator;
	iterator begin()
	{
		return iterator(map_.begin());
	}
	iterator end()
	{
		return iterator(map_.end());
	}
	const_iterator cbegin()const
	{
		return const_cast<TerminalTagSet *>(this)->begin();
	}
	const_iterator cend()const
	{
		return const_cast<TerminalTagSet *>(this)->end();
	}
	iterator find(TagType tag)
	{
		return iterator(map_.find(tag));
	}
	const_iterator find(TagType tag)const
	{
		return const_cast<TerminalTagSet *>(this)->find(tag);
	}
	void Load(const char *filename);
	void Save(const char *filename);
	void SaveAs(const char *filename);
	void Add(const BerTLV &);
	void Del(TagType);
private:
	TerminalTagSet() = default;
	friend class  Singleton < TerminalTagSet > ;
	std::map <TagType, BerTLV>  map_;
};

__LIB_NAME_SPACE_END__

#endif // !TERMINALTAG_H_RFT
