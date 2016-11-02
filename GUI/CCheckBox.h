#pragma once

#include "CGUI.h"

class CCheckBox : public CWidget
{
public:
	CCheckBox ( CDialog *pDialog );

	bool IsChecked(void);
	void SetChecked(bool bChecked);

	void Draw ( void );

	bool OnKeyDown ( WPARAM wParam );
	bool OnKeyUp ( WPARAM wParam );

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );

	void SetText(SIMPLEGUI_STRING sText, bool = false);

	void SetWidth(int nWidth);
	void SetHeight(int nHeight);

	void UpdateRects ( void );
	bool ContainsPoint ( CVector pos );

private:
	bool m_bChecked;

	SControlRect m_rBox;
	SControlRect m_rText;
};