#include "CListView.h"

#define TEXTBOX_TEXTSPACE 18
#define LISTVIEW_TITLESIZE 15
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
	d_columnCount = 0;
	m_pScrollbar = new CScrollablePane ( pDialog );

	m_pScrollbar->SetControl ( this );
	m_pDialog->LoadFont ( _UI ( "Arial" ), 10, true, &m_pTitleFont );
}

CListView::~CListView ( void )
{
	RemoveAllColumns ();
	SAFE_DELETE ( m_pScrollbar );
}

void CListView::InsertColumn ( const SIMPLEGUI_CHAR *szColumnName, int nWidth, UINT nPos )
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
	d_columnCount++;

	//// Insert a blank entry at the appropriate position in each row.
	//for ( UINT i = 0; i < m_nRowSize; ++i )
	//{
	//	d_grid [ i ].d_items.insert (
	//		d_grid [ i ].d_items.begin () + nPos,
	//		static_cast<SEntryItem*>( 0 ) );
	//}
}

void CListView::AddColumn ( const SIMPLEGUI_CHAR *szColumnName, int nWidth )
{
	InsertColumn ( szColumnName, nWidth, d_columnCount );
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
	{
		return;
	}


	


	//ColumnItem item;
	
	//item [ nColumnId ] = pEntry;
	//vColumnItemList.push_back ( item );

	//// Build the new row
	//ListRow row;
	//////row.d_sortColumn = getSortColumn ();
	row.d_items.resize ( d_columnCount, 0 );
	//////row.d_rowID = row_id;
	row [ nColumnId ] = pEntry;
	//row.d_items.push_back ( pEntry );

	d_grid.push_back ( row );
	m_nRowSize = d_grid.size ();
	//m_vColumnList [ nColumnId ].m_sItem.resize ( m_nRowSize,0 );
	//m_nRowSize = max ( m_nRowSize, m_vColumnList [ nColumnId ].m_sItem.size () );
	// Set up scroll bar ranges
	m_pScrollbar->SetTrackRange ( GetAllColumnsWidth (), m_nRowSize );
}

void CListView::SetColumnItemName ( UINT nColumnId, UINT nIndex, const SIMPLEGUI_CHAR *szItem )
{
	if (nColumnId >= m_vColumnList.size() ||
		nIndex >= m_vColumnList[nColumnId].m_sItem.size() ||
		EMPTYCHAR(szItem))
	{
		return;
	}

	m_vColumnList [ nColumnId ].m_sItem [ nIndex ]->m_sText = szItem;
}

