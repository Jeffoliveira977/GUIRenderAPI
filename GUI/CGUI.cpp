#include "CGUI.h"
#include <stdarg.h>
#include <stdio.h>

CDialog::CDialog ( IDirect3DDevice9 *pDevice )
{
	InitializeCriticalSection ( &cs );

	ZeroMemory ( &m_oldvp, sizeof ( D3DVIEWPORT9 ) );

	if ( !pDevice )
		MessageBox ( 0, _UI ( "pDevice invalid." ), 0, 0 );

	m_pDevice = pDevice;
	m_pFocussedWidget = m_pMouseOverWidget = NULL;

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
	SAFE_DELETE ( m_pFocussedWidget );
	SAFE_DELETE ( m_pMouseOverWidget );

	RemoveAllWidgets ();

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

	CVector pos = rect.m_pos;
	pFont->Print ( pos.m_nX, pos.m_nY, dwColor, szText, dwFlags );
}

//--------------------------------------------------------------------------------------
void CDialog::DrawBox ( SControlRect &rect, D3DCOLOR d3dColor, D3DCOLOR d3dColorOutline, bool bAntAlias )
{
	if ( !m_pRender )
		return;

	CVector pos = rect.m_pos;
	SIZE size = rect.m_size;

	m_pRender->D3DBox ( pos.m_nX, pos.m_nY, size.cx, size.cy, 0.f, d3dColor, d3dColorOutline, bAntAlias );
}

//--------------------------------------------------------------------------------------
void CDialog::DrawTriangle ( SControlRect &rect, float fAngle, D3DCOLOR d3dColor, D3DCOLOR d3dColorOutline, bool bAntAlias )
{
	if ( !m_pRender )
		return;

	CVector pos = rect.m_pos;
	SIZE size = rect.m_size;

	int nMax = max ( size.cx, size.cy );
	m_pRender->D3DTriangle ( pos.m_nX, pos.m_nY, size.cx - size.cy + nMax, fAngle, d3dColor, d3dColorOutline, bAntAlias );
}

//--------------------------------------------------------------------------------------
void CDialog::DrawCircle ( SControlRect &rect, D3DCOLOR d3dColor, D3DCOLOR d3dColorOutline, bool bAntAlias )
{
	if ( !m_pRender )
		return;

	CVector pos = rect.m_pos;
	SIZE size = rect.m_size;

	int nMax = max ( size.cx, size.cy );
	m_pRender->D3DCircle ( pos.m_nX, pos.m_nY, size.cx - size.cy + nMax, d3dColor, d3dColorOutline, bAntAlias );
}

CProgressBarHorizontal *CDialog::AddProgressBarHorizontal ( CWindow *pWindow, int iX, int iY, int iWidth, int iHeight, float fMax, float fValue, tAction Callback )
{
	CProgressBarHorizontal* pProgressBar = new CProgressBarHorizontal ( this );

	if ( pProgressBar )
	{
		pProgressBar->SetPos ( CVector ( iX, iY ) );
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
		pProgressBar->SetPos ( CVector ( iX, iY ) );
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
		pWindow->SetPos ( CVector ( X, Y ) );
		pWindow->SetSize ( Width, Height );
		pWindow->SetText ( szString );
		pWindow->SetAction ( Callback );
		pWindow->SetAlwaysOnTop ( bAlwaysOnTop );

		this->AddWidget ( pWindow );
	}

	return pWindow;
}

//--------------------------------------------------------------------------------------
CButton *CDialog::AddButton ( CWindow *pWindow, int X, int Y, int Width, int Height, const SIMPLEGUI_CHAR *szString, tAction Callback )
{
	CButton* pButton = new CButton ( this );

	if ( pButton )
	{
		pButton->SetPos ( CVector ( X, Y ) );
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
		if (pWindow)
			pWindow->AddControl(pCheckBox);

		pCheckBox->SetPos ( CVector ( X, Y ) );
		pCheckBox->SetText ( szString );
		pCheckBox->SetWidth(Width);
		pCheckBox->SetAction ( Callback );
		pCheckBox->SetChecked ( bChecked );
	}

	return pCheckBox;
}

