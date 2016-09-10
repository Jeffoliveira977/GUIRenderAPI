#include "CGUI.h"

std::map<CControl*, std::map<UINT, CControl*>> CRadioButton::mADD;

CRadioButton::CRadioButton ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_RADIOBUTTON );

	m_uGroup = 0;

}

CRadioButton::~CRadioButton ( void )
{

}

void CRadioButton::Draw ( void )
{
	if ( !m_bVisible )
		return;

	CControl::Draw ();

	CPos pos = m_rBoundingBox.pos;
	SIZE size = m_rText.size;
	size.cx = size.cy;

	m_pDialog->DrawCircle ( SControlRect ( pos, size ), m_sControlColor.d3dColorBox [ m_eState ], m_sControlColor.d3dColorOutline, m_bAntAlias );

	SIMPLEGUI_STRING str ( GetText () );
	m_pFont->CutString ( m_rText.size.cx, str );
	m_pDialog->DrawFont ( SControlRect ( m_rText.pos.GetX (), m_rText.pos.GetY () + ( m_rBoundingBox.size.cy / 2 ) ),
						  m_sControlColor.d3dColorFont, str.c_str (), D3DFONT_CENTERED_Y, m_pFont );

	if ( mADD [ m_pParent ] [ m_uGroup ] == this )
	{
		size.cx = size.cy -= 4;
		m_pDialog->DrawCircle ( SControlRect ( pos + 2, size ), m_sControlColor.d3dColorShape, 0, m_bAntAlias );
	}
}

bool CRadioButton::OnMouseButtonDown ( sMouseEvents e )
{
	if ( !CanHaveFocus () )
		return false;

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		if ( ContainsRect ( e.pos ) )
		{
			// Pressed while inside the control
			m_bPressed = true;

			if ( m_pParent )
				m_pParent->SetFocussedControl ( this );

			return true;
		}
	}

	return false;
}

bool CRadioButton::OnMouseButtonUp ( sMouseEvents e )
{
	if ( m_bPressed )
	{
		m_bPressed = false;

		// Button click
		if ( ContainsRect ( e.pos ) )
			SetChecked ( true );

		if ( m_pParent )
			m_pParent->ClearControlFocus ();

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
void CRadioButton::UpdateRects ( void )
{
	CControl::UpdateRects ();

	if ( m_pFont )
	{
		SIZE size;
		m_pFont->GetTextExtent ( GetText (), &size );

		if ( m_rBoundingBox.size.cx > size.cx )
			m_size.cx = size.cx + 20;
	}

	m_rText = m_rBoundingBox;
	m_rText.pos.SetX ( m_rText.pos.GetX () + 20 );
	m_rText.size.cx -= 20; m_rText.size.cy = 15;
}

//--------------------------------------------------------------------------------------
bool CRadioButton::ContainsRect ( CPos pos )
{
	if ( !CanHaveFocus () )
		return false;

	return ( m_rBoundingBox.InControlArea ( pos ) ||
			 m_rText.InControlArea ( pos ) );
}