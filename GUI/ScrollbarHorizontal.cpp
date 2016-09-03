#include "CGUI.h"

#define SCROLLBAR_MINTHUMBSIZE 8
#define SCROLLBAR_ARROWCLICK_START 0.05
#define SCROLLBAR_ARROWCLICK_REPEAT 0.33

#define CMPSTATE( a, b, c, d ) a == b ? c : d 

CScrollBarHorizontal::CScrollBarHorizontal ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_SCROLLBARHORIZONTAL );

	m_bShowThumb = true;
	m_bDrag = false;

	m_nStep = 1;
	m_nPosition = 0;
	m_nPageSize = 1;
	m_nStart = 0;
	m_nEnd = 1;
	nThumbOffset = 0;
	m_Arrow = CLEAR;
}

void CScrollBarHorizontal::Draw ( void )
{
	if ( !m_bVisible )
		return;

	CPos mPos = m_pDialog->GetMouse ()->GetPos ();

	// Check if the arrow button has been held for a while.
	// If so, update the thumb position to simulate repeated
	// scroll.
	if ( m_Arrow == CLICKED_UP &&
		 m_rUpButton.InControlArea ( mPos ) &&
		 !m_timer.Running () )
	{
		Scroll ( -m_nStep );
		m_timer.Start ( SCROLLBAR_ARROWCLICK_START );
	}
	else if ( m_Arrow == CLICKED_DOWN &&
			  m_rDownButton.InControlArea ( mPos ) &&
			  !m_timer.Running () )
	{
		Scroll ( m_nStep );
		m_timer.Start ( SCROLLBAR_ARROWCLICK_START );
	}

	// Check for click on track
	if ( m_bPressed &&
		 !m_timer.Running () &&
		 m_rBoundingBox.InControlArea ( mPos ) )
	{
		if ( m_Arrow == HELD_UP &&
			 m_rThumb.pos.GetX () > mPos.GetX () &&
			 m_rBoundingBox.pos.GetX () <= mPos.GetX () )
		{
			Scroll ( -( m_nPageSize - 1 ) );
			m_timer.Start ( SCROLLBAR_ARROWCLICK_START );
		}
		else if ( m_Arrow == HELD_DOWN &&
				  m_rThumb.pos.GetX () + m_rThumb.size.cx <= mPos.GetX () &&
				  ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cy ) + m_rBoundingBox.size.cx > mPos.GetX () )
		{
			Scroll ( m_nPageSize - 1 );
			m_timer.Start ( SCROLLBAR_ARROWCLICK_START );
		}
	}

	CControl::Draw ();

	SControlColor::SControlState eState = m_bShowThumb && m_bEnabled ?
		SControlColor::STATE_NORMAL :
		SControlColor::STATE_DISABLED;

	D3DCOLOR d3dColorThumb = m_sControlColor.d3dColorBox [ CMPSTATE ( m_Arrow, CLICKED_THUMB, SControlColor::STATE_PRESSED, eState ) ];
	D3DCOLOR d3dColorUp = m_sControlColor.d3dColorBox [ CMPSTATE ( m_Arrow, CLICKED_UP, SControlColor::STATE_PRESSED, eState ) ];
	D3DCOLOR d3dColorDown = m_sControlColor.d3dColorBox [ CMPSTATE ( m_Arrow, CLICKED_DOWN, SControlColor::STATE_PRESSED, eState ) ];

	if ( m_bShowThumb &&
		 !m_bPressed )
	{
		if ( m_rThumb.InControlArea ( mPos ) )
			d3dColorThumb = m_sControlColor.d3dColorBox [ m_eState ];
		else if ( m_rUpButton.InControlArea ( mPos ) )
			d3dColorUp = m_sControlColor.d3dColorBox [ m_eState ];
		else if ( m_rDownButton.InControlArea ( mPos ) )
			d3dColorDown = m_sControlColor.d3dColorBox [ m_eState ];
	}

	// Background track layer
	m_pDialog->DrawBox ( m_rBoundingBox, m_sControlColor.d3dColorBoxBack, m_sControlColor.d3dColorOutline, m_bAntAlias );

	// Up Arrow
	m_pDialog->DrawBox ( m_rUpButton, d3dColorUp, m_sControlColor.d3dColorOutline, m_bAntAlias );

	SControlRect rShape = m_rUpButton;
	rShape.pos.SetX ( m_rUpButton.pos.GetX () + m_rUpButton.size.cx / 2 +4);
	rShape.pos.SetY ( m_rUpButton.pos.GetY () + m_rUpButton.size.cx / 2  );
	rShape.size.cx = m_rUpButton.size.cx / 2 - 3;
	m_pDialog->DrawTriangle ( rShape, 90.f, m_sControlColor.d3dColorShape, 0 );

	// Down Arrow
	m_pDialog->DrawBox ( m_rDownButton, d3dColorDown, m_sControlColor.d3dColorOutline, m_bAntAlias );

	rShape.pos.SetX ( m_rDownButton.pos.GetX () + m_rDownButton.size.cx / 2 -4);
	rShape.pos.SetY ( m_rDownButton.pos.GetY () + m_rDownButton.size.cx / 2  );
	m_pDialog->DrawTriangle ( SControlRect ( rShape.pos, rShape.size ), 270.f, m_sControlColor.d3dColorShape, 0 );

	// Thumb button
	m_pDialog->DrawBox ( m_rThumb, d3dColorThumb, m_sControlColor.d3dColorOutline, m_bAntAlias );
}


