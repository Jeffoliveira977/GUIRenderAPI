#pragma once

#include "CGUI.h"

class CWindow : public CControl
{
public:
	CWindow ( CDialog *pDialog );
	~CWindow ( void );

	void AddControl ( CControl *pControl );
	void RemoveControl ( CControl *pControl );
	void RemoveAllControls ( void );

	void Draw ( void );

	bool ControlMessages ( sControlEvents e );

	bool OnMouseButtonDown ( sMouseEvents e);
	bool OnMouseButtonUp ( sMouseEvents e );
	bool OnMouseMove ( CPos pos );
	bool OnMouseWheel ( int zDelta );

	bool OnKeyDown ( WPARAM wParam );

	void UpdateRects ( void );
	bool ContainsRect ( CPos pos );

	void OnClickLeave ( void );
	bool OnClickEvent ( void );

	void OnFocusIn ( void );
	void OnFocusOut ( void );

	void OnMouseEnter ( void );
	void OnMouseLeave ( void );

	bool CanHaveFocus ( void );

	void OnMouseMove ( CControl *pControl, UINT uMsg );
	void ShowScrollbars ( bool bShow );

	void SetCloseButton ( bool bEnabled );

	void SetFocussedControl ( CControl *pControl );
	CControl *GetFocussedControl ( void );

	void ClearControlFocus ( void );

	CControl *GetNextControl ( CControl *pControl );
	CControl *GetPrevControl ( CControl *pControl );

	CControl *GetControlByText ( const SIMPLEGUI_CHAR *pszText );

	void BringControlToTop ( CControl *pControl );
	int GetTitleBarHeight ( void );

	CControl *GetControlAtArea ( CPos pos );
	CControl *GetControlClicked ( void );

	bool IsSizing ( void );

	void SetAlwaysOnTop ( bool bEnable );
	bool GetAlwaysOnTop ( void );

	void SetMovable ( bool bEnabled );
	bool GetMovable ( void );

	void SetSizable ( bool bEnabled );
	bool GetSizable ( void );

	void SetMaximized ( bool bMinimize );
	bool GetMaximized ( void );

	void SetSize ( SIZE size );
	void SetSize ( int iWidth, int iHeight );

	SIZE GetRealSize ( void );

	void UpdateScrollbars ( bool bUpdateHor, bool bUpdateVer );
	void ScrollPage ( int nDelta );

private:
	bool m_bDragging;
	bool m_bCloseButtonEnabled;
	bool m_bMovable;
	bool m_bSizable;
	bool m_bOnTop;
	bool m_bMaximized;
	bool m_bShowScrollbar;
	bool m_bControlClicked;

	SIZE m_realSize;
	SIZE m_maxControlSize;

	CScrollablePane *m_pScrollbar;
	
	enum E_WINDOW_AREA
	{
		OutArea,
		TopLeft, BottomLeft, TopRight, BottomRight,
		Top, Left, Right, Bottom

	};

	void SetCursorForPoint ( CPos pos );

	E_WINDOW_AREA GetSizingBorderAtArea ( CPos pos );
	SControlRect *GetWindowRect ( E_WINDOW_AREA eArea );

	SControlRect m_rButton;
	SControlRect m_rTitle;

	SControlRect m_rWindowTop;
	SControlRect m_rWindowLeft;
	SControlRect m_rWindowRight;
	SControlRect m_rWindowBottom;

	SControlRect m_rWindowTopLeft;
	SControlRect m_rWindowBottomLeft;
	SControlRect m_rWindowTopRight;
	SControlRect m_rWindowBottomRight;

	 CControl *m_pControlMouseOver;
	 CControl *m_pFocussedControl;
	SControlRect rFrame;
	int m_iTitleBarSize;
	int m_nDragX;
	int m_nDragY;

	CPos m_posDif;

	std::vector<CControl*> m_vControls;

	E_WINDOW_AREA m_eWindowArea;
};