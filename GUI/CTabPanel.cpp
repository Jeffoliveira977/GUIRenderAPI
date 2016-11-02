#include "CGUI.h"
#define HAS_CONTROL_TYPE( p, type ) ( p && p->GetType () == type )
#define TABPANEL_MINTABSIZE 50

CTabPanel::CTabPanel ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_TABPANEL );
	m_nSelectedTab = m_nNextTab = 0;
	m_nOverTabId = -1;

	m_pDialog->LoadFont ( _UI ( "Arial" ), 10, true, &m_pTitleFont );

	m_pScrollbar = new CScrollablePane ( pDialog );
	if ( !m_pScrollbar )
		MessageBox ( 0, _UI ( "CWindow::CWindow: Error for creating CScrollBarHorizontal" ), _UI ( "GUIAPI.asi" ), 0 );

	m_pScrollbar->SetControl ( this );
}

CTabPanel::~CTabPanel ( void )
{
	RemoveAllTabs ();
}

void CTabPanel::AddTab ( const SIMPLEGUI_CHAR *szTabName, int nWidth )
{
	if ( EMPTYCHAR ( szTabName ) )
		return;

	if ( nWidth < TABPANEL_MINTABSIZE )
		nWidth = TABPANEL_MINTABSIZE;

	STabList sTab;
	sTab.sTabName = szTabName;
	sTab.nWidth = nWidth;

	sTab.pFocussedControl = NULL;
	sTab.pMouseOverControl = NULL;

	sTab.nTrackX = 0;
	sTab.nTrackY = 0;

	m_TabList.push_back ( sTab );

	m_nSelectedTab = m_TabList.size () - 1;
	SetHeight ( m_rBoundingBox.m_size.cy );
}

void CTabPanel::RemoveTab ( const SIMPLEGUI_CHAR *szTabName )
{
	if ( EMPTYCHAR ( szTabName ) )
		return;

	for ( size_t i = 0; i < m_TabList.size(); i++ )
	{
		if ( m_TabList [ i ].sTabName.compare ( szTabName ) )
		{
			RemoveTab ( i );
			break;
		}
	}
}

void CTabPanel::RemoveTab ( UINT nTabID )
{
	if ( nTabID >= m_TabList.size () ) 
		return;

	m_TabList.erase ( m_TabList.begin () + nTabID );
}

void CTabPanel::RemoveAllTabs ( void )
{
	RemoveAllControls ();
	m_TabList.clear ();
}

void CTabPanel::AddControl ( UINT nTabID, CWidget *pControl )
{
	if ( nTabID >= m_TabList.size () ||
		 !pControl ||
		 pControl->GetType () == CWidget::EControlType::TYPE_TABPANEL ||
		 pControl->GetType () == CWidget::EControlType::TYPE_WINDOW )
	{
		return;
	}
	
	pControl->SetPos ( *pControl->GetPos ());
	pControl->SetParent ( this );
	pControl->UpdateRects ();
	m_TabList [ nTabID ].vControls.push_back ( pControl );
}

void CTabPanel::RemoveControl ( UINT nTabID, CWidget *pControl )
{
	if ( nTabID >= m_TabList.size () || 
		 !pControl )
		return;

	std::vector<CWidget*> &vWidgets = m_TabList [ nTabID ].vControls;
	std::vector<CWidget*>::iterator iter = std::find ( vWidgets.begin (), vWidgets.end (), pControl );
	if ( iter == vWidgets.end () )
		return;

	vWidgets.erase ( iter );
	SAFE_DELETE ( pControl );
}

void CTabPanel::RemoveAllControlsFromTab ( UINT nTabID )
{
	if ( nTabID >= m_TabList.size () ) 
		return;
	
	std::vector<CWidget*> &vWidgets = m_TabList [ nTabID ].vControls;
	for ( size_t i = 0; i <vWidgets.size (); i++ )
	{
		SAFE_DELETE ( vWidgets [ i ] );
	}

	vWidgets.clear ();
}

