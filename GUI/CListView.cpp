#include "CListView.h"

#define TEXTBOX_TEXTSPACE 18
#define LISTVIEW_TITLESIZE 8
#define LISTVIEW_MINCOLUMNSIZE 50

UINT CListView::m_nColumnSort = 0;

CListView::CListView ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_LISTVIEW );

	m_sControlColor.d3dColorFont = D3DCOLOR_RGBA ( 0, 0, 0, 255 );
	m_sControlColor.d3dColorSelectedFont = D3DCOLOR_RGBA ( 255, 255, 255, 255 );

	m_nSelected = m_nIndex = m_nOverColumnId = -1;
	m_nRowSize = 0;
	m_bSizable = m_bMovable = m_bSortable = true;

	m_pScrollbar = new CScrollablePane ( pDialog );

	m_pScrollbar->AddControl ( this );
	m_pDialog->LoadFont ( _UI ( "Arial" ), 10, true, &m_pTitleFont );
}

CListView::~CListView ( void )
{
	RemoveAllColumns ();
	SAFE_DELETE ( m_pScrollbar );
}

void CListView::AddColumn ( const SIMPLEGUI_CHAR *szColumnName, int nWidth )
{
	if ( EMPTYCHAR ( szColumnName ) )
		return;

	if ( nWidth < LISTVIEW_MINCOLUMNSIZE )
	{
		nWidth = LISTVIEW_MINCOLUMNSIZE;
	}

	SListViewColumn sColumn;
	sColumn.m_nWidth = nWidth;
	sColumn.m_sColumnName = szColumnName;

	m_vColumnList.push_back ( sColumn );
}

void CListView::RemoveColumn ( UINT nColumnId )
{
	if ( nColumnId >= m_vColumnList.size () )
		return;

	RemoveAllItemsFromColumn ( nColumnId );
	m_vColumnList.erase ( m_vColumnList.begin () + nColumnId );
}

void CListView::RemoveAllColumns ( void )
{
	RemoveAllItems ();
	m_vColumnList.clear ();
}

void CListView::RemoveAllItemsFromColumn ( UINT nColumnId )
{
	if ( nColumnId >= m_vColumnList.size () )
		return;

	m_vColumnList [ nColumnId ].m_sItem.clear ();
}

void CListView::RemoveAllItems ( void )
{
	for ( auto &column : m_vColumnList )
	{
		column.m_sItem.clear ();
	}
}

void CListView::SetColumnName ( UINT nColumnId, const SIMPLEGUI_CHAR *szColumnName )
{
	if ( nColumnId >= m_vColumnList.size () || 
		 EMPTYCHAR ( szColumnName ) )
		return;

	m_vColumnList [ nColumnId ].m_sColumnName = szColumnName;
}

const SIMPLEGUI_CHAR *CListView::GetColumnName ( UINT nColumnId )
{
	if ( nColumnId >= m_vColumnList.size () )
		return NULL;

	return m_vColumnList [ nColumnId ].m_sColumnName.c_str ();
}

const SEntryItem *CListView::GetColumnItemByRow ( UINT nColumnId, UINT nRow )
{
	if ( nColumnId >= m_vColumnList.size () ||
		 nRow >= m_vColumnList [ nColumnId ].m_sItem.size () )
	{
		return NULL;
	}

	return m_vColumnList [ nColumnId ].m_sItem [ nRow ];
}

void CListView::SetColumnWidth ( UINT nColumnId, int nWidth )
{
	if ( nColumnId >= m_vColumnList.size () )
		return;

	if ( nWidth < LISTVIEW_MINCOLUMNSIZE )
	{
		nWidth = LISTVIEW_MINCOLUMNSIZE;
	}

	m_vColumnList [ nColumnId ].m_nWidth = nWidth;

	// Set up horizontal scroll bar range
	if ( !m_bSizing )
		m_pScrollbar->SetTrackRange ( GetAllColumnsWidth (), 0 );
}

int CListView::GetColumnWidth ( UINT nColumnId )
{
	if ( nColumnId >= m_vColumnList.size () )
		return 0;

	return m_vColumnList [ nColumnId ].m_nWidth;
}

void CListView::AddColumnItem ( UINT nColumnId, const SIMPLEGUI_CHAR *szItem, const SIMPLEGUI_CHAR *szValue )
{
	AddColumnItem ( nColumnId, new SEntryItem ( szItem, szValue ) );
}

