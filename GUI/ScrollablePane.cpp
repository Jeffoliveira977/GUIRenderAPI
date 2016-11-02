#include "CGUI.h"

#define SCROLLBAR_SIZE 18

CScrollablePane::CScrollablePane ( CDialog *pDialog )
{
	m_pDialog = pDialog;
	m_bScrollVerShow = m_bScrollHorShow = true;
	ZeroMemory ( &m_pageSize, sizeof ( SIZE ) );

	m_pScrollbarHor = new CScrollBarHorizontal ( pDialog );

	// Assert if 'm_pScrollbarHor' is invalid
	assert ( m_pScrollbarHor &&
			 _UI ( "Error for creating horizontal scrollbar" ) );

	m_pScrollbarVer = new CScrollBarVertical ( pDialog );

	// Assert if 'm_pScrollbarVer' is invalid
	assert ( m_pScrollbarVer &&
			 _UI ( "Error for creating vertical scrollbar" ) );
}

CScrollablePane::~CScrollablePane ( void )
{
	SAFE_DELETE ( m_pScrollbarHor );
	SAFE_DELETE ( m_pScrollbarVer );
}

void CScrollablePane::OnMouseWheel ( int zDelta )
{
	if ( IsVerScrollbarNeeded () )
	{
		m_pScrollbarVer->Scroll ( zDelta * m_pScrollbarVer->GetStepSize () );
	}
	else if ( IsHorScrollbarNeeded () )
	{
		m_pScrollbarHor->Scroll ( zDelta * m_pScrollbarHor->GetStepSize () );
	}
}

void CScrollablePane::OnDraw ( void )
{
	if ( !m_pScrollbarHor ||
		 !m_pScrollbarVer )
		return;

	if ( IsHorScrollbarNeeded () )
		m_pScrollbarHor->Draw ();

	if ( IsVerScrollbarNeeded () )
		m_pScrollbarVer->Draw ();
}

bool CScrollablePane::OnMouseButtonDown ( sMouseEvents e )
{
	if ( m_pScrollbarHor && m_pScrollbarHor->OnMouseButtonDown ( e ) )
	{
		SetFocussedControl ();
		return true;
	}

	if ( m_pScrollbarVer && m_pScrollbarVer->OnMouseButtonDown ( e ) )
	{
		SetFocussedControl ();
		return true;
	}

	return false;
}

bool CScrollablePane::OnMouseButtonUp ( sMouseEvents e )
{
	if ( m_pScrollbarHor && m_pScrollbarHor->OnMouseButtonUp ( e ) )
	{
		SetFocussedControl ();
		return true;
	}

	if ( m_pScrollbarVer && m_pScrollbarVer->OnMouseButtonUp ( e ) )
	{
		SetFocussedControl ();
		return true;
	}

	return false;
}

bool CScrollablePane::OnMouseMove ( CVector pos )
{
	pos = pos;

	if ( m_pScrollbarHor && m_pScrollbarHor->OnMouseMove ( pos ) )
	{
		SetFocussedControl ();
		return true;
	}

	if ( m_pScrollbarVer && m_pScrollbarVer->OnMouseMove ( pos ) )
	{
		SetFocussedControl ();
		return true;
	}

	return false;
}

bool CScrollablePane::IsHorScrollbarNeeded ( void )
{
	return ( m_pScrollbarHor->IsThumbShowing () &&
			 m_bScrollVerShow );
}

bool CScrollablePane::IsVerScrollbarNeeded ( void )
{
	return ( m_pScrollbarVer->IsThumbShowing () &&
			 m_bScrollHorShow );
}

bool CScrollablePane::IsHorScrollbarShowing ( void )
{
	return m_bScrollHorShow;
}

bool CScrollablePane::IsVerScrollbarShowing ( void )
{
	return m_bScrollVerShow;
}

void CScrollablePane::ShowScrollHor ( bool bShow )
{
	m_bScrollHorShow = bShow;
}

void CScrollablePane::ShowScrollVer ( bool bShow )
{
	m_bScrollVerShow = bShow;
}

void CScrollablePane::SetControl ( CWidget *pControl )
{
	m_pControl = pControl;
}

CWidget *CScrollablePane::GetControl ( void )
{
	return m_pControl;
}

void CScrollablePane::RemoveControl ( void )
{
	m_pControl = NULL;
}

void CScrollablePane::SetFocussedControl ( void )
{
	// Check for a valid control, and if it unfocused
	if ( m_pControl /*&& m_pControl->HasFocus ()*/ )
	{
		CWidget *pParent = m_pControl->GetParent ();
		m_pDialog->SetFocussedWidget ( m_pControl );
		if ( !pParent )
		{
			// Give it focus
			m_pDialog->SetFocussedWidget ( m_pControl );
		}
		else // Otherwise, make sure the control has a parent
		{
			if ( pParent->GetType () == CWidget::TYPE_WINDOW )
			{
				static_cast< CWindow* >( pParent )->SetFocussedWidget ( m_pControl );
			}
			else if ( pParent->GetType () == CWidget::TYPE_TABPANEL )
			{
				static_cast< CTabPanel* >( pParent )->SetFocussedControl ( m_pControl );
			}

		}
	}
}

