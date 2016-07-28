#include "CFigure.h"

CPictureBox::CPictureBox ( CDialog *pDialog )
{
	SetControl ( pDialog, EControlType::TYPE_IMAGE );
}

CPictureBox::~CPictureBox ( void )
{}

void CPictureBox::Draw ( void )
{
	
}

void CPictureBox::SetTexture ( const SIMPLEGUI_CHAR *szPath )
{
	if ( !m_pDialog )
		return;

	// Check if there is already a texture
	if ( m_pTexture )
		m_pDialog->RemoveTexture ( m_pTexture );

	m_pDialog->LoadTexture ( szPath, &m_pTexture );
}

void CPictureBox::SetTexture ( LPCVOID pSrc, UINT uSrcSize )
{
	if ( !m_pDialog )
		return;

	// Check if there is already a texture
	if ( m_pTexture )
		m_pDialog->RemoveTexture ( m_pTexture );

	m_pDialog->LoadTexture ( pSrc, uSrcSize, &m_pTexture );
}

CD3DTexture *CPictureBox::GetTexture ( void )
{
	return m_pTexture;
}

void CPictureBox::RotateImage ( float fRot )
{}

void CPictureBox::OnClickLeave ( void )
{}

bool CPictureBox::OnClickEvent ( void )
{
	return false;
}

void CPictureBox::OnFocusIn ( void )
{}

void CPictureBox::OnFocusOut ( void )
{}

void CPictureBox::OnMouseEnter ( void )
{}

void CPictureBox::OnMouseLeave ( void )
{}

bool CPictureBox::CanHaveFocus ( void )
{
	return false;
}

bool CPictureBox::HandleMouse ( UINT uMsg, CPos pos, WPARAM wParam, LPARAM lParam )
{
	return false;
}

bool CPictureBox::HandleKeyboard ( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return false;
}

void CPictureBox::UpdateRects ( void )
{}

bool CPictureBox::ContainsRect ( CPos pos )
{
	return false;
}
