#include "CGUI.h"
#include <stdarg.h>
#include <stdio.h>

CDialog::CDialog ( IDirect3DDevice9 *pDevice )
{
	InitializeCriticalSection ( &cs );

	if ( !pDevice )
		MessageBox ( 0, _UI ( "pDevice invalid." ), 0, 0 );

	m_pDevice = pDevice;
	m_pFocussedWindow = m_pMouseOverWindow = NULL;

	m_pRender = new CD3DRender ( 128 );
	if ( m_pRender )
		m_pRender->Initialize ( pDevice );

	m_pMouse = new CMouse ( this );
	if( !m_pMouse )
		MessageBox ( 0, _UI ( "Error for creating mouse (CMouse::CMouse)." ), 0, 0 );

	m_bVisible = true;

	// Create a state block
	m_pState = new CD3DStateBlock ();
	if ( m_pState )
		m_pState->Initialize ( m_pDevice );
}

CDialog::~CDialog ( void )
{
	EnterCriticalSection ( &cs );

	for ( size_t i = 0; i < m_vFont.size (); i++ )
		SAFE_DELETE ( m_vFont[i] );

	for ( size_t i = 0; i < m_vTexture.size (); i++ )
		SAFE_DELETE ( m_vTexture[i]  );

	SAFE_DELETE ( m_pMouse );
	SAFE_DELETE ( m_pFocussedWindow );
	SAFE_DELETE ( m_pMouseOverWindow );

	RemoveAllWindows ();

	SAFE_DELETE ( m_pState );

	LeaveCriticalSection ( &cs );
	DeleteCriticalSection ( &cs );
}

void CDialog::LoadFont ( const SIMPLEGUI_CHAR *szFontName, DWORD dwHeight, bool bBold, CD3DFont **ppFont )
{
	CD3DFont *pFont = new CD3DFont ( szFontName, dwHeight, bBold );
	if ( !pFont )
		return;

	if ( FAILED ( pFont->Initialize ( m_pDevice ) ) )
		return;

	if ( ppFont != NULL )
		*ppFont = pFont;

	m_vFont.push_back ( pFont );
}

void CDialog::RemoveFont ( CD3DFont *pFont )
{
	SAFE_DELETE ( pFont );
	m_vFont.erase ( std::find ( m_vFont.begin (), m_vFont.end (), pFont ) );
}

//--------------------------------------------------------------------------------------
void CDialog::LoadTexture ( const SIMPLEGUI_CHAR *szPath, CD3DTexture **ppTexture )
{
	CD3DTexture *pTexture = new CD3DTexture ( szPath );

	if ( !pTexture )
		return;

	if ( FAILED ( pTexture->Initialize ( m_pDevice ) ) )
		return;

	if ( ppTexture != NULL )
		*ppTexture = pTexture;

	m_vTexture.push_back ( pTexture );
}

//--------------------------------------------------------------------------------------
void CDialog::LoadTexture ( LPCVOID pSrc, UINT uSrcSize, CD3DTexture **ppTexture )
{
	CD3DTexture *pTexture = new CD3DTexture ( pSrc, uSrcSize );
	if ( !pTexture )
		return;

	if ( FAILED ( pTexture->Initialize ( m_pDevice ) ) )
		return;

	if ( ppTexture != NULL )
		*ppTexture = pTexture;

	m_vTexture.push_back ( pTexture );
}

//--------------------------------------------------------------------------------------
void CDialog::RemoveTexture ( CD3DTexture *pTexture )
{
	m_vTexture.erase ( std::find ( m_vTexture.begin (), m_vTexture.end (), pTexture ) );
}

//--------------------------------------------------------------------------------------
void CDialog::DrawFont ( SControlRect &rect, DWORD dwColor, const SIMPLEGUI_CHAR *szText, DWORD dwFlags, CD3DFont *pFont )
{
	if ( !szText )
		return;

	if ( !pFont && !m_vFont.empty () )
		pFont = m_vFont [ 0 ];

	CPos pos = rect.pos;
	pFont->Print ( pos.GetX (), pos.GetY (), dwColor, szText, dwFlags );
}