void CTabPanel::RemoveAllControls ( void )
{
	for ( size_t i = 0; i < m_TabList.size (); i++ )
	{
		RemoveAllControlsFromTab ( i );
	}
}

bool CTabPanel::IsControlInList ( CWidget *pControl )
{
	for ( size_t i = 0; i < m_TabList.size (); i++ )
	{
		std::vector<CWidget*> vWidgets = m_TabList [ i ].vControls;
		std::vector<CWidget*>::iterator iter = std::find ( vWidgets.begin (), vWidgets.end (), pControl );
		if ( iter == vWidgets.end () )
			continue;

		return true;
	}

	return false;
}

void CTabPanel::SetFocussedControl ( CWidget *pControl )
{
	if ( !IsControlInList ( pControl ) )
		return;

	CWidget *&pFocussedWidget = m_TabList [ m_nSelectedTab ].pFocussedControl;

	if ( pFocussedWidget != pControl )
	{
		if ( !pControl->CanHaveFocus () )
			return;

		if ( pFocussedWidget )
			pFocussedWidget->OnFocusOut ();

		if ( pControl )
			pControl->OnFocusIn ();

		pFocussedWidget = pControl;
	}

	BringControlToTop ( m_nSelectedTab, pControl );
}

void CTabPanel::BringControlToTop ( UINT nTabID, CWidget *pControl )
{
	if ( nTabID >= m_TabList.size () || 
		 !pControl )
		return;

	std::vector<CWidget*> &vWidgets = m_TabList [ nTabID ].vControls;
	std::vector<CWidget*>::iterator iter = std::find ( vWidgets.begin (), vWidgets.end (), pControl );
	if ( iter == vWidgets.end () )
		return;

	vWidgets.erase ( iter );
	vWidgets.insert ( vWidgets.end (), pControl );

	// give it focus
	_SetFocus ();
}

void CTabPanel::MoveControl ( CWidget *pControl, UINT nTabPosition )
{
	if ( nTabPosition >= m_TabList.size () )
		nTabPosition = m_TabList.size () - 1;

	for ( size_t i = 0; i < m_TabList.size (); i++ )
	{
		std::vector<CWidget*> &vWidgets = m_TabList [ i ].vControls;
		std::vector<CWidget*>::iterator iter = std::find ( vWidgets.begin (), vWidgets.end (), pControl );
		if ( iter == vWidgets.end () )
			continue;

		if ( i != nTabPosition )
		{
			vWidgets.erase ( iter );
			m_TabList [ nTabPosition ].vControls.insert ( m_TabList [ nTabPosition ].vControls.end (), pControl );
			return;
		}
	}
}

CWidget *CTabPanel::GetControlAtArea ( UINT nTabID, CVector pos )
{
	if ( nTabID >= m_TabList.size () )
		return NULL;

	std::vector<CWidget*> vWidgets = m_TabList [ nTabID ].vControls;
	for ( std::vector<CWidget*>::reverse_iterator iter = vWidgets.rbegin (); iter != vWidgets.rend (); iter++ )
	{
		if ( ( *iter )->ContainsPoint ( pos ) )
			return ( *iter );
	}

	return NULL;
}

CWidget *CTabPanel::GetFocussedControl ( void )
{
	return m_TabList [ m_nSelectedTab ].pFocussedControl;
}

void CTabPanel::ClearControlFocus ( void )
{
	CWidget *&pFocussedWidget = m_TabList [ m_nSelectedTab ].pFocussedControl;
	CWidget *&pMouseOverWidget = m_TabList [ m_nSelectedTab ].pMouseOverControl;

	if ( pFocussedWidget )
	{
		pFocussedWidget->OnClickLeave ();
		pFocussedWidget->OnMouseLeave ();
		pFocussedWidget->OnFocusOut ();
		pFocussedWidget = NULL;
	}

	if ( pMouseOverWidget )
	{
		pMouseOverWidget->OnMouseLeave ();
		pMouseOverWidget = NULL;
	}
}

