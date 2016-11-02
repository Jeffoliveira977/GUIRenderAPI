#pragma once
#include "CGUI.h"

class CScrollablePane
{
public:
	CScrollablePane ( CDialog *pDialog );
	~CScrollablePane ( void );

	void OnMouseWheel ( int zDelta );
	void OnDraw ( void );

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );
	bool OnMouseMove ( CVector pos );

	void OnClickLeave ( void );

	bool OnClickEvent ( void );

	void OnFocusIn ( void );
	void OnFocusOut ( void );

	void OnMouseEnter ( void );
	void OnMouseLeave ( void );

	bool CanHaveFocus ( void );

	bool IsHorScrollbarShowing ( void );
	void ShowScrollHor ( bool bShow );

	bool IsVerScrollbarShowing ( void );
	void ShowScrollVer ( bool bShow );

	bool IsHorScrollbarNeeded ( void );
	bool IsVerScrollbarNeeded ( void );

	void SetPageSize ( int nSizeX, int nSizeY );
	void SetTrackRange (int nScrollHorValue, int nScrollVerValue );

	void UpdateScrollbars ( SControlRect rRect );

	void SetControl ( CWidget *pControl );
	CWidget *GetControl ( void );

	void RemoveControl ( void );
	void SetFocussedControl ( void );

	bool ContainsPoint ( CVector pos );

	CScrollBarHorizontal *GetHorScrollbar ( void );
	CScrollBarVertical *GetVerScrollbar ( void );
private:
	bool m_bScrollHorShow;
	bool m_bScrollVerShow;

	CVector pos;
	SIZE m_pageSize;

	SControlRect m_rRect;
	CDialog *m_pDialog;

	CScrollBarHorizontal *m_pScrollbarHor;
	CScrollBarVertical *m_pScrollbarVer;

	CWidget *m_pControl;
};