//--------------------------------------------------------------------------------------
void CScrollBarHorizontal::OnClickLeave ( void )
{
	m_bPressed = m_bDrag = false;
	m_Arrow = CLEAR;
}

//--------------------------------------------------------------------------------------
bool CScrollBarHorizontal::OnClickEvent ( void )
{
	return ( m_bPressed ||
			 m_bDrag ||
			 m_Arrow != CLEAR );
}

//--------------------------------------------------------------------------------------
bool CScrollBarHorizontal::HandleMouse ( UINT uMsg, CPos pos, WPARAM wParam, LPARAM lParam )
{
	if ( !CanHaveFocus () || 
		 !m_bShowThumb )
		return false;


	switch ( uMsg )
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			// Check for click on up button
			if ( m_rUpButton.InControlArea ( pos ) )
			{
				if ( m_nPosition > m_nStart )
					Scroll ( -m_nStep );

				UpdateThumbRect ();
				m_Arrow = CLICKED_UP;
				m_timer.Start ( SCROLLBAR_ARROWCLICK_REPEAT );
				m_bPressed = true;

				if ( m_pParent )
					m_pParent->SetFocussedControl ( this );

				return true;
			}

			// Check for click on down button
			if ( m_rDownButton.InControlArea ( pos ) )
			{
				if ( m_nPosition + m_nPageSize <= m_nEnd )
					Scroll ( m_nStep );

				UpdateThumbRect ();
				m_Arrow = CLICKED_DOWN;
				m_timer.Start ( SCROLLBAR_ARROWCLICK_REPEAT );
				m_bPressed = true;

				if ( m_pParent )
					m_pParent->SetFocussedControl ( this );

				return true;
			}

			// Check for click on thumb
			if ( m_rThumb.InControlArea ( pos ) )
			{
				m_Arrow = CLICKED_THUMB;
				m_bDrag = true;
				nThumbOffset = pos.GetX () - m_rThumb.pos.GetX ();
				m_bPressed = true;

				if ( m_pParent )
					m_pParent->SetFocussedControl ( this );

				return true;
			}

			// Check for click on track
			if ( m_rBoundingBox.InControlArea ( pos ) )
			{
				if ( m_rThumb.pos.GetX () > pos.GetX () &&
					 m_rBoundingBox.pos.GetX () <= pos.GetX () )
				{
					Scroll ( -( m_nPageSize - 1 ) );

					if ( m_pParent )
						m_pParent->SetFocussedControl ( this );

					m_timer.Start ( 0.5 );
					m_bPressed = true;
					m_Arrow = HELD_UP;

					return true;
				}
				else if ( m_rThumb.pos.GetX () + m_rThumb.size.cx <= pos.GetX () &&
						  ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cy ) + m_rBoundingBox.size.cx > pos.GetX () )
				{
					Scroll ( m_nPageSize - 1 );

					if ( m_pParent )
						m_pParent->SetFocussedControl ( this );

					m_timer.Start ( 0.5 );
					m_bPressed = true;
					m_Arrow = HELD_DOWN;

					return true;
				}
			}

			break;
		}

		case WM_LBUTTONUP:
		{
			if ( m_bPressed )
			{
				m_bPressed = false;
				m_bDrag = false;
				UpdateThumbRect ();
				m_Arrow = CLEAR;

				if ( m_pParent )
					m_pParent->ClearControlFocus ();

				return true;
			}

			break;
		}

		case WM_MOUSEMOVE:
		{
			if ( m_bDrag )
			{
				m_rThumb.pos.SetX ( pos.GetX () - nThumbOffset );

				if ( m_rThumb.pos.GetX () < ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cy ) )
				{
					m_rThumb.pos.SetX ( ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cy ) );
				}
				else if ( m_rThumb.pos.GetX () + m_rThumb.size.cx >= ( m_rBoundingBox.pos.GetX () ) + ( m_rBoundingBox.size.cx - m_rBoundingBox.size.cy ) )
				{
					m_rThumb.pos.SetX ( m_rBoundingBox.pos.GetX () + ( m_rBoundingBox.size.cx - m_rBoundingBox.size.cy - m_rThumb.size.cx ) );
				}

				// Compute first item index based on thumb position
				int nMaxFirstItem = m_nEnd - m_nStart - m_nPageSize + 1;  // Largest possible index for first item
				int nMaxThumb = ( m_rBoundingBox.size.cx - ( m_rBoundingBox.size.cy * 2 ) ) - m_rThumb.size.cx;  // Largest possible thumb position from the top

				m_nPosition = m_nStart +
					( m_rThumb.pos.GetX () - ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cy ) +
					  nMaxThumb / ( nMaxFirstItem * 2 ) ) * // Shift by half a row to avoid last row covered by only one pixel
					nMaxFirstItem / nMaxThumb;

				return true;
			}

			break;
		}
	}

	return false;
}

