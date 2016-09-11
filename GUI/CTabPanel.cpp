#include "CGUI.h"


CTabPanel::CTabPanel ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_TABPANEL );
	m_nSelectedTab = m_nNextTab = m_uNumTabToDraw = 0;
	m_nOverTabId = -1;

	m_pDialog->LoadFont ( _UI ( "Arial" ), 10, true, &m_pTitleFont );

}

CTabPanel::~CTabPanel ( void )
{
	
}

void CTabPanel::AddTab ( const SIMPLEGUI_CHAR *szTabName, int nWidth )
{
	STabList sTab;
	sTab.sTabName = szTabName;
	sTab.nWidth = nWidth;
	sTab.pFocussedControl = NULL;
	sTab.pMouseOverControl = NULL;

	m_TabList.push_back ( sTab );

	m_nSelectedTab = m_TabList.size () ? m_TabList.size () - 1 : 0;
}

void CTabPanel::RemoveTab ( const SIMPLEGUI_CHAR *szTabName )
{}

void CTabPanel::RemoveTab ( UINT nTabID )
{}

void CTabPanel::AddControl ( UINT nTabID, CControl *pControl )
{
	if ( nTabID >= m_TabList.size () ||
		 !pControl ||
		 pControl->GetType () == CControl::EControlType::TYPE_TABPANEL ||
		 pControl->GetType () == CControl::EControlType::TYPE_WINDOW )
		return;

	SIZE size;
	m_pTitleFont->GetTextExtent ( _UI ( "Y" ), &size );

	pControl->SetPos ( *pControl->GetPos ()+/**GetUpdatedPos() +*/ CPos ( 0, size.cy + 8 ) );
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

void CTabPanel::RemoveAllControls ( UINT nTabID )
{}

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

void CTabPanel::SetFocussedControl (  CControl *pControl )
{
	if ( !IsControlInList ( pControl ) )
		return;

	if ( m_TabList [ m_nSelectedTab ].pFocussedControl != pControl )
	{
		if ( pControl && !pControl->CanHaveFocus () )
			return;

		if ( m_TabList [ m_nSelectedTab ].pFocussedControl )
			m_TabList [ m_nSelectedTab ].pFocussedControl->OnFocusOut ();

		if ( pControl )
			pControl->OnFocusIn ();

		m_TabList [ m_nSelectedTab ].pFocussedControl = pControl;
	}

	if ( pControl )
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
	//for ( size_t i = m_nNextTab; i < m_TabList.size (); i++ )
	for ( auto tab : m_TabList )
	{
		nWidth += tab.nWidth;
		//nWidth += m_TabList [ i ].nWidth;
		if ( nWidth < m_rPanelArea.size.cx )
			m_uNumTabToDraw++;
	}

	return nWidth;
}

UINT CTabPanel::GetNumOfTabsVisible ( void )
{
	UINT nTabs = 0;
	int nWidth = 0;

	for ( size_t i = m_nNextTab; i < m_TabList.size(); i++ )
	{
		nWidth += m_TabList[i]. nWidth;
		if ( nWidth < m_rPanelArea.size.cx )
			nTabs++;
	}

	return nTabs;
}

void CTabPanel::MoveControl ( CControl *pControl, UINT nTabPosition )
{
	if ( nTabPosition > m_TabList.size () )
		nTabPosition = m_TabList.size ();

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

void CTabPanel::SetSelectedTab ( UINT nTabID )
{
	if ( nTabID > m_TabList.size () )
		return;

	m_nSelectedTab = nTabID;
}

int CTabPanel::GetSelectedTab ( void )
{
	return m_nSelectedTab;
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

CControl *CTabPanel::GetControlAtArea ( UINT nTabID, CPos pos )
{
	if ( nTabID >= m_TabList.size () )
		return NULL;

	for ( std::vector<CControl*>::iterator iter = m_TabList [ nTabID ].vControls.end (); iter != m_TabList [ nTabID ].vControls.begin (); )
	{
		iter--;
		if ( ( *iter )->ContainsRect ( pos ) )
			return ( *iter );
	}

	return NULL;
}

void CTabPanel::Draw ( void )
{
	UINT nTabs = 0;
	UINT nNumTabs = GetNumOfTabsVisible ();

	int nWidth = 0;	
	size_t size = min ( nNumTabs + m_nNextTab, m_TabList.size () );
	
	for ( size_t i = m_nNextTab; i < nNumTabs; i++ )
	{
		nWidth += m_TabList [ i ].nWidth;
		if ( nWidth >= m_rPanelArea.size.cx&&m_nNextTab > 0 )
		{
			m_nNextTab--;
		}
	}

	/*if ( nWidth < m_rPanelArea.size.cx && m_nNextTab >= 0 )
		m_nNextTab--;*/

	SControlRect rScissor = m_rScissor;
	IDirect3DDevice9 *pDevice = m_pDialog->GetDevice ();

	RECT rOldScissor;
	pDevice->GetScissorRect ( &rOldScissor );

	ZeroMemory ( &m_maxControlSize, sizeof ( SIZE ) );

	m_pDialog->DrawBox ( m_rPanelArea, m_sControlColor.d3dColorBoxBack, m_sControlColor.d3dColorOutline );

	SControlRect rTabBox = m_rTabArea;
	int nAllColumnsWidth = GetAllColumnsWidth ();

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
		m_pDialog->DrawFont ( SControlRect ( rTabBox.pos.GetX () + nColumnWidth / 2 , rTabBox.pos.GetY () + rTabBox.size.cy / 2 ),
							  m_sControlColor.d3dColorFont, strTabName.c_str (), D3DFONT_COLORTABLE | D3DFONT_CENTERED_X | D3DFONT_CENTERED_Y,
							  m_pTitleFont );
	}

	if ( nAllColumnsWidth >= m_rPanelArea.size.cx )
	{
		D3DCOLOR d3dColor [ 2 ] = { m_sControlColor.d3dColorBox [ SControlColor::STATE_NORMAL ],
									m_sControlColor.d3dColorBox [ SControlColor::STATE_NORMAL ] };

		if ( m_bClickedButton [ 0 ] )
			d3dColor [ 0 ] = m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ];
		else if ( m_bClickedButton [ 1 ] )
			d3dColor [ 1 ] = m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ];
		
		//if ( m_nNextTab > 0 )
			m_pDialog->DrawBox ( m_rButtonLeft, d3dColor[0], m_sControlColor.d3dColorOutline );

		m_pDialog->DrawBox ( m_rButtonRight, d3dColor [ 1 ], m_sControlColor.d3dColorOutline );
	}

	for ( auto control : m_TabList [ m_nSelectedTab ].vControls )
	{
		if ( control )
		{
			control->LinkPos ( *GetUpdatedPos () );

			SIZE size = control->GetSize ();
			CPos *pos = control->GetUpdatedPos ();

			if ( pos->GetX () + size.cx > m_rPanelArea.pos.GetX () &&
				 pos->GetY () + size.cy > m_rPanelArea.pos.GetY () &&
				 pos->GetX () < m_rPanelArea.pos.GetX () + m_rPanelArea.size.cx &&
				 pos->GetY () < m_rPanelArea.pos.GetY () + m_rPanelArea.size.cy )
			{
				rScissor = m_rScissor;
				rScissor.pos.SetY ( rScissor.pos.GetY () + m_rTabArea.size.cy );
				rScissor.size.cx = rScissor.size.cx ;
				rScissor.size.cy = rScissor.size.cy - m_rTabArea.size.cy;

				control->EnterScissorRect ( rScissor );
				control->Draw ();
				control->LeaveScissorRect ();
			}
		}
	}

}