int CTabPanel::GetAllColumnsWidth ( void )
{
	int nWidth = 0;
	for ( auto tab : m_TabList )
	{
		nWidth += tab.nWidth;
	}

	return nWidth;
}

UINT CTabPanel::GetNumOfTabsVisible ( void )
{
	UINT nTabs = 0;
	int nWidth = 0;

	for ( size_t i = m_nNextTab; i < m_TabList.size (); i++ )
	{
		nWidth += m_TabList [ i ].nWidth;
		if ( nWidth < m_rPanelArea.m_size.cx )
			nTabs++;
	}

	return nTabs;
}

int CTabPanel::GetTabIdAtArea ( CVector pos )
{
	int nColumnWidth = 0;
	SControlRect rTabBox = m_rTabArea;
	size_t size = GetNumOfTabsVisible ();

	for ( size_t i = m_nNextTab; i < size + m_nNextTab; i++ )
	{
		rTabBox.m_pos.m_nX += nColumnWidth;
		nColumnWidth = m_TabList [ i ].nWidth;
		rTabBox.m_size.cx = nColumnWidth;

		if ( rTabBox.ContainsPoint ( pos ) )
		{
			return i;
		}
	}

	return -1;
}

void CTabPanel::SetSelectedTab ( UINT nTabID )
{
	if ( nTabID >= m_TabList.size () )
		return;

	m_nSelectedTab = nTabID;
}

int CTabPanel::GetSelectedTab ( void )
{
	return m_nSelectedTab;
}

int CTabPanel::GetTabSizeY ( void )
{
	return m_rTabArea.m_size.cy;
}

SIZE CTabPanel::GetSize ( void )
{
	SIZE size;
	size.cx = m_rBoundingBox.m_size.cx;
	size.cy = m_rBoundingBox.m_size.cy;

	return size;
}

