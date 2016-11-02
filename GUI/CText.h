#pragma once

#include "CGUI.h"

class CLabel : public CWidget
{
public:
	CLabel ( CDialog *pDialog );
	~CLabel ( void );

	void Draw ( void );


	bool OnKeyDown ( WPARAM wParam );
	bool OnKeyUp ( WPARAM wParam );

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );

	void SetText(SIMPLEGUI_STRING str, bool = false);

	void SetWordwrap(bool bWordwrap);
	bool IsWordwrap(void);

	void SetHeight ( int nHeight );
	void SetWidth ( int nWidth );

	bool ContainsPoint(Pos pos);
	void UpdateRects(void);
private:

	bool m_bWordwrap;
	SIZE m_oldTextSize;
	SIMPLEGUI_STRING m_sFormatted;
};