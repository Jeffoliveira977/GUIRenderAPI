#include "CGUI.h"

CLabel::CLabel ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_LABEL );
}

CLabel::~CLabel ( void )
{}

void CLabel::Draw(void)
{
	if (!m_bVisible)
		return;

	CWidget::Draw();

	m_pDialog->DrawFont(SControlRect(m_rBoundingBox.m_pos.m_nX, m_rBoundingBox.m_pos.m_nY),
		m_sControlColor.d3dColorBox[m_eState], m_sFormatted.c_str(), 0, m_pFont);
}

bool CLabel::OnKeyDown ( WPARAM wParam )
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

bool CLabel::OnKeyUp ( WPARAM wParam )
{
	if ( m_bPressed )
	{
		m_bPressed = false;
		SendEvent ( EVENT_CONTROL_CLICKED, true );
	}

	return false;
}

bool CLabel::OnMouseButtonDown ( sMouseEvents e )
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

bool CLabel::OnMouseButtonUp ( sMouseEvents e )
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

void CLabel::SetText(SIMPLEGUI_STRING sText, bool)
{
	if (m_sText != sText)
	{
		SIZE size;
		if (m_pFont)
			m_pFont->GetTextExtent(sText.c_str(), &size);

		m_sFontInfo.m_size.cx = size.cx;

		SetWidth(m_realSize.cx);

		m_sFormatted = m_sText = sText;
	}
}

void CLabel::SetWordwrap(bool bWordwrap)
{
	m_bWordwrap = bWordwrap;
}

bool CLabel::IsWordwrap(void)
{
	return m_bWordwrap;
}

void CLabel::SetWidth(int nWidth)
{
	m_realSize.cx = m_rBoundingBox.m_size.cx = m_size.cx = 
		min(m_sFontInfo.m_size.cx, max(m_sFontInfo.m_size.cy, nWidth));
}

void CLabel::SetHeight ( int nHeight )
{
	m_realSize.cy = m_rBoundingBox.m_size.cy = m_size.cy = m_sFontInfo.m_size.cy;
}

void CLabel::UpdateRects(void)
{
	CWidget::UpdateRects();

	int nWidth = m_rBoundingBox.m_size.cx;
	if (m_oldTextSize.cx != nWidth)
	{
		SIZE size;
		if (m_bWordwrap)
		{
			if(m_pFont)
			{m_sFormatted = m_sText;
			m_pFont->RemoveLinesFromText(m_sFormatted);
			m_pFont->FormatText(m_sFormatted, nWidth);
			m_pFont->GetTextExtent(m_sFormatted.c_str(), &size); }
		}

		m_oldTextSize.cx = m_rBoundingBox.m_size.cx =
			min(m_bWordwrap ? size.cx : m_sFontInfo.m_size.cx, nWidth);

		m_realSize.cy = m_rBoundingBox.m_size.cy = m_size.cy = size.cy;
	}
}

bool CLabel::ContainsPoint(Pos pos)
{
	return m_rBoundingBox.ContainsPoint(pos);
}