//--------------------------------------------------------------------------------------
void CDialog::DrawBox ( SControlRect &rect, D3DCOLOR d3dColor, D3DCOLOR d3dColorOutline, bool bAntAlias )
{
	if ( !m_pRender )
		return;

	CPos pos = rect.pos;
	SIZE size = rect.size;

	m_pRender->D3DBox ( pos.GetX (), pos.GetY (), size.cx, size.cy, 0.f, d3dColor, d3dColorOutline, bAntAlias );
}

//--------------------------------------------------------------------------------------
void CDialog::DrawTriangle ( SControlRect &rect, float fAngle, D3DCOLOR d3dColor, D3DCOLOR d3dColorOutline, bool bAntAlias )
{
	if ( !m_pRender )
		return;

	CPos pos = rect.pos;
	SIZE size = rect.size;

	int nMax = max ( size.cx, size.cy );
	m_pRender->D3DTriangle ( pos.GetX (), pos.GetY (), size.cx - size.cy + nMax, fAngle, d3dColor, d3dColorOutline, bAntAlias );
}

//--------------------------------------------------------------------------------------
void CDialog::DrawCircle ( SControlRect &rect, D3DCOLOR d3dColor, D3DCOLOR d3dColorOutline, bool bAntAlias )
{
	if ( !m_pRender )
		return;

	CPos pos = rect.pos;
	SIZE size = rect.size;

	int nMax = max ( size.cx, size.cy );
	m_pRender->D3DCircle ( pos.GetX (), pos.GetY (), size.cx - size.cy + nMax, d3dColor, d3dColorOutline, bAntAlias );
}

CProgressBarHorizontal *CDialog::AddProgressBarHorizontal ( CWindow *pWindow, int iX, int iY, int iWidth, int iHeight, float fMax, float fValue, tAction Callback )
{
	CProgressBarHorizontal* pProgressBar = new CProgressBarHorizontal ( this );

	if ( pProgressBar )
	{
		pProgressBar->SetPos ( CPos ( iX, iY ) );
		pProgressBar->SetSize ( iWidth, iHeight );
		pProgressBar->SetMaxValue ( fMax );
		pProgressBar->SetValue ( fValue );
		pProgressBar->SetAction ( Callback );

		if ( pWindow )
			pWindow->AddControl ( pProgressBar );
	}

	return pProgressBar;
}

CProgressBarVertical *CDialog::AddProgressBarVertical ( CWindow *pWindow, int iX, int iY, int iWidth, int iHeight, float fMax, float fValue, tAction Callback )
{
	CProgressBarVertical* pProgressBar = new CProgressBarVertical ( this );

	if ( pProgressBar )
	{
		pProgressBar->SetPos ( CPos ( iX, iY ) );
		pProgressBar->SetSize ( iWidth, iHeight );
		pProgressBar->SetMaxValue ( fMax );
		pProgressBar->SetValue ( fValue );
		pProgressBar->SetAction ( Callback );

		if ( pWindow )
			pWindow->AddControl ( pProgressBar );
	}

	return pProgressBar;
}

//--------------------------------------------------------------------------------------
CWindow *CDialog::AddWindow ( int X, int Y, int Width, int Height, const SIMPLEGUI_CHAR *szString, bool bAlwaysOnTop, tAction Callback )
{
	CWindow* pWindow = new CWindow ( this );

	if ( pWindow )
	{
		pWindow->SetPos ( CPos ( X, Y ) );
		pWindow->SetSize ( Width, Height );
		pWindow->SetText ( szString );
		pWindow->SetAction ( Callback );
		pWindow->SetAlwaysOnTop ( bAlwaysOnTop );

		this->AddWindow ( pWindow );
	}

	return pWindow;
}

//--------------------------------------------------------------------------------------
CButton *CDialog::AddButton ( CWindow *pWindow, int X, int Y, int Width, int Height, const SIMPLEGUI_CHAR *szString, tAction Callback )
{
	CButton* pButton = new CButton ( this );

	if ( pButton )
	{
		pButton->SetPos ( CPos ( X, Y ) );
		pButton->SetSize ( Width, Height );
		pButton->SetText ( szString );
		pButton->SetAction ( Callback );

		if ( pWindow )
			pWindow->AddControl ( pButton );
	}

	return pButton;
}

