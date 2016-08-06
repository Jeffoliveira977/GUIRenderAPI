#pragma once

#include "CGUI.h"

class CPictureBox : public CControl
{
public:
	CPictureBox ( CDialog *pDialog );
	~CPictureBox ( void );

	void Draw ( void );

	void SetTexture ( const SIMPLEGUI_CHAR *szPath );
	void SetTexture ( LPCVOID pSrc, UINT uSrcSize );

	void RotateImage ( float fRot);
private:
	CD3DTexture *m_pTexture;
	float m_fRotImage;
};

