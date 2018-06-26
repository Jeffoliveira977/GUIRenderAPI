#include "CGUI.h"
#define SCROLLBAR_MINTHUMBSIZE 8

//CScrollbar::CScrollbar ()
//{}
//
//
//CScrollbar::~CScrollbar ()
//{}

//--------------------------------------------------------------------------------------
int CScrollbar::GetMinSize ( void )
{
	return m_nStart;
}

//--------------------------------------------------------------------------------------
int CScrollbar::GetEndSize ( void )
{
	return m_nEnd;
}

//--------------------------------------------------------------------------------------
int CScrollbar::GetTrackPos ( void )
{
	return m_nPosition;
}

//--------------------------------------------------------------------------------------
void CScrollbar::SetTrackPos ( int nPosition )
{
	m_nPosition = nPosition;
	
	// Cap position
	Cap ();
	
	// Update thumb position
	UpdateThumbRect ();
}

//--------------------------------------------------------------------------------------
int CScrollbar::GetPageSize ( void )
{
	return m_nPageSize;
}

//--------------------------------------------------------------------------------------
void CScrollbar::SetPageSize ( int nPageSize )
{
	if ( nPageSize > 0 )
		m_nPageSize = nPageSize;

	// Cap position
	Cap ();
	
	// Update thumb position
	UpdateThumbRect ();
}

//--------------------------------------------------------------------------------------
void CScrollbar::SetStepSize ( int nStep )
{
	m_nStep = nStep;
}

//--------------------------------------------------------------------------------------
int CScrollbar::GetStepSize ( void )
{
	return m_nStep;
}

//--------------------------------------------------------------------------------------
bool CScrollbar::IsThumbShowing ( void )
{
	return m_bShowThumb;
}

//--------------------------------------------------------------------------------------
// Scroll() scrolls by nDelta items.  A positive value scrolls down, while a negative
// value scrolls up.
void CScrollbar::Scroll ( int nDelta )
{
	// Perform scroll
	m_nPosition += nDelta;

	// Cap position
	Cap ();

	// Update thumb position
	UpdateThumbRect ();
}

//--------------------------------------------------------------------------------------
void CScrollbar::ShowItem ( int nIndex )
{
	// Cap the index
	if ( nIndex < 0 )
		nIndex = 0;

	if ( nIndex > m_nEnd )
		nIndex = m_nEnd - 1;

	// Adjust position
	if ( m_nPosition > nIndex )
	{
		m_nPosition = nIndex;
	}
	else if ( m_nPosition + m_nPageSize <= nIndex )
	{
		m_nPosition = nIndex - m_nPageSize + 1;
	}

	UpdateThumbRect ();
}

//--------------------------------------------------------------------------------------
void CScrollbar::SetTrackRange ( int nStart, int nEnd )
{
	if ( nStart > nEnd )
	{
		m_nEnd = nStart;
		m_nStart = nEnd;
	}
	else
	{
		m_nStart = nStart;
		m_nEnd = nEnd;
	}

	// Cap position
	Cap ();

	// Update thumb position
	UpdateThumbRect ();
}

//--------------------------------------------------------------------------------------
// Clips position at boundaries. Ensures it stays within legal range.
void CScrollbar::Cap ()
{
	if ( m_nPosition < m_nStart ||
		 m_nEnd - m_nStart <= m_nPageSize )
	{
		m_nPosition = m_nStart;
	}
	else if ( m_nPosition + m_nPageSize > m_nEnd )
	{
		m_nPosition = m_nEnd - m_nPageSize + 1;
	}
}

int CScrollbar::GetThumbPos ( void )
{
	return m_nThumbPos;
}

int CScrollbar::GetThumbSize ( void )
{
	return m_nThumbSize;
}

void CScrollbar::SetBackPos ( int nPos )
{
	m_nBackPos = nPos;
}

void CScrollbar::SetBackSize ( int nSize )
{
	m_nBackSize = nSize;
}

//--------------------------------------------------------------------------------------
// Compute the dimension of the scroll thumb
void CScrollbar::UpdateThumbRect ( void )
{
	int nMaxItem = m_nEnd - m_nStart;

	if ( nMaxItem > m_nPageSize )
	{
		if ( m_nBackSize > SCROLLBAR_MINTHUMBSIZE )
		{
			m_nThumbSize = __max ( m_nBackSize * m_nPageSize / nMaxItem, SCROLLBAR_MINTHUMBSIZE );

			m_nThumbPos = m_nBackPos + ( m_nPosition - m_nStart ) *
				( m_nBackSize - m_nThumbSize ) / ( nMaxItem - m_nPageSize + 1 );
		}
		else
		{
			m_nThumbSize = 0;
		}

		m_bShowThumb = true;
	}
	else
	{
		// No content to scroll
		m_nThumbSize = 0;
		m_bShowThumb = false;
	}
}