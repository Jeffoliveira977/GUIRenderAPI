#pragma once

#include "CGUI.h"

class CPictureBox : public CWidget
{
public:
	CPictureBox ( CDialog *pDialog );
	~CPictureBox ( void );

	void Draw ( void );

	bool OnKeyDown ( WPARAM wParam );
	bool OnKeyUp ( WPARAM wParam );

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );

	void SetTexture ( const SIMPLEGUI_CHAR *szPath );
	void SetTexture ( LPCVOID pSrc, UINT uSrcSize );

	void RotateImage ( float fRot);
private:
	CD3DTexture *m_pTexture;
	float m_fRotImage;
};

