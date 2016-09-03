#include "CGUI.h"

//--------------------------------------------------------------------------------------
CWindow::CWindow ( CDialog *pDialog ) :
	m_pFocussedControl ( NULL ),
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

	m_sControlColor.d3dColorWindowTitle = D3DCOLOR_RGBA ( 21, 75, 138, 255 );
	m_sControlColor.d3dCOlorWindowBack = D3DCOLOR_RGBA ( 60, 60, 60, 255 );
	m_sControlColor.d3dColorBoxSel = D3DCOLOR_RGBA ( 50, 50, 50, 255 );

	m_sControlColor.d3dColorBox [ SControlColor::STATE_NORMAL ] = D3DCOLOR_RGBA ( 180, 0, 0, 255 );
	m_sControlColor.d3dColorBox [ SControlColor::STATE_MOUSE_OVER ] = D3DCOLOR_XRGB ( 255, 0, 0, 255 );
	m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ] = D3DCOLOR_XRGB ( 100, 0, 0, 255 );
	m_sControlColor.d3dColorBox [ SControlColor::STATE_DISABLED ] = D3DCOLOR_XRGB ( 180, 180, 180, 255 );

	ZeroMemory ( &m_maxControlSize, sizeof ( SIZE ) );

	m_pScrollbar = new CScrollablePane ( pDialog );
	if ( !m_pScrollbar )
		MessageBox ( 0, _UI ( "CWindow::CWindow: Error for creating CScrollBarHorizontal" ), _UI ( "GUIAPI.asi" ), 0 );

	m_pScrollbar->AddControl ( this );
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
	else if ( m_bMouseOver &&  m_eWindowArea == OutArea )
		m_eState = SControlColor::STATE_MOUSE_OVER;
	else
		m_eState = SControlColor::STATE_NORMAL;

	IDirect3DDevice9 *pDevice = m_pDialog->GetDevice ();

	RECT rOldScissor;
	pDevice->GetScissorRect ( &rOldScissor );

	SControlRect rScissor = m_rBoundingBox;
	rScissor.pos.SetX ( rScissor.pos.GetX () - 2 );
	rScissor.pos.SetY ( rScissor.pos.GetY () - 2 );
	rScissor.size.cx = rScissor.size.cx + 4;
	rScissor.size.cy = rScissor.size.cy + 4;
	SetScissor ( pDevice, rScissor.GetRect () );

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
	m_pFont->CutString ( m_rBoundingBox.size.cx - m_rButton.size.cx - 5, str );
	m_pFont->Print ( m_pos.GetX () + 3, m_pos.GetY () + m_iTitleBarSize / 2, m_sControlColor.d3dColorFont, str.c_str (), D3DFONT_CENTERED_Y );

	// Draw button close
	if ( m_bCloseButtonEnabled )
	{
		D3DCOLOR d3dColorButton = m_sControlColor.d3dColorBox [ m_bPressed ? SControlColor::STATE_PRESSED : SControlColor::STATE_NORMAL ];
		if ( m_rButton.InControlArea ( m_pDialog->GetMouse ()->GetPos () ) && !m_bControlClicked )
			d3dColorButton = m_sControlColor.d3dColorBox [ m_eState ];

		m_pDialog->DrawBox ( m_rButton, d3dColorButton, m_sControlColor.d3dColorOutline, m_bAntAlias );

		CD3DRender *pRender = m_pDialog->GetRenderer ();
		if ( pRender )
		{
			static const int nSpace = 1;
			pRender->D3DLine ( m_pos.GetX () + m_rBoundingBox.size.cx - 20 - nSpace, m_pos.GetY () + 4, m_pos.GetX () + m_rBoundingBox.size.cx - 11.2 - nSpace, m_pos.GetY () + 12.2, 0xFFFFFFFF, true );
			pRender->D3DLine ( m_pos.GetX () + m_rBoundingBox.size.cx - 12 - nSpace, m_pos.GetY () + 4, m_pos.GetX () + m_rBoundingBox.size.cx - 20 - nSpace, m_pos.GetY () + 12, 0xFFFFFFFF, true );
		}
	}

	if ( !m_bMaximized )
	{
		bool bUpdate [ 2 ];
		ZeroMemory ( &m_maxControlSize, sizeof ( SIZE ) );

		m_pScrollbar->OnDraw ();

		if ( !GetAsyncKeyState ( VK_LBUTTON ) )
			m_bControlClicked = false;

		// Draw all controls
		for ( auto control : m_vControls )
		{
			if ( control )
			{
				if ( control->OnClickEvent () )
					m_bControlClicked = true;

				control->LinkPos ( m_pos - ( m_bShowScrollbar ?
								   CPos ( pScrollbarHor->GetTrackPos (), pScrollbarVer->GetTrackPos () ) :
								   CPos () ) );

				bUpdate [ 0 ] = control->IsSizingX () ;
				bUpdate [ 1 ] = control->IsSizingY () ;

				SIZE size = control->GetSize ();
				CPos *pos = control->GetUpdatedPos ();

				if ( pos->GetX () + size.cx > m_rBoundingBox.pos.GetX () &&
					 pos->GetY () + size.cy > m_rBoundingBox.pos.GetY () + m_iTitleBarSize &&
					 pos->GetX () < m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cx &&
					 pos->GetY () < m_rBoundingBox.pos.GetY () + m_rBoundingBox.size.cy )
				{
					rScissor = m_rBoundingBox;
					rScissor.pos.SetY ( rScissor.pos.GetY () + m_iTitleBarSize );
					rScissor.size.cx = rScissor.size.cx - ( m_pScrollbar->IsVerScrollbarNeeded () ? pScrollbarVer->GetWidth () : 0 );
					rScissor.size.cy = rScissor.size.cy - ( m_pScrollbar->IsHorScrollbarNeeded () ? pScrollbarHor->GetHeight () : 0 ) - m_iTitleBarSize;

					control->EnterScissorRect ( rScissor );
					control->Draw ();
					control->LeaveScissorRect ();
				}

				/*if ( !control->IsRelativePos () )
				{*/
				m_maxControlSize.cx = max ( m_maxControlSize.cx, pos->GetX () + size.cx );
				m_maxControlSize.cy = max ( m_maxControlSize.cy, pos->GetY () + size.cy );
				//}
			}
		}

		// Check that the control is changed position or size, or if
		// window is changed size
		UpdateScrollbars ( bUpdate [ 0 ], bUpdate [ 1 ] );

	}

	SetScissor ( pDevice, rOldScissor );
}