//--------------------------------------------------------------------------------------
CCheckBox *CDialog::AddCheckBox ( CWindow *pWindow, int X, int Y, int Width, bool bChecked, const SIMPLEGUI_CHAR *szString, tAction Callback )
{
	CCheckBox* pCheckBox = new CCheckBox ( this );

	if ( pCheckBox )
	{
		pCheckBox->SetPos ( CPos ( X, Y ) );
		pCheckBox->SetSize ( Width, 20 );
		pCheckBox->SetText ( szString );
		pCheckBox->SetAction ( Callback );
		pCheckBox->SetChecked ( bChecked );

		if ( pWindow )
			pWindow->AddControl ( pCheckBox );
	}

	return pCheckBox;
}

//--------------------------------------------------------------------------------------
CListBox *CDialog::AddListBox ( CWindow *pWindow, int X, int Y, int Width, int Height, tAction Callback )
{
	CListBox* pListBox = new CListBox ( this );

	if ( pListBox )
	{
		pListBox->SetPos ( CPos ( X, Y ) );
		pListBox->SetSize ( Width, Height );
		pListBox->SetAction ( Callback );

		if ( pWindow )
			pWindow->AddControl ( pListBox );
	}

	return pListBox;
}

CListView *CDialog::AddListView ( CWindow *pWindow, int X, int Y, int Width, int Height, const SIMPLEGUI_CHAR *szString, tAction Callback )
{
	CListView* pListView = new CListView ( this );

	if ( pListView )
	{
		pListView->SetPos ( CPos ( X, Y ) );
		pListView->SetSize ( Width, Height );
		pListView->SetAction ( Callback );

		if ( pWindow )
			pWindow->AddControl ( pListView );
	}

	return pListView;
}

//--------------------------------------------------------------------------------------
CLogBox *CDialog::AddTextBox ( CWindow *pWindow, int X, int Y, int Width, int Height, tAction Callback )
{
	CLogBox* pTextBox = new CLogBox ( this );

	if ( pTextBox )
	{
		pTextBox->SetPos ( CPos ( X, Y ) );
		pTextBox->SetSize ( Width, Height );
		pTextBox->SetAction ( Callback );

		if ( pWindow )
			pWindow->AddControl ( pTextBox );
	}

	return pTextBox;
}

//--------------------------------------------------------------------------------------
CLabel *CDialog::AddLabel ( CWindow *pWindow, int X, int Y, int Width, int Height, const SIMPLEGUI_CHAR *szString, tAction Callback )
{
	CLabel* pLabel = new CLabel ( this );

	if ( pLabel )
	{
		pLabel->SetPos ( CPos ( X, Y ) );
		pLabel->SetText ( szString );
		pLabel->SetSize ( Width, Height );
		pLabel->SetAction ( Callback );

		if ( pWindow )
			pWindow->AddControl ( pLabel );
	}

	return pLabel;
}

//--------------------------------------------------------------------------------------
CEditBox *CDialog::AddEditBox ( CWindow *pWindow, int X, int Y, int Width, int Height, const SIMPLEGUI_CHAR *szString, bool bSelected, tAction Callback )
{
	CEditBox* pEditBox = new CEditBox ( this );

	if ( pEditBox )
	{
		pEditBox->SetPos ( CPos ( X, Y ) );
		pEditBox->SetSize ( Width, Height );
		pEditBox->SetText ( szString, bSelected );
		pEditBox->SetAction ( Callback );

		if ( pWindow )
			pWindow->AddControl ( pEditBox );
	}

	return pEditBox;
}

//--------------------------------------------------------------------------------------
CDropDown *CDialog::AddDropDown ( CWindow *pWindow, int X, int Y, int Width, int Height, const SIMPLEGUI_CHAR *szString, tAction Callback )
{
	CDropDown* pDropDown = new CDropDown ( this );

	if ( pDropDown )
	{
		pDropDown->SetPos ( CPos ( X, Y ) );
		pDropDown->SetSize ( Width, Height );
		pDropDown->SetText ( szString );
		pDropDown->SetAction ( Callback );

		if ( pWindow )
			pWindow->AddControl ( pDropDown );
	}

	return pDropDown;
}

//--------------------------------------------------------------------------------------
CRadioButton *CDialog::AddRadioButton ( CWindow *pWindow, int iGroup, int X, int Y, int Width, const SIMPLEGUI_CHAR *szString, tAction Callback )
{
	CRadioButton *pRadioButton = new CRadioButton ( this );

	if ( pRadioButton )
	{
		pRadioButton->SetGroup ( iGroup );
		pRadioButton->SetPos ( CPos ( X, Y ) );
		pRadioButton->SetSize ( Width, 20 );
		pRadioButton->SetText ( szString );
		pRadioButton->SetAction ( Callback );

		if ( pWindow )
			pWindow->AddControl ( pRadioButton );
	}

	return pRadioButton;
}