void CListView::AddColumnItem ( UINT nColumnId, SEntryItem *pEntry )
{
	if ( nColumnId >= m_vColumnList.size () ||
		 !pEntry && EMPTYCHAR ( pEntry->m_sText.c_str () ) )
		return;

	m_vColumnList [ nColumnId ].m_sItem.push_back ( pEntry );
	m_nRowSize = max ( m_nRowSize, m_vColumnList [ nColumnId ].m_sItem.size () );

	// Set up scroll bar ranges
	m_pScrollbar->SetTrackRange ( GetAllColumnsWidth (), m_nRowSize );
}

void CListView::SetColumnItemName ( UINT nColumnId, UINT nIndex, const SIMPLEGUI_CHAR *szItem )
{
	if ( nColumnId >= m_vColumnList.size () || 
		 nIndex >= m_vColumnList [ nColumnId ].m_sItem.size () ||
		 EMPTYCHAR ( szItem ) )
		return;

	m_vColumnList [ nColumnId ].m_sItem [ nIndex ]->m_sText = szItem;
}

void CListView::RemoveColumnItem ( UINT nColumnId, UINT nIndex )
{
	if ( nColumnId >= m_vColumnList.size () ||  
		 nIndex >= m_vColumnList [ nColumnId ].m_sItem.size () )
		return;

	m_vColumnList [ nColumnId ].m_sItem.erase ( m_vColumnList [ nColumnId ].m_sItem.begin () + nIndex );
}

size_t CListView::GetNumOfColumns ( void )
{
	return m_vColumnList.size ();
}

size_t CListView::GetNumOfItemsFromColumn ( UINT nColumnId )
{
	if ( nColumnId >= m_vColumnList.size () )
		return 0;
	
	return m_vColumnList [ nColumnId ].m_sItem.size ();
}

size_t CListView::GetNumOfItems ( void )
{
	return m_nRowSize;
}

int CListView::GetAllColumnsWidth ( void )
{
	int nWidth = 0;
	for ( auto column : m_vColumnList )
	{
		nWidth += column.m_nWidth;
	}

	return nWidth;
}

const SEntryItem *CListView::GetSelectedItem ( UINT nColumnId )
{
	if ( nColumnId >= m_vColumnList.size () ||
		 m_nSelected < 0 ||
		 m_nSelected >= m_vColumnList [ nColumnId ].m_sItem.size () )
	{
		return NULL;
	}

	return m_vColumnList [ nColumnId ].m_sItem [ m_nSelected ];
}

int CListView::GetColumnIdAtArea ( CPos pos )
{
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	SControlRect rRect = m_rBoundingBox;
	rRect.pos.SetX ( rRect.pos.GetX () - pScrollbarHor->GetTrackPos () );

	for ( size_t i = 0; i < m_vColumnList.size (); i++ )
	{
		rRect.pos.SetX ( rRect.pos.GetX () + ( i ? GetColumnWidth ( i - 1 ) : 0 ) );
		rRect.size.cx = GetColumnWidth ( i );
		rRect.size.cy = m_rColumnArea.size.cy;

		if ( rRect.InControlArea ( pos ) )
		{
			return i;
		}
	}

	return -1;
}

int CListView::GetColumnIdAtAreaBorder ( CPos pos )
{
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	SControlRect rRect = m_rBoundingBox;
	rRect.pos.SetX ( rRect.pos.GetX () - pScrollbarHor->GetTrackPos () - 4 );

	for ( size_t i = 0; i < m_vColumnList.size (); i++ )
	{
		rRect.pos.SetX ( rRect.pos.GetX () + GetColumnWidth ( i ) );
		rRect.size.cx = 4;
		rRect.size.cy = m_rColumnArea.size.cy;

		if ( rRect.InControlArea ( pos ) )
		{
			return i;
		}
	}

	return -1;
}

int CListView::GetColumnOffset ( UINT nColumnId )
{
	if ( nColumnId >= m_vColumnList.size () )
		return 0;

	UINT nId = 0;
	int nX = m_rBoundingBox.pos.GetX ();

	while ( nColumnId != nId )
	{
		nX += GetColumnWidth ( nId );
		nId++;
	}

	return nX;
}

