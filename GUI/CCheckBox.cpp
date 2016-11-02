#include "CGUI.h"

CCheckBox::CCheckBox ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_CHECKBOX );
}

void CCheckBox::Draw(void)
{
	if (!m_bVisible)
		return;

	CWidget::Draw();

	m_pDialog->DrawFont(SControlRect(m_rText.m_pos.m_nX, m_rText.m_pos.m_nY),
		m_sControlColor.d3dColorFont, GetText(), 0, m_pFont);

	Pos pos = m_rBox.m_pos;
	SIZE size = m_rBox.m_size;

	m_pDialog->DrawBox(SControlRect(pos, size), m_sControlColor.d3dColorBox[m_eState], m_sControlColor.d3dColorOutline);

	if (m_bChecked)
	{
		size.cx = size.cy -= 4;

		m_pDialog->DrawBox(SControlRect(pos + 2, size), m_sControlColor.d3dColorShape, m_sControlColor.d3dColorOutline);
	}
}

bool CCheckBox::IsChecked(void)
{
	return m_bChecked;
}

void CCheckBox::SetChecked(bool bChecked)
{
	m_bChecked = bChecked;
	SendEvent(EVENT_CONTROL_SELECT, m_bChecked);
}

bool CCheckBox::OnKeyDown ( WPARAM wParam )
{
	if ( !CanHaveFocus () )
		return false;

	if ( wParam == VK_SPACE )
	{
		m_bPressed = true;
		return true;
	}

	return false;
}

bool CCheckBox::OnKeyUp ( WPARAM wParam )
{
	if ( m_bPressed )
	{
		m_bPressed = false;
		SetChecked ( !m_bChecked );
		return true;
	}

	return false;
}

bool CCheckBox::OnMouseButtonDown ( sMouseEvents e )
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

bool CCheckBox::OnMouseButtonUp ( sMouseEvents e )
{
	if ( m_bPressed )
	{
		m_bPressed = false;

		// Button click
		if ( ContainsPoint ( e.pos ) )
			SetChecked ( !m_bChecked );

		_ClearFocus ();

		return true;
	}

	return false;
}

void CCheckBox::SetText(SIMPLEGUI_STRING sText, bool)
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

void CCheckBox::SetWidth(int nWidth)
{
	SIZE size = m_sFontInfo.m_size;

	m_realSize.cx = m_rBoundingBox.m_size.cx = m_size.cx = size.cy + 4 +
		min(size.cx, max(size.cy, nWidth));
}

void CCheckBox::SetHeight(int nHeight)
{
	m_realSize.cy = m_rBoundingBox.m_size.cy = m_size.cy = m_sFontInfo.m_size.cy;
}

//--------------------------------------------------------------------------------------
void CCheckBox::UpdateRects(void)
{
	SIZE size;
	if (m_pFont)
	{
		m_pFont->GetTextExtent(_UI("Y"), &size);
	}

	m_rBox = m_rBoundingBox;
	m_rBox.m_size.cx = m_rBox.m_size.cy = size.cy;
	m_rBox.m_pos.m_nY += m_sFontInfo.m_size.cy / 2 - size.cy / 2;

	m_rText = m_rBoundingBox;
	m_rText.m_size.cx -= m_sFontInfo.m_size.cy+4;
	m_rText.m_pos.m_nX += size.cy + 4;
}

//--------------------------------------------------------------------------------------
bool CCheckBox::ContainsPoint ( CVector pos )
{
	if ( !CanHaveFocus () )
		return false;

	return (m_rBox.ContainsPoint(pos) || m_rText.ContainsPoint(pos));
}