CTabPanel *CDialog::AddTabPanel ( CWindow *pWindow, int X, int Y, int Width, int Height, tAction Callback )
{
	CTabPanel *pTabPanel = new CTabPanel ( this );

	if ( pTabPanel )
	{
		pTabPanel->SetPos ( CPos ( X, Y ) );
		pTabPanel->SetSize ( Width, Height );
		pTabPanel->SetAction ( Callback );

		if ( pWindow )
			pWindow->AddControl ( pTabPanel );
	}

	return pTabPanel;
}

/*//--------------------------------------------------------------------------------------
CPictureBox *CDialog::AddImage ( CWindow *pWindow, const TCHAR * szPath, int X, int Y, int Width, int Height, tAction Callback )
{
	CPictureBox *pImage = new CPictureBox ( this );

	if ( pImage )
	{
		pImage->SetPos ( CPos ( X, Y ) );
		pImage->SetHeight ( Height );
		pImage->SetTexture ( szPath );
		pImage->SetAction ( Callback );

		if ( pWindow )
			pWindow->AddControl ( pImage );
	}

	return pImage;
}
*/
//--------------------------------------------------------------------------------------
CTrackBarHorizontal *CDialog::AddTrackBar ( CWindow * pWindow, int X, int Y, int Width, int Height, int nMin, int nMax, int nValue, tAction Callback )
{
	CTrackBarHorizontal *pTrackBar = new CTrackBarHorizontal ( this );

	if ( pTrackBar )
	{
		pTrackBar->SetPos ( CPos ( X, Y ) );
		pTrackBar->SetSize ( Width, Height );
		pTrackBar->SetAction ( Callback );
		pTrackBar->SetRange ( nMin, nMax );
		pTrackBar->SetValue ( nValue );

		if ( pWindow )
			pWindow->AddControl ( pTrackBar );
	}

	return pTrackBar;
}

CTrackBarVertical *CDialog::AddTrackBarVertical ( CWindow * pWindow, int X, int Y, int Width, int Height, int nMin, int nMax, int nValue, tAction Callback )
{
	CTrackBarVertical *pTrackBar = new CTrackBarVertical ( this );

	if ( pTrackBar )
	{
		pTrackBar->SetPos ( CPos ( X, Y ) );
		pTrackBar->SetSize ( Width, Height );
		pTrackBar->SetAction ( Callback );
		pTrackBar->SetRange ( nMin, nMax );
		pTrackBar->SetValue ( nValue );

		if ( pWindow )
			pWindow->AddControl ( pTrackBar );
	}

	return pTrackBar;
}

//--------------------------------------------------------------------------------------
CScrollBarVertical *CDialog::AddScrollBar ( CWindow *pWindow, int X, int Y, int Width, int Height, int nMin, int nMax, int nPagSize, int nValue, tAction Callback )
{
	CScrollBarVertical *pScrollBar = new CScrollBarVertical ( this );

	if ( pScrollBar )
	{
		pScrollBar->SetPos ( CPos ( X, Y ) );
		pScrollBar->SetSize ( Width, Height );
		pScrollBar->SetAction ( Callback );
		pScrollBar->SetTrackRange ( nMin, nMax );
		pScrollBar->SetPageSize ( nPagSize );
		pScrollBar->ShowItem ( nValue );

		if ( pWindow )
			pWindow->AddControl ( pScrollBar );
	}

	return pScrollBar;
}

//--------------------------------------------------------------------------------------
CScrollBarHorizontal *CDialog::AddScrollBarHorizontal ( CWindow *pWindow, int X, int Y, int Width, int Height, int nMin, int nMax, int nPagSize, int nValue, tAction Callback )
{
	CScrollBarHorizontal *pScrollBar = new CScrollBarHorizontal ( this );

	if ( pScrollBar )
	{
		pScrollBar->SetPos ( CPos ( X, Y ) );
		pScrollBar->SetSize ( Width, Height );
		pScrollBar->SetAction ( Callback );
		pScrollBar->SetTrackRange ( nMin, nMax );
		pScrollBar->SetPageSize ( nPagSize );
		pScrollBar->ShowItem ( nValue );;

		if ( pWindow )
			pWindow->AddControl ( pScrollBar );
	}
	return pScrollBar;
}

