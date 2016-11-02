#include "CGUI.h"

CEntryList::CEntryList ( CDialog *pDialog ) :
	m_bSort ( false ), m_bMultipleSelected ( false )
{
	// Assert if 'pDialog' is invalid
	assert ( pDialog );

	m_pDialog = pDialog;

	m_pFont = m_pDialog->GetFont ();

	m_pScrollbar = new CScrollablePane ( pDialog );
}

CEntryList::~CEntryList ( void )
{
	SAFE_DELETE ( m_pFont );
	SAFE_DELETE ( m_pScrollbar );

	ResetList ();
}

void CEntryList::SetFont ( CD3DFont *pFont )
{
	if ( pFont )
		m_pFont = pFont;
}

void CEntryList::AddEntry ( SEntryItem *pEntry )
{
	if ( !pEntry )
		return;

	if ( m_bSort )
	{
		m_vEntryList.insert ( std::upper_bound ( m_vEntryList.begin (), m_vEntryList.end (), pEntry,
		[] ( const SEntryItem* a, const SEntryItem* b )
		{
			return ( a->m_sText < b->m_sText );
		} ), pEntry );
	}
	else
	{
		m_vEntryList.push_back ( pEntry );
	}

	if ( m_pScrollbar &&
		 m_pFont )
	{
		SIZE size;
		m_pFont->GetTextExtent ( pEntry->m_sText.c_str (), &size );

		m_TextSize.cy = size.cy;
		m_TextSize.cx = max ( m_TextSize.cx, size.cx, );

		// Set up scroll bar values
		m_pScrollbar->SetTrackRange ( m_TextSize.cx, m_vEntryList.size () );
		m_pScrollbar->GetVerScrollbar ()->SetTrackPos ( m_vEntryList.size () );
	}
}

void CEntryList::RemoveEntry ( SEntryItem *pEntry )
{
	if ( !IsEntryInList ( pEntry ) )
		return;

	m_vEntryList.erase ( std::find ( m_vEntryList.begin (), m_vEntryList.end (), pEntry ) );
	SAFE_DELETE ( pEntry );

	m_pScrollbar->SetTrackRange ( m_TextSize.cx, m_vEntryList.size () );
}

SEntryItem *CEntryList::GetEntryByIndex ( UINT nIndex )
{
	if ( nIndex >= m_vEntryList.size () )
		return NULL;

	return m_vEntryList [ nIndex ];
}

SIZE CEntryList::GetTextSize ( void )
{
	return m_TextSize;
}

void CEntryList::Render ( SControlRect rRect, D3DCOLOR d3dColorSelected,
						  D3DCOLOR d3dColorNormal,
						  D3DCOLOR d3dColorSelectedFont,
						  D3DCOLOR d3dColorFont, UINT uIndex )
{
	if ( !m_pScrollbar || 
		 !m_vEntryList.size () )
		return;

	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	int nHorScrollbarTrackPos = pScrollbarVer->GetTrackPos();
	rRect.m_pos.m_nX -= nHorScrollbarTrackPos + 4;

	// Draw all contexts
	for ( int i = pScrollbarVer->GetTrackPos (); i < nHorScrollbarTrackPos + pScrollbarVer->GetPageSize (); i++ )
	{
		if ( i < ( int ) GetSize () )
		{
			if (i != pScrollbarVer->GetTrackPos())
				rRect.m_pos.m_nY += GetTextSize().cy;

			SEntryItem *pEntry = GetEntryByIndex ( i );

			// Check for a valid 'pEntry' pointer and if text is not NULL
			if ( pEntry &&
				 pEntry->m_sText.c_str () != NULL )
			{
				if ( IsEntrySelected ( pEntry ) ||
					 uIndex == i )
				{
					SControlRect rBoxSel = rRect;
					rBoxSel.m_pos.m_nX -= 2 ;
					rBoxSel.m_pos.m_nY += 1;
					rBoxSel.m_size.cx = rRect.m_size.cx + nHorScrollbarTrackPos;
					rBoxSel.m_size.cy = GetTextSize ().cy - 1;

					D3DCOLOR d3dColor = d3dColorNormal;
					if ( IsEntrySelected ( pEntry ) )
						d3dColor = d3dColorSelected;

					m_pDialog->DrawBox ( rBoxSel, d3dColor, 0, false );
					m_pDialog->DrawFont ( rRect, d3dColorSelectedFont, pEntry->m_sText.c_str (), 0, m_pFont );
				}
				else
					m_pDialog->DrawFont ( rRect, d3dColorFont, pEntry->m_sText.c_str (), D3DFONT_COLORTABLE, m_pFont );
			}
		}
	}

	m_pScrollbar->OnDraw ();
}

