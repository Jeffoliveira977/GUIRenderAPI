#include "CGUI.h"

#define HAS_CONTROL_TYPE( p, type ) (p && p->GetType () == type)

//--------------------------------------------------------------------------------------
CWindow::CWindow ( CDialog *pDialog ) :
	/*m_pFocussedControl ( NULL ),*/
	m_bDragging ( false ),
	m_bCloseButtonEnabled ( true ),
	m_iTitleBarSize ( 26 ),
	m_bMovable ( true ),
	m_bSizable ( true ),
	m_bShowScrollbar ( true )
{
	m_bMaximized = false;
	m_eWindowArea = OutArea;
	m_nDragX = m_nDragY = 0;
	SetControl ( pDialog, TYPE_WINDOW );
	m_bOnTop = false;
	m_sControlColor.d3dColorWindowTitle = D3DCOLOR_RGBA ( 21, 75, 138, 255 );
	m_sControlColor.d3dCOlorWindowBack = D3DCOLOR_RGBA ( 30, 30, 30, 255 );
	m_sControlColor.d3dColorBoxSel = D3DCOLOR_RGBA ( 40, 40, 40, 255 );

	m_sControlColor.d3dColorBox [ SControlColor::STATE_NORMAL ] = D3DCOLOR_RGBA ( 180, 0, 0, 255 );
	m_sControlColor.d3dColorBox [ SControlColor::STATE_MOUSE_OVER ] = D3DCOLOR_XRGB ( 255, 0, 0, 255 );
	m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ] = D3DCOLOR_XRGB ( 100, 0, 0, 255 );
	m_sControlColor.d3dColorBox [ SControlColor::STATE_DISABLED ] = D3DCOLOR_XRGB ( 180, 180, 180, 255 );

	ZeroMemory ( &m_maxControlSize, sizeof ( SIZE ) );

	m_pScrollbar = new CScrollablePane ( pDialog );
	if ( !m_pScrollbar )
		MessageBox ( 0, _UI ( "CWindow::CWindow: Error for creating CScrollBarHorizontal" ), _UI ( "GUIAPI.asi" ), 0 );

	m_pScrollbar->SetControl ( this );
}

//--------------------------------------------------------------------------------------
CWindow::~CWindow ( void )
{
	SAFE_DELETE ( m_pFocussedControl );
	SAFE_DELETE ( m_pControlMouseOver );

	SAFE_DELETE ( m_pScrollbar );

	RemoveAllControls ();
}

