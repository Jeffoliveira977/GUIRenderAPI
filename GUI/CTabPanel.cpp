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

	m_pScrollbar->AddControl ( this );
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

	m_TabList.push_back ( sTab );

	m_nSelectedTab = m_TabList.size () - 1;
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

void CTabPanel::AddControl ( UINT nTabID, CControl *pControl )
{
	if ( nTabID >= m_TabList.size () ||
		 !pControl ||
		 pControl->GetType () == CControl::EControlType::TYPE_TABPANEL ||
		 pControl->GetType () == CControl::EControlType::TYPE_WINDOW )
	{
		return;
	}

	SIZE size;
	m_pTitleFont->GetTextExtent ( _UI ( "Y" ), &size );

	pControl->SetPos ( *pControl->GetPos ()+ CPos ( 0, GetTabSizeY() ));
	pControl->SetParent ( this );

	m_TabList [ nTabID ].vControls.push_back ( pControl );
}

void CTabPanel::RemoveControl ( UINT nTabID, CControl *pControl )
{
	if ( nTabID >= m_TabList.size () || 
		 !pControl )
		return;

	std::vector<CControl*>::iterator iter = std::find ( m_TabList [ nTabID ].vControls.begin (), m_TabList [ nTabID ].vControls.end (), pControl );
	if ( iter == m_TabList [ nTabID ].vControls.end () )
		return;

	m_TabList [ nTabID ].vControls.erase ( iter );
	SAFE_DELETE ( pControl );
}

void CTabPanel::RemoveAllControlsFromTab ( UINT nTabID )
{
	if ( nTabID >= m_TabList.size () ) 
		return;

	for ( size_t i = 0; i < m_TabList [ nTabID ].vControls.size (); i++ )
	{
		SAFE_DELETE ( m_TabList [ nTabID ].vControls [ i ] );
	}

	m_TabList [ nTabID ].vControls.clear ();
}

void CTabPanel::RemoveAllControls ( void )
{
	for ( size_t i = 0; i < m_TabList.size (); i++ )
	{
		RemoveAllControlsFromTab ( i );
	}
}

bool CTabPanel::IsControlInList ( CControl *pControl )
{
	for ( size_t i = 0; i < m_TabList.size (); i++ )
	{
		std::vector<CControl*>::iterator iter = std::find ( m_TabList [ i ].vControls.begin (), m_TabList [ i ].vControls.end (), pControl );
		if ( iter == m_TabList [ i ].vControls.end () )
			continue;

		return true;
	}

	return false;
}

void CTabPanel::SetFocussedControl ( CControl *pControl )
{
	if ( !IsControlInList ( pControl ) )
		return;

	if ( m_TabList [ m_nSelectedTab ].pFocussedControl != pControl )
	{
		if ( !pControl->CanHaveFocus () )
			return;

		if ( m_TabList [ m_nSelectedTab ].pFocussedControl )
			m_TabList [ m_nSelectedTab ].pFocussedControl->OnFocusOut ();

		if ( pControl )
			pControl->OnFocusIn ();

		m_TabList [ m_nSelectedTab ].pFocussedControl = pControl;
	}

	BringControlToTop ( m_nSelectedTab, pControl );
}

void CTabPanel::BringControlToTop ( UINT nTabID, CControl *pControl )
{
	if ( nTabID >= m_TabList.size () || 
		 !pControl )
		return;

	std::vector<CControl*>::iterator iter = std::find ( m_TabList [ nTabID ].vControls.begin (), m_TabList [ nTabID ].vControls.end (), pControl );
	if ( iter == m_TabList [ nTabID ].vControls.end () )
		return;

	m_TabList [ nTabID ].vControls.erase ( iter );
	m_TabList [ nTabID ].vControls.insert ( m_TabList [ nTabID ].vControls.end (), pControl );

	// Make sure the control has focus, otherwise give it focus.
	if ( m_pParent && !m_bHasFocus )
		m_pParent->SetFocussedControl ( this );
}

