#pragma once

#include "CGUI.h"


class CLogBox : public CWidget
{
public:
	enum E_LOGTYPE
	{
		LOG_NONE,
		LOG_DEBUG,
		LOG_INFO,
		LOG_ERROR,
		LOG_WARNING
	};

	CLogBox ( CDialog *pDialog );
	~CLogBox ( void );

	void Draw ( void );

	void AddText ( E_LOGTYPE eType, const SIMPLEGUI_CHAR *szText, ... );
	void RemoveText ( UINT uIndex );

	const SIMPLEGUI_CHAR *GetText ( UINT uIndex );

	void ResetList ( void );
	void SetSortedList ( bool bSort );

	void OnClickLeave ( void );
	bool OnClickEvent ( void );

	void OnFocusIn ( void );
	void OnFocusOut ( void );

	void OnMouseEnter ( void );
	void OnMouseLeave ( void );

	bool CanHaveFocus ( void );

	bool OnKeyDown ( WPARAM wParam );

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );

	bool OnMouseWheel ( int zDelta );
	bool OnMouseMove ( CVector pos );

	void UpdateRects ( void );
	bool ContainsPoint ( CVector pos );

	void GenerateLogFile ( bool bSet );

private:
	const SIMPLEGUI_CHAR *GetModeType ( E_LOGTYPE eType );

	bool m_bGenerateLog;
	CLogFile *m_pLogFile;
	CEntryList *m_pEntryList;
};