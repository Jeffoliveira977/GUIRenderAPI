#pragma once

#include "CGUI.h"

class CListBox : public CWidget
{
public:
	CListBox ( CDialog *pDialog );
	~CListBox ( void );

	void Draw ( void );

	void AddItem ( SEntryItem *pEntry );
	void AddItem ( const SIMPLEGUI_CHAR *szText, const SIMPLEGUI_CHAR *szValue = _UI("") );

	void RemoveItem ( SEntryItem *pEntry );
	void RemoveItem ( UINT uIndex );

	void SetMultiSelection ( bool bMultiSelection );
	void SetSortedList ( bool bSort );

	SEntryItem *GetSelectedItem ( void );

	int GetSelectedIndex ( void )
	{
		return m_nSelected;
	}

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

	bool OnMouseMove ( CVector pos );
	bool OnMouseWheel ( int zDelta );

	void UpdateRects ( void );
	bool ContainsPoint ( CVector pos );

private:
	CEntryList *m_pEntryList;

	int m_nOldIndex;
	int m_iIndex;
	int m_nSelected;

	bool m_bMultiSelection;
	SControlRect m_rText;
};