CControl *CTabPanel::GetFocussedControl ( )
{
	return m_TabList [ m_nSelectedTab ].pFocussedControl;
}

void CTabPanel::ClearControlFocus ( void )
{
	if ( m_TabList [ m_nSelectedTab ].pFocussedControl )
	{
		m_TabList [ m_nSelectedTab ].pFocussedControl->OnClickLeave ();
		m_TabList [ m_nSelectedTab ].pFocussedControl->OnMouseLeave ();
		m_TabList [ m_nSelectedTab ].pFocussedControl->OnFocusOut ();
		m_TabList [ m_nSelectedTab ].pFocussedControl = NULL;
	}

	if ( m_TabList [ m_nSelectedTab ].pMouseOverControl )
	{
		m_TabList [ m_nSelectedTab ].pMouseOverControl->OnMouseLeave ();
		m_TabList [ m_nSelectedTab ].pMouseOverControl = NULL;
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
		if ( nWidth < m_rPanelArea.size.cx )
			nTabs++;
	}

	return nTabs;
}

int CTabPanel::GetTabIdAtArea ( CPos pos )
{
	int nColumnWidth = 0;
	SControlRect rTabBox = m_rTabArea;
	size_t size = GetNumOfTabsVisible ();

	for ( size_t i = m_nNextTab; i < size + m_nNextTab; i++ )
	{
		rTabBox.pos.SetX ( rTabBox.pos.GetX () + nColumnWidth );
		nColumnWidth = m_TabList [ i ].nWidth;
		rTabBox.size.cx = nColumnWidth;

		if ( rTabBox.InControlArea ( pos ) )
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

void CTabPanel::MoveControl ( CControl *pControl, UINT nTabPosition )
{
	if ( nTabPosition >= m_TabList.size () )
		nTabPosition = m_TabList.size () - 1;

	for ( size_t i = 0; i < m_TabList.size (); i++ )
	{
		std::vector<CControl*>::iterator iter = std::find ( m_TabList [ i ].vControls.begin (), m_TabList [ i ].vControls.end (), pControl );
		if ( iter == m_TabList [ i ].vControls.end () )
			continue;

		if ( i != nTabPosition )
		{
			m_TabList [ i ].vControls.erase ( iter );
			m_TabList [ nTabPosition ].vControls.insert ( m_TabList [ nTabPosition ].vControls.end (), pControl );
			return;
		}
	}
}

CControl *CTabPanel::GetControlAtArea ( UINT nTabID, CPos pos )
{
	if ( nTabID >= m_TabList.size () )
		return NULL;

	for ( std::vector<CControl*>::reverse_iterator iter = m_TabList [ nTabID ].vControls.rbegin (); iter != m_TabList [ nTabID ].vControls.rend (); iter++ )
	{
		if ( ( *iter )->ContainsRect ( pos ) )
			return ( *iter );
	}

	return NULL;
}

int CTabPanel::GetTabSizeY ( void )
{
	SIZE size;
	m_pTitleFont->GetTextExtent ( _UI ( "Y" ), &size );

	return size.cy + 8;
}

SIZE CTabPanel::GetSize ( void )
{
	SIZE size;
	size.cx = m_rBoundingBox.size.cx;
	size.cy = m_rBoundingBox.size.cy;

	return size;
}

void CTabPanel::Draw ( void )
{
	int nWidth = 0;
	size_t size = min ( GetNumOfTabsVisible () + m_nNextTab, m_TabList.size () );

	for ( size_t i = m_nNextTab ? m_nNextTab - 1 : 0; i < size; i++ )
	{
		nWidth += m_TabList [ i ].nWidth;
	}

	if ( m_rPanelArea.size.cx != m_nOldAreaX )
	{
		if ( m_rPanelArea.size.cx > m_nOldAreaX )
		{
			if ( m_rPanelArea.size.cx > nWidth && m_nNextTab > 0 )
			{
				m_nNextTab--;
			}
		}
		m_nOldAreaX = m_rPanelArea.size.cx;
	}

	m_pDialog->DrawBox ( m_rPanelArea, m_sControlColor.d3dColorBoxBack, m_sControlColor.d3dColorOutline );

	SControlRect rTabBox = m_rTabArea;

	int nColumnWidth = 0;
	for ( size_t i = m_nNextTab; i < size; i++ )
	{
		rTabBox.pos.SetX ( rTabBox.pos.GetX () + nColumnWidth );
		nColumnWidth = m_TabList [ i ].nWidth;
		rTabBox.size.cx = nColumnWidth;

		D3DCOLOR d3dColorColumn = m_sControlColor.d3dColorBox [ SControlColor::STATE_NORMAL ];
		if ( i == m_nSelectedTab || m_nOverTabId == i )
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
		m_pDialog->DrawFont ( SControlRect ( rTabBox.pos.GetX () + nColumnWidth / 2, rTabBox.pos.GetY () + rTabBox.size.cy / 2 ),
							  m_sControlColor.d3dColorFont, strTabName.c_str (), D3DFONT_COLORTABLE | D3DFONT_CENTERED_X | D3DFONT_CENTERED_Y,
							  m_pTitleFont );
	}

	if ( GetAllColumnsWidth () >= m_rPanelArea.size.cx )
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
		m_sRightButton.bVisible = ( m_nNextTab < m_TabList.size () - GetNumOfTabsVisible () );

		SControlRect rShape = m_sLeftButton.m_rButton;
		rShape.size.cx = m_sLeftButton.m_rButton.size.cx / 2 - 5;

		if ( m_sLeftButton.bVisible )
		{
			m_pDialog->DrawBox ( m_sLeftButton.m_rButton, d3dButtonLeftColor, m_sControlColor.d3dColorOutline );

			rShape.pos.SetX ( rShape.pos.GetX () + m_sLeftButton.m_rButton.size.cx / 2 + 2 );
			rShape.pos.SetY ( rShape.pos.GetY () + m_sLeftButton.m_rButton.size.cx / 2 - 2 );
			m_pDialog->DrawTriangle ( rShape, 90.f, m_sControlColor.d3dColorShape, 0 );
		}
		if ( m_sRightButton.bVisible )
		{
			m_pDialog->DrawBox ( m_sRightButton.m_rButton, d3dButtonRightColor, m_sControlColor.d3dColorOutline );

			rShape.pos.SetX ( m_sRightButton.m_rButton.pos.GetX () + m_sRightButton.m_rButton.size.cx / 2 - 1 );
			rShape.pos.SetY ( m_sRightButton.m_rButton.pos.GetY () + m_sRightButton.m_rButton.size.cx / 2 - 2 );
			m_pDialog->DrawTriangle ( SControlRect ( rShape.pos, rShape.size ), 270.f, m_sControlColor.d3dColorShape, 0 );
		}
	}

	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	m_nTrackX [ m_nSelectedTab ] = pScrollbarHor->GetTrackPos ();
	m_nTrackY [ m_nSelectedTab ] = pScrollbarVer->GetTrackPos ();

	ZeroMemory ( &m_maxControlSize, sizeof ( SIZE ) );

	SControlRect rScissor = m_rScissor;
	rScissor.pos.SetY ( rScissor.pos.GetY () - 1 );
	rScissor.size.cy += 1;

	for ( auto control : m_TabList [ m_nSelectedTab ].vControls )
	{
		if ( control )
		{
			control->LinkPos ( m_rBoundingBox.pos - ( CPos ( m_nTrackX [ m_nSelectedTab ], m_nTrackY [ m_nSelectedTab ] ) ) );

			CPos *pos = control->GetPos ();
			SIZE size = control->GetSize ();

			if ( !( control->GetRelativeX () == CControl::RELATIVE_SIZE || control->GetRelativeX () == CControl::RELATIVE_POS ) ||
				 ( control->GetRelativeX () == CControl::RELATIVE_POS && pos->GetX () <= 0 ) ||
				 ( control->GetMinSize ().cx == size.cx && pos->GetX () > 0 ) )
			{
				m_maxControlSize.cx = max ( m_maxControlSize.cx, pos->GetX () + size.cx );
			}

			if ( !( control->GetRelativeY () == CControl::RELATIVE_SIZE || control->GetRelativeY () == CControl::RELATIVE_POS ) ||
				 ( control->GetRelativeY () == CControl::RELATIVE_POS && pos->GetY () <= GetTabSizeY () ) ||
				 ( control->GetRelativeY () == CControl::RELATIVE_SIZE && control->GetMinSize ().cy == size.cy && pos->GetY () > GetTabSizeY () ) )
			{
				m_maxControlSize.cy = max ( m_maxControlSize.cy, pos->GetY () + size.cy );
			}

			pos = control->GetUpdatedPos ();

			if ( pos->GetX () + size.cx > m_rPanelArea.pos.GetX () &&
				 pos->GetY () + size.cy > m_rPanelArea.pos.GetY () &&
				 pos->GetX () < m_rPanelArea.pos.GetX () + m_rPanelArea.size.cx &&
				 pos->GetY () < m_rPanelArea.pos.GetY () + m_rPanelArea.size.cy )
			{
				SControlRect rParentArea = GetRect ();
				rParentArea.pos.SetY ( rParentArea.pos.GetY () + static_cast< CWindow* >( m_pParent )->GetTitleBarHeight () );
				rParentArea.size = m_pParent->GetSize ();

				m_rScissor = m_rPanelArea;

				int nDragOffSet;
				if ( rScissor.pos.GetY () + rScissor.size.cy < m_rPanelArea.pos.GetY () + m_rPanelArea.size.cy  )
				{
					m_rScissor.size.cy = rScissor.pos.GetY () + rScissor.size.cy - m_rPanelArea.pos.GetY () ;
				}

				if ( m_rPanelArea.pos.GetY () < rScissor.pos.GetY ()  )
				{
					nDragOffSet = rScissor.pos.GetY () - m_rPanelArea.pos.GetY ();
					m_rScissor.pos.SetY ( m_rScissor.pos.GetY () + nDragOffSet );
					m_rScissor.size.cy = m_rScissor.size.cy - nDragOffSet;
				}

				if ( rScissor.pos.GetX () + rScissor.size.cx < m_rPanelArea.pos.GetX () + m_rPanelArea.size.cx  )
				{
					m_rScissor.size.cx = rScissor.pos.GetX () + rScissor.size.cx - m_rPanelArea.pos.GetX ();
				}

				if ( m_rPanelArea.pos.GetX () < rScissor.pos.GetX ()  )
				{
					nDragOffSet = rScissor.pos.GetX () - m_rPanelArea.pos.GetX () ;
					m_rScissor.pos.SetX ( m_rScissor.pos.GetX () + nDragOffSet );
					m_rScissor.size.cx = m_rScissor.size.cx - nDragOffSet;
				}

				control->EnterScissorRect ( m_rScissor );
				control->Draw ();
				control->LeaveScissorRect ();
			}
		}
	}

	m_rScissor = rScissor;
	EnterScissorRect ( m_rScissor );
	UpdateScrollbars ();
	m_pScrollbar->OnDraw ();
}

bool CTabPanel::OnMouseButtonDown ( sMouseEvents e )
{
	if ( !CanHaveFocus () )
		return false;

	// Check if mouse is over window boundaries
	if ( !HAS_CONTROL_TYPE ( m_TabList [ m_nSelectedTab ].pFocussedControl, CControl::TYPE_DROPDOWN ) )
	{
		// Let the scroll bar handle it first.
		if ( m_pScrollbar->OnMouseButtonDown ( e ) )
		{
			ClearControlFocus ();
			return true;
		}
	}	
	
	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		m_pParent->SetFocussedControl ( this );

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

			pScrollbarHor->SetTrackPos ( m_nTrackX [ nId ] );
			pScrollbarVer->SetTrackPos ( m_nTrackY [ nId ] ); 
			return true;
		}

		if ( m_rPanelArea.InControlArea ( e.pos ) )
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

	if ( !HAS_CONTROL_TYPE ( m_TabList [ m_nSelectedTab ].pFocussedControl, CControl::TYPE_DROPDOWN ) )
	{
		// Let the scroll bar handle it first.
		if ( m_pScrollbar->OnMouseButtonUp ( e ) )
			return true;
	}

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		if ( m_TabList [ m_nSelectedTab ].pMouseOverControl )
		{
			m_TabList [ m_nSelectedTab ].pMouseOverControl->OnMouseLeave ();
			m_TabList [ m_nSelectedTab ].pMouseOverControl = NULL;
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

bool CTabPanel::OnMouseMove ( CPos pos )
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
	CControl *pMouseOverControl = m_TabList [ m_nSelectedTab ].pMouseOverControl;
	CControl *pFocussedControl = m_TabList [ m_nSelectedTab ].pFocussedControl;

	bool bHasDropDown = HAS_CONTROL_TYPE ( pFocussedControl, CControl::TYPE_DROPDOWN );

	if ( !CanHaveFocus () ||
		 ( m_pScrollbar->ContainsRect ( e.mouseEvent.pos ) && !bHasDropDown ) ||
		 !m_rBoundingBox.InControlArea ( e.mouseEvent.pos ) && !( bHasDropDown || HAS_CONTROL_TYPE ( pFocussedControl, CControl::TYPE_EDITBOX )) )
	{
		return false;
	}

	if ( pFocussedControl && pFocussedControl->InjectKeyboard ( e.keyEvent ) )
		return true;

	CControl* pControl = GetControlAtArea ( m_nSelectedTab, e.mouseEvent.pos );
	if ( !pControl &&
		 e.mouseEvent.eMouseMessages == sMouseEvents::ButtonDown &&
		 e.mouseEvent.eButton == sMouseEvents::LeftButton &&
		 pFocussedControl )
	{
		ClearControlFocus ();
	}

	// it the first chance at handling the message.
	if ( pFocussedControl && pFocussedControl->InjectMouse ( e.mouseEvent ) )
		return true;

	if ( pControl && pControl->InjectMouse ( e.mouseEvent ) )
		return true;

	if ( !GetAsyncKeyState ( VK_LBUTTON ) )
	{
		// If the mouse is still over the same control, nothing needs to be done
		if ( pControl == pMouseOverControl )
			return false;

		// Handle mouse leaving the old control
		if ( pMouseOverControl )
			pMouseOverControl->OnMouseLeave ();

		// Handle mouse entering the new control
		m_TabList [ m_nSelectedTab ].pMouseOverControl = pControl;
		if ( pControl )
			pControl->OnMouseEnter ();
	}

	return false;
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnClickLeave ( void )
{
	CControl::OnClickLeave ();
	m_pScrollbar->OnClickLeave ();

	m_nOverTabId = -1;
	m_sLeftButton.bClickedButton = m_sRightButton.bClickedButton = false;

	if ( m_TabList [ m_nSelectedTab ].pFocussedControl )
		m_TabList [ m_nSelectedTab ].pFocussedControl->OnClickLeave ();
}

//--------------------------------------------------------------------------------------
bool CTabPanel::OnClickEvent ( void )
{
	return ( ( m_bPressed || m_sLeftButton.bClickedButton || m_sRightButton.bClickedButton ) ||
			 ( m_TabList [ m_nSelectedTab ].pFocussedControl && m_TabList [ m_nSelectedTab ].pFocussedControl->OnClickEvent () ) ||
			 m_pScrollbar->OnClickEvent () );
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnFocusIn ( void )
{
	CControl::OnFocusIn ();

	m_pScrollbar->OnFocusIn ();
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnFocusOut ( void )
{
	CControl::OnFocusOut ();

	m_pScrollbar->OnFocusOut ();

	ClearControlFocus ();
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnMouseEnter ( void )
{
	CControl::OnMouseEnter ();

	bool bHasDropDown = HAS_CONTROL_TYPE ( m_TabList [ m_nSelectedTab ].pFocussedControl, CControl::TYPE_DROPDOWN );

	if ( m_bMouseOver && !bHasDropDown )
		m_pScrollbar->OnMouseEnter ();
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnMouseLeave ( void )
{
	CControl::OnMouseLeave ();

	m_pScrollbar->OnMouseLeave ();

	m_sLeftButton.bOverButton = m_sRightButton.bOverButton = false;
	m_nOverTabId = -1;

	if ( m_TabList [ m_nSelectedTab ].pMouseOverControl )
	{
		m_TabList [ m_nSelectedTab ].pMouseOverControl->OnMouseLeave ();
		m_TabList [ m_nSelectedTab ].pMouseOverControl = NULL;
	}
}

void CTabPanel::UpdateScrollbars ( void )
{
	SControlRect rRect = m_rBoundingBox;
	rRect.size.cy -= GetTabSizeY ();
	rRect.pos.SetX ( rRect.pos.GetX () );
	rRect.pos.SetY ( rRect.pos.GetY () + GetTabSizeY () );

	m_pScrollbar->SetTrackRange ( m_maxControlSize.cx, m_maxControlSize.cy );
	m_pScrollbar->SetPageSize ( m_rBoundingBox.size.cx - ( m_pScrollbar->IsVerScrollbarNeeded () ? 18 : 0 ), m_rBoundingBox.size.cy - ( m_pScrollbar->IsHorScrollbarNeeded () ? 18 : 0 ) );
	m_pScrollbar->UpdateScrollbars ( rRect );
}

#define BUTTONSIZEY 15
void CTabPanel::UpdateRects ( void )
{
	CControl::UpdateRects ();

	SIZE size;
	m_pTitleFont->GetTextExtent ( _UI ( "Y" ), &size );

	m_rTabArea = m_rBoundingBox;
	m_rTabArea.size.cy = size.cy + 8;

	m_rPanelArea = m_rBoundingBox;
	m_rPanelArea.pos.SetY ( m_rPanelArea.pos.GetY () + m_rTabArea.size.cy );
	m_rPanelArea.size.cy -= m_rTabArea.size.cy;

	m_sLeftButton.m_rButton.pos.SetX ( m_rTabArea.pos.GetX () );
	m_sLeftButton.m_rButton.pos.SetY ( m_rTabArea.pos.GetY () + m_rTabArea.size.cy / 2 - BUTTONSIZEY / 2 );
	m_sLeftButton.m_rButton.size.cx = 21;
	m_sLeftButton.m_rButton.size.cy = BUTTONSIZEY;

	m_sRightButton.m_rButton.pos.SetX ( m_rTabArea.pos.GetX () + m_rPanelArea.size.cx - 21 );
	m_sRightButton.m_rButton.pos.SetY ( m_rTabArea.pos.GetY () + m_rTabArea.size.cy / 2 - BUTTONSIZEY / 2 );
	m_sRightButton.m_rButton.size.cx = 21;
	m_sRightButton.m_rButton.size.cy = BUTTONSIZEY;

}

bool CTabPanel::ContainsRect ( CPos pos )
{
	int nWidth = 0;
	size_t size = GetNumOfTabsVisible ();

	for ( size_t i = m_nNextTab; i < size + m_nNextTab; i++ )
	{
		nWidth += m_TabList [ i ].nWidth;
		if ( nWidth > m_rPanelArea.size.cx )
			break;
	}

	m_rTabArea.size.cx = nWidth;
	CControl *pFocussedControl = m_TabList [ m_nSelectedTab ].pFocussedControl;

	return ( m_rPanelArea.InControlArea ( pos ) ||
			 m_sLeftButton.InArea ( pos ) ||
			 m_sRightButton.InArea ( pos ) ||
			 m_rTabArea.InControlArea ( pos ) ||
			 ( HAS_CONTROL_TYPE ( pFocussedControl, CControl::TYPE_DROPDOWN ) && pFocussedControl->ContainsRect ( pos ) ) );
}
