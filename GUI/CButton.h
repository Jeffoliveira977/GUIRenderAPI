#pragma once

#include "CGUI.h"

class CButton : public CControl
{
public:
	CButton ( CDialog *pDialog );

	void Draw ( void );

	bool OnKeyDown ( WPARAM wParam );
	bool OnKeyUp ( WPARAM wParam );

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );
};