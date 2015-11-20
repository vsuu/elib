#include "PCSCReader.h"
#include  <winscard.h>
#include  <map>
#include <vector>
#include <system_error>

using namespace std;
__LIB_NAME_SPACE_BEGIN__

namespace {
	std::map<LONG, const char *> map_err{
		{ ERROR_BROKEN_PIPE, "The client attempted a smart card operation in a remote session, such as a client session running on a terminal server, and the operating system in use does not support smart card redirection." },
		{ SCARD_E_BAD_SEEK, "An error occurred in setting the smart card file object pointer." },
		{ SCARD_E_CANCELLED, "The action was canceled by an SCardCancel request." },
		{ SCARD_E_CANT_DISPOSE, "The system could not dispose of the media in the requested manner." },
		{ SCARD_E_CARD_UNSUPPORTED, "The smart card does not meet minimal requirements for support." },
		{ SCARD_E_CERTIFICATE_UNAVAILABLE, "The requested certificate could not be obtained." },
		{ SCARD_E_COMM_DATA_LOST, "A communications error with the smart card has been detected." },
		{ SCARD_E_DIR_NOT_FOUND, "The specified directory does not exist in the smart card." },
		{ SCARD_E_DUPLICATE_READER, "The reader driver did not produce a unique reader name." },
		{ SCARD_E_FILE_NOT_FOUND, "The specified file does not exist in the smart card." },
		{ SCARD_E_ICC_CREATEORDER, "The requested order of object creation is not supported." },
		{ SCARD_E_ICC_INSTALLATION, "No primary provider can be found for the smart card." },
		{ SCARD_E_INSUFFICIENT_BUFFER, "The data buffer for returned data is too small for the returned data." },
		{ SCARD_E_INVALID_ATR, "An ATR string obtained from the registry is not a valid ATR string." },
		{ SCARD_E_INVALID_CHV, "The supplied PIN is incorrect." },
		{ SCARD_E_INVALID_HANDLE, "The supplied handle was not valid." },
		{ SCARD_E_INVALID_PARAMETER, "One or more of the supplied parameters could not be properly interpreted." },
		{ SCARD_E_INVALID_TARGET, "Registry startup information is missing or not valid." },
		{ SCARD_E_INVALID_VALUE, "One or more of the supplied parameter values could not be properly interpreted." },
		{ SCARD_E_NO_ACCESS, "Access is denied to the file." },
		{ SCARD_E_NO_DIR, "The supplied path does not represent a smart card directory." },
		{ SCARD_E_NO_FILE, "The supplied path does not represent a smart card file." },
		{ SCARD_E_NO_KEY_CONTAINER, "The requested key container does not exist on the smart card." },
		{ SCARD_E_NO_MEMORY, "Not enough memory available to complete this command." },
		{ SCARD_E_NO_PIN_CACHE, "The smart card PIN cannot be cached.Windows Server 2008, Windows Vista, Windows Server 2003, and Windows XP:  This error code is not available." },
		{ SCARD_E_NO_READERS_AVAILABLE, "No smart card reader is available." },
		{ SCARD_E_NO_SERVICE, "The smart card resource manager is not running." },
		{ SCARD_E_NO_SMARTCARD, "The operation requires a smart card, but no smart card is currently in the device." },
		{ SCARD_E_NO_SUCH_CERTIFICATE, "The requested certificate does not exist." },
		{ SCARD_E_NOT_READY, "The reader or card is not ready to accept commands." },
		{ SCARD_E_NOT_TRANSACTED, "An attempt was made to end a nonexistent transaction." },
		{ SCARD_E_PCI_TOO_SMALL, "The PCI receive buffer was too small." },
		{ SCARD_E_PIN_CACHE_EXPIRED, "The smart card PIN cache has expired.Windows Server 2008, Windows Vista, Windows Server 2003, and Windows XP:  This error code is not available." },
		{ SCARD_E_PROTO_MISMATCH, "The requested protocols are incompatible with the protocol currently in use with the card." },
		{ SCARD_E_READ_ONLY_CARD, "The smart card is read-only and cannot be written to.Windows Server 2008, Windows Vista, Windows Server 2003, and Windows XP:  This error code is not available." },
		{ SCARD_E_READER_UNAVAILABLE, "The specified reader is not currently available for use." },
		{ SCARD_E_READER_UNSUPPORTED, "The reader driver does not meet minimal requirements for support." },
		{ SCARD_E_SERVER_TOO_BUSY, "The smart card resource manager is too busy to complete this operation." },
		{ SCARD_E_SERVICE_STOPPED, "The smart card resource manager has shut down." },
		{ SCARD_E_SHARING_VIOLATION, "The smart card cannot be accessed because of other outstanding connections." },
		{ SCARD_E_SYSTEM_CANCELLED, "The action was canceled by the system, presumably to log off or shut down." },
		{ SCARD_E_TIMEOUT, "The user-specified time-out value has expired." },
		{ SCARD_E_UNEXPECTED, "An unexpected card error has occurred." },
		{ SCARD_E_UNKNOWN_CARD, "The specified smart card name is not recognized." },
		{ SCARD_E_UNKNOWN_READER, "The specified reader name is not recognized." },
		{ SCARD_E_UNKNOWN_RES_MNG, "An unrecognized error code was returned." },
		{ SCARD_E_UNSUPPORTED_FEATURE, "This smart card does not support the requested feature." },
		{ SCARD_E_WRITE_TOO_MANY, "An attempt was made to write more data than would fit in the target object." },
		{ SCARD_F_COMM_ERROR, "An internal communications error has been detected." },
		{ SCARD_F_INTERNAL_ERROR, "An internal consistency check failed." },
		{ SCARD_F_UNKNOWN_ERROR, "An internal error has been detected, but the source is unknown." },
		{ SCARD_F_WAITED_TOO_LONG, "An internal consistency timer has expired." },
		{ SCARD_P_SHUTDOWN, "The operation has been aborted to allow the server application to exit." },
		{ SCARD_S_SUCCESS, "No error was encountered." },
		{ SCARD_W_CANCELLED_BY_USER, "The action was canceled by the user." },
		{ SCARD_W_CACHE_ITEM_NOT_FOUND, "The requested item could not be found in the cache." },
		{ SCARD_W_CACHE_ITEM_STALE, "The requested cache item is too old and was deleted from the cache." },
		{ SCARD_W_CACHE_ITEM_TOO_BIG, "The new cache item exceeds the maximum per-item size defined for the cache." },
		{ SCARD_W_CARD_NOT_AUTHENTICATED, "No PIN was presented to the smart card." },
		{ SCARD_W_CHV_BLOCKED, "The card cannot be accessed because the maximum number of PIN entry attempts has been reached." },
		{ SCARD_W_EOF, "The end of the smart card file has been reached." },
		{ SCARD_W_REMOVED_CARD, "The smart card has been removed, so further communication is not possible." },
		{ SCARD_W_RESET_CARD, "The smart card was reset." },
		{ SCARD_W_SECURITY_VIOLATION, "Access was denied because of a security violation." },
		{ SCARD_W_UNPOWERED_CARD, "Power has been removed from the smart card, so that further communication is not possible." },
		{ SCARD_W_UNRESPONSIVE_CARD, "The smart card is not responding to a reset." },
		{ SCARD_W_UNSUPPORTED_CARD, "The reader cannot communicate with the card, due to ATR string configuration conflicts." },
		{ SCARD_W_WRONG_CHV, "The card cannot be accessed because the wrong PIN was presented" },
		{ ERROR_NOT_SUPPORTED, "Attribute value not supported." }
	};