//--------------------------------------------------------------------------------------
CListBox *CDialog::AddListBox ( CWindow *pWindow, int X, int Y, int Width, int Height, tAction Callback )
{
	CListBox* pListBox = new CListBox ( this );

	if ( pListBox )
	{
		pListBox->SetPos ( CVector ( X, Y ) );
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
		pListView->SetPos ( CVector ( X, Y ) );
		if ( pWindow )
			pWindow->AddControl ( pListView );
		pListView->SetSize ( Width, Height );
		pListView->SetAction ( Callback );
	}

	return pListView;
}

//--------------------------------------------------------------------------------------
CLogBox *CDialog::AddTextBox ( CWindow *pWindow, int X, int Y, int Width, int Height, tAction Callback )
{
	CLogBox* pTextBox = new CLogBox ( this );

	if ( pTextBox )
	{
		pTextBox->SetPos ( CVector ( X, Y ) );
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
		pLabel->SetPos ( CVector ( X, Y ) );
		pLabel->SetText ( szString );

		if (pWindow)
			pWindow->AddControl(pLabel);
		pLabel->SetSize ( Width, Height );
		pLabel->SetAction ( Callback );
	}

	return pLabel;
}

//--------------------------------------------------------------------------------------
CEditBox *CDialog::AddEditBox ( CWindow *pWindow, int X, int Y, int Width, int Height, const SIMPLEGUI_CHAR *szString, bool bSelected, tAction Callback )
{
	CEditBox* pEditBox = new CEditBox ( this );

	if ( pEditBox )
	{
		
		pEditBox->SetPos ( CVector ( X, Y ) );
		if ( pWindow )
			pWindow->AddControl ( pEditBox );
		pEditBox->SetSize ( Width, Height );
		pEditBox->SetText ( szString, bSelected );
		pEditBox->SetAction ( Callback );

		
	}

	return pEditBox;
}

//--------------------------------------------------------------------------------------
CDropDown *CDialog::AddDropDown ( CWindow *pWindow, int X, int Y, int Width, int Height, const SIMPLEGUI_CHAR *szString, tAction Callback )
{
	CDropDown* pDropDown = new CDropDown ( this );

	if ( pDropDown )
	{
		pDropDown->SetPos ( CVector ( X, Y ) );
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
		if (pWindow)
			pWindow->AddControl(pRadioButton);
		pRadioButton->SetGroup ( iGroup );
		pRadioButton->SetPos ( CVector ( X, Y ) );
		pRadioButton->SetText(szString);
		pRadioButton->SetWidth ( Width );
		pRadioButton->SetAction ( Callback );
	}

	return pRadioButton;
}

CTabPanel *CDialog::AddTabPanel ( CWindow *pWindow, int X, int Y, int Width, int Height, tAction Callback )
{
	CTabPanel *pTabPanel = new CTabPanel ( this );

	if ( pTabPanel )
	{
		
		pTabPanel->SetPos ( CVector ( X, Y ) );
		if ( pWindow )
			pWindow->AddControl ( pTabPanel );
		pTabPanel->SetSize ( Width, Height );
		pTabPanel->SetAction ( Callback );


	}

	return pTabPanel;
}

//--------------------------------------------------------------------------------------
CPictureBox *CDialog::AddImage ( CWindow *pWindow, const TCHAR * szPath, int X, int Y, int Width, int Height, tAction Callback )
{
	CPictureBox *pImage = new CPictureBox ( this );

	if ( pImage )
	{
		pImage->SetPos ( CVector ( X, Y ) );

		if ( pWindow )
			pWindow->AddControl ( pImage );

		pImage->SetSize ( Width, Height );
		pImage->SetTexture ( szPath );
		pImage->SetAction ( Callback );
	
	}

	return pImage;
}