bool CScrollBarHorizontal::OnMouseButtonDown ( CPos pos )
{
	// Check for click on up button
	if ( m_rUpButton.InControlArea ( pos ) )
	{
		if ( m_nPosition > m_nStart )
			Scroll ( -m_nStep );

		UpdateThumbRect ();
		m_Arrow = CLICKED_UP;
		m_timer.Start ( SCROLLBAR_ARROWCLICK_REPEAT );
		m_bPressed = true;

		if ( m_pParent )
			m_pParent->SetFocussedControl ( this );

		return true;
	}

	// Check for click on down button
	if ( m_rDownButton.InControlArea ( pos ) )
	{
		if ( m_nPosition + m_nPageSize <= m_nEnd )
			Scroll ( m_nStep );

		UpdateThumbRect ();
		m_Arrow = CLICKED_DOWN;
		m_timer.Start ( SCROLLBAR_ARROWCLICK_REPEAT );
		m_bPressed = true;

		if ( m_pParent )
			m_pParent->SetFocussedControl ( this );

		return true;
	}

	// Check for click on thumb
	if ( m_rThumb.InControlArea ( pos ) )
	{
		m_Arrow = CLICKED_THUMB;
		m_bDrag = true;
		nThumbOffset = pos.GetX () - m_rThumb.pos.GetX ();
		m_bPressed = true;

		if ( m_pParent )
			m_pParent->SetFocussedControl ( this );

		return true;
	}

	// Check for click on track
	if ( m_rBoundingBox.InControlArea ( pos ) )
	{
		if ( m_rThumb.pos.GetX () > pos.GetX () &&
			 m_rBoundingBox.pos.GetX () <= pos.GetX () )
		{
			Scroll ( -( m_nPageSize - 1 ) );

			if ( m_pParent )
				m_pParent->SetFocussedControl ( this );

			m_timer.Start ( 0.5 );
			m_bPressed = true;
			m_Arrow = HELD_UP;

			return true;
		}
		else if ( m_rThumb.pos.GetX () + m_rThumb.size.cx <= pos.GetX () &&
				  ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cy ) + m_rBoundingBox.size.cx > pos.GetX () )
		{
			Scroll ( m_nPageSize - 1 );

			if ( m_pParent )
				m_pParent->SetFocussedControl ( this );

			m_timer.Start ( 0.5 );
			m_bPressed = true;
			m_Arrow = HELD_DOWN;

			return true;
		}
	}

	return false;
}