void CTabPanel::Draw ( void )
{
	int nWidth = 0;

	UINT nNumTabsVisible = GetNumOfTabsVisible ();
	size_t tabSize = m_TabList.size ();
	size_t size = min ( nNumTabsVisible + m_nNextTab, tabSize );

	for ( size_t i = m_nNextTab ? m_nNextTab - 1 : 0; i < size; i++ )
	{
		nWidth += m_TabList [ i ].nWidth;
	}

	if ( m_rPanelArea.m_size.cx != m_nOldAreaX )
	{
		if ( m_rPanelArea.m_size.cx > m_nOldAreaX )
		{
			if ( m_rPanelArea.m_size.cx > nWidth &&
				 m_nNextTab > 0 )
			{
				m_nNextTab--;
			}
		}
		m_nOldAreaX = m_rPanelArea.m_size.cx;
	}

	m_pDialog->DrawBox ( m_rPanelArea, m_sControlColor.d3dColorBoxBack, m_sControlColor.d3dColorOutline );

	SControlRect rTabBox = m_rTabArea;

	int nColumnWidth = 0;
	int nAllColumnsWidth = GetAllColumnsWidth ();
	for ( size_t i = m_nNextTab; i < size; i++ )
	{
		rTabBox.m_pos.m_nX += nColumnWidth;
		nColumnWidth = m_TabList [ i ].nWidth;
		rTabBox.m_size.cx = nColumnWidth;

		D3DCOLOR d3dColorColumn = m_sControlColor.d3dColorBox [ SControlColor::STATE_NORMAL ];
		if ( i == m_nSelectedTab ||
			 i == m_nOverTabId )
		{
			if ( i == m_nSelectedTab )
			{
				d3dColorColumn = m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ];
			}
			else
			{
				d3dColorColumn = m_sControlColor.d3dColorBox [ SControlColor::STATE_MOUSE_OVER ];
			}
		}

		m_pDialog->DrawBox ( rTabBox, d3dColorColumn, m_sControlColor.d3dColorOutline );

		SIMPLEGUI_STRING strTabName = m_TabList [ i ].sTabName;
		m_pTitleFont->CutString ( nColumnWidth - 4, strTabName );
		m_pDialog->DrawFont ( SControlRect ( rTabBox.m_pos.m_nX + nColumnWidth / 2, rTabBox.m_pos.m_nY + rTabBox.m_size.cy / 2 ),
							  m_sControlColor.d3dColorFont, strTabName.c_str (), D3DFONT_COLORTABLE | D3DFONT_CENTERED_X | D3DFONT_CENTERED_Y,
							  m_pTitleFont );
	}

	if ( nAllColumnsWidth >= m_rPanelArea.m_size.cx )
	{
		D3DCOLOR d3dButtonLeftColor = m_sControlColor.d3dColorBox [ SControlColor::STATE_NORMAL ];
		D3DCOLOR d3dButtonRightColor = m_sControlColor.d3dColorBox [ SControlColor::STATE_NORMAL ];

		if ( m_sLeftButton.bOverButton )
		{
			d3dButtonLeftColor = m_sControlColor.d3dColorBox [ SControlColor::STATE_MOUSE_OVER ];
		}
		else if ( m_sRightButton.bOverButton )
		{
			d3dButtonRightColor = m_sControlColor.d3dColorBox [ SControlColor::STATE_MOUSE_OVER ];
		}
		if ( m_sLeftButton.bClickedButton )
		{
			d3dButtonLeftColor = m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ];
		}
		else if ( m_sRightButton.bClickedButton )
		{
			d3dButtonRightColor = m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ];
		}

		m_sLeftButton.bVisible = ( m_nNextTab > 0 );
		m_sRightButton.bVisible = ( m_nNextTab < tabSize - nNumTabsVisible );

		SControlRect rShape = m_sLeftButton.m_rButton;
		rShape.m_size.cx = m_sLeftButton.m_rButton.m_size.cx / 2 - 5;

		if ( m_sLeftButton.bVisible )
		{
			m_pDialog->DrawBox ( m_sLeftButton.m_rButton, d3dButtonLeftColor, m_sControlColor.d3dColorOutline );

			rShape.m_pos.m_nX += m_sLeftButton.m_rButton.m_size.cx / 2 + 2;
			rShape.m_pos.m_nY += m_sLeftButton.m_rButton.m_size.cx / 2 - 2;
			m_pDialog->DrawTriangle ( rShape, 90.f, m_sControlColor.d3dColorShape, 0 );
		}
		if ( m_sRightButton.bVisible )
		{
			m_pDialog->DrawBox ( m_sRightButton.m_rButton, d3dButtonRightColor, m_sControlColor.d3dColorOutline );

			rShape.m_pos.m_nX = m_sRightButton.m_rButton.m_pos.m_nX + m_sRightButton.m_rButton.m_size.cx / 2 - 1;
			rShape.m_pos.m_nY = m_sRightButton.m_rButton.m_pos.m_nY + m_sRightButton.m_rButton.m_size.cx / 2 - 2;
			m_pDialog->DrawTriangle ( SControlRect ( rShape.m_pos, rShape.m_size ), 270.f, m_sControlColor.d3dColorShape, 0 );
		}
	}

	/*ZeroMemory ( &m_maxControlSize, sizeof ( SIZE ) );
*/
	SControlRect rScissor = m_rScissor;
	rScissor.m_pos.m_nY -= 1;
	rScissor.m_size.cy += 1;
	CScissor sCissor;
	sCissor.SetScissor ( m_pDialog->GetDevice (), rScissor.GetRect () );

	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	STabList &sTab = m_TabList [ m_nSelectedTab ];

	if ( sTab.vControls.size () )
	{
		sTab.nTrackX = pScrollbarHor->GetTrackPos ();
		sTab.nTrackY = pScrollbarVer->GetTrackPos ();
	}
	else
	{
		m_maxControlSize .cx = 0;
		m_maxControlSize .cy = 0;
	}

	for ( auto control : sTab.vControls )
	{
		if ( control )
		{
			Pos *pos = control->GetPos ();
			SIZE size = control->GetSize ();
			if ( pos->m_nY <= GetTabSizeY () )
				control->SetPosY ( 0 );
			control->LinkPos ( m_rBoundingBox.m_pos + Pos ( 0, m_rTabArea.m_size.cy ) - Pos ( sTab.nTrackX, sTab.nTrackY ) );
		
	
			CWidget::eRelative relativeX = control->GetRelativeX ();
			CWidget::eRelative relativeY = control->GetRelativeY ();

			if ( !( relativeX == CWidget::RELATIVE_SIZE || relativeX == CWidget::RELATIVE_POS ) ||
				( relativeX == CWidget::RELATIVE_POS &&  pos->m_nX <= 0 ) || ( control->GetMinSize ().cx == size.cx &&pos->m_nX > 0 ) )
			{
				m_maxControlSize.cx = max ( m_maxControlSize.cx, pos->m_nX + size.cx );
			}

			if ( !( relativeY == CWidget::RELATIVE_SIZE || relativeY == CWidget::RELATIVE_POS ) ||
				( relativeY == CWidget::RELATIVE_POS && pos->m_nY <= GetTabSizeY() ) ||
				 /*( control->GetRelativeY () == CWidget::RELATIVE_SIZE &&*/( control->GetMinSize ().cy == size.cy && pos->m_nY >= GetTabSizeY () ) )
			{

			m_maxControlSize.cy = max ( m_maxControlSize.cy, pos->m_nY + size.cy );
			}

			pos = control->GetUpdatedPos ();

			if ( pos->m_nX + size.cx > m_rPanelArea.m_pos.m_nX &&
				 pos->m_nY + size.cy > m_rPanelArea.m_pos.m_nY &&
				 pos->m_nX < m_rPanelArea.m_pos.m_nX + m_rPanelArea.m_size.cx &&
				 pos->m_nY < m_rPanelArea.m_pos.m_nY + m_rPanelArea.m_size.cy )
			{
				m_rScissor = m_rPanelArea;
	
			/*	int nDragOffSet;
				if ( rScissor.m_pos.m_nY + rScissor.m_size.cy < m_rPanelArea.m_pos.m_nY + m_rPanelArea.m_size.cy )
				{
					m_rScissor.m_size.cy = rScissor.m_pos.m_nY + rScissor.m_size.cy - m_rPanelArea.m_pos.m_nY;
				}

				if ( m_rPanelArea.m_pos.m_nY < rScissor.m_pos.m_nY )
				{
					nDragOffSet = rScissor.m_pos.m_nY - m_rPanelArea.m_pos.m_nY;
					m_rScissor.m_pos.m_nY += nDragOffSet;
					m_rScissor.m_size.cy = m_rScissor.m_size.cy - nDragOffSet;
				}

				if ( rScissor.m_pos.m_nX + rScissor.m_size.cx < m_rPanelArea.m_pos.m_nX + m_rPanelArea.m_size.cx )
				{
					m_rScissor.m_size.cx = rScissor.m_pos.m_nX + rScissor.m_size.cx - m_rPanelArea.m_pos.m_nX + 1;
				}

				if ( m_rPanelArea.m_pos.m_nX < rScissor.m_pos.m_nX )
				{
					nDragOffSet = rScissor.m_pos.m_nX - m_rPanelArea.m_pos.m_nX;
					m_rScissor.m_pos.m_nX += nDragOffSet;
					m_rScissor.m_size.cx = m_rScissor.m_size.cx - nDragOffSet + 1;
				}*/

				control->EnterScissorRect ( m_rScissor );
				control->Draw ();
				control->LeaveScissorRect ();
			}
		}
	}

	m_rScissor = rScissor;
	sCissor.SetScissor ( m_pDialog->GetDevice (), m_rScissor.GetRect () );
	UpdateScrollbars ();
	m_pScrollbar->OnDraw ();
}