//--------------------------------------------------------------------------------------
void CWindow::Draw ( void )
{
	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	if ( !m_bEnabledStateColor )
		m_eState = SControlColor::STATE_NORMAL;
	else if ( !m_bEnabled )
		m_eState = SControlColor::STATE_DISABLED;
	else if ( m_bPressed )
		m_eState = SControlColor::STATE_PRESSED;
	else if ( m_bMouseOver && m_eWindowArea == OutArea )
		m_eState = SControlColor::STATE_MOUSE_OVER;
	else
		m_eState = SControlColor::STATE_NORMAL;

	IDirect3DDevice9 *pDevice = m_pDialog->GetDevice ();

	SControlRect rScissor = m_rBoundingBox;
	rScissor.m_pos - 2;
	rScissor.m_size.cx = rScissor.m_size.cx + 4;
	rScissor.m_size.cy = rScissor.m_size.cy + 4;
	sCissor.SetScissor ( pDevice, rScissor.GetRect () );

	if ( !m_bMaximized )
	{
		// Draw background
		D3DCOLOR d3dColor = m_sControlColor.d3dCOlorWindowBack;
		if ( m_bHasFocus )
			d3dColor = m_sControlColor.d3dColorBoxSel;

		m_pDialog->DrawBox ( m_rBoundingBox, d3dColor, m_sControlColor.d3dColorOutline, m_bAntAlias );
	}

	// Draw title bar
	m_pDialog->DrawBox ( m_rTitle, m_sControlColor.d3dColorWindowTitle, m_sControlColor.d3dColorOutline, m_bAntAlias );

	SIMPLEGUI_STRING str = GetText ();
	m_pFont->CutString ( m_rBoundingBox.m_size.cx - m_rButton.m_size.cx - 5, str );
	m_pFont->Print ( m_pos.m_nX + 3, m_pos.m_nY + m_iTitleBarSize / 2, m_sControlColor.d3dColorFont, str.c_str (), D3DFONT_CENTERED_Y );

	// Draw button close
	if ( m_bCloseButtonEnabled )
	{
		D3DCOLOR d3dColorButton = m_sControlColor.d3dColorBox [ m_bPressed ? SControlColor::STATE_PRESSED : SControlColor::STATE_NORMAL ];
		if ( m_rButton.ContainsPoint ( m_pDialog->GetMouse ()->GetPos () ) && !( m_pFocussedControl && m_pFocussedControl->OnClickEvent () ) )
			d3dColorButton = m_sControlColor.d3dColorBox [ m_eState ];

		m_pDialog->DrawBox ( m_rButton, d3dColorButton, m_sControlColor.d3dColorOutline, m_bAntAlias );

		CD3DRender *pRender = m_pDialog->GetRenderer ();
		if ( pRender )
		{
			static const int nSpace = 1;
			pRender->D3DLine ( m_pos.m_nX + m_rBoundingBox.m_size.cx - 20 - nSpace, m_pos.m_nY + 4, m_pos.m_nX + m_rBoundingBox.m_size.cx - 11.2 - nSpace, m_pos.m_nY + 12.2, 0xFFFFFFFF, true );
			pRender->D3DLine ( m_pos.m_nX + m_rBoundingBox.m_size.cx - 12 - nSpace, m_pos.m_nY + 4, m_pos.m_nX + m_rBoundingBox.m_size.cx - 20 - nSpace, m_pos.m_nY + 12, 0xFFFFFFFF, true );
		}
	}

	if ( !m_bMaximized )
	{
		ZeroMemory ( &m_maxControlSize, sizeof ( SIZE ) );
		m_pScrollbar->OnDraw ();

		// Draw all controls
		for ( auto control : m_vControls )
		{
			if ( control )
			{
				Pos *pos = control->GetPos ();
				SIZE size = control->GetSize ();

				if ( pos->m_nY <= m_iTitleBarSize )
					control->SetPosY ( 0 );
				ResizeWidget ( control );
				control->LinkPos ( m_pos + Pos ( 0, m_iTitleBarSize ) - ( m_bShowScrollbar ?
																		  Pos ( pScrollbarHor->GetTrackPos (), pScrollbarVer->GetTrackPos () ) :
																		  Pos () ) );

				CWidget::eRelative relativeX = control->GetRelativeX ();
				CWidget::eRelative relativeY = control->GetRelativeY ();
				/*bool bRelativePosX = control->GetRelativeX () == CWidget::RELATIVE_POS;
				bool bRelativeSizeX = control->GetRelativeX () == CWidget::RELATIVE_SIZE;
				bool bRelativePosX = control->GetRelativeY () == CWidget::RELATIVE_POS;
				bool bRelativeSizeX = control->GetRelativeY () == CWidget::RELATIVE_SIZE;
*/
				if ( !( relativeX == CWidget::RELATIVE_SIZE || relativeX == CWidget::RELATIVE_POS ) ||
					 ( relativeX == CWidget::RELATIVE_POS &&  pos->m_nX <= 0 ) ||  ( control->GetMinSize ().cx == size.cx &&pos->m_nX > 0 ))
				{
					m_maxControlSize.cx = max ( m_maxControlSize.cx, pos->m_nX + size.cx );
				}

				if ( !( relativeY == CWidget::RELATIVE_SIZE || relativeY == CWidget::RELATIVE_POS ) ||
					 ( relativeY == CWidget::RELATIVE_POS && pos->m_nY <= m_iTitleBarSize ) ||
					 /*( control->GetRelativeY () == CWidget::RELATIVE_SIZE &&*/( control->GetMinSize ().cy == size.cy && pos->m_nY >= m_iTitleBarSize) )
				{

					m_maxControlSize.cy = max ( m_maxControlSize.cy, pos->m_nY + size.cy );
				}

				pos = control->GetUpdatedPos ();

				if ( pos->m_nX + size.cx > m_rBoundingBox.m_pos.m_nX &&
					 pos->m_nY + size.cy > m_rBoundingBox.m_pos.m_nY + m_iTitleBarSize &&
					 pos->m_nX < m_rBoundingBox.m_pos.m_nX + m_rBoundingBox.m_size.cx &&
					 pos->m_nY < m_rBoundingBox.m_pos.m_nY + m_rBoundingBox.m_size.cy )
				{
					rScissor = m_rBoundingBox;
					rScissor.m_pos.m_nY += m_iTitleBarSize;
					rScissor.m_size.cx = rScissor.m_size.cx - ( m_pScrollbar->IsVerScrollbarNeeded () ? pScrollbarVer->GetWidth () : 0 );
					rScissor.m_size.cy = rScissor.m_size.cy - ( m_pScrollbar->IsHorScrollbarNeeded () ? pScrollbarHor->GetHeight () : 0 ) - m_iTitleBarSize;

					control->EnterScissorRect ( rScissor );
					control->Draw ();
					control->LeaveScissorRect ();
				}

			}
		}
	}

	sCissor.RestoreScissor ();
	//SetScissor ( pDevice, rOldScissor );
}


void CWindow::ResizeWidget ( CWidget *pWidget, bool bCheckInList )
{/*
	if ( bCheckInList )
	{
		if ( !IsControlInList ( pWidget ) )
			return;
	}
	else if ( !pWidget )
		return;*/

	SIZE size = pWidget->GetRealSize ();

	if ( size.cx >= m_realSize.cx )
	{
		pWidget->SetWidth ( m_realSize.cx );
	}

	int nSize = m_realSize.cy-m_iTitleBarSize ;
	if ( size.cy >= nSize )
	{
		pWidget->SetHeight ( nSize );
	}
}

