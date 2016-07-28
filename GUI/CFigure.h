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
	CD3DTexture *GetTexture ( void );

	void RotateImage ( float fRot);

	void OnClickLeave ( void );
	bool OnClickEvent ( void );

	void OnFocusIn ( void );
	void OnFocusOut ( void );

	void OnMouseEnter ( void );
	void OnMouseLeave ( void );

	bool CanHaveFocus ( void );

	bool HandleMouse ( UINT uMsg, CPos pos, WPARAM wParam, LPARAM lParam );
	bool HandleKeyboard ( UINT uMsg, WPARAM wParam, LPARAM lParam );

	void UpdateRects ( void );
	bool ContainsRect ( CPos pos );

private:
	CD3DTexture *m_pTexture;
	float m_fRotImage;
};

