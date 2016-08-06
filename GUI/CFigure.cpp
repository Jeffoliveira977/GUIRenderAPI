#include "CFigure.h"

CPictureBox::CPictureBox ( CDialog *pDialog )
{
	SetControl ( pDialog, EControlType::TYPE_IMAGE );
}

CPictureBox::~CPictureBox ( void )
{
	m_pDialog->RemoveTexture ( m_pTexture );
}

void CPictureBox::Draw ( void )
{
	if ( !m_bVisible )
		return;

	CControl::Draw ();

	m_pTexture->Draw ( m_rBoundingBox.pos.GetX (), m_rBoundingBox.pos.GetY (), m_rBoundingBox.size.cx, m_rBoundingBox.size.cy, m_fRotImage, m_sControlColor.d3dColorBox [ m_eState ] );
}

void CPictureBox::SetTexture ( const SIMPLEGUI_CHAR *szPath )
{
	if ( !m_pDialog || m_pTexture )
		return;

	m_pDialog->LoadTexture ( szPath, &m_pTexture );
}

void CPictureBox::SetTexture ( LPCVOID pSrc, UINT uSrcSize )
{
	if ( !m_pDialog || m_pTexture )
		return;

	m_pDialog->LoadTexture ( pSrc, uSrcSize, &m_pTexture );
}

void CPictureBox::RotateImage ( float fRot )
{
	m_fRotImage = fRot;
}
