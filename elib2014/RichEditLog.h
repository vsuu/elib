#ifndef RICHEDIT_H_RFT
#define RICHEDIT_H_RFT

#include "Log.h"
#include "afxcmn.h"
#include "strop.h"

class RichEditLog :public elib::LogFileBase
{
public:
	RichEditLog(CRichEditCtrl & ctrl, elib::LogLevel level) :rich_edit_(ctrl), LogFileBase("richedit", level)
	{
		ZeroMemory(&cf_default_, sizeof(CHARFORMAT));
		cf_default_.cbSize = sizeof(CHARFORMAT);
		cf_default_.dwMask |= CFM_COLOR;
		cf_err_ = cf_default_;
		cf_notify_ = cf_default_;
		cf_warn_ = cf_default_;
		cf_err_.crTextColor = RGB(255, 0, 0);
		cf_notify_.crTextColor = RGB(0, 255, 0);
		cf_warn_.crTextColor = RGB(205, 92, 92);
		cf_default_.crTextColor = RGB(0, 0, 0);
	}
	void WriteLog(elib::LogType logtype, const char *str, size_t len)
	{
		if (rich_edit_.GetSafeHwnd() == NULL)return;
		
		switch (logtype)
		{
		case elib::LogType::ERR:
			rich_edit_.SetSelectionCharFormat(cf_err_);
			break;
		case elib::LogType::NOTIFY:
			rich_edit_.SetSelectionCharFormat(cf_notify_);
			break;
		case elib::LogType::WARN:
			rich_edit_.SetSelectionCharFormat(cf_warn_);
			break;
		default:
			rich_edit_.SetSelectionCharFormat(cf_default_);
			break;
		}

		rich_edit_.SetSel(-1, -1);
		rich_edit_.ReplaceSel(elib::str2wstr(std::string(str, len)).c_str());
		rich_edit_.PostMessage(WM_VSCROLL, SB_BOTTOM, 0);
	}
private:
	CRichEditCtrl  &rich_edit_;
	CHARFORMAT cf_err_;
	CHARFORMAT cf_notify_;
	CHARFORMAT cf_warn_;
	CHARFORMAT cf_default_;
};

#endif // !RICHEDIT_H_RFT