bool CTabPanel::OnMouseButtonDown ( sMouseEvents e )
{
	if ( !CanHaveFocus () )
		return false;

	CWidget *pFocussedWidget = m_TabList [ m_nSelectedTab ].pFocussedControl;

	// Check if mouse is over window boundaries
	if ( !HAS_CONTROL_TYPE ( pFocussedWidget, CWidget::TYPE_DROPDOWN ) )
	{
		// Let the scroll bar handle it first.
		if ( m_pScrollbar->OnMouseButtonDown ( e ) )
		{
			ClearControlFocus ();
			return true;
		}
	}	
	
	if ( e.eButton == sMouseEvents::LeftButton )
	{
		_SetFocus ();

		if ( m_sLeftButton.InArea ( e.pos ) )
		{
			m_sLeftButton.bClickedButton = true;
			return true;
		}

		if ( m_sRightButton.InArea ( e.pos ) )
		{
			m_sRightButton.bClickedButton = true;
			return true;
		}

		int nId = GetTabIdAtArea ( e.pos );
		if ( nId > -1 )
		{
			m_nSelectedTab = nId;
			m_bPressed = true;

			m_pScrollbar->GetHorScrollbar ()->SetTrackPos ( m_TabList [ nId ].nTrackX );
			m_pScrollbar->GetVerScrollbar ()->SetTrackPos ( m_TabList [ nId ].nTrackY );
			return true;
		}

		if ( m_rPanelArea.ContainsPoint ( e.pos ) )
		{
			// Pressed while inside the control
			m_bPressed = true;
			return true;
		}
	}

	return false;
}