	void CHECK(const char * where, LONG ret)
	{
		if (ret != SCARD_S_SUCCESS)
		{
			std::string err(where);
			auto it = map_err.find(ret);
			if (it != map_err.end())
			{
				err += it->second;
			}
			else
			{
				err += system_error(error_code((int)ret, system_category())).what();
			}
			if (ret == SCARD_E_NO_SMARTCARD)
			{
				throw ScardNoCard(err);
			}
			if (ret == SCARD_E_NO_READERS_AVAILABLE)
			{
				throw ScardNoReader(err);
			}
			throw ScardError(ret, err);
		}
	}
}

bool ScardError::IsCancel()const
{
	return code_ == SCARD_E_CANCELLED;
}
class PCSCReader::impl
{
public:
	SCARDCONTEXT context_;
	SCARDHANDLE card_handle_;
	DWORD active_protocol_;
};

PCSCReader::PCSCReader() :impl_(new PCSCReader::impl)
{
	CHECK(ERR_WHERE, SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &(impl_->context_)));
	impl_->active_protocol_ = 0;
	impl_->card_handle_ = 0;
}
PCSCReader::~PCSCReader()
{
	SCardCancel(impl_->context_);
	SCardReleaseContext(impl_->context_);
	delete impl_;
}

void PCSCReader::Cancel()
{
	SCardCancel(impl_->context_);
}
std::vector<std::string> PCSCReader::ListReaderName(std::function<bool(const char *)> filter)
{
	DWORD len = SCARD_AUTOALLOCATE;
	LPTSTR readers = NULL;
	try
	{
		CHECK(ERR_WHERE, SCardListReaders(impl_->context_, SCARD_ALL_READERS, (LPTSTR)&readers, &len));
	}
	catch (const ScardNoReader &)
	{
		return std::vector<std::string>();
	}

	//parse
#ifndef UNICODE
	std::string tmp(readers, len);
#else
	std::string tmp(wstr2str(wstring(readers, len)));
#endif
	SCardFreeMemory(impl_->context_, readers);

	std::vector<std::string> name_list;
	size_t pos = 0;
	size_t pos1 = 0;
	while (true)
	{
		pos1 = tmp.find('\0', pos);
		if (pos == pos1)break;
		string name(tmp.substr(pos, pos1 - pos));
		if (!filter || filter(name.c_str()))
		{
			name_list.push_back(std::move(name));
		}
		if (pos1 == string::npos)break;
		pos = pos1 + 1;
	}
	return name_list;
}
void PCSCReader::UseReader(const std::string &name)
{
	if (name != GetReaderNameInUse())
	{
		if (impl_->card_handle_ != 0)
		{
			DisConnect();
		}
		SCardInterface::UseReader(name);
		//Connect();
	}
}

