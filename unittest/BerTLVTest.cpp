#include "stdafx.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iterator>
#include "BerTLV.h"
#include "strop.h"

using namespace std;
using namespace elib;

void TestBerTLV()
{
	try
	{
		TagType tag;
		char * tagstr = "\x70\x80";
		char * tagstr1 = "\x1F\x70";
		BerTLV::ParseTag(tagstr, tagstr + 2, tag);
		assert(tag == 0x70);
		BerTLV::ParseTag(tagstr1, tagstr1 + 2, tag);
		assert(tag == 0x1F70);

		uint32_t len;
		char *lenstr = "\x81\x10";
		char *lenstr2 = "\x79\x10";
		char *lenstr3 = "\x82\x10\x10";
		BerTLV::ParseLen(lenstr, lenstr + 2, len);
		assert(len == 0x10);
		BerTLV::ParseLen(lenstr2, lenstr2 + 2, len);
		assert(len == 0x79);
		BerTLV::ParseLen(lenstr3, lenstr3 + 3, len);
		assert(len == 0x1010);

		BerTLV tlv(0x70);
		tlv.SetValue(Hex2Bin("5F340101"));
		tlv.AppendChild(BerTLV(0x5A, Hex2Bin("123456")));
		tlv.AppendChild(BerTLV(0xBF0C));
		auto ptr = tlv.Find(0xBF0C);
		ptr->AppendChild(BerTLV(0x9F4D, Hex2Bin("0B0A")));
		tlv.InsertChild(0x9F4D, BerTLV(Hex2Bin("95051122334455")));

		BinData out_buf;
		BerTLV::EncapTLV(tlv, back_inserter(out_buf));

		cout << out_buf.ToHex().c_str() << endl;

		BerTLV tlv2;
		BerTLV::ParseTLV(begin(out_buf), end(out_buf), tlv2);

		stringstream ss(ios_base::in | ios_base::out | ios_base::binary);
		BerTLV::EncapTLV(tlv, ss);

		tlv2.Clear();
		BerTLV::ParseTLV(ss, tlv2);
		out_buf.clear();
		tlv2.EncapTLV(back_inserter(out_buf));
		cout << out_buf.ToHex().c_str() << endl;
	}
	catch (const exception &e)
	{
		cout << "error:" << endl;
		cout << e.what() << endl;
	}
}