//--------------------------------------------------------------------------------------
void CDialog::Draw ( void )
{
	if ( !m_bVisible )
		return;

	if ( FAILED ( m_pState->BeginState () ) )
		return;

	EnterCriticalSection ( &cs );

	m_pState->SetRenderStates ();

	for ( size_t i = 0; i < m_vWindows.size (); i++ )
	{
		if ( !m_vWindows [ i ] )
			continue;

		if ( !m_vWindows [ i ]->IsVisible ())
			continue;

		m_vWindows [ i ]->UpdateRects ();
		m_vWindows [ i ]->Draw ();
	}

	m_pMouse->Draw ();

	m_pState->EndState ();

	LeaveCriticalSection ( &cs );
}

//--------------------------------------------------------------------------------------
void CDialog::MsgProc ( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	EnterCriticalSection ( &cs );

	if ( !m_pMouse )
		return;

	m_pMouse->HandleMessage ( uMsg, wParam, lParam );

	if ( m_vWindows.empty () )
		return;

	CPos pos = m_pMouse->GetPos ();

	/*if ( !GetAsyncKeyState(VK_LBUTTON) )
		m_pMouse->SetCursorType ( CMouse::DEFAULT );
*/

// First handle messages from the windows widgets

//// Check for any window with focus
//if ( m_pFocussedWindow )
//{
//	CControl *pControl = NULL;
//	pControl = m_pFocussedWindow->GetFocussedControl ();
//	if ( pControl && pControl->GetType () == CControl::TYPE_TABPANEL )
//	{
//		pControl = static_cast< CTabPanel* >( pControl )->GetFocussedControl ();
//	}

//	// If the widget is a dropdown, leave handling message outside the windows
//	if ( pControl )
//	{
//		m_pFocussedWindow->OnMouseMove ( pControl, uMsg );

//		if ( pControl->GetType () == CControl::TYPE_DROPDOWN )
//		{
//			// Let then give it the first chance at handling keyboard.
//			if ( pControl->HandleKeyboard ( uMsg, wParam, lParam ) )
//				return;

//			if ( pControl->HandleMouse ( uMsg, pos, wParam, lParam ) ||
//				 pControl->ContainsRect ( pos ) )
//				return;
//		}
//		else if ( pControl->GetType () == CControl::TYPE_EDITBOX )
//		{
//			if ( uMsg == WM_CHAR && 
//				 pControl->MsgProc ( uMsg, wParam, lParam ) )
//				return;

//			if ( pControl->HandleKeyboard ( uMsg, wParam, lParam ) )
//				return;
//		}
//	}
//}
	sControlEvents e;
	e.keyEvent;
	if ( uMsg == WM_KEYDOWN ||
		 uMsg == WM_KEYUP ||
		 uMsg == WM_CHAR )
	{
		e.keyEvent.uMsg = uMsg;
		e.keyEvent.wKey = wParam;
	}

	e.mouseEvent.pos = pos;
	switch ( uMsg )
	{
		case WM_MOUSEMOVE:
		{
			e.mouseEvent.eMouseMessages = sMouseEvents::MouseMove;
			break;
		}
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		{
			e.mouseEvent.eMouseMessages = sMouseEvents::ButtonDown;

			if ( uMsg == WM_LBUTTONDOWN )
				e.mouseEvent.eButton = sMouseEvents::LeftButton;
			else if ( uMsg == WM_RBUTTONDOWN )
				e.mouseEvent.eButton = sMouseEvents::RightButton;
			else
				e.mouseEvent.eButton = sMouseEvents::MiddleButton;
			break;
		}
		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		{
			e.mouseEvent.eMouseMessages = sMouseEvents::ButtonUp;

			if ( uMsg == WM_LBUTTONUP )
				e.mouseEvent.eButton = sMouseEvents::LeftButton;
			else if ( uMsg == WM_RBUTTONUP )
				e.mouseEvent.eButton = sMouseEvents::RightButton;
			else
				e.mouseEvent.eButton = sMouseEvents::MiddleButton;
			break;
		}
		case WM_MOUSEWHEEL:
		{
			e.mouseEvent.eMouseMessages = sMouseEvents::MouseWheel;
			e.mouseEvent.nDelta = int ( ( short ) HIWORD ( wParam ) ) / WHEEL_DELTA;
			break;
		}
	}
	// See if the mouse is over any windows
	CWindow* pWindow = GetWindowAtPos ( pos );
	if ( m_pFocussedWindow  )
	{
		CControl *pControl = m_pFocussedWindow->GetFocussedControl ();

		bool bOnDrag = false;


		if( e.keyEvent.uMsg == uMsg )bOnDrag = true;
		if ( (pControl &&pWindow== m_pFocussedWindow/*&&(pControl->GetType () == CControl::EControlType::TYPE_DROPDOWN ||
			 pControl->GetType() ==CControl::EControlType::TYPE_EDITBOX )*/&&  uMsg != WM_LBUTTONDOWN )||
			 ( GetAsyncKeyState ( VK_LBUTTON ) && uMsg == WM_MOUSEMOVE ) )
		{
			bOnDrag = true;
		}

		if ( bOnDrag&& m_pFocussedWindow->ControlMessages ( e ) )
			return;
	}


	if ( pWindow && pWindow->ControlMessages ( e ) )
		return;

	// If a window is in focus, and it's enabled, then give
	// it the first chance at handling the message.
	if ( m_pFocussedWindow &&
		 m_pFocussedWindow->IsEnabled () )
	{
		if ( m_pFocussedWindow&&m_pFocussedWindow->InjectKeyboard ( e.keyEvent ) )
			return;
	}

	if ( pWindow )
	{
		pWindow->InjectMouse ( e.mouseEvent );
	}
	else
	{
		if ( uMsg == WM_LBUTTONDOWN )
		{
			ClearFocussedWindow ();
		}
	}

	if ( m_pFocussedWindow )
	{
		// If the control is in focus, and if the mouse is outside the window, then leave 
		// the click event
		if ( uMsg == WM_LBUTTONUP )
		{
			m_pFocussedWindow->OnClickLeave ();
		}
	}

	if ( m_pFocussedWindow && m_pFocussedWindow->InjectMouse ( e.mouseEvent ) )
		return;

	if ( !( GetAsyncKeyState ( VK_LBUTTON ) && pWindow ) &&
		 uMsg == WM_MOUSEMOVE )
	{
		// If the mouse is still over the same window, nothing needs to be done
		if ( pWindow == m_pMouseOverWindow )
			return;

		// Handle mouse leaving the old window
		if ( m_pMouseOverWindow )
		{
			m_pMouseOverWindow->OnMouseLeave ();
		}

		// Handle mouse entering the new window
		m_pMouseOverWindow = pWindow;
		if ( m_pMouseOverWindow )
			m_pMouseOverWindow->OnMouseEnter ();
	}

	LeaveCriticalSection ( &cs );
}

