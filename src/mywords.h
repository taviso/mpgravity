#pragma once

#include "superstl.h"

class TMyWords
{
public:
	TMyWords();
	~TMyWords();

	void Save();

	bool Lookup (const CString & word);

	void AddWord (const CString & word);
	void RemoveWord (const CString & word);

	void FillListbox (CListBox & lbx);

private:
	set<CString> m_set;
	bool m_fNeedSave;
};