const SEntryItem *CListView::FindItemInRow ( UINT nRow )
{
	UINT nColumnId = 0;
	while ( nColumnId < m_vColumnList.size () )
	{
		const SEntryItem *pEntry = GetColumnItemByRow ( nColumnId, nRow );
		if ( pEntry )
			return pEntry;

		nColumnId++;
	}

	return NULL;
}

void CListView::MoveColumn ( UINT nColumnId, UINT nPosition )
{
	if ( nColumnId >= m_vColumnList.size () || 
		 nColumnId == nPosition ) 
		return;

	// if position is too big, insert at end.
	if ( nPosition > m_vColumnList.size () )
	{
		nPosition = m_vColumnList.size () - 1;
	}

	SListViewColumn sTmp = m_vColumnList [ nColumnId ];
	m_vColumnList.erase ( m_vColumnList.begin () + nColumnId );
	m_vColumnList.insert ( m_vColumnList.begin () + nPosition, sTmp );
}

void CListView::SortColumn ( UINT nColumnId )
{
	if ( !m_bSortable )
		return;

	ColumnItem mColumnItem;
	std::vector<ColumnItem>	vColumnItemList;

	for ( size_t i = 0; i < m_nRowSize; i++ )
	{
		for ( size_t j = 0; j < GetNumOfColumns (); j++ )
		{
			if ( m_nRowSize > m_vColumnList [ j ].m_sItem.size () )
			{
				m_vColumnList [ j ].m_sItem.resize ( m_nRowSize );
			}
			mColumnItem [ j ] = m_vColumnList [ j ].m_sItem [ i ];
		}
		vColumnItemList.push_back ( mColumnItem );
	}

	m_nColumnSort = nColumnId;
	if ( std::is_sorted ( vColumnItemList.begin (), vColumnItemList.end (), ColumnItemLess ) )
	{
		std::sort ( vColumnItemList.begin (), vColumnItemList.end (), ColumnItemGreater );
	}
	else
	{
		std::sort ( vColumnItemList.begin (), vColumnItemList.end (), ColumnItemLess );
	}

	for ( size_t i = 0; i < m_nRowSize; i++ )
	{
		for ( size_t j = 0; j < GetNumOfColumns (); j++ )
			m_vColumnList [ j ].m_sItem [ i ] = vColumnItemList [ i ] [ j ];
	}
}

void CListView::SetSortable ( bool bSortable )
{
	m_bSortable = bSortable;
}

void CListView::SetTitleSizable ( bool bSizable )
{
	m_bSizable = bSizable;
}

void CListView::SetTitleMovable ( bool bMovable )
{
	m_bMovable = bMovable;
}

