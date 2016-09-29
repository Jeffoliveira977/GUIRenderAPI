#include "CGUI.h"

#define PROGRESSBAR_ARROWCLICK_REPEAT 0.05
#define PROGRESSBAR_ARROWCLICK_START 0.33

//--------------------------------------------------------------------------------------
CProgressBarHorizontal::CProgressBarHorizontal ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_PROGRESSBARHORIZONTAL );

	m_fStep = 1.f;
	m_fValue = 0.f;
	m_fMax = 100.f;
}

//--------------------------------------------------------------------------------------
void CProgressBarHorizontal::Draw ( void )
{
	if ( !m_bVisible )
		return;

	CMouse *pMouse = m_pDialog->GetMouse ();

	if ( pMouse &&
		 m_bPressed &&
		 m_rBoundingBox.InControlArea ( pMouse->GetPos () ) && 
		 !m_timer.Running() )
	{
		if ( pMouse->GetPos ().GetX () > m_rProgress.pos.GetX () + m_rProgress.size.cx )
		{
			m_timer.Start ( PROGRESSBAR_ARROWCLICK_REPEAT );
			SetValue ( m_fValue + m_fStep );
		}
		else if ( pMouse->GetPos ().GetX () < m_rProgress.pos.GetX () + m_rProgress.size.cx )
		{
			m_timer.Start ( PROGRESSBAR_ARROWCLICK_REPEAT );
			SetValue ( m_fValue - m_fStep );
		}
	}

	m_pDialog->DrawBox ( m_rBoundingBox, m_sControlColor.d3dColorBoxBack, m_sControlColor.d3dColorOutline, m_bAntAlias );
	m_pDialog->DrawBox ( m_rProgress, m_sControlColor.d3dColorBoxSel, m_sControlColor.d3dColorOutline, m_bAntAlias );
}

//--------------------------------------------------------------------------------------
bool CProgressBarHorizontal::OnMouseMove ( CPos pos )
{
	if ( m_bPressed )
	{
		SetValue ( ValueFromPos ( pos.GetX () - m_rProgress.pos.GetX () ) );
		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------
bool CProgressBarHorizontal::OnMouseButtonDown ( sMouseEvents e )
{
	if ( !CanHaveFocus () )
		return false;

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		if ( m_rBoundingBox.InControlArea ( e.pos ) )
		{
			// Pressed while inside the control
			m_bPressed = true;

			if ( m_pParent )
				m_pParent->SetFocussedControl ( this );

			m_timer.Start ( PROGRESSBAR_ARROWCLICK_START );

			if ( e.pos.GetX () > m_rProgress.pos.GetX () + m_rProgress.size.cx )
			{
				SetValue ( m_fValue + m_fStep );
				return true;
			}
			else if ( e.pos.GetX () < m_rProgress.pos.GetX () + m_rProgress.size.cx )
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
bool CProgressBarHorizontal::OnMouseButtonUp ( sMouseEvents e )
{
	if ( m_bPressed )
	{
		m_bPressed = false;

		if ( m_pParent )
			m_pParent->ClearControlFocus ();

		return true;
	}
	return false;
}

//--------------------------------------------------------------------------------------
float CProgressBarHorizontal::ValueFromPos ( int iX )
{
	float fValuePerPixel = ( float ) ( m_fMax / m_rBoundingBox.size.cx );
	return  ( 0.5 + fValuePerPixel * iX );
}

//--------------------------------------------------------------------------------------
void CProgressBarHorizontal::SetValue ( float fValue )
{
	// Clamp to range
	fValue = __max ( 0.f, fValue );
	fValue = __min ( m_fMax, fValue );

	m_fValue = fValue;
}

//--------------------------------------------------------------------------------------
float CProgressBarHorizontal::GetValue ( void )
{
	return m_fValue;
}

//--------------------------------------------------------------------------------------
void CProgressBarHorizontal::SetMaxValue ( float fMax )
{
	m_fMax = fMax;
}

//--------------------------------------------------------------------------------------
float CProgressBarHorizontal::GetMaxValue ( void )
{
	return m_fMax;
}

//--------------------------------------------------------------------------------------
void CProgressBarHorizontal::SetStep ( float fStep )
{
	m_fStep = fStep;
}

//--------------------------------------------------------------------------------------
float CProgressBarHorizontal::GetStep ( void )
{
	return m_fStep;
}

//--------------------------------------------------------------------------------------
void CProgressBarHorizontal::UpdateRects ( void )
{
	CControl::UpdateRects ();

	m_rProgress = m_rBoundingBox;
	m_rProgress.size.cx = ( m_fValue * ( float ) ( m_rBoundingBox.size.cx ) / m_fMax );
}

//--------------------------------------------------------------------------------------
bool CProgressBarHorizontal::ContainsRect ( CPos pos )
{
	if ( !CanHaveFocus () )
		return false;

	return ( m_rBoundingBox.InControlArea ( pos ) ||
			 m_rProgress.InControlArea ( pos ) );
}
