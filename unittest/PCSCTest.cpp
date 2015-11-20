#include "stdafx.h"
#include "PCSCReader.h"
#include <iostream>

using namespace std;
using namespace elib;

#pragma comment(lib,"Winscard.lib")

void PCSC_Test()
{
	SCardInterface *pControl = new PCSCReader;

	try
	{
		auto names = pControl->ListReaderName();
		if (names.empty())
		{
			cout << "error : " << "need reader" << endl;
			return;
		}
		for (auto &x : names)
		{
			cout << x << endl;
		}
		pControl->UseReader(names[0]);
		cout << "please insert the card!" << endl;
		pControl->WaitForCardIn();
		pControl->Connect();
		pControl->CardReset();
		pControl->CardRestart();
		cout << pControl->GetATR().ToHex() << endl;
		cout << pControl->TransmitMsg(ApduCmd(0x00, 0xA4, 0x04, 0x00, HexData("315041592E5359532E4444463031").ToBin())).ToBin().ToHex() << endl;
	}
	catch (const ScardNoCard &e)
	{
		cout << "NoCard :*** " << e.what() << endl;
	}
	catch (const ScardNoReader &e)
	{
		cout << "NoReader :*** " << e.what() << endl;
	}
	catch (const ScardError &e)
	{
		cout << e.what() << endl;
	}

	delete pControl;
}