bool CTabPanel::OnMouseButtonUp ( sMouseEvents e )
{
	m_bPressed = false;

	CWidget *pFocussedWidget = m_TabList [ m_nSelectedTab ].pFocussedControl;
	CWidget *&pMouseOverWidget = m_TabList [ m_nSelectedTab ].pMouseOverControl;

	if ( !HAS_CONTROL_TYPE ( pFocussedWidget, CWidget::TYPE_DROPDOWN ) )
	{
		// Let the scroll bar handle it first.
		if ( m_pScrollbar->OnMouseButtonUp ( e ) )
			return true;
	}

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		if ( pMouseOverWidget )
		{
			pMouseOverWidget->OnMouseLeave ();
			pMouseOverWidget = NULL;
		}
	}

	if ( m_sLeftButton.OnClickEvent ( e.pos ) )
	{
		m_sLeftButton.bClickedButton = false;

		if ( m_nNextTab > 0 )
			m_nNextTab--;

		return true;
	}

	if ( m_sRightButton.OnClickEvent ( e.pos ) )
	{
		m_sRightButton.bClickedButton = false;

		if ( m_nNextTab < m_TabList.size () - GetNumOfTabsVisible () )
			m_nNextTab++;

		return true;
	}

	return false;
}

bool CTabPanel::OnMouseMove ( CVector pos )
{
	if ( !CanHaveFocus () )
		return false;

	// Let the scroll bar handle it first.
	if ( m_pScrollbar->OnMouseMove ( pos ) )
		return true;

	if ( m_bMouseOver && !m_sRightButton.bOverButton && m_sLeftButton.InArea ( pos ) )
	{
		m_nOverTabId = -1;
		m_sLeftButton.bOverButton = true;
		return false;
	}

	if ( m_bMouseOver && !m_sLeftButton.bOverButton && m_sRightButton.InArea ( pos ) )
	{
		m_nOverTabId = -1;
		m_sRightButton.bOverButton = true;
		return false;
	}

	int nId = GetTabIdAtArea ( pos );
	if ( m_bMouseOver && !OnClickEvent () )
	{
		m_nOverTabId = nId;
		m_sRightButton.bOverButton = m_sLeftButton.bOverButton = false;
	}

	return false;
}

bool CTabPanel::OnMouseWheel ( int zDelta )
{
	if ( !m_TabList [ m_nSelectedTab ].pFocussedControl ||
		 !m_TabList [ m_nSelectedTab ].pMouseOverControl && m_bMouseOver )
	{
		m_pScrollbar->OnMouseWheel ( -zDelta );
		return true;
	}

	return false;
}