void CScrollablePane::OnClickLeave ( void )
{
	if ( m_pScrollbarVer )
		m_pScrollbarVer->OnClickLeave ();

	if ( m_pScrollbarHor )
		m_pScrollbarHor->OnClickLeave ();
}

bool CScrollablePane::OnClickEvent ( void )
{
	return ( m_pScrollbarHor && m_pScrollbarHor->OnClickEvent () ||
			 m_pScrollbarVer &&  m_pScrollbarVer->OnClickEvent () );
}

void CScrollablePane::OnFocusIn ( void )
{
	if ( m_pScrollbarVer )
		m_pScrollbarVer->OnFocusIn ();

	if ( m_pScrollbarHor )
		m_pScrollbarHor->OnFocusIn ();
}

void CScrollablePane::OnFocusOut ( void )
{
	if ( m_pScrollbarVer )
		m_pScrollbarVer->OnFocusOut ();

	if ( m_pScrollbarHor )
		m_pScrollbarHor->OnFocusOut ();
}

void CScrollablePane::OnMouseEnter ( void )
{
	if ( m_pScrollbarVer )
		m_pScrollbarVer->OnMouseEnter ();

	if ( m_pScrollbarHor )
		m_pScrollbarHor->OnMouseEnter ();
}

void CScrollablePane::OnMouseLeave ( void )
{
	if ( m_pScrollbarVer )
		m_pScrollbarVer->OnMouseLeave ();

	if ( m_pScrollbarHor )
		m_pScrollbarHor->OnMouseLeave ();
}

bool CScrollablePane::CanHaveFocus ( void )
{
	if ( !IsHorScrollbarNeeded () ||
		 !IsVerScrollbarNeeded () )
		return false;

	return ( m_pScrollbarHor && m_pScrollbarHor->CanHaveFocus () ||
			 m_pScrollbarVer && m_pScrollbarVer->CanHaveFocus () );
}

void CScrollablePane::SetPageSize ( int nSizeX, int nSizeY )
{
	m_pageSize.cx = nSizeX;
	m_pageSize.cy = nSizeY;
}

void CScrollablePane::SetTrackRange ( int nScrollHorValue, int nScrollVerValue )
{
	if ( !m_pScrollbarHor ||
		 !m_pScrollbarVer )
		return;

	if ( nScrollHorValue )
		m_pScrollbarHor->SetTrackRange ( 0, nScrollHorValue );
	
	if ( nScrollVerValue )
		m_pScrollbarVer->SetTrackRange ( 0, nScrollVerValue );
}

void CScrollablePane::UpdateScrollbars ( SControlRect rRect )
{
	if ( !m_pScrollbarHor ||
		 !m_pScrollbarVer )
		return;


	int nScrollVerAmount = IsVerScrollbarNeeded () ? SCROLLBAR_SIZE : 0;
	int nScrollHorAmount = IsHorScrollbarNeeded () ? SCROLLBAR_SIZE : 0;

	/*int nScrollVerAmount = 18;
	int nScrollHorAmount = 18;*/

	m_pScrollbarHor->SetPageSize ( m_pageSize.cx ? m_pageSize.cx : rRect.m_size.cx - nScrollVerAmount );
	m_pScrollbarHor->SetMinSize ( 0, 0 );
	m_pScrollbarHor->SetSize ( rRect.m_size.cx - nScrollVerAmount, SCROLLBAR_SIZE );
	m_pScrollbarHor->SetPos ( rRect.m_pos.m_nX, rRect.m_pos.m_nY + rRect.m_size.cy - SCROLLBAR_SIZE );
	m_pScrollbarHor->UpdateRects ();

	m_pScrollbarVer->SetPageSize ( m_pageSize.cy ? m_pageSize.cy : rRect.m_size.cy - nScrollHorAmount );
	m_pScrollbarVer->SetMinSize ( 0, 0 );
	m_pScrollbarVer->SetSize ( SCROLLBAR_SIZE, rRect.m_size.cy - nScrollHorAmount );
	m_pScrollbarVer->SetPos ( rRect.m_pos.m_nX + rRect.m_size.cx - SCROLLBAR_SIZE, rRect.m_pos.m_nY );
	m_pScrollbarVer->UpdateRects ();
}

bool CScrollablePane::ContainsPoint ( Pos pos )
{
	return ( IsHorScrollbarNeeded () && m_pScrollbarHor->ContainsPoint ( pos ) ||
			 IsVerScrollbarNeeded () && m_pScrollbarVer->ContainsPoint ( pos ) );
}

CScrollBarHorizontal *CScrollablePane::GetHorScrollbar ( void )
{
	return m_pScrollbarHor;
}

CScrollBarVertical *CScrollablePane::GetVerScrollbar ( void )
{
	return m_pScrollbarVer;
}