void CListView::Draw ( void )
{
	if ( !m_bVisible ||
		 m_vColumnList.empty () )
	{
		return;
	}

	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	m_pDialog->DrawBox ( m_rBoundingBox, m_sControlColor.d3dColorBoxBack, m_sControlColor.d3dColorOutline );

	int nVerScrollTrackPos = pScrollbarVer->GetTrackPos ();
	int nHorScrollTrackPos = pScrollbarHor->GetTrackPos ();

	m_rColumnArea.pos.SetX ( m_rColumnArea.pos.GetX () + 4 - nHorScrollTrackPos );

	CD3DRender *pRender = m_pDialog->GetRenderer ();
	pRender->D3DLine ( m_rColumnArea.pos.GetX (), m_rColumnArea.pos.GetY () + m_rColumnArea.size.cy,
					   m_rColumnArea.pos.GetX () + m_rColumnArea.size.cx + nHorScrollTrackPos,
					   m_rColumnArea.pos.GetY () + m_rColumnArea.size.cy, m_sControlColor.d3dColorOutline );

	SControlRect rListBoxText = m_rListBoxArea;
	rListBoxText.pos.SetX ( rListBoxText.pos.GetX () + 4 - nHorScrollTrackPos );
	
	RECT rOldScissor;
	m_pDialog->GetDevice ()->GetScissorRect ( &rOldScissor );

	SControlRect rScissor = m_rScissor;
	rScissor.pos.SetX ( rScissor.pos.GetX () + 1 );
	rScissor.pos.SetY ( rScissor.pos.GetY () + 1 );
	rScissor.size.cx -= m_pScrollbar->IsVerScrollbarNeeded () ? pScrollbarVer->GetWidth () : 2;
	SetScissor ( m_pDialog->GetDevice (), rScissor.GetRect () );

	// Draw all contexts
	for ( size_t i = 0; i < m_vColumnList.size (); i++ )
	{
		int nPrevColumnWidth = ( i ? GetColumnWidth ( i - 1 ) : 0 );
		int nColumnWidth = GetColumnWidth ( i );

		m_rColumnArea.pos.SetX ( m_rColumnArea.pos.GetX () + nPrevColumnWidth );

		D3DCOLOR d3dColorColumn = m_sControlColor.d3dColorBox [ SControlColor::STATE_NORMAL ];
		if ( i == m_nOverColumnId )
		{
			if ( m_nId == i &&
				 ( m_bSorting || m_bMoving ) )
			{
				d3dColorColumn = m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ];
			}
			else
			{
				d3dColorColumn = m_sControlColor.d3dColorBox [ SControlColor::STATE_MOUSE_OVER ];
			}
		}

		rListBoxText.pos.SetX ( rListBoxText.pos.GetX () + nPrevColumnWidth );
		rListBoxText.pos.SetY ( m_rListBoxArea.pos.GetY () );

		SControlRect rColumnBox = m_rColumnArea;
		rColumnBox.pos.SetX ( m_rColumnArea.pos.GetX () - 4 );
		rColumnBox.size.cx = nColumnWidth;
		m_pDialog->DrawBox ( rColumnBox, d3dColorColumn, m_sControlColor.d3dColorOutline );

		SIMPLEGUI_STRING strColumnName = GetColumnName ( i );
		m_pTitleFont->CutString ( nColumnWidth - 4, strColumnName );
		m_pDialog->DrawFont ( SControlRect ( m_rColumnArea.pos.GetX () + nColumnWidth / 2 - 4, m_rColumnArea.pos.GetY () + m_rColumnArea.size.cy / 2 ),
							  m_sControlColor.d3dColorFont, strColumnName.c_str (), D3DFONT_COLORTABLE | D3DFONT_CENTERED_X | D3DFONT_CENTERED_Y,
							  m_pTitleFont );

		if ( i != 0 )
		{
			pRender->D3DLine ( m_rColumnArea.pos.GetX () - 4, m_rColumnArea.pos.GetY (), m_rColumnArea.pos.GetX () - 4,
							   m_rColumnArea.pos.GetY () + m_rBoundingBox.size.cy,
							   m_sControlColor.d3dColorOutline );
		}

		for ( size_t j = nVerScrollTrackPos; j < nVerScrollTrackPos + pScrollbarVer->GetPageSize (); j++ )
		{
			if ( rListBoxText.pos.GetX () < m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cx )
			{
				SIMPLEGUI_STRING str;
				const SEntryItem *pEntry = GetColumnItemByRow ( i, j );
				if ( pEntry )
					 str = pEntry->m_sText;

				SIZE size;
				m_pFont->GetTextExtent ( str.c_str (), &size );
				rListBoxText.pos.SetY ( rListBoxText.pos.GetY () + size.cy );

				D3DCOLOR d3dColorFont = m_sControlColor.d3dColorFont;

				if ( m_nSelected == j ||
					 m_nIndex == j )
				{
					SControlRect rBoxSel = rListBoxText;
					rBoxSel.pos.SetX ( rListBoxText.pos.GetX () - 2 );
					rBoxSel.pos.SetY ( rBoxSel.pos.GetY () );
					rBoxSel.size.cx = rListBoxText.size.cx + nHorScrollTrackPos;
					rBoxSel.size.cy = size.cy - 1;

					d3dColorFont = m_sControlColor.d3dColorSelectedFont;
					D3DCOLOR d3dColorBox = m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ];

					if ( m_nSelected == j )
						d3dColorBox = m_sControlColor.d3dColorBoxSel;

					m_pDialog->DrawBox ( rBoxSel, d3dColorBox, 0, false );
				}

				if ( pEntry &&
					 !str.empty () )
				{
					m_pFont->CutString ( GetColumnWidth ( i ) - 4, str );
					m_pDialog->DrawFont ( rListBoxText, d3dColorFont, str.c_str (), D3DFONT_COLORTABLE, m_pFont );
				}
			}
		}
	}

	CPos mPos = m_pDialog->GetMouse ()->GetPos ();
	if ( m_bMoving )
	{
		int nColumnWidth = GetColumnWidth ( m_nId );
		int nId = GetColumnIdAtArea ( mPos );
		int nOffset = GetColumnOffset ( nId == -1 ? m_vColumnList.size () - 1 : nId ) - nHorScrollTrackPos;

		SIMPLEGUI_STRING szStr = GetColumnName ( m_nId );

		SControlRect rRect = m_rBoundingBox;
		rRect.pos.SetX ( ( mPos.GetX () - m_nDragX ) - nHorScrollTrackPos );
		rRect.size.cx = nColumnWidth;
		rRect.size.cy = m_rColumnArea.size.cy;

		D3DCOLOR color	= D3DCOLOR_ARGB ( 140, 100, 100, 100 );
		D3DCOLOR color1 = D3DCOLOR_ARGB ( 80, 0, 0, 0 );
		D3DCOLOR color2 = D3DCOLOR_ARGB ( 255, 0, 0, 200 );

		pRender->D3DLine ( nOffset, m_rBoundingBox.pos.GetY (), nOffset, m_rBoundingBox.pos.GetY () + m_rColumnArea.size.cy, color2 );
		pRender->D3DLine ( nOffset + 1, m_rBoundingBox.pos.GetY (), nOffset + 1, m_rBoundingBox.pos.GetY () + m_rColumnArea.size.cy, color2 );

		m_pDialog->DrawBox ( rRect, color, m_sControlColor.d3dColorOutline );

		m_pTitleFont->CutString ( nColumnWidth - 4, szStr );
		m_pDialog->DrawFont ( SControlRect ( rRect.pos.GetX () + nColumnWidth / 2, rRect.pos.GetY () + m_rColumnArea.size.cy / 2 ),
							  color1, szStr.c_str (),
							  D3DFONT_COLORTABLE | D3DFONT_CENTERED_X | D3DFONT_CENTERED_Y, m_pTitleFont );
	}
	else if ( m_bSizing )
	{
		int nOffset = GetColumnOffset ( m_nId );
		if ( nOffset >= mPos.GetX () )
			mPos.SetX ( nOffset );

		D3DCOLOR d3dLineColor = m_sControlColor.d3dColorOutline;
		if ( nOffset + LISTVIEW_MINCOLUMNSIZE >= mPos.GetX () )
			d3dLineColor = D3DCOLOR_XRGB ( 200, 0, 0 );

		pRender->D3DLine ( mPos.GetX (), m_rBoundingBox.pos.GetY () + 1, mPos.GetX (), m_rBoundingBox.pos.GetY () + m_rColumnArea.size.cy, d3dLineColor );
		pRender->D3DLine ( mPos.GetX () + 1, m_rBoundingBox.pos.GetY () + 1, mPos.GetX () + 1, m_rBoundingBox.pos.GetY () + m_rColumnArea.size.cy, d3dLineColor );
	}

	rScissor.size.cx = m_rScissor.size.cx - 2;
	SetScissor ( m_pDialog->GetDevice (), rScissor.GetRect () );
	m_pScrollbar->OnDraw ();
}