//--------------------------------------------------------------------------------------
void CWindow::AddControl ( CControl *pControl )
{
	if ( !pControl || 
		 pControl->GetType() == CControl::EControlType::TYPE_WINDOW )
		return;

	pControl->SetPos ( *pControl->GetPos () /*+ m_pos*/ + CPos ( 0, m_iTitleBarSize ) );
	pControl->SetParent ( this );

	m_vControls.push_back ( pControl );
}

//--------------------------------------------------------------------------------------
void CWindow::RemoveControl ( CControl *pControl )
{
	if ( !pControl )
		return;

	std::vector<CControl*>::iterator iter = std::find ( m_vControls.begin (), m_vControls.end (), pControl );
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
void CWindow::SetFocussedControl ( CControl *pControl )
{
	if ( m_pFocussedControl != pControl )
	{
		if ( pControl &&
			 !pControl->CanHaveFocus () )
			return;

		if ( m_pFocussedControl )
			m_pFocussedControl->OnFocusOut ();

		if ( pControl )
			pControl->OnFocusIn ();

		m_pFocussedControl = pControl;
	}

	if ( pControl )
		BringControlToTop ( pControl );
}

//--------------------------------------------------------------------------------------
CControl *CWindow::GetFocussedControl ( void )
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
		m_pControlMouseOver->OnMouseLeave ();
	m_pControlMouseOver = NULL;
}

//--------------------------------------------------------------------------------------
void CWindow::BringControlToTop ( CControl *pControl )
{
	std::vector<CControl*>::iterator iter = std::find ( m_vControls.begin (), m_vControls.end (), pControl );
	if ( iter == m_vControls.end () )
		return;

	m_vControls.erase ( iter );
	m_vControls.insert ( m_vControls.end (), pControl );

	// Make sure the window has focus, otherwise give it focus.
	if ( !m_bHasFocus )
		m_pDialog->SetFocussedWindow ( this );
}