bool CTabPanel::ControlMessages ( sControlEvents e )
{
	CWidget *&pMouseOverWidget = m_TabList [ m_nSelectedTab ].pMouseOverControl;
	CWidget *pFocussedWidget = m_TabList [ m_nSelectedTab ].pFocussedControl;

	bool bHasDropDown = HAS_CONTROL_TYPE ( pFocussedWidget, CWidget::TYPE_DROPDOWN );

	if ( !CanHaveFocus () ||
		( m_pScrollbar->ContainsPoint ( e.mouseEvent.pos ) && !bHasDropDown ) ||
		 !m_rBoundingBox.ContainsPoint ( e.mouseEvent.pos ) &&
		 !( bHasDropDown || HAS_CONTROL_TYPE ( pFocussedWidget, CWidget::TYPE_EDITBOX  || pFocussedWidget && pFocussedWidget->OnClickEvent () )) )
	{
		return false;
	}

	if ( pFocussedWidget && pFocussedWidget->InjectKeyboard ( e.keyEvent ) )
		return true;

	CWidget* pControl = GetControlAtArea ( m_nSelectedTab, e.mouseEvent.pos );
	if ( !pControl &&
		 e.mouseEvent.eMouseMessages == sMouseEvents::ButtonDown &&
		 e.mouseEvent.eButton == sMouseEvents::LeftButton &&
		 pFocussedWidget )
	{
		ClearControlFocus ();
	}

	// it the first chance at handling the message.
	if ( pFocussedWidget && pFocussedWidget->InjectMouse ( e.mouseEvent ) )
		return true;

	if ( pControl && pControl->InjectMouse ( e.mouseEvent ) )
		return true;

	if ( !GetAsyncKeyState ( VK_LBUTTON ) )
	{
		// If the mouse is still over the same control, nothing needs to be done
		if ( pControl == pMouseOverWidget )
			return false;

		// Handle mouse leaving the old control
		if ( pMouseOverWidget )
			pMouseOverWidget->OnMouseLeave ();

		// Handle mouse entering the new control
		pMouseOverWidget = pControl;
		if ( pControl )
			pControl->OnMouseEnter ();
	}

	return false;
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnClickLeave ( void )
{
	CWidget::OnClickLeave ();
	m_pScrollbar->OnClickLeave ();

	m_nOverTabId = -1;
	m_sLeftButton.bClickedButton = m_sRightButton.bClickedButton = false;

	CWidget *&pFocussedWidget = m_TabList [ m_nSelectedTab ].pFocussedControl;

	if ( pFocussedWidget )
		pFocussedWidget->OnClickLeave ();
}

//--------------------------------------------------------------------------------------
bool CTabPanel::OnClickEvent ( void )
{
	CWidget *pFocussedWidget = m_TabList [ m_nSelectedTab ].pFocussedControl;

	return ( ( m_bPressed || m_sLeftButton.bClickedButton || m_sRightButton.bClickedButton ) ||
			 ( pFocussedWidget && pFocussedWidget->OnClickEvent () ) ||
			 m_pScrollbar->OnClickEvent () );
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnFocusIn ( void )
{
	CWidget::OnFocusIn ();

	m_pScrollbar->OnFocusIn ();
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnFocusOut ( void )
{
	CWidget::OnFocusOut ();

	m_pScrollbar->OnFocusOut ();

	ClearControlFocus ();
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnMouseEnter ( void )
{
	CWidget::OnMouseEnter ();

	CWidget *pFocussedWidget = m_TabList [ m_nSelectedTab ].pFocussedControl;

	bool bHasDropDown = HAS_CONTROL_TYPE ( pFocussedWidget, CWidget::TYPE_DROPDOWN );

	if ( m_bMouseOver && !bHasDropDown )
		m_pScrollbar->OnMouseEnter ();
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnMouseLeave ( void )
{
	CWidget::OnMouseLeave ();

	m_pScrollbar->OnMouseLeave ();

	m_sLeftButton.bOverButton = m_sRightButton.bOverButton = false;
	m_nOverTabId = -1;

	CWidget *&pMouseOverWidget = m_TabList [ m_nSelectedTab ].pMouseOverControl;

	if ( pMouseOverWidget )
	{
		pMouseOverWidget->OnMouseLeave ();
		pMouseOverWidget = NULL;
	}
}

void CTabPanel::UpdateScrollbars ( void )
{
	SControlRect rRect = m_rBoundingBox;
	rRect.m_size.cy -= m_rTabArea.m_size.cy;
	rRect.m_pos.m_nY += m_rTabArea.m_size.cy;

	m_pScrollbar->SetTrackRange ( m_maxControlSize.cx, m_maxControlSize.cy );
	m_pScrollbar->GetHorScrollbar ()->SetStepSize ( m_rBoundingBox.m_size.cx / 10 );
	m_pScrollbar->GetVerScrollbar ()->SetStepSize ( m_rBoundingBox.m_size.cy / 10 );
	m_pScrollbar->UpdateScrollbars ( rRect );

	//m_pScrollbar->SetTrackRange ( m_maxControlSize.cx, m_maxControlSize.cy );
	//m_pScrollbar->SetPageSize ( m_rBoundingBox.m_size.cx - ( m_pScrollbar->IsVerScrollbarNeeded () ? 18 : 0 ), m_rBoundingBox.m_size.cy - ( m_pScrollbar->IsHorScrollbarNeeded () ? 18 : 0 ) );
	//m_pScrollbar->UpdateScrollbars ( rRect );
}

#define BUTTONSIZEY 15
void CTabPanel::UpdateRects ( void )
{
	CWidget::UpdateRects ();

	SIZE size;
	m_pTitleFont->GetTextExtent ( _UI ( "Y" ), &size );

	m_rTabArea = m_rBoundingBox;
	m_rTabArea.m_size.cy = size.cy + 8;

	m_rPanelArea = m_rBoundingBox;
	m_rPanelArea.m_pos.m_nY += m_rTabArea.m_size.cy;
	m_rPanelArea.m_size.cy -= m_rTabArea.m_size.cy;

	m_sLeftButton.m_rButton.m_pos.m_nX = m_rTabArea.m_pos.m_nX;
	m_sLeftButton.m_rButton.m_pos.m_nY = m_rTabArea.m_pos.m_nY + (m_rTabArea.m_size.cy / 2) - (BUTTONSIZEY / 2);
	m_sLeftButton.m_rButton.m_size.cx = 21;
	m_sLeftButton.m_rButton.m_size.cy = BUTTONSIZEY;

	m_sRightButton.m_rButton.m_pos.m_nX = m_rTabArea.m_pos.m_nX + m_rPanelArea.m_size.cx - 21;
	m_sRightButton.m_rButton.m_pos.m_nY = m_rTabArea.m_pos.m_nY + (m_rTabArea.m_size.cy / 2) - (BUTTONSIZEY / 2);
	m_sRightButton.m_rButton.m_size.cx = 21;
	m_sRightButton.m_rButton.m_size.cy = BUTTONSIZEY;

}

bool CTabPanel::ContainsPoint ( CVector pos )
{
	int nWidth = 0;
	size_t size = GetNumOfTabsVisible ();

	for ( size_t i = m_nNextTab; i < size + m_nNextTab; i++ )
	{
		nWidth += m_TabList [ i ].nWidth;
		if ( nWidth > m_rPanelArea.m_size.cx )
			break;
	}

	m_rTabArea.m_size.cx = nWidth;

	CWidget *pFocussedWidget = m_TabList [ m_nSelectedTab ].pFocussedControl;

	return ( m_rPanelArea.ContainsPoint ( pos ) ||
			 m_sLeftButton.InArea ( pos ) ||
			 m_sRightButton.InArea ( pos ) ||
			 m_rTabArea.ContainsPoint ( pos ) ||
			 ( HAS_CONTROL_TYPE ( pFocussedWidget, CWidget::TYPE_DROPDOWN ) && pFocussedWidget && pFocussedWidget->ContainsPoint ( pos ) ) );
}
