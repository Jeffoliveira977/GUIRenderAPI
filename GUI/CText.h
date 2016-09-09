#pragma once

#include "CGUI.h"

class CLabel : public CControl
{
public:
	CLabel ( CDialog *pDialog );
	~CLabel ( void );

	void SetAlign ( DWORD dwAlign );
	void Draw ( void );

	bool OnKeyDown ( WPARAM wParam );
	bool OnKeyUp ( WPARAM wParam );

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );

	void UpdateRects ( void );

	void SetHeight ( int nHeight );
	void SetWidth ( int nWidth );

	void SetSize ( SIZE size );
	void SetSize ( int nWidth, int nHeight );

private:
	DWORD m_dwAlign;
	int m_nSizeX;
	int m_nSizeY;
};