//--------------------------------------------------------------------------------------
void CDialog::AddWindow ( CWindow *pWindow )
{
	if ( !pWindow )
		return;

	m_vWindows.push_back ( pWindow );
	SetFocussedWindow ( pWindow );
}

//--------------------------------------------------------------------------------------
void CDialog::RemoveWindow ( CWindow *pWindow )
{
	if ( !pWindow )
		return;

	std::vector<CWindow*>::iterator iter = std::find ( m_vWindows.begin (), m_vWindows.end (), pWindow );
	if ( iter == m_vWindows.end () )
		return;

	m_vWindows.erase ( iter );
	SAFE_DELETE ( pWindow );
}

//--------------------------------------------------------------------------------------
void CDialog::RemoveAllWindows ( void )
{
	for ( auto &window : m_vWindows )
		SAFE_DELETE ( window );

	m_vWindows.clear ();
}

//--------------------------------------------------------------------------------------
void CDialog::SetFocussedWindow ( CWindow *pWindow )
{
	if ( m_pFocussedWindow == pWindow )
		return;

	EnterCriticalSection ( &cs );

	if ( m_pFocussedWindow )
		m_pFocussedWindow->OnFocusOut ();

	if ( pWindow )
		pWindow->OnFocusIn ();

	if ( pWindow )
		BringWindowToTop ( pWindow );

	m_pFocussedWindow = pWindow;

	LeaveCriticalSection ( &cs );
}