bool CEntryList::IsEntryInList ( SEntryItem *pEntry )
{
	if ( !pEntry )
		return false;

	return std::find ( m_vEntryList.begin (), m_vEntryList.end (), pEntry ) != m_vEntryList.end ();
}

/*************************************************************************
Insert an item into the CEntryList after a specified item already in
the list.
*************************************************************************/
void CEntryList::InsertItem ( SEntryItem *pEntry, SEntryItem *pEntryPos )
{
	if ( !IsEntryInList ( pEntry ) )
		return;

	std::vector<SEntryItem*>::iterator iter;

	// If pEntryPos is NULL begin insert at begining, else insert after item 'pEntryPos'
	if ( !pEntryPos )
	{
		iter = m_vEntryList.begin ();
	}
	else
	{
		iter = std::find ( m_vEntryList.begin (), m_vEntryList.end (), pEntryPos );

		// Assert if item 'pEntryPos' is not in the list
		if ( iter == m_vEntryList.end () )
		{
			assert ( iter != m_vEntryList.end () && 
					 _UI ( "The specified SEntryItem for parameter 'pEntryPos' is not attached to this CEntryList." ) );
		}
	}

	m_vEntryList.insert ( iter, pEntry );
}

void CEntryList::SetSortedList ( bool bSort )
{
	m_bSort = bSort;
}

int CEntryList::GetIndexByEntry (  SEntryItem *pEntry )
{
	if ( !IsEntryInList ( pEntry ) )
		return -1;

	return std::distance ( m_vEntryList.begin (), std::find ( m_vEntryList.begin (), m_vEntryList.end (), pEntry ) );
}

void CEntryList::ClearSelection ( void )
{
	for ( auto entry : m_vEntryList )
	{
		// Check for a valid entry
		if ( entry )
		{
			// Deselect entry
			m_mSelectedItem [ entry ] = false;
		}
	}
}

void CEntryList::SetAllSelected ( void )
{
	for ( auto entry : m_vEntryList )
	{
		// Check for a valid entry
		if ( entry )
		{
			// Select entry
			m_mSelectedItem [ entry ] = true;
		}
	}
}

size_t CEntryList::GetSize ( void )
{
	return m_vEntryList.size ();
}

int CEntryList::GetItemAtPos ( SControlRect rRect, CVector pos )
{
	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();
	int iIndex = -1;

	rRect.m_size.cx -= ( m_pScrollbar->IsHorScrollbarNeeded () ? pScrollbarVer->GetWidth () + 3 : 0 );

	for ( int i = pScrollbarVer->GetTrackPos (); i < pScrollbarVer->GetTrackPos () + pScrollbarVer->GetPageSize (); i++ )
	{
		if ( i < ( int ) m_vEntryList.size() )
		{
			if (i != pScrollbarVer->GetTrackPos())
				rRect.m_pos.m_nY += m_TextSize.cy;

			SEntryItem *pEntry = GetEntryByIndex ( i );

			// Check for a valid 'pEntry' pointer and if text is not NULL
			// and determine which item has been selected
			if ( pEntry &&
				 pEntry->m_sText.c_str () != NULL &&
				 rRect.ContainsPoint ( pos ) )
			{
				iIndex = i;
				break;
			}
		}
	}

	return iIndex;
}