void CListView::OnClickLeave ( void )
{
	CControl::OnClickLeave ();
	m_pScrollbar->OnClickLeave ();

	m_nId = m_nOverColumnId = -1;
	m_bSizing = m_bMoving  = false;
}

bool CListView::OnClickEvent ( void )
{
	return ( CControl::OnClickEvent () ||
			 m_pScrollbar->OnClickEvent () ||
			 m_bSizing || m_bMoving );
}

void CListView::OnFocusIn ( void )
{
	CControl::OnFocusIn ();

	if ( m_pScrollbar )
	{
		m_pScrollbar->OnFocusIn ();
	}
}

void CListView::OnFocusOut ( void )
{
	CControl::OnFocusOut ();

	if ( m_pScrollbar )
	{
		m_pScrollbar->OnFocusOut ();
	}
}

void CListView::OnMouseEnter ( void )
{
	CControl::OnMouseEnter ();

	if ( m_pScrollbar )
	{
		m_pScrollbar->OnMouseEnter ();
	}
}

void CListView::OnMouseLeave ( void )
{
	CControl::OnMouseLeave ();

	if ( !m_bSizing &&
		 !m_bMoving )
	{
		m_nOverColumnId = m_nIndex = -1;
	}

	if ( m_pScrollbar )
	{
		m_pScrollbar->OnMouseLeave ();
	}
}

