#include "stdafx.h"
#include "Apdu.h"
#include <iostream>
#include <exception>
using namespace elib;
using namespace std;
void ApduTest()
{
    try
    {
        BinData buf(HexData("9000").ToBin());
        ApduRsp rsp(begin(buf), end(buf));
        assert(rsp.Sw() == 0x9000);
        assert(rsp.Sw1() == 0x90);
        assert(rsp.Sw2() == 0x00);

        buf = HexData("11223344556677889000").ToBin();
        ApduRsp rsp1(begin(buf), end(buf));
        assert(rsp.Sw() == 0x9000);
        assert(rsp.Sw1() == 0x90);
        assert(rsp.Sw2() == 0x00);

        const char *pse = "1PAY.SYS.DDF01";
        ApduCmd cmd1(0x00, 0xA4, 0x04, 0x00, BinData(pse, pse + 0x0E));

        cout << cmd1.ToBin().ToHex() << endl;
        ApduCmd cmd2(0x00, 0xA4, 0x04, 0x00, BinData(pse, pse + 0x0E), 0x00);
        cout << cmd2.ToBin().ToHex() << endl;

        ApduCmd cmd3(0x00, 0xA4, 0x04, 0x00);
        cout << cmd3.ToBin().ToHex() << endl;
        ApduCmd cmd4(0x00, 0xA4, 0x04, 0x00, 0x00);
        cout << cmd4.ToBin().ToHex() << endl;
    }
    catch (const exception &e)
    {
        cout << "error:" << endl;
        cout << e.what() << endl;
    }
}