SEntryItem *CEntryList::GetPrevItem ( SEntryItem *pEntry )
{
	for ( size_t i = 0; i < m_vEntryList.size (); i++ )
	{
		if ( m_vEntryList [ i ] )
		{
			if ( m_vEntryList [ i ] == pEntry && int ( i - 1 ) > 0 )
				return m_vEntryList [ i - 1 ];
		}
	}

	return NULL;
}

SEntryItem *CEntryList::GetNextItem ( SEntryItem *pEntry )
{
	size_t size = m_vEntryList.size ();
	for ( size_t i = 0; i < size; i++ )
	{
		if ( m_vEntryList [ i ] )
		{
			if ( m_vEntryList [ i ] == pEntry && i + 1 < size )
				return m_vEntryList [ i + 1 ];
		}
	}

	return NULL;
}

void CEntryList::ResetList ( void )
{
	for ( auto entry : m_vEntryList )
	{
		SAFE_DELETE ( entry );
	}

	m_vEntryList.clear ();
}

void CEntryList::SetMultiSelection ( bool bMultipleSelected )
{
	m_bMultipleSelected = bMultipleSelected;
}

void CEntryList::SetSelectedEntryByIndex ( UINT nIndex, bool bSelect )
{
	if ( nIndex >= m_vEntryList.size () )
		return;

	SetSelectedEntry ( GetEntryByIndex ( nIndex ), bSelect );
}

void CEntryList::SetSelectedEntry ( SEntryItem *pEntry, bool bSelect )
{
	if ( !pEntry ) 
		return;

	// Enter new selected entry
	m_mSelectedItem [ pEntry ] = bSelect;

	// If multiple selected is unable, leave old selected entry 
	if ( !m_bMultipleSelected )
	{	
		if ( m_pSelectedEntry )
			m_mSelectedItem [ m_pSelectedEntry ] = false;
	}

	m_pSelectedEntry = pEntry;

	m_pScrollbar->GetVerScrollbar ()->ShowItem ( GetIndexByEntry ( pEntry ) );
}

bool CEntryList::IsEntrySelectedByIndex ( UINT nIndex )
{
	if ( nIndex >= m_vEntryList.size () )
		return false;

	return IsEntrySelected ( GetEntryByIndex ( nIndex ) );
}

bool CEntryList::IsEntrySelected (  SEntryItem *pEntry )
{
	return ( pEntry && m_mSelectedItem [ pEntry ] );
}

SEntryItem *CEntryList::GetSelectedEntry ( void )
{
	if ( m_pSelectedEntry &&
		 m_mSelectedItem [ m_pSelectedEntry ] )
	{
		return m_pSelectedEntry;
	}

	return NULL;
}

int CEntryList::GetSelectedEntryIndex ( void )
{
	SEntryItem *pEntry = GetSelectedEntry ();
	if ( pEntry )
		return GetIndexByEntry ( pEntry );

	return -1;
}

void CEntryList::UpdateScrollbars ( SControlRect rRect )
{
	if ( !m_pScrollbar )
		return;

	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	// Set up scroll bar values
	m_pScrollbar->SetPageSize ( rRect.m_size.cx - ( pScrollbarHor->GetHeight () + 4 ), rRect.m_size.cy / m_TextSize.cy );
	m_pScrollbar->UpdateScrollbars ( rRect );
}

bool CEntryList::ContainsRects ( SControlRect rRect, CVector pos )
{
	if ( !m_pScrollbar )
		return false;

	return ( m_pScrollbar->ContainsPoint ( pos ) ||
			 rRect.ContainsPoint ( pos ) );
}

CScrollablePane *CEntryList::GetScrollbar ( void )
{
	return m_pScrollbar;
}