bool CListView::CanHaveFocus ( void )
{
	return ( CControl::CanHaveFocus () ||
			 m_pScrollbar->CanHaveFocus () );
}

bool CListView::OnMouseButtonDown ( sMouseEvents e )
{
	m_pParent->SetFocussedControl ( this );

	if ( !m_bSizing &&
		 !m_bMoving )
	{
		if ( m_pScrollbar->OnMouseButtonDown ( e ) )
			return true;
	}

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		if ( m_bSizable )
		{
			m_nId = GetColumnIdAtAreaBorder ( e.pos );
			if ( m_nId > -1 )
			{
				m_bPressed = m_bSizing = true;
				m_nDragX = m_rBoundingBox.pos.GetX () + GetColumnWidth ( m_nId ) - e.pos.GetX ();
				return true;
			}
		}

		m_nId = GetColumnIdAtArea ( e.pos );
		if ( m_nId > -1 && m_bSortable )
		{
			m_bPressed = m_bSorting = true;
		}

		if ( m_bMovable )
		{
			if ( m_nId > -1 )
			{
				m_bPressed = m_bMoving = true;
				m_nDragX = e.pos.GetX () - GetColumnOffset ( m_nId );
				return true;
			}
		}

		if ( m_rBoundingBox.InControlArea ( e.pos ) )
		{
			// Pressed while inside the control
			m_bPressed = true;
			return true;
		}
	}

	return false;
}

bool CListView::OnMouseButtonUp ( sMouseEvents e )
{
	if ( !m_bSizing &&
		 !m_bMoving )
	{
		if ( m_pScrollbar->OnMouseButtonUp ( e ) )
			return true;
	}

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		if ( m_bMoving )
		{
			int nId = GetColumnIdAtArea ( e.pos );
			nId == -1 ? m_vColumnList.size () - 1 : nId;

			MoveColumn ( m_nId, nId );
			m_bMoving = false;
		}

		if ( m_bSizing )
		{
			SetColumnWidth ( m_nId, e.pos.GetX () - m_rBoundingBox.pos.GetX () + m_nDragX );
			m_pScrollbar->SetTrackRange ( GetAllColumnsWidth (), 0 );
			m_bSizing = false;
		}

		if ( m_bSorting )
		{
			SortColumn ( m_nId );
			m_bSorting = false;
		}

		if ( m_bPressed )
		{
			m_bPressed = false;

			if ( m_rListBoxArea.InControlArea ( e.pos ) )
			{
				if ( m_nIndex != -1 )
				{
					m_nSelected = m_nIndex;
				}

				SendEvent ( EVENT_CONTROL_SELECT, m_nSelected );
				return true;
			}
		}
	}
	return false;
}

bool CListView::OnMouseMove ( CPos pos )
{
	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	if ( !CanHaveFocus () ||
		 !pScrollbarVer ||
		 !pScrollbarHor )
	{
		return false;
	}

	if ( m_bMouseOver && !OnClickEvent () )
	{
		m_nOverColumnId = GetColumnIdAtArea ( pos );
	}

	if ( !m_bSizing &&
		 !m_bMoving )
	{
		if ( m_pScrollbar->OnMouseMove ( pos ) )
			return true;
	}

	m_nIndex = -1;

	/*if ( ( GetAsyncKeyState ( VK_LBUTTON ) && !m_bHasFocus ) ||
		 m_pScrollbar->ContainsRect ( pos ) ||
		 m_pScrollbar->OnClickEvent () )
	{
		return false;
	}*/

	int nId = GetColumnIdAtAreaBorder ( pos );
	if ( m_bMouseOver &&m_bMoving )
	{
		m_bSorting = false;
		return true;
	}
	else if ( m_bMouseOver && m_bSizing || nId > -1 )
	{
		if ( nId > -1 )
		{
			m_pDialog->GetMouse ()->SetCursorType ( CMouse::E_RESIZE );
		}
		return true;
	}

	if ( !m_bMouseOver ) 
		return false;

	SControlRect rText = m_rListBoxArea;
	rText.pos.SetX ( rText.pos.GetX () + 4 );
	rText.size.cx -= ( pScrollbarVer->GetWidth () + 4 );
	rText.size.cy = TEXTBOX_TEXTSPACE - 2;

	for ( int i = pScrollbarVer->GetTrackPos (); i < pScrollbarVer->GetTrackPos () + pScrollbarVer->GetPageSize (); i++ )
	{
		if ( i < m_nRowSize )
		{
			SIMPLEGUI_STRING str;
			const SEntryItem *pEntry = FindItemInRow ( i );

			if ( pEntry )
				str = pEntry->m_sText.c_str ();

			SIZE size;
			m_pFont->GetTextExtent ( str.c_str (), &size );

			rText.pos.SetY ( rText.pos.GetY () + size.cy );

			// Check if selected text is not NULL and determine 
			// which item has been selected
			if ( str.c_str () != NULL && rText.InControlArea ( pos ) )
			{
				m_nIndex = i;
				return true;
			}
		}
	}

	return false;
}

