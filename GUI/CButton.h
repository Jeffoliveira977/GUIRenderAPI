#pragma once

#include "CGUI.h"

class CButton : public CControl
{
public:
	CButton ( CDialog *pDialog );

	void Draw ( void );

	bool OnKeyDown ( WPARAM wParam );
	bool OnKeyUp ( WPARAM wParam );

	bool OnMouseButtonDown ( CPos pos );
	bool OnMouseButtonUp ( CPos pos );

	bool HandleMouse ( UINT uMsg, CPos pos, WPARAM wParam, LPARAM lParam );
	bool HandleKeyboard ( UINT uMsg, WPARAM wParam, LPARAM lParam );
};