//--------------------------------------------------------------------------------------
void CWindow::AddControl ( CWidget *pControl )
{
	if ( !pControl || HAS_CONTROL_TYPE ( pControl, CWidget::TYPE_WINDOW ) )
		return;

	pControl->SetPos ( *pControl->GetPos () );
	pControl->SetParent ( this );

	m_vControls.push_back ( pControl );
	ResizeWidget ( pControl );
}

//--------------------------------------------------------------------------------------
void CWindow::RemoveControl ( CWidget *pControl )
{
	if ( !pControl )
		return;

	std::vector<CWidget*>::iterator iter = std::find ( m_vControls.begin (), m_vControls.end (), pControl );
	if ( iter == m_vControls.end () )
		return;

	m_vControls.erase ( iter );
	SAFE_DELETE ( pControl );
}

//--------------------------------------------------------------------------------------
void CWindow::RemoveAllControls ( void )
{
	for ( auto &control : m_vControls )
		SAFE_DELETE ( control );

	m_vControls.clear ();
}

//--------------------------------------------------------------------------------------
void CWindow::SetFocussedWidget ( CWidget *pControl )
{
	if ( m_pFocussedControl != pControl )
	{
		if ( pControl && !pControl->CanHaveFocus () )
			return;

		if ( m_pFocussedControl )
			ClearControlFocus ();

		if ( pControl )
			pControl->OnFocusIn ();

		m_pFocussedControl = pControl;
	}

	if ( pControl )
		BringControlToTop ( pControl );
}

//--------------------------------------------------------------------------------------
CWidget *CWindow::GetFocussedControl ( void )
{
	return m_pFocussedControl;
}

//--------------------------------------------------------------------------------------
void CWindow::ClearControlFocus ( void )
{
	if ( m_pFocussedControl )
	{
		m_pFocussedControl->OnClickLeave ();
		m_pFocussedControl->OnMouseLeave ();
		m_pFocussedControl->OnFocusOut ();
		m_pFocussedControl = NULL;
	}

	if ( m_pControlMouseOver )
	{
		m_pControlMouseOver->OnMouseLeave ();
		m_pControlMouseOver = NULL;
	}
}

//--------------------------------------------------------------------------------------
void CWindow::BringControlToTop ( CWidget *pControl )
{
	std::vector<CWidget*>::iterator iter = std::find ( m_vControls.begin (), m_vControls.end (), pControl );
	if ( iter == m_vControls.end () )
		return;

	m_vControls.erase ( iter );
	m_vControls.insert ( m_vControls.end (), pControl );

	// Make sure the window has focus, otherwise give it focus.
	if ( !m_bHasFocus )
		m_pDialog->SetFocussedWidget ( this );
}

