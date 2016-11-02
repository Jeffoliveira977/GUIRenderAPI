#pragma once

#include "CGUI.h"

class CButton : public CWidget
{
public:
	CButton ( CDialog *pDialog );

	void Draw ( void );

	bool OnKeyDown ( WPARAM wParam );
	bool OnKeyUp ( WPARAM wParam );

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );
};