//--------------------------------------------------------------------------------------
CTrackBarHorizontal *CDialog::AddTrackBar ( CWindow * pWindow, int X, int Y, int Width, int Height, int nMin, int nMax, int nValue, tAction Callback )
{
	CTrackBarHorizontal *pTrackBar = new CTrackBarHorizontal ( this );

	if ( pTrackBar )
	{
		pTrackBar->SetPos ( CVector ( X, Y ) );
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
		pTrackBar->SetPos ( CVector ( X, Y ) );
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
		pScrollBar->SetPos ( CVector ( X, Y ) );
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
		pScrollBar->SetPos ( CVector ( X, Y ) );
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

	for ( auto widget : m_vWidgets )
	{
		if ( !widget )
			continue;

		if ( !widget->IsVisible () )
			continue;

		widget->UpdateRects ();
		widget->EnterScissorRect ();
		widget->Draw ();
		widget->LeaveScissorRect ();
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

	if ( m_vWidgets.empty () )
		return;

	Pos pos = m_pMouse->GetPos ();

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
	CWidget* pWindow = GetWidgetAtPos ( pos );

	if ( !pWindow
		 && e.mouseEvent.eMouseMessages == sMouseEvents::ButtonDown &&
		 e.mouseEvent.eButton == sMouseEvents::LeftButton &&
		 m_pFocussedWidget && m_pFocussedWidget->GetType () != CWidget::TYPE_WINDOW )
	{
		ClearFocussedWidget ();
	}

	if ( m_pFocussedWidget&& m_pFocussedWidget->GetType () == CWidget::TYPE_TABPANEL )
	{
		if ( static_cast< CTabPanel* >( m_pFocussedWidget )->ControlMessages ( e ) )
			return;
	}

	if ( m_pFocussedWidget &&m_pFocussedWidget->GetType () == CWidget::TYPE_WINDOW&& m_pFocussedWidget->InjectKeyboard ( e.keyEvent ) )
		return;

	if ( pWindow && 
		 pWindow->GetType () == CWidget::TYPE_TABPANEL )
	{
		if ( static_cast< CTabPanel* >( pWindow )->ControlMessages ( e ) )
			return;
	}

	// it the first chance at handling the message.
	if ( m_pFocussedWidget &&
		 m_pFocussedWidget->GetType () != CWidget::TYPE_WINDOW &&
		 ( pWindow &&pWindow->GetType () != CWidget::TYPE_WINDOW ) )
	{
		if ( m_pFocussedWidget->InjectMouse ( e.mouseEvent ) )
			return;
	}

	if ( pWindow &&
		 pWindow->GetType () != CWidget::TYPE_WINDOW &&
		 pWindow->InjectMouse ( e.mouseEvent ) )
		return;

	if ( m_pFocussedWidget &&
		 m_pFocussedWidget->IsEnabled () )
	{
		if ( m_pFocussedWidget->InjectKeyboard ( e.keyEvent ) )
			return;
	}

	if ( m_pFocussedWidget &&
		 m_pFocussedWidget->IsEnabled () &&
		 m_pFocussedWidget->GetType () == CWidget::TYPE_WINDOW )
	{
		CWindow *pFocussedWindow = reinterpret_cast< CWindow* >( m_pFocussedWidget );
		CWidget *pControl = pFocussedWindow->GetFocussedControl ();

		bool bOnDrag = false;

		if ( e.keyEvent.uMsg == uMsg )
			bOnDrag = true;

		if ( ( pControl && pWindow == pFocussedWindow && uMsg != WM_LBUTTONDOWN ) ||
			( GetAsyncKeyState ( VK_LBUTTON ) && uMsg == WM_MOUSEMOVE ) )
		{
			bOnDrag = true;
		}

		if ( bOnDrag && pFocussedWindow->ControlMessages ( e ) )
			return;
	}

	if ( pWindow &&
		 pWindow->IsEnabled () &&
		 pWindow->GetType () == CWidget::TYPE_WINDOW )
	{
		if ( reinterpret_cast< CWindow* >( pWindow )->ControlMessages ( e ) )
			return;
	}

	// If a window is in focus, and it's enabled, then give
	// it the first chance at handling the message.
	if ( m_pFocussedWidget &&
		 m_pFocussedWidget->IsEnabled () )
	{
		if ( m_pFocussedWidget && m_pFocussedWidget->InjectKeyboard ( e.keyEvent ) )
			return;
	}

	if ( pWindow )
	{
		if ( pWindow->InjectMouse ( e.mouseEvent ) )return;
	}
	else
	{
		if ( uMsg == WM_LBUTTONDOWN )
		{
			ClearFocussedWidget ();
		}
	}

	if ( m_pFocussedWidget )
	{
		// If the control is in focus, and if the mouse is outside the window, then leave 
		// the click event
		if ( uMsg == WM_LBUTTONUP )
		{
			m_pFocussedWidget->OnClickLeave ();
		}
	}

	if ( m_pFocussedWidget && m_pFocussedWidget->InjectMouse ( e.mouseEvent ) )
		return;

	if ( !( GetAsyncKeyState ( VK_LBUTTON ) ) )
	{
		// If the mouse is still over the same window, nothing needs to be done
		if ( pWindow == m_pMouseOverWidget )
			return;

		// Handle mouse leaving the old window
		if ( m_pMouseOverWidget )
		{
			m_pMouseOverWidget->OnMouseLeave ();
		}

		// Handle mouse entering the new window
		m_pMouseOverWidget = pWindow;
		if ( m_pMouseOverWidget )
			m_pMouseOverWidget->OnMouseEnter ();
	}

	//// See if the mouse is over any widgets
	//CWidget* pWidget = GetWidgetAtPos ( pos );

	//if ( m_pFocussedWidget &&
	//	 m_pFocussedWidget->IsEnabled () &&
	//	 m_pFocussedWidget->GetType () == CWidget::TYPE_WINDOW )
	//{
	//	CWindow *pFocussedWindow = reinterpret_cast< CWindow* >( m_pFocussedWidget );
	//	CWidget *pControl = pFocussedWindow->GetFocussedControl ();

	//	bool bOnDrag = false;

	//	if ( e.keyEvent.uMsg == uMsg )
	//		bOnDrag = true;

	//	if ( ( pControl && ( pWidget == pFocussedWindow ) && uMsg != WM_LBUTTONDOWN ) ||
	//		( GetAsyncKeyState ( VK_LBUTTON ) && uMsg == WM_MOUSEMOVE ) )
	//	{
	//		bOnDrag = true;
	//	}

	//	if ( bOnDrag &&pFocussedWindow->ControlMessages ( e ) )
	//		return;
	//}

	//if ( pWidget &&
	//	 pWidget->IsEnabled () &&
	//	 pWidget->GetType () == CWidget::TYPE_WINDOW )
	//{
	//	if ( reinterpret_cast< CWindow* >( pWidget )->ControlMessages ( e ) )
	//		return;
	//}

	//CTabPanel *pTabPanel = NULL;

	//if ( m_pFocussedWidget &&
	//	 m_pFocussedWidget->GetType () == CWidget::TYPE_TABPANEL )
	//{
	//	pTabPanel = reinterpret_cast< CTabPanel* >( m_pFocussedWidget );
	//}
	//else if ( pWidget &&
	//		  pWidget->GetType () == CWidget::TYPE_TABPANEL )
	//{
	//	pTabPanel = reinterpret_cast< CTabPanel* >( pWidget );
	//}

	//if ( pTabPanel &&
	//	 pTabPanel->IsEnabled () )
	//{
	//	if ( pTabPanel->ControlMessages ( e ) )
	//		return;
	//}

	//if ( m_pFocussedWidget &&
	//	 m_pFocussedWidget->IsEnabled () &&
	//	 ( pWidget && pWidget->GetType () != CWidget::TYPE_WINDOW ) )
	//{
	//	if ( m_pFocussedWidget->InjectMouse ( e.mouseEvent ) )
	//		return;
	//}

	//if ( pWidget &&
	//	 pWidget->IsEnabled () )
	//{
	//	if ( pWidget->InjectMouse ( e.mouseEvent ) )
	//		return;
	//}
	//else
	//{
	//	if ( uMsg == WM_LBUTTONDOWN )
	//	{
	//		ClearFocussedWidget ();
	//	}
	//}

	////if ( m_pFocussedWidget )
	////{
	////	// If the control is in focus, and if the mouse is outside the window, then leave 
	////	// the click event
	////	if ( uMsg == WM_LBUTTONUP )
	////	{
	////		m_pFocussedWidget->OnClickLeave ();
	////	}
	////}

	//if ( m_pFocussedWidget &&
	//	 m_pFocussedWidget->IsEnabled () )
	//{
	//	if ( m_pFocussedWidget->InjectMouse ( e.mouseEvent ) )
	//		return;
	//}

	//if ( !( GetAsyncKeyState ( VK_LBUTTON ) ) )
	//{
	//	// If the mouse is still over the same window, nothing needs to be done
	//	if ( pWidget == m_pMouseOverWidget )
	//		return;

	//	// Handle mouse leaving the old window
	//	if ( m_pMouseOverWidget )
	//	{
	//		m_pMouseOverWidget->OnMouseLeave ();
	//	}

	//	// Handle mouse entering the new window
	//	m_pMouseOverWidget = pWidget;
	//	if ( m_pMouseOverWidget )
	//	{
	//		m_pMouseOverWidget->OnMouseEnter ();
	//	}
	//}

	LeaveCriticalSection ( &cs );
}

//--------------------------------------------------------------------------------------
void CDialog::AddWidget ( CWidget *pWidget )
{
	if ( !pWidget )
		return;

	m_vWidgets.push_back ( pWidget );
	SetFocussedWidget ( pWidget );
}

//--------------------------------------------------------------------------------------
void CDialog::RemoveWidget ( CWidget *pWidget )
{
	if ( !pWidget )
		return;

	std::vector<CWidget*>::iterator iter = std::find ( m_vWidgets.begin (), m_vWidgets.end (), pWidget );
	if ( iter == m_vWidgets.end () )
		return;

	m_vWidgets.erase ( iter );
	SAFE_DELETE ( pWidget );
}

//--------------------------------------------------------------------------------------
void CDialog::RemoveAllWidgets ( void )
{
	for ( auto &window : m_vWidgets )
		SAFE_DELETE ( window );

	m_vWidgets.clear ();
}

//--------------------------------------------------------------------------------------
bool CDialog::IsWidgetInList ( CWidget *pWidget )
{
	std::vector<CWidget*>::iterator iter = std::find ( m_vWidgets.begin (), m_vWidgets.end (), pWidget );
	if ( iter != m_vWidgets.end () )
		return true;

	return false;
}

//--------------------------------------------------------------------------------------
void CDialog::SetFocussedWidget ( CWidget *pWidget )
{
	if ( !IsWidgetInList ( pWidget ) )
		return;

	if ( m_pFocussedWidget == pWidget )
		return;

	EnterCriticalSection ( &cs );

	if ( m_pFocussedWidget )
		m_pFocussedWidget->OnFocusOut ();

	if ( pWidget )
		pWidget->OnFocusIn ();

	if ( pWidget )
		BringWidgetToTop ( pWidget );

	m_pFocussedWidget = pWidget;

	LeaveCriticalSection ( &cs );
}

//--------------------------------------------------------------------------------------
void CDialog::ClearFocussedWidget ( void )
{
	if ( m_pFocussedWidget )
	{
		if ( m_pFocussedWidget->GetType () == CWidget::TYPE_WINDOW )
		{
			reinterpret_cast< CWindow* >( m_pFocussedWidget )->ClearControlFocus ();
		}

		m_pFocussedWidget->OnClickLeave ();
		m_pFocussedWidget->OnFocusOut ();
		m_pFocussedWidget = NULL;
	}
}

//--------------------------------------------------------------------------------------
CWidget *CDialog::GetFocussedWidget ( void )
{
	return m_pFocussedWidget;
}

//--------------------------------------------------------------------------------------
void CDialog::BringWidgetToTop (CWidget *pWidget )
{
	std::vector<CWidget*>::iterator iter = std::find ( m_vWidgets.begin (), m_vWidgets.end (), pWidget );
	if ( iter == m_vWidgets.end () )
		return;

	if ( pWidget->GetType () == CWidget::TYPE_WINDOW )
	{
		CWindow *pWindow = static_cast< CWindow* >( pWidget );
		if ( !pWindow->GetAlwaysOnTop () )

		{	// Get amount of windows on top
			int nCount = 0;
			for ( auto widget : m_vWidgets )
			{
				if ( widget->GetType () == CWidget::TYPE_WINDOW )
				{
					if ( static_cast< CWindow* >( widget )->GetAlwaysOnTop () )
						nCount++;
				}
			}

			m_vWidgets.erase ( iter );
			m_vWidgets.insert ( m_vWidgets.end () - nCount, pWindow );
		}
	}
	else
	{
		// Get amount of windows to skip it
		int nWindowsCount = 0;
		for ( auto widget : m_vWidgets )
		{
			if ( widget->GetType () == CWidget::TYPE_WINDOW )
				nWindowsCount++;
		}

		m_vWidgets.erase ( iter );
		m_vWidgets.insert ( m_vWidgets.end () - nWindowsCount, pWidget );
	}
}

//--------------------------------------------------------------------------------------
CWidget *CDialog::GetWidgetAtPos ( Pos pos )
{
	for ( std::vector<CWidget*>::reverse_iterator iter = m_vWidgets.rbegin (); iter != m_vWidgets.rend (); iter++ )
	{
		if ( ( *iter )->ContainsPoint ( pos ) )
			return ( *iter );
	}

	return NULL;
}

//--------------------------------------------------------------------------------------
CWidget* CDialog::GetWindowByText ( const SIMPLEGUI_CHAR *pszText )
{
	for ( auto &window : m_vWidgets )
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

	D3DVIEWPORT9 vp;
	m_pDevice->GetViewport ( &vp );

	if ( m_oldvp.Width != vp.Width ||
		 m_oldvp.Height != vp.Height )
	{
		if ( vp.Width < m_oldvp.Width && m_oldvp.Width > 0 ||
			 vp.Height < m_oldvp.Height && m_oldvp.Height > 0 )
		{
			for ( auto &widget : m_vWidgets )
			{
				if ( !widget )
					continue;

				CVector pos = *widget->GetPos ();
				SIZE size = widget->GetSize ();

				if ( pos.m_nX + size.cx >= vp.Width || 
					pos.m_nY + size.cy >= vp.Height)
				{
					int nX = vp.Width - size.cx;
					int nY = vp.Height - size.cy;

					widget->SetPos ( nX <= 1 ? 1 : nX, nY <= 1 ? 1 : nY );
				}

				widget->SetSize ( size.cx >= vp.Width ? vp.Width : size.cx,
								  size.cy >= vp.Height ? vp.Height : size.cy );
			}
		}

		m_oldvp.Width = vp.Width;
		m_oldvp.Height = vp.Height;
	}

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
