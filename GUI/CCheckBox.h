#pragma once

#include "CGUI.h"

class CCheckBox : public CControl
{
public:
	CCheckBox ( CDialog *pDialog );

	bool GetChecked ( void )
	{
		return m_bChecked;
	}

	void SetChecked ( bool bChecked )
	{
		m_bChecked = bChecked;
		SendEvent ( EVENT_CONTROL_SELECT, m_bChecked );
	}

	void Draw ( void );

	bool OnKeyDown ( WPARAM wParam );
	bool OnKeyUp ( WPARAM wParam );

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );

	void UpdateRects ( void );
	bool ContainsRect ( CPos pos );

private:
	bool m_bChecked;
	SControlRect m_rText;
};