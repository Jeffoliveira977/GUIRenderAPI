#pragma once

#include "CGUI.h"

class CTabPanel : public CControl
{
public:
	CTabPanel ( CDialog *pDialog );
	~CTabPanel ( void );

	void AddTab ( const SIMPLEGUI_CHAR *szTabName, int nWidth );

	void RemoveTab ( const SIMPLEGUI_CHAR *szTabName );
	void RemoveTab ( UINT nTabID );
	void RemoveAllTabs ( void );

	void AddControl ( UINT nTabID, CControl *pControl );
	void RemoveControl ( UINT nTabID, CControl *pControl );
	
	void RemoveAllControlsFromTab ( UINT nTabID );
	void RemoveAllControls ( void );

	bool IsControlInList ( CControl *pControl );

	void SetFocussedControl ( CControl *pControl );
	void BringControlToTop ( UINT nTabID, CControl *pControl );

	CControl *GetFocussedControl ( void );

	int GetAllColumnsWidth ( void );
	UINT GetNumOfTabsVisible ( void );

	void ClearControlFocus ( void );

	void MoveControl ( CControl *pControl, UINT nTabPosition );

	void SetSelectedTab ( UINT nTabID );
	int GetSelectedTab ( void );

	int GetTabIdAtArea ( CPos pos );

	CControl *GetControlAtArea ( UINT nTabID, CPos pos );

	int GetTabSizeY ( void );
	SIZE GetSize ( void );

	void Draw ( void );

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );

	bool OnMouseMove ( CPos pos );
	bool OnMouseWheel ( int zDelta );

	bool ControlMessages ( sControlEvents e );

	void UpdateRects ( void );
	bool ContainsRect ( CPos pos );

	void OnClickLeave ( void );
	bool OnClickEvent ( void );

	void OnFocusIn ( void );
	void OnFocusOut ( void );

	void OnMouseEnter ( void );
	void OnMouseLeave ( void );

private:

	void UpdateScrollbars ( void );

	struct STabList
	{
		CControl *pFocussedControl;
		CControl *pMouseOverControl;

		std::vector<CControl*> vControls;
		SIMPLEGUI_STRING sTabName;
		int nWidth;
	};

	struct STabButton
	{
		SControlRect m_rButton;

		bool bClickedButton;
		bool bOverButton;
		bool bVisible;

		bool InArea ( CPos pos )
		{
			return ( bVisible && m_rButton.InControlArea ( pos ) );
		}

		bool OnMouseOver ( CPos pos )
		{
			return ( bOverButton && InArea ( pos ) );
		}

		bool OnClickEvent ( CPos pos )
		{
			return ( bClickedButton && InArea ( pos ) );
		}
	};

	STabButton m_sLeftButton;
	STabButton m_sRightButton;

 	int m_nOldAreaX;

	std::map<int, int> m_nTrackX;
	std::map<int, int> m_nTrackY;

	UINT m_nNextTab;
	UINT m_nSelectedTab;

	int m_nOverTabId;

	std::vector<STabList> m_TabList;
	SIZE m_maxControlSize;

	CD3DFont *m_pTitleFont;

	SControlRect m_rTabArea;
	SControlRect m_rPanelArea;

	CScrollablePane *m_pScrollbar;
};