//--------------------------------------------------------------------------------------
CControl *CWindow::GetNextControl ( CControl *pControl )
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
CControl *CWindow::GetPrevControl ( CControl *pControl )
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
CControl *CWindow::GetControlByText ( const SIMPLEGUI_CHAR *pszText )
{
	for ( auto &control : m_vControls )
	{
		if ( control &&
			 control->GetText () &&
			 pszText )
		{
			if ( !SIMPLEGUI_STRCMP ( control->GetText (), pszText ) )
				return control;
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------------------
CControl *CWindow::GetControlAtArea ( CPos pos )
{
	for ( std::vector<CControl*>::iterator iter = m_vControls.end (); iter != m_vControls.begin (); )
	{
		iter--;
		if ( ( *iter )->ContainsRect ( pos ) )
			return ( *iter );
	}

	return NULL;
}

CControl *CWindow::GetControlClicked ( void )
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
	return ( ( m_eWindowArea != OutArea &&
			 m_bSizable ) &&
			 CControl::IsSizing () );
}

//--------------------------------------------------------------------------------------
void CWindow::OnClickLeave ( void )
{
	m_bDragging = false;
	m_eWindowArea = OutArea;

	if( m_pFocussedControl )
		m_pFocussedControl->OnClickLeave ();

	if ( m_pScrollbar )
		m_pScrollbar->OnClickLeave ();
}

//--------------------------------------------------------------------------------------
bool CWindow::OnClickEvent ( void )
{
	return ( m_bPressed ||
			 m_bDragging ||
			 m_eWindowArea != OutArea ||
			 ( m_pScrollbar && m_pScrollbar->OnClickEvent () ) );
}

//--------------------------------------------------------------------------------------
void CWindow::OnFocusIn ( void )
{
	CControl::OnFocusIn ();

	if ( m_pScrollbar )
		m_pScrollbar->OnFocusIn ();
}

//--------------------------------------------------------------------------------------
void CWindow::OnFocusOut ( void )
{
	CControl::OnFocusOut ();

	ClearControlFocus ();

	if ( m_pScrollbar )
		m_pScrollbar->OnFocusOut ();
}

//--------------------------------------------------------------------------------------
void CWindow::OnMouseEnter ( void )
{
	CControl::OnMouseEnter ();

	if ( m_pScrollbar && !m_bControlClicked )
		m_pScrollbar->OnMouseEnter ();

	if ( m_pControlMouseOver )
	{
		m_pControlMouseOver->OnMouseEnter ();
	}
}

//--------------------------------------------------------------------------------------
void CWindow::OnMouseLeave ( void )
{
	CControl::OnMouseLeave ();

	if(m_eWindowArea == OutArea )
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
	return ( CControl::CanHaveFocus () ||
			 ( m_pScrollbar && 
			 m_pScrollbar->CanHaveFocus () ) );
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
	UpdateScrollbars (true,true);
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
void CWindow::OnMouseMove ( CControl *pControl, UINT uMsg )
{
	if ( !( GetAsyncKeyState ( VK_LBUTTON ) /*&& 
		 pControl */) &&
		 uMsg == WM_MOUSEMOVE )
	{
		// If the mouse is still over the same control, nothing needs to be done
		if ( pControl == m_pControlMouseOver )
			return;

		// Handle mouse leaving the old control
		if ( m_pControlMouseOver )
			m_pControlMouseOver->OnMouseLeave ();

		// Handle mouse entering the new control
		m_pControlMouseOver = pControl;
		if ( pControl != NULL )
			pControl->OnMouseEnter ();
	}
}

//--------------------------------------------------------------------------------------
bool CWindow::ControlMessages ( UINT uMsg, CPos pos, WPARAM wParam, LPARAM lParam )
{
	if ( !CControl::CanHaveFocus () ||
		 m_eWindowArea != OutArea ||
		 m_pScrollbar->ContainsRect ( pos ) ||
		 GetSizingBorderAtArea(pos) != OutArea ||
		 m_rTitle.InControlArea ( pos ) )
	{
		return false;
	}

	switch ( uMsg )
	{
		// Keyboard messages
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_CHAR:
		{
			// If a control is in focus, and it's enabled, then give
			// it the first chance at handling the message.
			if ( m_pFocussedControl &&
				 m_pFocussedControl->IsEnabled ())
			{
				/*if ( uMsg == WM_KEYDOWN )
				{
					if ( m_pFocussedControl->OnKeyDown (  wParam) )
						return true;
				}
				else if ( uMsg == WM_KEYUP )
				{
					if ( m_pFocussedControl->OnKeyUp ( wParam ) )
						return true;
				}
				else if ( uMsg == WM_CHAR )
				{
					if ( m_pFocussedControl->OnKeyCharacter ( wParam ) )
						return true;
				}*/
				sKeyEvents e;
				e.uMsg = uMsg;
				e.wKey = wParam;
				if ( m_pFocussedControl->InjectKeyboard ( e ) )return true;
			}
			break;
		}

		// Mouse messages
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_LBUTTONDBLCLK:
		case WM_MOUSEWHEEL:
		{
			CControl* pControl = GetControlAtArea ( pos );
			if ( !pControl&& uMsg == WM_LBUTTONDOWN &&
				 m_pFocussedControl )
			{
				ClearControlFocus ();
			}

			int zDelta = int ( ( short ) HIWORD ( wParam ) ) / WHEEL_DELTA;

			// it the first chance at handling the message.
			if ( SetControlMouseStates ( uMsg, -zDelta, pos, m_pFocussedControl ) )
				return true;

			if ( SetControlMouseStates ( uMsg, -zDelta, pos, pControl ) )
				return true;

			OnMouseMove ( pControl, uMsg );

			if ( !m_pControlMouseOver &&
				 uMsg == WM_MOUSEWHEEL )
			{
				ScrollPage ( -zDelta );
				return true;
			}
		}
		break;
	}

	return false;
}

bool CWindow::SetControlMouseStates ( UINT uMsg, int zDelta, CPos pos, CControl *pControl )
{
	if ( pControl &&
		 pControl->IsEnabled () )
	{
		if ( uMsg == WM_MOUSEMOVE )
		{
			if ( pControl->OnMouseMove ( pos ) )
				return true;
		}
		else if ( uMsg == WM_LBUTTONDOWN )
		{
			if ( pControl->OnMouseButtonDown ( pos ) )
				return true;
		}
		else if ( uMsg == WM_LBUTTONUP )
		{
			if ( pControl->OnMouseButtonUp ( pos ) )
				return true;
		}
		else if ( uMsg == WM_MOUSEHWHEEL )
		{
			if ( pControl->OnMouseWheel ( zDelta ) )
				return true;
		}
	}

	return false;
}

bool CWindow::OnMouseButtonDown ( CPos pos )

{	// Check if mouse is over window boundaries
	if ( GetSizingBorderAtArea ( pos ) == OutArea )
	{
		// Let the scroll bar handle it first.
		if ( m_pScrollbar->OnMouseButtonDown ( pos ) )
			return true;
	}

	if ( m_rButton.InControlArea ( pos ) &&
		 m_bCloseButtonEnabled )
	{
		// Pressed while inside the control
		m_bPressed = true;

		if ( m_pDialog && !m_bHasFocus )
			m_pDialog->SetFocussedWindow ( this );

		return true;
	}

	if ( m_bSizable )
	{
		E_WINDOW_AREA eArea = GetSizingBorderAtArea ( pos );
		if ( eArea != OutArea )
		{
			//SetCursorForPoint ( pos );

			m_eWindowArea = eArea;

			if ( m_eWindowArea == Top ||
				 m_eWindowArea == TopLeft ||
				 m_eWindowArea == TopRight )
			{
				m_nDragY = m_pos.GetY () - pos.GetY ();
				if ( m_eWindowArea == TopLeft )
				{
					m_nDragX = m_pos.GetX () - pos.GetX ();
				}
				else if ( m_eWindowArea == TopRight )
				{
					m_nDragX = m_pos.GetX () + m_rBoundingBox.size.cx - pos.GetX ();
				}
			}
			else if ( m_eWindowArea == Left ||
					  m_eWindowArea == BottomLeft )
			{
				m_nDragX = m_pos.GetX () - pos.GetX ();

				if ( m_eWindowArea == BottomLeft )
				{
					m_nDragY = m_pos.GetY () + m_rBoundingBox.size.cy - pos.GetY ();
				}
			}
			else if ( m_eWindowArea == Right ||
					  m_eWindowArea == BottomRight )
			{
				m_nDragX = m_pos.GetX () + m_rBoundingBox.size.cx - pos.GetX ();

				if ( m_eWindowArea == BottomRight )
				{
					m_nDragY = m_pos.GetY () + m_rBoundingBox.size.cy - pos.GetY ();
				}
			}
			else if ( m_eWindowArea == Bottom )
			{
				m_nDragY = m_pos.GetY () + m_rBoundingBox.size.cy - pos.GetY ();
			}

			if ( m_pDialog && !m_bHasFocus )
				m_pDialog->SetFocussedWindow ( this );

			return true;
		}
	}

	if ( m_rTitle.InControlArea ( pos ) )
	{
		
		//ClearControlFocus ();

		if ( m_pDialog->GetMouse ()->GetLeftButton () == 2 )
			m_bMaximized = !m_bMaximized;

		if ( m_bMovable )
		{
			m_bDragging = true;
			m_posDif = m_pos - pos;
		}

		if ( m_pDialog )
			m_pDialog->SetFocussedWindow ( this );

		return true;
	}

	if ( m_rBoundingBox.InControlArea ( pos ) && !m_bMaximized )
	{
		if ( m_pDialog && !m_bHasFocus )
			m_pDialog->SetFocussedWindow ( this );

		return true;
	}

	return false;
}

bool CWindow::OnMouseButtonUp ( CPos pos )
{
	// Check if mouse is over window boundaries
	if ( GetSizingBorderAtArea ( pos ) == OutArea )
	{
		// Let the scroll bar handle it first.
		if ( m_pScrollbar->OnMouseButtonUp ( pos ) )
			return true;
	}

	m_bDragging		= false;
	m_eWindowArea	= OutArea;
	m_size			= m_rBoundingBox.size;

	if ( m_bPressed )
	{
		m_bPressed = false;

		// Button click
		if ( m_rButton.InControlArea ( pos ) )
		{
			SendEvent ( EVENT_CONTROL_CLICKED, true );
			SetVisible ( false );

			if ( m_pDialog )
				m_pDialog->ClearFocussedWindow ();

		}

		return true;
	}

	return false;
}

bool CWindow::OnMouseMove ( CPos pos )
{
	// Check if mouse is over window boundaries
	if ( GetSizingBorderAtArea ( pos ) == OutArea )
	{
		// Let the scroll bar handle it first.
		if ( m_pScrollbar->OnMouseMove ( pos ) )
			return true;
	}

	if ( m_rTitle.InControlArea ( pos ) )
	{
		if ( m_pControlMouseOver )
			m_pControlMouseOver->OnMouseLeave ();

		m_pControlMouseOver = NULL;
	}

	if ( m_bSizable )
	{
		if ( m_bMouseOver &&
			 m_eWindowArea == OutArea &&
			 !m_rButton.InControlArea ( pos ) )
		{
			SetCursorForPoint ( pos );
		}

		if ( m_eWindowArea == Top ||
			 m_eWindowArea == TopLeft ||
			 m_eWindowArea == TopRight )
		{
			m_rBoundingBox.size.cy = m_rBoundingBox.size.cy + ( m_pos.GetY () - pos.GetY () ) - m_nDragY;
			m_pos.SetY ( pos.GetY () + m_nDragY );

			if ( m_eWindowArea == TopLeft )
			{
				m_rBoundingBox.size.cx = m_rBoundingBox.size.cx + ( m_pos.GetX () - pos.GetX () ) - m_nDragX;
				m_pos.SetX ( pos.GetX () + m_nDragX );
			}
			else if ( m_eWindowArea == TopRight )
				m_rBoundingBox.size.cx = pos.GetX () - m_pos.GetX () + m_nDragX;
		}
		else if ( m_eWindowArea == Left ||
				  m_eWindowArea == BottomLeft )
		{
			m_rBoundingBox.size.cx = m_rBoundingBox.size.cx + ( m_pos.GetX () - pos.GetX () ) - m_nDragX;
			m_pos.SetX ( pos.GetX () + m_nDragX );

			if ( m_eWindowArea == BottomLeft )
				m_rBoundingBox.size.cy = pos.GetY () - m_pos.GetY () + m_nDragY;
		}
		else if ( m_eWindowArea == Right ||
				  m_eWindowArea == BottomRight )
		{
			m_rBoundingBox.size.cx = pos.GetX () - m_pos.GetX () + m_nDragX;

			if ( m_eWindowArea == BottomRight )
				m_rBoundingBox.size.cy = pos.GetY () - m_pos.GetY () + m_nDragY;
		}
		else if ( m_eWindowArea == Bottom )
			m_rBoundingBox.size.cy = pos.GetY () - m_pos.GetY () + m_nDragY;

		if ( m_rBoundingBox.size.cx < m_minSize.cx )
			m_rBoundingBox.size.cx = m_minSize.cx;

		if ( m_rBoundingBox.size.cy < m_minSize.cy )
			m_rBoundingBox.size.cy = m_minSize.cy;

	}

	if ( m_bDragging &&
		 m_bMovable )
	{
		m_pos = pos + m_posDif;
	}

	// Adjust position
	m_rBoundingBox.pos = m_pos;
	return false;
}

bool CWindow::OnMouseWheel ( int zDelta )
{
	ScrollPage ( -zDelta );
	return true;
}

bool CWindow::OnKeyDown ( WPARAM wParam )
{
	if ( !CControl::CanHaveFocus () )
		return false;

	if ( wParam == VK_TAB )
	{
		SetFocussedControl ( m_vControls [ 0 ] );
		return true;
	}

	return false;
}

bool CWindow::OnKeyUp ( WPARAM wParam )
{
	return false;
}

bool CWindow::OnKeyCharacter ( WPARAM wParam )
{
	return false;
}

//--------------------------------------------------------------------------------------
bool CWindow::HandleMouse ( UINT uMsg, CPos pos, WPARAM wParam, LPARAM lParam )
{
	if ( !CControl::CanHaveFocus () ||
		 !m_pScrollbar )
		return false;

	// Check if mouse is over window boundaries
	if ( GetSizingBorderAtArea ( pos ) == OutArea )
	{
		// Let the scroll bar handle it first.
		if ( m_pScrollbar->HandleMouse ( uMsg, pos, wParam, lParam ) )
			return true;
	}

	switch ( uMsg )
	{
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONDOWN:
		{
			if ( m_rButton.InControlArea ( pos ) &&
				 m_bCloseButtonEnabled )
			{
				// Pressed while inside the control
				m_bPressed = true;

				if ( m_pDialog && !m_bHasFocus )
					m_pDialog->SetFocussedWindow ( this );

				return true;
			}

			if ( m_bSizable )
			{
				E_WINDOW_AREA eArea = GetSizingBorderAtArea ( pos );
				if ( eArea != OutArea )
				{
					//SetCursorForPoint ( pos );

					m_eWindowArea = eArea;

					if ( m_eWindowArea == Top ||
						 m_eWindowArea == TopLeft ||
						 m_eWindowArea == TopRight )
					{
						m_nDragY = m_pos.GetY () - pos.GetY ();
						if ( m_eWindowArea == TopLeft )
						{
							m_nDragX = m_pos.GetX () - pos.GetX ();
						}
						else if ( m_eWindowArea == TopRight )
						{
							m_nDragX = m_pos.GetX () + m_rBoundingBox.size.cx - pos.GetX ();
						}
					}
					else if ( m_eWindowArea == Left ||
							  m_eWindowArea == BottomLeft )
					{
						m_nDragX = m_pos.GetX () - pos.GetX ();

						if ( m_eWindowArea == BottomLeft )
						{
							m_nDragY = m_pos.GetY () + m_rBoundingBox.size.cy - pos.GetY ();
						}
					}
					else if ( m_eWindowArea == Right ||
							  m_eWindowArea == BottomRight )
					{
						m_nDragX = m_pos.GetX () + m_rBoundingBox.size.cx - pos.GetX ();

						if ( m_eWindowArea == BottomRight )
						{
							m_nDragY = m_pos.GetY () + m_rBoundingBox.size.cy - pos.GetY ();
						}
					}
					else if ( m_eWindowArea == Bottom )
					{
						m_nDragY = m_pos.GetY () + m_rBoundingBox.size.cy - pos.GetY ();
					}

					if ( m_pDialog && !m_bHasFocus )
						m_pDialog->SetFocussedWindow ( this );

					return true;
				}
			}

			if ( m_rTitle.InControlArea ( pos ) )
			{
				m_pControlMouseOver = NULL;
				m_pFocussedControl = NULL;

				if ( m_pDialog->GetMouse ()->GetLeftButton () == 2 )
					m_bMaximized = !m_bMaximized;

				if ( m_bMovable )
				{
					m_bDragging = true;
					m_posDif = m_pos - pos;
				}

				if ( m_pDialog )
					m_pDialog->SetFocussedWindow ( this );

				return true;
			}

			if ( m_rBoundingBox.InControlArea ( pos ) && !m_bMaximized )
			{
				if ( m_pDialog && !m_bHasFocus )
					m_pDialog->SetFocussedWindow ( this );

				return true;
			}

			break;
		}

		case WM_LBUTTONUP:
		{
			m_bDragging = false;
			m_eWindowArea = OutArea;
			m_size = m_rBoundingBox.size;

			if ( m_bPressed )
			{
				m_bPressed = false;

				// Button click
				if ( m_rButton.InControlArea ( pos ) )
				{
					SendEvent ( EVENT_CONTROL_CLICKED, true );
					SetVisible ( false );

					if ( m_pDialog )
						m_pDialog->ClearFocussedWindow ();

				}

				return true;
			}

			break;
		}

		case WM_MOUSEMOVE:
		{
			if ( m_rTitle.InControlArea ( pos ) )
			{
				if ( m_pControlMouseOver )
					m_pControlMouseOver->OnMouseLeave ();

				m_pControlMouseOver = NULL;
			}

			if ( m_bSizable )
			{
				if ( m_bMouseOver &&
					 m_eWindowArea == OutArea &&
					 !m_rButton.InControlArea ( pos ) )
				{
					SetCursorForPoint ( pos );
				}

				if ( m_eWindowArea == Top ||
					 m_eWindowArea == TopLeft ||
					 m_eWindowArea == TopRight )
				{
					m_rBoundingBox.size.cy = m_rBoundingBox.size.cy + ( m_pos.GetY () - pos.GetY () ) - m_nDragY;
					m_pos.SetY ( pos.GetY () + m_nDragY );

					if ( m_eWindowArea == TopLeft )
					{
						m_rBoundingBox.size.cx = m_rBoundingBox.size.cx + ( m_pos.GetX () - pos.GetX () ) - m_nDragX;
						m_pos.SetX ( pos.GetX () + m_nDragX );
					}
					else if ( m_eWindowArea == TopRight )
						m_rBoundingBox.size.cx = pos.GetX () - m_pos.GetX () + m_nDragX;
				}
				else if ( m_eWindowArea == Left ||
						  m_eWindowArea == BottomLeft )
				{
					m_rBoundingBox.size.cx = m_rBoundingBox.size.cx + ( m_pos.GetX () - pos.GetX () ) - m_nDragX;
					m_pos.SetX ( pos.GetX () + m_nDragX );

					if ( m_eWindowArea == BottomLeft )
						m_rBoundingBox.size.cy = pos.GetY () - m_pos.GetY () + m_nDragY;
				}
				else if ( m_eWindowArea == Right ||
						  m_eWindowArea == BottomRight )
				{
					m_rBoundingBox.size.cx = pos.GetX () - m_pos.GetX () + m_nDragX;

					if ( m_eWindowArea == BottomRight )
						m_rBoundingBox.size.cy = pos.GetY () - m_pos.GetY () + m_nDragY;
				}
				else if ( m_eWindowArea == Bottom )
					m_rBoundingBox.size.cy = pos.GetY () - m_pos.GetY () + m_nDragY;

				if ( m_rBoundingBox.size.cx < m_minSize.cx )
					m_rBoundingBox.size.cx = m_minSize.cx;

				if ( m_rBoundingBox.size.cy < m_minSize.cy )
					m_rBoundingBox.size.cy = m_minSize.cy;

			}

			if ( m_bDragging &&
				 m_bMovable )
			{
				m_pos = pos + m_posDif;
			}

			// Adjust position
			m_rBoundingBox.pos = m_pos;
			return false;
		}

		case WM_MOUSEWHEEL:
		{
			int zDelta = int ( ( short ) HIWORD ( wParam ) ) / WHEEL_DELTA;
			ScrollPage ( -zDelta );
			return true;
		}
	};

	return false;
}

//--------------------------------------------------------------------------------------
bool CWindow::HandleKeyboard ( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( !CControl::CanHaveFocus () )
		return false;

	switch ( uMsg )
	{
		case WM_KEYDOWN:
		{
			switch ( wParam )
			{
				case VK_TAB:
					SetFocussedControl ( m_vControls [ 0 ] );
					return true;
			}
		}
	}
	return false;
}

//--------------------------------------------------------------------------------------
void CWindow::UpdateScrollbars ( bool a, bool b )
{
	if ( !m_pScrollbar )
		return;

	CScrollBarVertical *pScrollbarVer = m_pScrollbar->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pScrollbar->GetHorScrollbar ();

	int nValueX = m_rBoundingBox.size.cx + ( m_maxControlSize.cx - ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cx ) );
	int nValueY = ( ( m_rBoundingBox.size.cy - m_iTitleBarSize ) +
					( m_maxControlSize.cy - ( ( m_rBoundingBox.pos.GetY () + m_iTitleBarSize ) + ( m_rBoundingBox.size.cy - m_iTitleBarSize ) ) ) );

	pScrollbarHor->SetStepSize ( m_rBoundingBox.size.cx / 10 );
	pScrollbarVer->SetStepSize ( m_rBoundingBox.size.cy / 10 );

	// Set scrollbars max range
	m_pScrollbar->SetTrackRange ( a?nValueX:0, b?nValueY:0 );
}

#define WINDOW_SIZE_CORNERS 5

//--------------------------------------------------------------------------------------
void CWindow::UpdateRects ( void )
{
	if ( !m_pScrollbar )
		return;

	SControlRect rRect = m_rBoundingBox;
	rRect.size.cy -= m_iTitleBarSize;
	rRect.pos.SetY ( rRect.pos.GetY () + m_iTitleBarSize );

	m_pScrollbar->UpdateScrollbars ( rRect );

	m_rTitle = m_rBoundingBox;
	m_rTitle.size.cy = m_iTitleBarSize;

	// Window corners
	m_rWindowTop = m_rBoundingBox;
	m_rWindowTop.pos.SetY ( m_rBoundingBox.pos.GetY () - WINDOW_SIZE_CORNERS );
	m_rWindowTop.size.cx = m_rWindowTop.size.cx - 2;
	m_rWindowTop.size.cy = WINDOW_SIZE_CORNERS;

	m_rWindowLeft = m_rBoundingBox;
	m_rWindowLeft.pos.SetX ( m_rBoundingBox.pos.GetX () - WINDOW_SIZE_CORNERS );
	m_rWindowLeft.size.cx = WINDOW_SIZE_CORNERS;

	m_rWindowRight = m_rBoundingBox;
	m_rWindowRight.pos.SetX ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cx );
	m_rWindowRight.size.cx = WINDOW_SIZE_CORNERS;

	m_rWindowBottom = m_rBoundingBox;
	m_rWindowBottom.pos.SetY ( m_rBoundingBox.pos.GetY () + m_rBoundingBox.size.cy );
	m_rWindowBottom.size.cx = m_rWindowBottom.size.cx - 2;
	m_rWindowBottom.size.cy = WINDOW_SIZE_CORNERS;

	m_rWindowTopLeft = m_rBoundingBox;
	m_rWindowTopLeft.pos.SetX ( m_rBoundingBox.pos.GetX () - WINDOW_SIZE_CORNERS );
	m_rWindowTopLeft.pos.SetY ( m_rBoundingBox.pos.GetY () - WINDOW_SIZE_CORNERS );
	m_rWindowTopLeft.size.cx = WINDOW_SIZE_CORNERS;
	m_rWindowTopLeft.size.cy = WINDOW_SIZE_CORNERS;

	m_rWindowTopRight = m_rBoundingBox;
	m_rWindowTopRight.pos.SetX ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cx );
	m_rWindowTopRight.pos.SetY ( m_rWindowTopRight.pos.GetY () - WINDOW_SIZE_CORNERS );
	m_rWindowTopRight.size.cx = WINDOW_SIZE_CORNERS;
	m_rWindowTopRight.size.cy = WINDOW_SIZE_CORNERS;

	m_rWindowBottomLeft = m_rBoundingBox;
	m_rWindowBottomLeft.pos.SetX ( m_rBoundingBox.pos.GetX () - WINDOW_SIZE_CORNERS );
	m_rWindowBottomLeft.pos.SetY ( m_rBoundingBox.pos.GetY () + m_rBoundingBox.size.cy );
	m_rWindowBottomLeft.size.cx = WINDOW_SIZE_CORNERS;
	m_rWindowBottomLeft.size.cy = WINDOW_SIZE_CORNERS;

	m_rWindowBottomRight = m_rBoundingBox;
	m_rWindowBottomRight.pos.SetX ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cx - WINDOW_SIZE_CORNERS );
	m_rWindowBottomRight.pos.SetY ( m_rBoundingBox.pos.GetY () + m_rBoundingBox.size.cy );
	m_rWindowBottomRight.size.cx = WINDOW_SIZE_CORNERS;
	m_rWindowBottomRight.size.cy = WINDOW_SIZE_CORNERS;

	// Button 
	m_rButton = m_rBoundingBox;
	m_rButton.pos.SetX ( m_rBoundingBox.pos.GetX () + m_rBoundingBox.size.cx - 35 );
	m_rButton.size.cx = 35.f;
	m_rButton.size.cy = 16.f;
}