bool CScrollBarHorizontal::OnMouseButtonUp ( CPos pos )
{
	if ( m_bPressed )
	{
		m_bPressed = false;
		m_bDrag = false;
		UpdateThumbRect ();
		m_Arrow = CLEAR;

		if ( m_pParent )
			m_pParent->ClearControlFocus ();

		return true;
	}

	return false;
}

bool CScrollBarHorizontal::OnMouseMove ( CPos pos )
{
	if ( m_bDrag )
	{
		m_rThumb.pos.SetX ( pos.GetX () - nThumbOffset );

		if ( m_rThumb.pos.GetX () < ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cy ) )
		{
			m_rThumb.pos.SetX ( ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cy ) );
		}
		else if ( m_rThumb.pos.GetX () + m_rThumb.size.cx >= ( m_rBoundingBox.pos.GetX () ) + ( m_rBoundingBox.size.cx - m_rBoundingBox.size.cy ) )
		{
			m_rThumb.pos.SetX ( m_rBoundingBox.pos.GetX () + ( m_rBoundingBox.size.cx - m_rBoundingBox.size.cy - m_rThumb.size.cx ) );
		}

		// Compute first item index based on thumb position
		int nMaxFirstItem = m_nEnd - m_nStart - m_nPageSize + 1;  // Largest possible index for first item
		int nMaxThumb = ( m_rBoundingBox.size.cx - ( m_rBoundingBox.size.cy * 2 ) ) - m_rThumb.size.cx;  // Largest possible thumb position from the top

		m_nPosition = m_nStart +
			( m_rThumb.pos.GetX () - ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cy ) +
			  nMaxThumb / ( nMaxFirstItem * 2 ) ) * // Shift by half a row to avoid last row covered by only one pixel
			nMaxFirstItem / nMaxThumb;

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
void CScrollBarHorizontal::UpdateRects ( void )
{
	CControl::UpdateRects ();

	// Make the buttons square
	m_rUpButton = m_rBoundingBox;
	m_rUpButton.pos.SetX ( m_rBoundingBox.pos.GetX () );
	m_rUpButton.size.cx = m_rBoundingBox.size.cy;

	m_rDownButton = m_rBoundingBox;
	m_rDownButton.pos.SetX ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cx - m_rBoundingBox.size.cy );
	m_rDownButton.size.cx = m_rBoundingBox.size.cy;

	CScrollbar::SetBackPos ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cy );
	CScrollbar::SetBackSize ( m_rBoundingBox.size.cx - ( m_rBoundingBox.size.cy * 2 ) );

	CScrollbar::UpdateThumbRect ();

	m_rThumb = m_rBoundingBox;
	m_rThumb.pos.SetX ( CScrollbar::GetThumbPos () );
	m_rThumb.size.cx = CScrollbar::GetThumbSize ();
}

//--------------------------------------------------------------------------------------
bool CScrollBarHorizontal::ContainsRect ( CPos pos )
{
	if ( !CanHaveFocus () )
		return false;

	return ( m_rBoundingBox.InControlArea ( pos ) ||
			 m_rUpButton.InControlArea ( pos ) ||
			 m_rDownButton.InControlArea ( pos ) ||
			 m_rThumb.InControlArea ( pos ) );
}
