#include "CGUI.h"

CButton::CButton ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_BUTTON );
}

//--------------------------------------------------------------------------------------
void CButton::Draw ( void )
{
	if ( !m_bVisible )
		return;
	
	CWidget::Draw ();

	SIMPLEGUI_STRING str = GetText ();
	m_pFont->CutString ( m_rBoundingBox.m_size.cx , str );

	m_pDialog->DrawBox ( m_rBoundingBox, m_sControlColor.d3dColorBox [ m_eState ], m_sControlColor.d3dColorOutline );
	m_pDialog->DrawFont ( SControlRect ( m_rBoundingBox.m_pos.m_nX + m_rBoundingBox.m_size.cx / 2, m_rBoundingBox.m_pos.m_nY + m_rBoundingBox.m_size.cy / 2 ),
						  m_sControlColor.d3dColorFont, str.c_str (), D3DFONT_CENTERED_X | D3DFONT_CENTERED_Y, m_pFont );
}

bool CButton::OnKeyDown ( WPARAM wParam )
{
	if ( !CanHaveFocus () )
		return false;

	if ( wParam == VK_SPACE )
	{
		m_bPressed = true;
		return  true;
	}

	return false;
}

bool CButton::OnKeyUp ( WPARAM wParam )
{
	if ( wParam == VK_SPACE )
	{
		if ( m_bPressed )
		{
			m_bPressed = false;
			SendEvent ( EVENT_CONTROL_CLICKED, true );
			return true;
		}
	}
	
	return false;
}

bool CButton::OnMouseButtonDown ( sMouseEvents e )
{
	if ( !CanHaveFocus () )
		return false;

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		if ( m_rBoundingBox.ContainsPoint ( e.pos ) )
		{
			// Pressed while inside the control
			m_bPressed = true;

			_SetFocus ();
			return true;
		}
	}

	return false;
}

bool CButton::OnMouseButtonUp ( sMouseEvents e )
{
	if ( m_bPressed )
	{
		m_bPressed = false;

		_ClearFocus ();

		// Button click
		if ( m_rBoundingBox.ContainsPoint ( e.pos ) )
			SendEvent ( EVENT_CONTROL_CLICKED, true );

		return true;
	}

	return false;
}