bool CListView::OnMouseWheel ( int zDelta )
{
	UINT uLines;
	SystemParametersInfo ( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
	m_pScrollbar->GetVerScrollbar ()->Scroll ( -zDelta * uLines );
	return true;
}

bool CListView::HandleMouse ( UINT uMsg, CPos pos, WPARAM wParam, LPARAM lParam )
{
	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	if ( !CanHaveFocus () ||
		 !pScrollbarVer ||
		 !pScrollbarHor )
		return false;

	if ( !OnClickEvent () )
	{
		m_nOverColumnId = GetColumnIdAtArea ( pos );
	}

	if ( !m_bSizing &&
		 !m_bMoving )
	{
	/*	if ( m_pScrollbar->HandleMouse ( uMsg, pos, wParam, lParam ) )
			return true;*/
	}

	switch ( uMsg )
	{
		case WM_MOUSEMOVE:
		{
			m_nIndex = -1;

			if ( ( GetAsyncKeyState ( VK_LBUTTON ) &&
				 !m_bHasFocus ) ||
				 m_pScrollbar->ContainsRect ( pos ) ||
				 m_pScrollbar->OnClickEvent () )
			{
				break;
			}

			int nId = GetColumnIdAtAreaBorder ( pos );
			if ( m_bMoving )
			{
				m_bSorting = false;
				return true;
			}
			else if ( m_bSizing || nId > -1 )
			{
				if ( nId > -1 )
				{
					m_pDialog->GetMouse ()->SetCursorType ( CMouse::E_RESIZE );
				}
				return true;
			}

			SControlRect rText = m_rListBoxArea;
			rText.pos.SetX ( rText.pos.GetX () + 4 );
			rText.size.cx -= ( pScrollbarVer->GetWidth () + 4 );
			rText.size.cy = TEXTBOX_TEXTSPACE - 2;

			for ( int i = pScrollbarVer->GetTrackPos (); i < pScrollbarVer->GetTrackPos () + pScrollbarVer->GetPageSize (); i++ )
			{
				if ( i < m_nRowSize )
				{
					SIMPLEGUI_STRING str;
					const SEntryItem *pEntry = FindItemInRow ( i );

					if ( pEntry )			
						str = pEntry->m_sText.c_str ();

					SIZE size;
					m_pFont->GetTextExtent ( str.c_str (), &size );

					rText.pos.SetY ( rText.pos.GetY () + size.cy );

					// Check if selected text is not NULL and determine 
					// which item has been selected
					if ( str.c_str () != NULL &&
						 rText.InControlArea ( pos ) )
					{
						m_nIndex = i;
						return true;
					}
				}
			}

			break;
		}

		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			m_pParent->SetFocussedControl ( this );

			if ( m_bSizable )
			{
				m_nId = GetColumnIdAtAreaBorder ( pos );
				if ( m_nId > -1 )
				{
					m_bPressed = m_bSizing = true;
					m_nDragX = m_rBoundingBox.pos.GetX () + GetColumnWidth ( m_nId ) - pos.GetX ();
					return true;
				}
			}

			m_nId = GetColumnIdAtArea ( pos );
			if ( m_nId > -1 &&
				 m_bSortable )
			{
				m_bPressed = m_bSorting = true;
			}

			if ( m_bMovable )
			{
				if ( m_nId > -1 )
				{
					m_bPressed = m_bMoving = true;
					m_nDragX = pos.GetX ()- GetColumnOffset ( m_nId );
					return true;
				}
			}

			if ( m_rBoundingBox.InControlArea ( pos ) )
			{
				// Pressed while inside the control
				m_bPressed = true;
				return true;
			}

			break;
		}

		case WM_LBUTTONUP:
		{
			if ( m_bMoving )
			{
				int nId = GetColumnIdAtArea ( pos );
				nId == -1 ? m_vColumnList.size () - 1 : nId;

				MoveColumn ( m_nId, nId );
				m_bMoving = false;
			}

			if ( m_bSizing )
			{
				SetColumnWidth ( m_nId, pos.GetX () - m_rBoundingBox.pos.GetX () + m_nDragX );
				m_pScrollbar->SetTrackRange ( GetAllColumnsWidth (), 0 );
				m_bSizing = false;
			}

			if ( m_bSorting )
			{
				SortColumn ( m_nId );
				m_bSorting = false;
			}
			
			if ( m_bPressed )
			{
				m_bPressed = false;

				if ( m_rListBoxArea.InControlArea ( pos ) )
				{
					if ( m_nIndex != -1 )
					{
						m_nSelected = m_nIndex;
					}

					SendEvent ( EVENT_CONTROL_SELECT, m_nSelected );
					return true;
				}
			}

			break;
		}

		case WM_MOUSEWHEEL:
		{
			UINT uLines;
			SystemParametersInfo ( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
			int zDelta = ( short ) HIWORD ( wParam ) / WHEEL_DELTA;

			pScrollbarVer->Scroll ( -zDelta * uLines );

			return true;
		}
	};

	return false;
}

bool CListView::HandleKeyboard ( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	if ( !CanHaveFocus () ||
		 !pScrollbarVer ||
		 !pScrollbarHor )
	{
		return false;
	}

	switch ( uMsg )
	{
		case WM_KEYDOWN:
		{
			switch ( wParam )
			{
				case VK_LEFT:
				case VK_UP:
				{
					if ( m_nIndex > 0 )
					{
						m_nIndex--;
						m_nSelected = m_nIndex;
						pScrollbarVer->ShowItem ( m_nSelected );

						SendEvent ( EVENT_CONTROL_SELECT, m_nSelected );
					}
					else
					{
						m_nSelected = m_nIndex = m_nRowSize - 1;
						pScrollbarVer->Scroll ( m_nRowSize - 1 );
					}
					return true;
				}

				case VK_RIGHT:
				case VK_DOWN:
				{
					if ( m_nIndex + 1 < m_nRowSize )
					{
						m_nIndex++;
						m_nSelected = m_nIndex;
						pScrollbarVer->ShowItem ( m_nSelected );
					}
					else
					{
						m_nSelected = m_nIndex = 0;
						pScrollbarVer->ShowItem ( 0 );
					}

					SendEvent ( EVENT_CONTROL_SELECT, m_nSelected );

					return true;
				}
			}
			break;
		}
	}

	return false;
}

void CListView::UpdateRects ( void )
{
	if ( !m_pScrollbar )
		return;

	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();
	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();

	CControl::UpdateRects ();

	SIZE size;
	m_pTitleFont->GetTextExtent ( _UI ( "Y" ), &size );

	m_rColumnArea = m_rBoundingBox;
	m_rColumnArea.size.cy = size.cy + LISTVIEW_TITLESIZE;

	m_rListBoxArea = m_rBoundingBox;
	m_rListBoxArea.pos.SetY ( m_rListBoxArea.pos.GetY () + m_rColumnArea.size.cy / 2 );
	m_rListBoxArea.size.cy = m_rListBoxArea.size.cy - m_rColumnArea.size.cy;

	m_pFont->GetTextExtent ( _UI ( "Y" ), &size );

	// Set up scroll bar values
	m_pScrollbar->SetPageSize ( m_rListBoxArea.size.cx - ( m_pScrollbar->IsVerScrollbarNeeded () ? pScrollbarHor->GetHeight () : 0 ),
								m_rListBoxArea.size.cy / size.cy );


	pScrollbarHor->SetStepSize ( GetAllColumnsWidth () / 10 );

	m_pScrollbar->UpdateScrollbars ( m_rBoundingBox );
}

bool CListView::ContainsRect ( CPos pos )
{
	if ( !CanHaveFocus () )
		return false;

	return ( m_pScrollbar->ContainsRect ( pos ) || 
			 m_rBoundingBox.InControlArea ( pos ) );
}