void CListView::RemoveColumnItem ( UINT nColumnId, UINT nIndex )
{
	if (nColumnId >= m_vColumnList.size() ||
		nIndex >= m_vColumnList[nColumnId].m_sItem.size())
	{
		return;
	}

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

int CListView::GetColumnIdAtArea ( Pos pos )
{
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	SControlRect rRect = m_rBoundingBox;
	rRect.m_pos.m_nX -= pScrollbarHor->GetTrackPos()-4;

	for ( size_t i = 0; i < m_vColumnList.size (); i++ )
	{
		rRect.m_pos.m_nX += (i ? GetColumnWidth(i - 1) : 0);
		rRect.m_size.cx = GetColumnWidth ( i );
		rRect.m_size.cy = m_rColumnArea.m_size.cy;

		if ( rRect.ContainsPoint ( pos ) )
		{
			return i;
		}
	}

	return -1;
}

int CListView::GetColumnIdAtAreaBorder ( Pos pos )
{
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	SControlRect rRect = m_rBoundingBox;
	rRect.m_pos.m_nX -= pScrollbarHor->GetTrackPos () ;

	for ( size_t i = 0; i < m_vColumnList.size (); i++ )
	{
		rRect.m_pos.m_nX += GetColumnWidth(i);
		rRect.m_size.cx = 4;
		rRect.m_size.cy = m_rColumnArea.m_size.cy;

		if ( rRect.ContainsPoint ( pos ) )
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

	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	UINT nId = 0;
	int nX = m_rBoundingBox.m_pos.m_nX - pScrollbarHor->GetTrackPos ();

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
		if (pEntry)
		{
			return pEntry;
		}

		nColumnId++;
	}

	return NULL;
}

void CListView::MoveColumn ( UINT nColumnId, UINT nPosition )
{
	if (nColumnId >= m_vColumnList.size() ||
		nColumnId == nPosition)
	{
		return;
	}

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

	//ColumnItem mColumnItem;
	//std::vector<ColumnItem>	vColumnItemList;

	/*for ( size_t i = 0; i < m_nRowSize; i++ )
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
*/
	//m_nColumnSort = nColumnId;
	//if ( std::is_sorted ( vColumnItemList.begin (), vColumnItemList.end (), ColumnItemLess ) )
	//{
	//	std::sort ( vColumnItemList.begin (), vColumnItemList.end (), ColumnItemGreater );
	//}
	//else
	//{
	//	std::sort ( vColumnItemList.begin (), vColumnItemList.end (), ColumnItemLess );
	//}

	//for ( size_t i = 0; i < m_nRowSize; i++ )
	//{
	//	for ( size_t j = 0; j < GetNumOfColumns (); j++ )
	//		m_vColumnList [ j ].m_sItem [ i ] = vColumnItemList [ i ] [ j ];
	//}
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

	m_rColumnArea.m_pos.m_nX -= nHorScrollTrackPos - 4;

	CD3DRender *pRender = m_pDialog->GetRenderer ();

	SControlRect rListBoxText = m_rListBoxArea;
	rListBoxText.m_pos.m_nX -= nHorScrollTrackPos - 4;
	
	SControlRect rScissor = m_rScissor;
	rScissor.m_pos += 1;
	rScissor.m_size.cx -= 2;

	CScissor sCissor;
	sCissor.SetScissor ( m_pDialog->GetDevice (), rScissor.GetRect () );

	pRender->D3DLine ( m_rColumnArea.m_pos.m_nX, m_rColumnArea.m_pos.m_nY + m_rColumnArea.m_size.cy,
					   m_rColumnArea.m_pos.m_nX + m_rColumnArea.m_size.cx + nHorScrollTrackPos,
					   m_rColumnArea.m_pos.m_nY + m_rColumnArea.m_size.cy, m_sControlColor.d3dColorOutline );

	SControlRect rect = m_rListBoxArea;
	int sd = m_nRowSize;

	for ( size_t j = 0; j <  d_columnCount; j++ )
	{
		rect.m_pos.m_nX += 100;
		rect.m_pos.m_nY = m_rListBoxArea.m_pos.m_nY;
		for ( size_t i = 0; i < m_nRowSize; i++ )
		{
			
			SIMPLEGUI_STRING str ;
			if ( i < d_grid.size() )
			{
				const SEntryItem *pEntry = d_grid [ i ] [ j ];
				if ( pEntry )
				{
					str = pEntry->m_sText;
				}
			}

			//rect.m_pos.m_nX += GetColumnWidth ( j );
			SIZE size;
			if ( !str.empty () )

			{
				m_pDialog->DrawFont ( rect, m_sControlColor.d3dColorSelectedFont, str.c_str (), D3DFONT_COLORTABLE, m_pFont );
				
			}
			m_pFont->GetTextExtent ( str.c_str (), &size );
			rect.m_pos.m_nY += size.cy;
			
		}sd -= 1;
		
	}

	return;
	for ( size_t i = 0; i < m_nRowSize; i++ )
	{
		int count = 0;
		for ( size_t j = 0; j < d_columnCount; j++, count++ )
		{
			/*	if ( m_nRowSize > m_vColumnList [ j ].m_sItem.size () )
				{
					m_vColumnList [ j ].m_sItem.resize ( m_nRowSize );
				}*/
			m_vColumnList [ j ].m_sItem.size ();
			if ( i < m_vColumnList [ j ].m_sItem.size () )
				m_vColumnList [ j ].m_sItem [ i ] = vColumnItemList [ i ] [ j ];
		}
	}

	// Draw all contexts
	for ( size_t i = 0; i < m_vColumnList.size (); i++ )
	{
		int nPrevColumnWidth = ( i ? GetColumnWidth ( i - 1 ) : 0 );
		int nColumnWidth = GetColumnWidth ( i );

		m_rColumnArea.m_pos.m_nX += nPrevColumnWidth;

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

		rListBoxText.m_pos.m_nX += nPrevColumnWidth;
		rListBoxText.m_pos.m_nY = m_rListBoxArea.m_pos.m_nY;

		if ( m_rColumnArea.m_pos.m_nX > m_rBoundingBox.m_pos.m_nX + m_rBoundingBox.m_size.cx )
			break;

		if ( m_rColumnArea.m_pos.m_nX + nColumnWidth < m_rBoundingBox.m_pos.m_nX )
			continue;

		SControlRect rColumnBox = m_rColumnArea;
		rColumnBox.m_pos.m_nX = m_rColumnArea.m_pos.m_nX - 4;
		rColumnBox.m_size.cx = nColumnWidth;
		m_pDialog->DrawBox ( rColumnBox, d3dColorColumn, m_sControlColor.d3dColorOutline );

		SIMPLEGUI_STRING strColumnName = GetColumnName ( i );
		m_pTitleFont->CutString ( nColumnWidth - 4, strColumnName );
		m_pDialog->DrawFont ( SControlRect ( m_rColumnArea.m_pos.m_nX + nColumnWidth / 2 - 4, m_rColumnArea.m_pos.m_nY + m_rColumnArea.m_size.cy / 2 ),
							  m_sControlColor.d3dColorFont, strColumnName.c_str (), D3DFONT_COLORTABLE | D3DFONT_CENTERED_X | D3DFONT_CENTERED_Y,
							  m_pTitleFont );

		if ( i != 0 )
		{
			pRender->D3DLine ( m_rColumnArea.m_pos.m_nX - 4, m_rColumnArea.m_pos.m_nY, m_rColumnArea.m_pos.m_nX - 4,
							   m_rColumnArea.m_pos.m_nY + m_rBoundingBox.m_size.cy,
							   m_sControlColor.d3dColorOutline );
		}

		for ( size_t j = nVerScrollTrackPos; j < nVerScrollTrackPos + pScrollbarVer->GetPageSize (); j++ )
		{
			SIMPLEGUI_STRING str;
			const SEntryItem *pEntry = GetColumnItemByRow ( i, j );
			if ( pEntry )
			{
				str = pEntry->m_sText;
			}

			SIZE size;
			m_pFont->GetTextExtent ( str.c_str (), &size );

			if ( j != pScrollbarVer->GetTrackPos () )
			{
				rListBoxText.m_pos.m_nY += size.cy;
			}

			D3DCOLOR d3dColorFont = m_sControlColor.d3dColorFont;

			if ( m_nSelected == j ||
				 m_nIndex == j )
			{
				SControlRect rBoxSel = rListBoxText;
				rBoxSel.m_pos.m_nX -= 2;
				rBoxSel.m_pos.m_nY += 1;
				rBoxSel.m_size.cx += nHorScrollTrackPos;
				rBoxSel.m_size.cy = size.cy - 1;

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

	Pos mPos = m_pDialog->GetMouse ()->GetPos ();
	if ( m_bMoving )
	{
		int nColumnWidth = GetColumnWidth ( m_nId );
		int nId = GetColumnIdAtArea ( mPos );
		int nOffset = GetColumnOffset ( nId == -1 ? m_vColumnList.size () - 1 : nId ) - nHorScrollTrackPos;

		SIMPLEGUI_STRING szStr = GetColumnName ( m_nId );

		SControlRect rRect = m_rBoundingBox;
		rRect.m_pos.m_nX = mPos.m_nX - m_nDragX - nHorScrollTrackPos;
		rRect.m_size.cx = nColumnWidth;
		rRect.m_size.cy = m_rColumnArea.m_size.cy;

		D3DCOLOR color	= D3DCOLOR_ARGB ( 140, 100, 100, 100 );
		D3DCOLOR color1 = D3DCOLOR_ARGB ( 80, 0, 0, 0 );
		D3DCOLOR color2 = D3DCOLOR_ARGB ( 255, 0, 0, 200 );

		pRender->D3DLine ( nOffset, m_rBoundingBox.m_pos.m_nY, nOffset, m_rBoundingBox.m_pos.m_nY + m_rColumnArea.m_size.cy, color2 );
		pRender->D3DLine ( nOffset + 1, m_rBoundingBox.m_pos.m_nY, nOffset + 1, m_rBoundingBox.m_pos.m_nY + m_rColumnArea.m_size.cy, color2 );

		m_pDialog->DrawBox ( rRect, color, m_sControlColor.d3dColorOutline );

		m_pTitleFont->CutString ( nColumnWidth - 4, szStr );
		m_pDialog->DrawFont ( SControlRect ( rRect.m_pos.m_nX + nColumnWidth / 2, rRect.m_pos.m_nY + m_rColumnArea.m_size.cy / 2 ),
							  color1, szStr.c_str (),
							  D3DFONT_COLORTABLE | D3DFONT_CENTERED_X | D3DFONT_CENTERED_Y, m_pTitleFont );
	}
	else if ( m_bSizing )
	{
		int nOffset = GetColumnOffset ( m_nId );
		if ( nOffset >= mPos.m_nX)
			mPos.m_nX = nOffset;

		D3DCOLOR d3dLineColor = m_sControlColor.d3dColorOutline;
		if ( nOffset + LISTVIEW_MINCOLUMNSIZE >= mPos.m_nX)
			d3dLineColor = D3DCOLOR_XRGB ( 200, 0, 0 );

		pRender->D3DLine ( mPos.m_nX, m_rBoundingBox.m_pos.m_nY + 1, mPos.m_nX, m_rBoundingBox.m_pos.m_nY + m_rColumnArea.m_size.cy, d3dLineColor );
		pRender->D3DLine ( mPos.m_nX + 1, m_rBoundingBox.m_pos.m_nY + 1, mPos.m_nX + 1, m_rBoundingBox.m_pos.m_nY + m_rColumnArea.m_size.cy, d3dLineColor );
	}

	rScissor.m_size.cx = m_rScissor.m_size.cx - 2;
	sCissor.SetScissor ( m_pDialog->GetDevice (), rScissor.GetRect () );

	m_pScrollbar->OnDraw ();
}

void CListView::OnClickLeave ( void )
{
	CWidget::OnClickLeave ();
	m_pScrollbar->OnClickLeave ();

	m_nId = m_nOverColumnId = -1;
	m_bSizing = m_bMoving  = false;
}

bool CListView::OnClickEvent ( void )
{
	return ( CWidget::OnClickEvent () ||
			 m_pScrollbar->OnClickEvent () ||
			 m_bSizing || m_bMoving );
}

void CListView::OnFocusIn ( void )
{
	CWidget::OnFocusIn ();

	m_pScrollbar->OnFocusIn ();
}

void CListView::OnFocusOut ( void )
{
	CWidget::OnFocusOut ();

	m_pScrollbar->OnFocusOut ();
}

void CListView::OnMouseEnter ( void )
{
	CWidget::OnMouseEnter ();

	m_pScrollbar->OnMouseEnter ();
}

void CListView::OnMouseLeave ( void )
{
	CWidget::OnMouseLeave ();

	if ( !m_bSizing &&
		 !m_bMoving )
	{
		m_nOverColumnId = m_nIndex = -1;
	}

	m_pScrollbar->OnMouseLeave ();
	m_pDialog->GetMouse ()->SetCursorType ( CMouse::DEFAULT );
}

bool CListView::CanHaveFocus ( void )
{
	return ( CWidget::CanHaveFocus () ||
			 m_pScrollbar->CanHaveFocus () );
}

bool CListView::OnMouseButtonDown ( sMouseEvents e )
{
	if ( !m_bSizing &&
		 !m_bMoving )
	{
		if ( m_pScrollbar->OnMouseButtonDown ( e ) )
			return true;
	}

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		/*if ( !m_bMouseOver )
			return false;*/

		if ( m_bSizable )
		{
			m_nId = GetColumnIdAtAreaBorder ( e.pos );
			if ( m_nId > -1 )
			{
				m_bPressed = m_bSizing = true;
				m_nDragX = m_rBoundingBox.m_pos.m_nX + GetColumnWidth ( m_nId ) - e.pos.m_nX;

				_SetFocus ();
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
				m_nDragX = e.pos.m_nX - GetColumnOffset ( m_nId );

				_SetFocus ();
				return true;
			}
		}

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
			nId == -1 ?
				m_vColumnList.size () - 1 :
				nId;

			MoveColumn ( m_nId, nId );
			m_bMoving = false;
		}

		if ( m_bSizing )
		{
			SetColumnWidth ( m_nId, e.pos.m_nX - m_rBoundingBox.m_pos.m_nX + m_nDragX );
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
			if ( m_rListBoxArea.ContainsPoint ( e.pos ) )
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

bool CListView::OnMouseMove ( CVector pos )
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

	if ( ( GetAsyncKeyState ( VK_LBUTTON ) && !m_bHasFocus ) )
		return false;

	if ( m_pScrollbar->ContainsPoint ( pos ) ||
		 m_pScrollbar->OnClickEvent () )
	{
		return true;
	}

	if ( m_bMouseOver && m_bMoving )
	{
		m_bSorting = false;
		return true;
	}
	else
	{
		int nId = GetColumnIdAtAreaBorder ( pos );
		if ( m_bMouseOver && (m_bSizing || nId > -1) )
		{
			if ( nId > -1 )
			{
				m_pDialog->GetMouse ()->SetCursorType ( CMouse::E_RESIZE );
			}
			return true;
		}
		else
		{
			m_pDialog->GetMouse ()->SetCursorType ( CMouse::DEFAULT );
		}
	}

	if ( !m_bMouseOver ) 
		return false;

	SControlRect rText = m_rListBoxArea;
	rText.m_pos.m_nX += 4;
	rText.m_size.cx -= ( pScrollbarVer->GetWidth () + 4 );
	rText.m_size.cy = TEXTBOX_TEXTSPACE - 2;

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

			if ( i != pScrollbarVer->GetTrackPos () )
				rText.m_pos.m_nY += size.cy;

			// Check if selected text is not NULL and determine 
			// which item has been selected
			if ( str.c_str () != NULL && rText.ContainsPoint ( pos ) )
			{
				m_nIndex = i;
				//return true;
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
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();
	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();

	CWidget::UpdateRects ();

	SIZE size;
	m_pTitleFont->GetTextExtent ( _UI ( "Y" ), &size );

	m_rColumnArea = m_rBoundingBox;
	m_rColumnArea.m_size.cy = size.cy + LISTVIEW_TITLESIZE;

	m_rListBoxArea = m_rBoundingBox;
	m_rListBoxArea.m_pos.m_nY += m_rColumnArea.m_size.cy;
	m_rListBoxArea.m_size.cy = m_rListBoxArea.m_size.cy - m_rColumnArea.m_size.cy;

	m_pFont->GetTextExtent ( _UI ( "Y" ), &size );

	// Set up scroll bar values
	pScrollbarHor->SetStepSize ( GetAllColumnsWidth () / 10 );
	m_pScrollbar->SetPageSize ( m_rListBoxArea.m_size.cx-4, m_rListBoxArea.m_size.cy / size.cy );
	m_pScrollbar->UpdateScrollbars ( m_rListBoxArea );
}

bool CListView::ContainsPoint ( CVector pos )
{
	if ( !CanHaveFocus () )
		return false;

	return ( m_pScrollbar->ContainsPoint ( pos ) || 
			 m_rBoundingBox.ContainsPoint ( pos ) );
}