//--------------------------------------------------------------------------------------
CWidget *CWindow::GetNextControl ( CWidget *pControl )
{
	size_t size = m_vControls.size ();
	for ( size_t i = 0; i < size; i++ )
	{
		if ( m_vControls [ i ] )
		{
			if ( m_vControls [ i ] == pControl && i + 1 < size )
				return m_vControls [ i + 1 ];
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------------------
CWidget *CWindow::GetPrevControl ( CWidget *pControl )
{
	for ( size_t i = 0; i < m_vControls.size (); i++ )
	{
		if ( m_vControls [ i ] )
		{
			if ( m_vControls [ i ] == pControl && int ( i - 1 ) > 0 )
				return m_vControls [ i - 1 ];
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------------------
CWidget *CWindow::GetControlByText ( const SIMPLEGUI_CHAR *pszText )
{
	for ( auto &control : m_vControls )
	{
		if ( control && control->GetText () &&
			 pszText )
		{
			if ( !SIMPLEGUI_STRCMP ( control->GetText (), pszText ) )
				return control;
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------------------
CWidget *CWindow::GetControlAtArea ( Pos pos )
{
	for ( std::vector<CWidget*>::reverse_iterator iter = m_vControls.rbegin (); iter != m_vControls.rend (); iter++ )
	{
		if ( ( *iter )->ContainsPoint ( pos ) )
			return ( *iter );
	}

	return NULL;
}

CWidget *CWindow::GetControlClicked ( void )
{
	for ( auto& control : m_vControls )
	{
		if ( control->OnClickEvent () )
			return control;
	}

	return NULL;
}

//--------------------------------------------------------------------------------------
bool CWindow::IsSizing ( void )
{
	return ( m_bSizable && m_eWindowArea != OutArea );
}

//--------------------------------------------------------------------------------------
void CWindow::OnClickLeave ( void )
{
	m_bDragging = false;
	m_eWindowArea = OutArea;

	if ( m_pFocussedControl )
		m_pFocussedControl->OnClickLeave ();

	if ( m_pScrollbar )
		m_pScrollbar->OnClickLeave ();
}

//--------------------------------------------------------------------------------------
bool CWindow::OnClickEvent ( void )
{
	return ( ( m_bPressed || m_bDragging || m_eWindowArea != OutArea ) ||
			 m_pScrollbar->OnClickEvent () );
}

//--------------------------------------------------------------------------------------
void CWindow::OnFocusIn ( void )
{
	CWidget::OnFocusIn ();

	if ( m_pScrollbar )
		m_pScrollbar->OnFocusIn ();
}

//--------------------------------------------------------------------------------------
void CWindow::OnFocusOut ( void )
{
	CWidget::OnFocusOut ();

	ClearControlFocus ();

	if ( m_pScrollbar )
		m_pScrollbar->OnFocusOut ();
}

//--------------------------------------------------------------------------------------
void CWindow::OnMouseEnter ( void )
{
	CWidget::OnMouseEnter ();
}

//--------------------------------------------------------------------------------------
void CWindow::OnMouseLeave ( void )
{
	CWidget::OnMouseLeave ();

	if ( m_eWindowArea == OutArea )
		m_pDialog->GetMouse ()->SetCursorType ( CMouse::DEFAULT );

	if ( m_pScrollbar )
		m_pScrollbar->OnMouseLeave ();

	if ( m_pControlMouseOver )
	{
		m_pControlMouseOver->OnMouseLeave ();
		m_pControlMouseOver = NULL;
	}
}

//--------------------------------------------------------------------------------------
bool CWindow::CanHaveFocus ( void )
{
	return ( CWidget::CanHaveFocus () || m_pScrollbar->CanHaveFocus () );
}

//--------------------------------------------------------------------------------------
void CWindow::SetAlwaysOnTop ( bool bEnable )
{
	m_bOnTop = bEnable;
};

//--------------------------------------------------------------------------------------
bool CWindow::GetAlwaysOnTop ( void )
{
	return m_bOnTop;
}

//--------------------------------------------------------------------------------------
void CWindow::SetMovable ( bool bEnabled )
{
	m_bMovable = bEnabled;
}

//--------------------------------------------------------------------------------------
bool CWindow::GetMovable ( void )
{
	return m_bMovable;
}

//--------------------------------------------------------------------------------------
void CWindow::SetSizable ( bool bEnabled )
{
	m_bSizable = bEnabled;
}

//--------------------------------------------------------------------------------------
bool CWindow::GetSizable ( void )
{
	return m_bSizable;
}

//--------------------------------------------------------------------------------------
void CWindow::SetMaximized ( bool bMinimize )
{
	m_bMaximized;
}

//--------------------------------------------------------------------------------------
bool CWindow::GetMaximized ( void )
{
	return m_bMaximized;
}

//--------------------------------------------------------------------------------------
void CWindow::SetSize ( SIZE size )
{
	SetSize ( size.cx, size.cy );
}

//--------------------------------------------------------------------------------------
void CWindow::SetSize ( int iWidth, int iHeight )
{
	m_realSize.cx = iWidth;
	m_realSize.cy = iHeight;

	SetWidth ( iWidth );
	SetHeight ( iHeight );
}

//--------------------------------------------------------------------------------------
SIZE CWindow::GetRealSize ( void )
{
	return m_realSize;
}

//--------------------------------------------------------------------------------------
void CWindow::ScrollPage ( int nDelta )
{
	if ( !m_pScrollbar )
		return;

	m_pScrollbar->OnMouseWheel ( nDelta );
}

CWidget *CWindow::GetTabPanelFocussedControl ( void )
{
	CWidget *pControl = m_pFocussedControl;
	if ( HAS_CONTROL_TYPE ( pControl, CWidget::TYPE_TABPANEL ) )
		pControl = static_cast< CTabPanel* >( pControl )->GetFocussedControl ();

	return pControl;
}

//--------------------------------------------------------------------------------------
int CWindow::GetTitleBarHeight ( void )
{
	return m_iTitleBarSize;
}

//--------------------------------------------------------------------------------------
void CWindow::SetCloseButton ( bool bEnabled )
{
	m_bCloseButtonEnabled = bEnabled;
}

void CWindow::ShowScrollbars ( bool bShow )
{
	m_bShowScrollbar = bShow;
	m_pScrollbar->ShowScrollHor ( bShow );
	m_pScrollbar->ShowScrollVer ( bShow );
}

//--------------------------------------------------------------------------------------
bool CWindow::ControlMessages ( sControlEvents e )
{
	CWidget *pControl = GetTabPanelFocussedControl ();

	if ( !CWidget::CanHaveFocus () ||
		 m_eWindowArea != OutArea ||
		 ( m_pScrollbar->ContainsPoint ( e.mouseEvent.pos ) && !HAS_CONTROL_TYPE ( pControl, CWidget::TYPE_DROPDOWN ) ) ||
		 GetSizingBorderAtArea ( e.mouseEvent.pos ) != OutArea ||
		 m_rTitle.ContainsPoint ( e.mouseEvent.pos ) )
	{
		return false;
	}

	if ( HAS_CONTROL_TYPE ( m_pFocussedControl, CWidget::TYPE_TABPANEL ) )
	{
		if ( static_cast< CTabPanel* >( m_pFocussedControl )->ControlMessages ( e ) )
			return true;
	}

	if ( m_pFocussedControl && m_pFocussedControl->InjectKeyboard ( e.keyEvent ) )
		return true;

	pControl = GetControlAtArea ( e.mouseEvent.pos );
	if ( !pControl
		 && e.mouseEvent.eMouseMessages == sMouseEvents::ButtonDown &&
		 e.mouseEvent.eButton == sMouseEvents::LeftButton &&
		 m_pFocussedControl )
	{
		ClearControlFocus ();
	}

	if ( HAS_CONTROL_TYPE ( pControl, CWidget::TYPE_TABPANEL ) )
	{
		if ( static_cast< CTabPanel* >( pControl )->ControlMessages ( e ) )
			return true;
	}

	// it the first chance at handling the message.
	if ( m_pFocussedControl && m_pFocussedControl->InjectMouse ( e.mouseEvent ) )
		return true;

	if ( pControl && pControl->InjectMouse ( e.mouseEvent ) )
		return true;

	if ( !GetAsyncKeyState ( VK_LBUTTON ) )
	{
		// If the mouse is still over the same control, nothing needs to be done
		if ( pControl == m_pControlMouseOver )
			return false;

		// Handle mouse leaving the old control
		if ( m_pControlMouseOver )
			m_pControlMouseOver->OnMouseLeave ();

		// Handle mouse entering the new control
		m_pControlMouseOver = pControl;
		if ( m_pControlMouseOver )
			m_pControlMouseOver->OnMouseEnter ();

	}

	return false;
}

bool CWindow::OnMouseButtonDown ( sMouseEvents e )
{
	CWidget *pControl = GetTabPanelFocussedControl ();

	// Check if mouse is over window boundaries
	if ( GetSizingBorderAtArea ( e.pos ) == OutArea && !HAS_CONTROL_TYPE ( pControl, CWidget::TYPE_DROPDOWN ) )
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
		if ( m_rButton.ContainsPoint ( e.pos ) &&
			 m_bCloseButtonEnabled )
		{
			// Pressed while inside the control
			m_bPressed = true;

			if ( !m_bHasFocus )
				m_pDialog->SetFocussedWidget ( this );

			return true;
		}

		if ( m_bSizable )
		{
			E_WINDOW_AREA eArea = GetSizingBorderAtArea ( e.pos );
			if ( eArea != OutArea )
			{
				m_eWindowArea = eArea;

				if ( m_eWindowArea == Top ||
					 m_eWindowArea == TopLeft ||
					 m_eWindowArea == TopRight )
				{
					m_nDragY = m_pos.m_nY - e.pos.m_nY;
					if ( m_eWindowArea == TopLeft )
					{
						m_nDragX = m_pos.m_nX - e.pos.m_nX;
					}
					else if ( m_eWindowArea == TopRight )
					{
						m_nDragX = m_pos.m_nX + m_rBoundingBox.m_size.cx - e.pos.m_nX;
					}
				}
				else if ( m_eWindowArea == Left ||
						  m_eWindowArea == BottomLeft )
				{
					m_nDragX = m_pos.m_nX - e.pos.m_nX;

					if ( m_eWindowArea == BottomLeft )
					{
						m_nDragY = m_pos.m_nY + m_rBoundingBox.m_size.cy - e.pos.m_nY;
					}
				}
				else if ( m_eWindowArea == Right ||
						  m_eWindowArea == BottomRight )
				{
					m_nDragX = m_pos.m_nX + m_rBoundingBox.m_size.cx - e.pos.m_nX;

					if ( m_eWindowArea == BottomRight )
					{
						m_nDragY = m_pos.m_nY + m_rBoundingBox.m_size.cy - e.pos.m_nY;
					}
				}
				else if ( m_eWindowArea == Bottom )
				{
					m_nDragY = m_pos.m_nY + m_rBoundingBox.m_size.cy - e.pos.m_nY;
				}

				if ( !m_bHasFocus )
					m_pDialog->SetFocussedWidget ( this );

				return true;
			}
		}

		if ( m_rTitle.ContainsPoint ( e.pos ) )
		{
			if ( m_pDialog->GetMouse ()->GetLeftButton () == 2 )
				m_bMaximized = !m_bMaximized;

			if ( m_bMovable )
			{
				m_bDragging = true;
				m_posDif = m_pos - e.pos;
			}

			if ( !m_bHasFocus )
				m_pDialog->SetFocussedWidget ( this );

			return true;
		}

		if ( m_rBoundingBox.ContainsPoint ( e.pos ) && !m_bMaximized )
		{
			if ( !m_bHasFocus )
				m_pDialog->SetFocussedWidget ( this );

			return true;
		}
	}

	return false;
}

bool CWindow::OnMouseButtonUp ( sMouseEvents e )
{
	CWidget *pControl = GetTabPanelFocussedControl ();

	// Check if mouse is over window boundaries
	if ( GetSizingBorderAtArea ( e.pos ) == OutArea && !HAS_CONTROL_TYPE ( pControl, CWidget::TYPE_DROPDOWN ) )
	{
		// Let the scroll bar handle it first.
		if ( m_pScrollbar->OnMouseButtonUp ( e ) )
			return true;
	}

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		if ( m_pControlMouseOver )
			m_pControlMouseOver->OnMouseLeave ();

		m_pControlMouseOver = NULL;
		m_bDragging = false;
		m_eWindowArea = OutArea;
		m_size = m_rBoundingBox.m_size;
	}

	if ( m_bPressed )
	{
		m_bPressed = false;

		// Button click
		if ( m_rButton.ContainsPoint ( e.pos ) )
		{
			SendEvent ( EVENT_CONTROL_CLICKED, true );
			SetVisible ( false );

			if ( m_pDialog )
				m_pDialog->ClearFocussedWidget ();

		}

		return true;
	}

	return false;
}

bool CWindow::OnMouseMove ( CVector pos )
{
	CWidget *pControl = GetTabPanelFocussedControl ();
	bool bHasDropDown = HAS_CONTROL_TYPE ( pControl, CWidget::TYPE_DROPDOWN );

	if ( m_pScrollbar->ContainsPoint ( pos ) && !bHasDropDown && m_bMouseOver )
		m_pScrollbar->OnMouseEnter ();
	else
		m_pScrollbar->OnMouseLeave ();

	// Check if mouse is over window boundaries
	if ( GetSizingBorderAtArea ( pos ) == OutArea )
	{
		// Let the scroll bar handle it first.
		if ( m_pScrollbar->OnMouseMove ( pos ) )
			return true;
	}

	if ( m_rTitle.ContainsPoint ( pos ) )
	{
		if ( m_pControlMouseOver )
			m_pControlMouseOver->OnMouseLeave ();

		m_pControlMouseOver = NULL;
	}

	if ( !(bHasDropDown && pControl->ContainsPoint ( pos )) &&
		 m_bMouseOver &&
		 m_eWindowArea == OutArea )
	{
		SetCursorForPoint ( pos );
	}

	if ( m_bSizable && m_eWindowArea != OutArea )
	{
		if ( m_eWindowArea == Top ||
			 m_eWindowArea == TopLeft ||
			 m_eWindowArea == TopRight )
		{
			m_rBoundingBox.m_size.cy +=  m_pos.m_nY - pos.m_nY  - m_nDragY;
			m_pos.m_nY = pos.m_nY + m_nDragY;

			if ( m_eWindowArea == TopLeft )
			{
				m_rBoundingBox.m_size.cx += (m_pos.m_nX - pos.m_nX) - m_nDragX;
				m_pos.m_nX = pos.m_nX + m_nDragX;
			}
			else if ( m_eWindowArea == TopRight )
				m_rBoundingBox.m_size.cx = pos.m_nX - m_pos.m_nX + m_nDragX;
		}
		else if ( m_eWindowArea == Left ||
				  m_eWindowArea == BottomLeft )
		{
			m_rBoundingBox.m_size.cx += (m_pos.m_nX - pos.m_nX ) - m_nDragX;
			m_pos.m_nX = pos.m_nX + m_nDragX;

			if ( m_eWindowArea == BottomLeft )
				m_rBoundingBox.m_size.cy = pos.m_nY - m_pos.m_nY + m_nDragY;
		}
		else if ( m_eWindowArea == Right ||
				  m_eWindowArea == BottomRight )
		{
			m_rBoundingBox.m_size.cx = pos.m_nX - m_pos.m_nX + m_nDragX;

			if ( m_eWindowArea == BottomRight )
				m_rBoundingBox.m_size.cy = pos.m_nY - m_pos.m_nY + m_nDragY;
		}
		else if ( m_eWindowArea == Bottom )
			m_rBoundingBox.m_size.cy = pos.m_nY - m_pos.m_nY + m_nDragY;

		if ( m_rBoundingBox.m_size.cx < m_minSize.cx )
			m_rBoundingBox.m_size.cx = m_minSize.cx;

		if ( m_rBoundingBox.m_size.cy < m_minSize.cy )
			m_rBoundingBox.m_size.cy = m_minSize.cy;

		// Adjust position
		m_rBoundingBox.m_pos = m_pos;
		return true;
	}

	if ( m_bDragging &&
		 m_bMovable )
	{
		// Adjust position
		m_rBoundingBox.m_pos = m_pos = pos + m_posDif;
		return true;
	}

	return false;
}

bool CWindow::OnMouseWheel ( int zDelta )
{
	if (  !m_pFocussedControl || !m_pControlMouseOver && m_bMouseOver )
	{
		ScrollPage ( -zDelta );
		return true;
	}

	return false;
}

bool CWindow::OnKeyDown ( WPARAM wParam )
{
	if ( !CWidget::CanHaveFocus () )
		return false;

	switch ( wParam )
	{
		case VK_TAB:
		{
			SetFocussedWidget ( m_vControls [ 0 ] );
			break;
		}
		case VK_PRIOR:
		{
			ScrollPage ( -int ( m_rBoundingBox.m_size.cy / 10 ) );
			break;
		}
		case VK_NEXT:
		{
			ScrollPage ( ( m_rBoundingBox.m_size.cy / 10 ) );
			break;
		}
		case VK_UP:
		{
			ScrollPage ( -int ( m_maxControlSize.cy / m_rBoundingBox.m_size.cy ) );
			break;
		}
		case VK_DOWN:
		{
			ScrollPage ( ( m_maxControlSize.cy / m_rBoundingBox.m_size.cy ) );
			break;
		}
	}

	return false;
}

#define WINDOW_SIZE_CORNERS 5

//--------------------------------------------------------------------------------------
void CWindow::UpdateRects ( void )
{
	if ( !m_pScrollbar )
		return;

	SControlRect rRect = m_rBoundingBox;
	rRect.m_size.cy -= m_iTitleBarSize;
	rRect.m_pos.m_nY += m_iTitleBarSize;

	m_pScrollbar->SetTrackRange ( m_maxControlSize.cx, m_maxControlSize.cy );
	m_pScrollbar->GetHorScrollbar ()->SetStepSize ( m_rBoundingBox.m_size.cx / 10 );
	m_pScrollbar->GetVerScrollbar ()->SetStepSize ( m_rBoundingBox.m_size.cy / 10 );
	m_pScrollbar->UpdateScrollbars ( rRect );

	m_rTitle = m_rBoundingBox;
	m_rTitle.m_size.cy = m_iTitleBarSize;

	// Window corners
	m_rWindowTop = m_rBoundingBox;
	m_rWindowTop.m_pos.m_nY -= WINDOW_SIZE_CORNERS;
	m_rWindowTop.m_size.cx = m_rWindowTop.m_size.cx - 2;
	m_rWindowTop.m_size.cy = WINDOW_SIZE_CORNERS;

	m_rWindowLeft = m_rBoundingBox;
	m_rWindowLeft.m_pos.m_nX -= WINDOW_SIZE_CORNERS ;
	m_rWindowLeft.m_size.cx = WINDOW_SIZE_CORNERS;

	m_rWindowRight = m_rBoundingBox;
	m_rWindowRight.m_pos.m_nX += m_rBoundingBox.m_size.cx ;
	m_rWindowRight.m_size.cx = WINDOW_SIZE_CORNERS;

	m_rWindowBottom = m_rBoundingBox;
	m_rWindowBottom.m_pos.m_nY += m_rBoundingBox.m_size.cy;
	m_rWindowBottom.m_size.cx = m_rWindowBottom.m_size.cx - 2;
	m_rWindowBottom.m_size.cy = WINDOW_SIZE_CORNERS;

	m_rWindowTopLeft = m_rBoundingBox;
	m_rWindowTopLeft.m_pos.m_nX -= WINDOW_SIZE_CORNERS;
	m_rWindowTopLeft.m_pos.m_nY -= WINDOW_SIZE_CORNERS;
	m_rWindowTopLeft.m_size.cx = WINDOW_SIZE_CORNERS;
	m_rWindowTopLeft.m_size.cy = WINDOW_SIZE_CORNERS;

	m_rWindowTopRight = m_rBoundingBox;
	m_rWindowTopRight.m_pos.m_nX += m_rBoundingBox.m_size.cx;
	m_rWindowTopRight.m_pos.m_nY -= WINDOW_SIZE_CORNERS;
	m_rWindowTopRight.m_size.cx = WINDOW_SIZE_CORNERS;
	m_rWindowTopRight.m_size.cy = WINDOW_SIZE_CORNERS;

	m_rWindowBottomLeft = m_rBoundingBox;
	m_rWindowBottomLeft.m_pos.m_nX -= WINDOW_SIZE_CORNERS;
	m_rWindowBottomLeft.m_pos.m_nY += m_rBoundingBox.m_size.cy;
	m_rWindowBottomLeft.m_size.cx = WINDOW_SIZE_CORNERS;
	m_rWindowBottomLeft.m_size.cy = WINDOW_SIZE_CORNERS;

	m_rWindowBottomRight = m_rBoundingBox;
	m_rWindowBottomRight.m_pos.m_nX += m_rBoundingBox.m_size.cx - WINDOW_SIZE_CORNERS;
	m_rWindowBottomRight.m_pos.m_nY += m_rBoundingBox.m_size.cy;
	m_rWindowBottomRight.m_size.cx = WINDOW_SIZE_CORNERS;
	m_rWindowBottomRight.m_size.cy = WINDOW_SIZE_CORNERS;

	// Button 
	m_rButton = m_rBoundingBox;
	m_rButton.m_pos.m_nX += m_rBoundingBox.m_size.cx - 35;
	m_rButton.m_size.cx = 35.f;
	m_rButton.m_size.cy = 16.f;
}

//--------------------------------------------------------------------------------------
bool CWindow::ContainsPoint ( CVector pos )
{
	if ( !CWidget::CanHaveFocus () ||
		 !m_pScrollbar )
		return false;

	CWidget *pControl = GetTabPanelFocussedControl ();

	return ( ( m_pScrollbar->ContainsPoint ( pos ) && m_eWindowArea == OutArea ) ||
			 ( m_rBoundingBox.ContainsPoint ( pos ) && !m_bMaximized ) ||
			 m_rTitle.ContainsPoint ( pos ) ||
			 m_rButton.ContainsPoint ( pos ) ||
			 GetSizingBorderAtArea ( pos ) != OutArea ||
			 ( HAS_CONTROL_TYPE ( pControl, CWidget::EControlType::TYPE_DROPDOWN ) && pControl->ContainsPoint ( pos ) ) );
}

void CWindow::SetCursorForPoint ( CVector pos )
{
	if ( m_rButton.ContainsPoint ( pos ) )
	{
		m_pDialog->GetMouse ()->SetCursorType ( CMouse::DEFAULT );
		return;
	}

	switch ( GetSizingBorderAtArea ( pos ) )
	{
		case TopLeft:
		case BottomRight:
			m_pDialog->GetMouse ()->SetCursorType ( CMouse::NE_RESIZE );
			break;

		case BottomLeft:
		case TopRight:
			m_pDialog->GetMouse ()->SetCursorType ( CMouse::SE_RESIZE );
			break;

		case Top:
			m_pDialog->GetMouse ()->SetCursorType ( CMouse::S_RESIZE );
			break;

		case Left:
		case Right:
			m_pDialog->GetMouse ()->SetCursorType ( CMouse::E_RESIZE );
			break;

		case Bottom:
			m_pDialog->GetMouse ()->SetCursorType ( CMouse::N_RESIZE );
			break;

		default:
			m_pDialog->GetMouse ()->SetCursorType ( CMouse::DEFAULT );
			break;
	}
}

CWindow::E_WINDOW_AREA CWindow::GetSizingBorderAtArea ( CVector pos )
{
	if ( ( m_pFocussedControl && m_pFocussedControl->OnClickEvent () ) ||
		( m_bDragging || m_bPressed || m_bMaximized ) ||
		 m_pScrollbar->OnClickEvent () )
	{
		return OutArea;
	}

	if ( m_rWindowTopLeft.ContainsPoint ( pos ) )
	{
		return TopLeft;
	}
	else if ( m_rWindowBottomLeft.ContainsPoint ( pos ) )
	{
		return BottomLeft;
	}
	else if ( m_rWindowTopRight.ContainsPoint ( pos ) )
	{
		return TopRight;
	}
	else if ( m_rWindowBottomRight.ContainsPoint ( pos ) )
	{
		return BottomRight;
	}
	else if ( m_rWindowTop.ContainsPoint ( pos ) )
	{
		return Top;
	}
	else if ( m_rWindowLeft.ContainsPoint ( pos ) )
	{
		return Left;
	}
	else if ( m_rWindowRight.ContainsPoint ( pos ) )
	{
		return Right;
	}
	else if ( m_rWindowBottom.ContainsPoint ( pos ) )
	{
		return Bottom;
	}
	else
	{
		return OutArea;
	}
}
//--------------------------------------------------------------------------------------
SControlRect *CWindow::GetWindowRect ( E_WINDOW_AREA eArea )
{
	if ( m_pFocussedControl &&
		 m_pFocussedControl->OnClickEvent () ||
		 ( m_bDragging || m_bPressed || m_bMaximized ) ||
		 m_pScrollbar->OnClickEvent () )
		return NULL;

	switch ( eArea )
	{
		case TopLeft:	return &m_rWindowTopLeft;	case BottomLeft:	return &m_rWindowBottomLeft;
		case TopRight:	return &m_rWindowTopRight;	case BottomRight:	return &m_rWindowBottomRight;
		case Top:		return &m_rWindowTop;		case Left:			return &m_rWindowLeft;
		case Right:		return &m_rWindowRight;		case Bottom:		return &m_rWindowBottom;
		
		default:		return NULL;
	}
}