bool CTabPanel::OnKeyUp ( WPARAM wParam )
{
	return false;
}

bool CTabPanel::OnKeyDown ( WPARAM wParam )
{
	return false;
}

bool CTabPanel::OnMouseButtonDown ( sMouseEvents e )
{
	if ( !CanHaveFocus () )
		return false;

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		m_pParent->SetFocussedControl ( this );

		if ( m_rButtonLeft.InControlArea ( e.pos ) )
		{
			if ( m_nNextTab > 0 )
				m_nNextTab--;

			m_bClickedButton [ 0 ] = true;
			m_bPressed = true;
			return true;
		}

		if ( m_rButtonRight.InControlArea ( e.pos ) )
		{
			if ( m_nNextTab < m_TabList.size () - GetNumOfTabsVisible () )
				m_nNextTab++;

			m_bClickedButton [ 1 ] = true;
			m_bPressed = true;
			return true;
		}

		int nId = GetTabIdAtArea ( e.pos );
		if ( nId > -1 )
		{
			m_nSelectedTab = nId;
			m_bPressed = true;
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
	if ( m_bClickedButton [ 0 ] )
	{
		m_bClickedButton [ 0 ] = false;
		return true;
	}

	if ( m_bClickedButton [ 1 ] )
	{
		m_bClickedButton [ 1 ] = false;
		return true;
	}

	return false;
}

bool CTabPanel::OnMouseMove ( CPos pos )
{
	if ( !CanHaveFocus () )
		return false;

	if ( m_rButtonLeft.InControlArea ( pos ) ||
		 m_rButtonRight.InControlArea ( pos ) )
	{
		m_nOverTabId = -1;
		return false;
	}
	
	int nId = GetTabIdAtArea ( pos );
	if ( !OnClickEvent () )
	{
		m_nOverTabId = nId;

		/*if ( nId != -1)
		{
			return true;
		}*/
	}

	return false;
}

bool CTabPanel::OnMouseWheel ( int zDelta )
{
	return false;
}

bool CTabPanel::ControlMessages ( sControlEvents e )
{
	CControl *pMouseOverControl = m_TabList [ m_nSelectedTab ].pMouseOverControl;
	CControl *pFocussedControl = m_TabList [ m_nSelectedTab ].pFocussedControl;

	if ( pFocussedControl && pFocussedControl->InjectKeyboard ( e.keyEvent ) )
		return true;

	CControl* pControl = GetControlAtArea ( m_nSelectedTab, e.mouseEvent.pos );
	if ( !pControl
		 && e.mouseEvent.eMouseMessages == sMouseEvents::ButtonDown &&
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

	if ( m_TabList [ m_nSelectedTab ].pFocussedControl )
		m_TabList [ m_nSelectedTab ].pFocussedControl->OnClickLeave ();

	m_nOverTabId = -1;
}

//--------------------------------------------------------------------------------------
bool CTabPanel::OnClickEvent ( void )
{
	return ( m_bPressed ||
			 ( m_TabList [ m_nSelectedTab ].pFocussedControl && m_TabList [ m_nSelectedTab ].pFocussedControl->OnClickEvent () ) );
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnFocusIn ( void )
{
	CControl::OnFocusIn ();
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnFocusOut ( void )
{
	CControl::OnFocusOut ();

	ClearControlFocus ();
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnMouseEnter ( void )
{
	CControl::OnMouseEnter ();
}

//--------------------------------------------------------------------------------------
void CTabPanel::OnMouseLeave ( void )
{
	CControl::OnMouseLeave ();

	m_nOverTabId = -1;
	if ( m_TabList [ m_nSelectedTab ].pMouseOverControl )
	{
		m_TabList [ m_nSelectedTab ].pMouseOverControl->OnMouseLeave ();
		m_TabList [ m_nSelectedTab ].pMouseOverControl = NULL;
	}
}

//--------------------------------------------------------------------------------------
bool CTabPanel::CanHaveFocus ( void )
{
	return ( CControl::CanHaveFocus () );
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
	m_rPanelArea.size.cy = m_rPanelArea.size.cy - m_rTabArea.size.cy;

	m_rButtonRight.pos.SetX ( m_rTabArea.pos.GetX () + m_rPanelArea.size.cx - 22 );
	m_rButtonRight.pos.SetY ( m_rTabArea.pos.GetY () + m_rTabArea.size.cy / 2 - BUTTONSIZEY / 2 );
	m_rButtonRight.size.cx = 21;
	m_rButtonRight.size.cy = BUTTONSIZEY;

	m_rButtonLeft.pos.SetX ( m_rButtonRight.pos.GetX () - 21 );
	m_rButtonLeft.pos.SetY ( m_rTabArea.pos.GetY () + m_rTabArea.size.cy / 2 - BUTTONSIZEY / 2 );
	m_rButtonLeft.size.cx = 21;
	m_rButtonLeft.size.cy = BUTTONSIZEY;
}

bool CTabPanel::ContainsRect ( CPos pos )
{
	int nWidth = 0;
	size_t size = GetNumOfTabsVisible ();
	int nAllColumnsWidth = GetAllColumnsWidth ();

	for ( size_t i = m_nNextTab; i < size + m_nNextTab; i++ )
	{
		nWidth += m_TabList [ i ].nWidth;
		if ( nWidth > m_rPanelArea.size.cx )
			break;
	}

	m_rTabArea.size.cx = nWidth;
	CControl *pFocussedControl = m_TabList [ m_nSelectedTab ].pFocussedControl;

	return ( m_rPanelArea.InControlArea ( pos ) ||
			 ( m_nNextTab > 0 && m_rButtonLeft.InControlArea ( pos ) ) ||
			 ( nAllColumnsWidth >= m_rPanelArea.size.cx && m_rButtonRight.InControlArea ( pos ) ) ||
			 m_rTabArea.InControlArea ( pos ) ||
			 ( pFocussedControl && pFocussedControl->GetType () == CControl::TYPE_DROPDOWN && pFocussedControl->ContainsRect ( pos ) ) );
}