//--------------------------------------------------------------------------------------
void CDialog::ClearFocussedWindow ( void )
{
	if ( m_pFocussedWindow )
	{
		m_pFocussedWindow->ClearControlFocus ();
		m_pFocussedWindow->OnClickLeave ();
		m_pFocussedWindow->OnFocusOut ();
		m_pFocussedWindow = NULL;
	}
}

//--------------------------------------------------------------------------------------
CWindow *CDialog::GetFocussedWindow ( void )
{
	return m_pFocussedWindow;
}

//--------------------------------------------------------------------------------------
void CDialog::BringWindowToTop ( CWindow *pWindow )
{
	auto iter = std::find ( m_vWindows.begin (), m_vWindows.end (), pWindow );
	if ( iter == m_vWindows.end () )
		return;

	if ( !pWindow->GetAlwaysOnTop () )

	{	// Get amount of windows on top
		int nCount = 0;
		for ( auto &window : m_vWindows )
		{
			if ( window->GetAlwaysOnTop () )
				nCount++;
		}

		m_vWindows.erase ( iter );
		m_vWindows.insert ( m_vWindows.end () - nCount, pWindow );
	}
}

//--------------------------------------------------------------------------------------
CWindow *CDialog::GetWindowAtPos ( CPos pos )
{
	for ( int i = static_cast< int >( m_vWindows.size () ) - 1; i >= 0; i-- )
	{
		if ( m_vWindows [ i ]->ContainsRect ( pos ) )
			return m_vWindows [ i ];
	}
	return NULL;
}

//--------------------------------------------------------------------------------------
CWindow* CDialog::GetWindowByText ( const SIMPLEGUI_CHAR *pszText )
{
	for ( auto &window : m_vWindows )
	{
		if ( window )
		{
			if ( !SIMPLEGUI_STRCMP ( window->GetText (), pszText ) )
				return window;
		}
	}

	return NULL;
}

//--------------------------------------------------------------------------------------
void CDialog::OnLostDevice ( void )
{
	EnterCriticalSection ( &cs );

	for ( size_t i = 0; i < m_vTexture.size (); i++ )
		SAFE_INVALIDATE ( m_vTexture [ i ] );

	for ( size_t i = 0; i < m_vFont.size (); i++ )
		SAFE_INVALIDATE ( m_vFont [ i ] );

	SAFE_INVALIDATE ( m_pRender );

	if ( m_pState )
		m_pState->Invalidate ();

	LeaveCriticalSection ( &cs );
}

//--------------------------------------------------------------------------------------
void CDialog::OnResetDevice ( void )
{
	EnterCriticalSection ( &cs );

	for ( size_t i = 0; i < m_vTexture.size (); i++ )
		SAFE_INITIALIZE ( m_vTexture [ i ], m_pDevice );

	for ( size_t i = 0; i < m_vFont.size (); i++ )
		SAFE_INITIALIZE ( m_vFont [ i ], m_pDevice );

	if ( m_pRender )
		m_pRender->Initialize ( m_pDevice );

	if ( m_pState )
		m_pState->Initialize ( m_pDevice );

	LeaveCriticalSection ( &cs );
}

//--------------------------------------------------------------------------------------
CD3DRender *CDialog::GetRenderer ( void )
{
	return m_pRender;
}

//--------------------------------------------------------------------------------------
CMouse* CDialog::GetMouse ( void )
{
	return m_pMouse;
}

//--------------------------------------------------------------------------------------
IDirect3DDevice9* CDialog::GetDevice ( void )
{
	return m_pDevice;
}

//--------------------------------------------------------------------------------------
CD3DFont* CDialog::GetFont ( int ID )
{
	if ( ID > -1 &&
		 ID < m_vFont.size () )
		return m_vFont [ ID ];

	return NULL;
}

CD3DTexture *CDialog::GetTexture ( int ID )
{
	if ( ID > -1 &&
		 ID < m_vTexture.size () )
		return m_vTexture [ ID ];

	return NULL;
}

//--------------------------------------------------------------------------------------
void CDialog::SetVisible ( bool bVisible )
{
	EnterCriticalSection ( &cs );
	m_bVisible = bVisible;
	LeaveCriticalSection ( &cs );
}

//--------------------------------------------------------------------------------------
bool CDialog::IsVisible ( void )
{
	return m_bVisible;
}