//--------------------------------------------------------------------------------------
bool CWindow::ContainsRect ( CPos pos )
{
	if ( !CControl::CanHaveFocus () ||
		 !m_pScrollbar )
		return false;

	return ( ( m_pScrollbar->ContainsRect ( pos ) && m_eWindowArea == OutArea ) ||
			 ( m_rBoundingBox.InControlArea ( pos ) && !m_bMaximized ) ||
			 m_rTitle.InControlArea ( pos ) ||
			 m_rButton.InControlArea ( pos )|| GetSizingBorderAtArea ( pos ) != OutArea );
}

void CWindow::SetCursorForPoint ( CPos pos )
{
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

CWindow::E_WINDOW_AREA CWindow::GetSizingBorderAtArea ( CPos pos )
{
	if ( ( m_pFocussedControl && m_pFocussedControl->OnClickEvent () ) ||
		 m_bDragging ||
		 m_bPressed ||
		 m_bMaximized ||
		 m_pScrollbar->OnClickEvent () )
	{
		return OutArea;
	}

	if ( m_rWindowTopLeft.InControlArea ( pos ) )
	{
		return TopLeft;
	}
	else if ( m_rWindowBottomLeft.InControlArea ( pos ) )
	{
		return BottomLeft;
	}
	else if ( m_rWindowTopRight.InControlArea ( pos ) )
	{
		return TopRight;
	}
	else if ( m_rWindowBottomRight.InControlArea ( pos ) )
	{
		return BottomRight;
	}
	else if ( m_rWindowTop.InControlArea ( pos ) )
	{
		return Top;
	}
	else if ( m_rWindowLeft.InControlArea ( pos ) )
	{
		return Left;
	}
	else if ( m_rWindowRight.InControlArea ( pos ) )
	{
		return Right;
	}
	else if ( m_rWindowBottom.InControlArea ( pos ) )
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
		 ( m_bDragging ||
		 m_bPressed ||
		 m_bMaximized ) ||
		 m_pScrollbar->OnClickEvent () )
		return NULL;

	switch ( eArea )
	{
		case TopLeft:
			return &m_rWindowTopLeft;
		case BottomLeft:
			return &m_rWindowBottomLeft;
		case TopRight:
			return &m_rWindowTopRight;
		case BottomRight:
			return &m_rWindowBottomRight;
		case Top:
			return &m_rWindowTop;
		case Left:
			return &m_rWindowLeft;
		case Right:
			return &m_rWindowRight;
		case Bottom:
			return &m_rWindowBottom;
		default:
			return NULL;
	}
}