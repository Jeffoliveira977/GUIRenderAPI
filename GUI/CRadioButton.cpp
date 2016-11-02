#include "CGUI.h"

std::map<CWidget*, std::map<UINT, CWidget*>> CRadioButton::mADD;

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

	CWidget::Draw ();

	m_pDialog->DrawFont(SControlRect(m_rText.m_pos.m_nX, m_rText.m_pos.m_nY),
		m_sControlColor.d3dColorFont, GetText(), 0, m_pFont);

	Pos pos = m_rBox.m_pos;
	SIZE size = m_rBox.m_size;

	m_pDialog->DrawCircle(SControlRect(pos, size), m_sControlColor.d3dColorBox[m_eState], m_sControlColor.d3dColorOutline, m_bAntAlias);

	if ( mADD [ m_pParent ] [ m_uGroup ] == this )
	{
		size.cx = size.cy -= 4;
		m_pDialog->DrawCircle ( SControlRect ( pos + 2, size ), m_sControlColor.d3dColorShape, m_sControlColor.d3dColorOutline, m_bAntAlias );
	}
}

void CRadioButton::SetGroup(UINT uGroup)
{
	m_uGroup = uGroup;
}

UINT CRadioButton::GetGroup(void)
{
	return m_uGroup;
}

bool CRadioButton::IsChecked(void)
{
	return (mADD[m_pParent][m_uGroup] == this);
}

void CRadioButton::SetChecked(bool bChecked)
{
	mADD[m_pParent][m_uGroup] = bChecked ? this : NULL;
	SendEvent(EVENT_CONTROL_SELECT, bChecked);
}

bool CRadioButton::OnMouseButtonDown ( sMouseEvents e )
{
	if ( !CanHaveFocus () )
		return false;

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		if ( ContainsPoint ( e.pos ) )
		{
			// Pressed while inside the control
			m_bPressed = true;

			_SetFocus ();

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
		if ( ContainsPoint ( e.pos ) )
			SetChecked ( true );

		_ClearFocus ();

		return true;
	}

	return false;
}

void CRadioButton::SetText(SIMPLEGUI_STRING sText, bool)
{
	if (m_sText != sText || sText.empty())
	{
		SIZE size;
		if (m_pFont)
		{
			m_pFont->GetTextExtent(sText.empty() ? _UI("Y") : sText.c_str(), &size);
		}

		m_sFontInfo.m_size.cx = sText.empty() ? 0 : size.cx;
		m_sFontInfo.m_size.cy = size.cy;

		SetWidth(m_realSize.cx);
		SetHeight(0);

		m_sText = sText;
	}
}

void CRadioButton::SetWidth(int nWidth)
{
	SIZE size = m_sFontInfo.m_size;

	m_realSize.cx = m_rBoundingBox.m_size.cx = m_size.cx = size.cy + 4 +
		min(size.cx, max(size.cy, nWidth));
}

void CRadioButton::SetHeight(int nHeight)
{
	m_realSize.cy = m_rBoundingBox.m_size.cy = m_size.cy = m_sFontInfo.m_size.cy;
}

//--------------------------------------------------------------------------------------
void CRadioButton::UpdateRects(void)
{
	SIZE size;
	if (m_pFont)
	{
		m_pFont->GetTextExtent(_UI("Y"), &size);
	}

	m_rBox = m_rBoundingBox;
	m_rBox.m_size.cx = m_rBox.m_size.cy = size.cy;
	m_rBox.m_pos.m_nY += (m_sFontInfo.m_size.cy / 2) - (size.cy / 2);

	m_rText = m_rBoundingBox;
	m_rText.m_size.cx -= m_sFontInfo.m_size.cy + 4;
	m_rText.m_pos.m_nX += size.cy + 4;
}

//--------------------------------------------------------------------------------------
bool CRadioButton::ContainsPoint(Pos pos)
{
	if (!CanHaveFocus())
		return false;

	return (m_rBox.ContainsPoint(pos) || m_rText.ContainsPoint(pos));
}