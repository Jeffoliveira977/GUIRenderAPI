#pragma once

#include "CGUI.h"

class CWindow : public CWidget
{
public:
	CWindow ( CDialog *pDialog );
	~CWindow ( void );

	void AddControl ( CWidget *pControl );
	void RemoveControl ( CWidget *pControl );
	void RemoveAllControls ( void );

	void Draw ( void );
	CScrollablePane *GetScrollbar ( void ) { return m_pScrollbar; }
	bool ControlMessages ( sControlEvents e );

	bool OnMouseButtonDown ( sMouseEvents e);
	bool OnMouseButtonUp ( sMouseEvents e );
	bool OnMouseMove ( CVector pos );
	bool OnMouseWheel ( int zDelta );

	bool OnKeyDown ( WPARAM wParam );

	void UpdateRects ( void );
	bool ContainsPoint ( CVector pos );

	void OnClickLeave ( void );
	bool OnClickEvent ( void );

	void OnFocusIn ( void );
	void OnFocusOut ( void );

	void OnMouseEnter ( void );
	void OnMouseLeave ( void );

	bool CanHaveFocus ( void );

	void ShowScrollbars ( bool bShow );

	void SetCloseButton ( bool bEnabled );

	void SetFocussedWidget ( CWidget *pControl );
	CWidget *GetFocussedControl ( void );

	void ClearControlFocus ( void );

	CWidget *GetNextControl ( CWidget *pControl );
	CWidget *GetPrevControl ( CWidget *pControl );

	CWidget *GetControlByText ( const SIMPLEGUI_CHAR *pszText );

	void BringControlToTop ( CWidget *pControl );
	int GetTitleBarHeight ( void );

	CWidget *GetControlAtArea ( Pos pos );
	CWidget *GetControlClicked ( void );

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

	void ScrollPage ( int nDelta );

	CWidget *GetTabPanelFocussedControl ( void );

	void ResizeWidget ( CWidget *pWidget, bool bCheckInList =false);

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
	SIZE m_minControlSize;
	CScrollablePane *m_pScrollbar;
	
	enum E_WINDOW_AREA
	{
		OutArea,
		TopLeft, BottomLeft, TopRight, BottomRight,
		Top, Left, Right, Bottom

	};

	void SetCursorForPoint ( CVector pos );

	E_WINDOW_AREA GetSizingBorderAtArea ( CVector pos );
	SControlRect *GetWindowRect ( E_WINDOW_AREA eArea );

	SControlRect m_rButton;
	SControlRect m_rTitle;

	SIZE sizeOld;

	SControlRect m_rWindowTop;
	SControlRect m_rWindowLeft;
	SControlRect m_rWindowRight;
	SControlRect m_rWindowBottom;

	SControlRect m_rWindowTopLeft;
	SControlRect m_rWindowBottomLeft;
	SControlRect m_rWindowTopRight;
	SControlRect m_rWindowBottomRight;

	 CWidget *m_pControlMouseOver;
	 CWidget *m_pFocussedControl;

	SControlRect rFrame;
	int m_iTitleBarSize;

	int m_nDragX;
	int m_nDragY;

	CVector m_posDif;

	std::vector<CWidget*> m_vControls;

	E_WINDOW_AREA m_eWindowArea;
};