void PCSCReader::Connect()
{
#ifndef UNICODE
	const string &reader_name = GetReaderNameInUse();
#else
	wstring reader_name(str2wstr(GetReaderNameInUse()));
#endif
	CHECK(ERR_WHERE, SCardConnect(impl_->context_, reader_name.c_str(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &(impl_->card_handle_), &(impl_->active_protocol_)));
}
void PCSCReader::DisConnect()
{
	if (impl_->card_handle_)
	{
		CHECK(ERR_WHERE, SCardDisconnect(impl_->card_handle_, SCARD_EJECT_CARD));
		impl_->card_handle_ = 0;
		impl_->active_protocol_ = 0;
	}
}
void PCSCReader::WaitForCardIn()
{
	SCARD_READERSTATE tmp{ 0 };
	tmp.dwCurrentState = SCARD_STATE_UNAWARE;
	//tmp.dwEventState = SCARD_STATE_PRESENT;
#ifndef UNICODE
	string &readername = GetReaderNameInUse();
#else
	wstring readername = str2wstr(GetReaderNameInUse());
#endif
	tmp.szReader = readername.c_str();
	CHECK(ERR_WHERE, SCardGetStatusChange(impl_->context_, INFINITE, &tmp, 1));
	if (tmp.dwEventState & SCARD_STATE_PRESENT)
	{
		return;
	}
	tmp.dwCurrentState = tmp.dwEventState;
	tmp.dwEventState = SCARD_STATE_PRESENT;
	CHECK(ERR_WHERE, SCardGetStatusChange(impl_->context_, INFINITE, &tmp, 1));
}
void PCSCReader::WaitForCardOut()
{
	SCARD_READERSTATE tmp{ 0 };
	tmp.dwCurrentState = SCARD_STATE_UNAWARE;
	//tmp.dwEventState = SCARD_STATE_PRESENT;
#ifndef UNICODE
	string &readername = GetReaderNameInUse();
#else
	wstring readername = str2wstr(GetReaderNameInUse());
#endif
	tmp.szReader = readername.c_str();
	CHECK(ERR_WHERE, SCardGetStatusChange(impl_->context_, INFINITE, &tmp, 1));
	if (tmp.dwEventState & SCARD_STATE_EMPTY)
	{
		return;
	}
	tmp.dwCurrentState = tmp.dwEventState;
	tmp.dwEventState = SCARD_STATE_EMPTY;
	CHECK(ERR_WHERE, SCardGetStatusChange(impl_->context_, INFINITE, &tmp, 1));
}
void PCSCReader::CardReset()
{
	CHECK(ERR_WHERE, SCardReconnect(impl_->card_handle_, SCARD_SHARE_SHARED, impl_->active_protocol_, SCARD_RESET_CARD, &(impl_->active_protocol_)));
}
void PCSCReader::CardRestart()
{
	CHECK(ERR_WHERE, SCardReconnect(impl_->card_handle_, SCARD_SHARE_SHARED, impl_->active_protocol_, SCARD_UNPOWER_CARD, &(impl_->active_protocol_)));
}
BinData PCSCReader::GetATR()
{
	LPBYTE tmp = NULL;
	DWORD len = SCARD_AUTOALLOCATE;
	CHECK(ERR_WHERE, SCardGetAttrib(impl_->card_handle_, SCARD_ATTR_ATR_STRING, (LPBYTE)&tmp, &len));
	BinData ret(tmp, tmp + len);
	SCardFreeMemory(impl_->context_, tmp);
	return ret;
}
ApduRsp PCSCReader::TransmitMsg(const ApduCmd&cmd)
{
	BinData send = cmd.ToBin();
	unsigned char buff[1024];
	DWORD len = 1024;
	CHECK(ERR_WHERE, SCardBeginTransaction(impl_->card_handle_));
	try
	{
		CHECK(ERR_WHERE, SCardTransmit(impl_->card_handle_, SCARD_PROTOCOL_T1 == impl_->active_protocol_ ? SCARD_PCI_T1 : SCARD_PCI_T0, send.data(), send.size(), NULL, buff, &len));
	}
	catch (...)
	{
		SCardEndTransaction(impl_->card_handle_, SCARD_LEAVE_CARD);
		throw;
	}

	CHECK(ERR_WHERE, SCardEndTransaction(impl_->card_handle_, SCARD_LEAVE_CARD));
	ApduRsp ret(buff, buff + len);
	if (ret.Sw1() == 0x6C)
	{
		ApduCmd new_cmd(cmd);
		new_cmd.Le() = ret.Sw2();
		return PCSCReader::TransmitMsg(new_cmd);
	}
	else if (ret.Sw1() == 0x61)
	{
		ApduRsp ret1 = PCSCReader::TransmitMsg(ApduCmd(0x00, 0xC0, 0x00, 0x00, ret.Sw2()));
		if (ret1.Sw() != 0x9000)
		{
			return ret1;
		}
		else
		{
			BinData tmp = ret.Data() + ret1.ToBin();
			return ApduRsp(tmp.begin(), tmp.end());
		}
	}
	else
	{
		return ret;
	}
}
bool PCSCReader::IsCardIn()
{
	SCARD_READERSTATE tmp{ 0 };
	tmp.dwCurrentState = SCARD_STATE_UNAWARE;
	//tmp.dwEventState = SCARD_STATE_PRESENT;
#ifndef UNICODE
	string &readername = GetReaderNameInUse();
#else
	wstring readername = str2wstr(GetReaderNameInUse());
#endif
	tmp.szReader = readername.c_str();
	CHECK(ERR_WHERE, SCardGetStatusChange(impl_->context_, INFINITE, &tmp, 1));
	return ((SCARD_STATE_PRESENT & tmp.dwEventState) != 0);
}

bool PCSCReader::IsConnected()
{
	return impl_->card_handle_ != 0;
}
__LIB_NAME_SPACE_END__