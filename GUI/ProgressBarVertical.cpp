#include "CGUI.h"

#define PROGRESSBAR_ARROWCLICK_REPEAT 0.05
#define PROGRESSBAR_ARROWCLICK_START 0.33

//--------------------------------------------------------------------------------------
CProgressBarVertical::CProgressBarVertical ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_PROGRESSBARVERTICAL );

	m_fStep = 1.f;
	m_fValue = 0.f;
	m_fMax = 100.f;
}

//--------------------------------------------------------------------------------------
void CProgressBarVertical::Draw ( void )
{
	if ( !m_bVisible )
		return;

	CMouse *pMouse = m_pDialog->GetMouse ();

	if ( pMouse && 
		 m_bPressed && 
		 m_rBoundingBox.ContainsPoint ( pMouse->GetPos () ) && 
		 !m_timer.Running() )
	{
		if ( pMouse->GetPos ().m_nY > m_rProgress.m_pos.m_nY + m_rProgress.m_size.cx )
		{
			m_timer.Start ( PROGRESSBAR_ARROWCLICK_START );
			SetValue ( m_fValue + m_fStep );
		}
		else if ( pMouse->GetPos ().m_nY < m_rProgress.m_pos.m_nY + m_rProgress.m_size.cx )
		{
			m_timer.Start ( PROGRESSBAR_ARROWCLICK_START );
			SetValue ( m_fValue - m_fStep );
		}
	}

	m_pDialog->DrawBox ( m_rBoundingBox, m_sControlColor.d3dColorBoxBack, m_sControlColor.d3dColorOutline, m_bAntAlias );
	m_pDialog->DrawBox ( m_rProgress, m_sControlColor.d3dColorBoxSel, m_sControlColor.d3dColorOutline, m_bAntAlias );
}

//--------------------------------------------------------------------------------------
bool CProgressBarVertical::OnMouseMove ( CVector pos )
{
	if ( m_bPressed )
	{
		SetValue ( ValueFromPos ( pos.m_nY - m_rProgress.m_pos.m_nY) );
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
bool CProgressBarVertical::OnMouseButtonDown ( sMouseEvents e )
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

			m_timer.Start ( PROGRESSBAR_ARROWCLICK_START );

			if ( e.pos.m_nY > m_rProgress.m_pos.m_nY + m_rProgress.m_size.cy )
			{
				SetValue ( m_fValue + m_fStep );
				return true;
			}
			else if ( e.pos.m_nY < m_rProgress.m_pos.m_nY + m_rProgress.m_size.cy )
			{
				SetValue ( m_fValue - m_fStep );
				return true;
			}

			return true;
		}
	}

	return false;
}

//--------------------------------------------------------------------------------------
bool CProgressBarVertical::OnMouseButtonUp ( sMouseEvents e )
{
	if ( m_bPressed )
	{
		m_bPressed = false;

		_ClearFocus ();
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
float CProgressBarVertical::ValueFromPos ( int iY )
{
	float fValuePerPixel = ( float ) ( m_fMax / m_rBoundingBox.m_size.cy );
	return  ( 0.5 + fValuePerPixel * iY );
}

//--------------------------------------------------------------------------------------
void CProgressBarVertical::SetValue ( float fValue )
{
	// Clamp to range
	fValue = __max ( 0.f, fValue );
	fValue = __min ( m_fMax, fValue );

	if ( m_fValue == fValue )
		return;

	m_fValue = fValue;
}

//--------------------------------------------------------------------------------------
float CProgressBarVertical::GetValue ( void )
{
	return m_fValue;
}

//--------------------------------------------------------------------------------------
void CProgressBarVertical::SetMaxValue ( float fMax )
{
	m_fMax = fMax;
}

//--------------------------------------------------------------------------------------
float CProgressBarVertical::GetMaxValue ( void )
{
	return m_fMax;
}

//--------------------------------------------------------------------------------------
void CProgressBarVertical::SetStep ( float fStep )
{
	m_fStep = fStep;
}

//--------------------------------------------------------------------------------------
float CProgressBarVertical::GetStep ( void )
{
	return m_fStep;
}

//--------------------------------------------------------------------------------------
void CProgressBarVertical::UpdateRects ( void )
{
	CWidget::UpdateRects ();

	m_rProgress = m_rBoundingBox;
	m_rProgress.m_size.cy = ( m_fValue * ( float ) ( m_rBoundingBox.m_size.cy ) / m_fMax );
}

//--------------------------------------------------------------------------------------
bool CProgressBarVertical::ContainsPoint ( CVector pos )
{
	if ( !CanHaveFocus () )
		return false;

	return ( m_rBoundingBox.ContainsPoint ( pos ) ||
			 m_rProgress.ContainsPoint ( pos ) );
}