// GUIAPI.cpp : Defines the exported functions for the DLL application.
//
#include "GUIRenderAPI.h"
#include "GUI\CGUI.h"

// Windows
CWindow * fServBrowser;
CWindow * fChat;
CWindow * fOption;
CWindow * fUserRegister;
CWindow * fUserLogin;
CWindow * fInfo;
CWindow * fEnterNick;
// Chat elements
CLogBox * cc_tChat;
CEditBox * cc_tEnter;
CButton * cc_bEnter;
// Server Browser elemenets
CListView *sbServList, *sbPlayerList;
CButton *sbTab [ 4 ];
CEditBox *sbEnterIP, *sbEnterPort;
CButton *sbConnect, *sbRefresh, *sbAddToFav;
CDropDown *sbFltPing;
CCheckBox *sbFltNotFull, *sbFltNoPassword, *sbFltNotEmpty;
CEditBox *sbFltLocation, *sbFltMode;
// Register elements
CEditBox *urLogin, *urPass, *urConfirm, *urEmail, *urNick;
CLabel *urLoginText, *urPassText, *urConfirmText, *urEmailText, *urNickText, *urInfoText;
CButton *urSendReg, *urCancelReg;
// Login elements
CLabel *upLoginInfo, *upStrLogin, *upStrPass;
CEditBox *upLogin, *upPassword;
CButton *upSendLogin, *upShowRegister;
CCheckBox *upRemeberMe;
// Enter nick
CEditBox * enNick;
CButton * enOK;
CDialog *pGui = NULL;

#include "injector\utility.hpp"
#include "internal\CallbackResetDevice.hpp"
#include "injector\calling.hpp"

static CallbackResetDevice ResetDevice;
static CallbackManager1C<0x53E4FF> DrawHUDFunc;
static CallbackManager1C<0x46937B> InitialiseTheScriptsFunc;
static CallbackManager1C<0x53BC21> TerminateGame;
static CallbackManager1C<0x748CFB> InitGameFunc;
static CallbackManager2C<0x748E09, 0x748E48> ReInitGameFunc;

CRITICAL_SECTION cs_gui;

VOID RegisterFuncs ()
{
	InitGameFunc.RegisterFuncAfter ( Init );
	DrawHUDFunc.RegisterFuncAfter ( Draw );
	ResetDevice.RegisterFuncBefore ( OnLostDevice );
	ResetDevice.RegisterFuncAfter ( OnResetDevice );

	//InitializeCriticalSection ( &cs_gui );
}

WNDPROC gameProc;
POINT mousePoint;
#define MainWndProc( hWnd, message, wParam, lParam ) ( ( int ( __stdcall* )( HWND, UINT, WPARAM, LPARAM ) ) 0x00747EB0 )( hWnd, message, wParam, lParam )

LRESULT DefWndProc ( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( pGui )
	{
		pGui->MsgProc ( uMsg, wParam, lParam );
	}

	return CallWindowProc ( gameProc, hWnd, uMsg, wParam, lParam );
}


int RandU ( int nMin, int nMax )
{
	return nMin + ( int ) ( ( double ) rand () / ( RAND_MAX + 1 ) * ( nMax - nMin + 1 ) );
}

CD3DFont *g_font = 0;
VOID Init ()
{
	IDirect3DDevice9* pDevice = *( IDirect3DDevice9** ) 0x00C97C28;
	//EnterCriticalSection ( &cs_gui );

	pGui = new CDialog ( pDevice );
	pGui->LoadFont ( _UI ( "arial" ), 9, false );


	
	CWindow *pWindow = pGui->CreateWidget<CWindow> ( Pos ( 200, 200 ), 700, 100, _UI ( "text" ) );
	CWindow *pWindow2 = pGui->CreateWidget<CWindow> ( Pos ( 200, 200 ), 700, 500, _UI ( "text 2" ) );
	pWindow2->SetAlwaysOnTop ( true );

	CEditBox *pEditBox = pGui->CreateWidget<CEditBox> ( Pos ( 0, 220 ), 700, 20, _UI ( "text 2" ) );

	CButton *pButton = pGui->CreateWidget<CButton> ( Pos ( 200, 250 ), 100, 10, _UI ( "text" ) );
	CButton *pButton2 = pGui->CreateWidget<CButton> ( Pos ( 200, 350 ), 100, 10, _UI ( "text 2" ) );
	CListView *pListView = pGui->CreateWidget<CListView> ( Pos ( 210, 0 ), 600, 500, _UI ( "text 2" ) );
	CRadioButton *pRadio = pGui->CreateWidget<CRadioButton> ( Pos ( 210, 120 ), 600, 500, _UI ( "text 2" ) );


	pListView->AddColumn ( _UI ( "Column 1" ),350 );
	pListView->AddColumn ( _UI ( "Column 2" ), 350 );
	//pListView->AddColumn ( _UI ( "Column 3" ), 50 );
	//pListView->AddColumn ( _UI ( "Column 4" ), 50 );

	pListView->AddColumnItem ( 0, _UI ( "1 3" ) );
	pListView->AddColumnItem ( 0, _UI ( "1 3" ) );
	pListView->AddColumnItem ( 0, _UI ( "13" ) );
	pListView->AddColumnItem ( 1, _UI ( "1 4" ) );
	pListView->AddColumnItem ( 1, _UI ( "1 4" ) );
	pListView->AddColumnItem ( 0, _UI ( "1 3" ) );
	pListView->AddColumnItem ( 1, _UI ( "1 4" ) );

	

	/*for ( size_t i = 0; i < 400; i++ )
	{
		int n = 0;
		if ( i >= 100 )
			n = i / 100;
		TCHAR szChar [ 128 ];
		SIMPLEGUI_SPRINTF ( szChar, _UI ( "Item %i" ), i );
		pListView->AddColumnItem ( n, szChar );
	}*/

	pListView->SetSortable ( true );


	CTabPanel *pTabPanel = pGui->CreateWidget<CTabPanel> ( Pos ( 123, 0 ), 1100, 100, _UI ( "text 2" ) );
	pTabPanel->AddTab ( _UI ( "Tab 1" ), 200 );
	pTabPanel->AddTab ( _UI ( "Tab 2" ), 200 );
	pTabPanel->AddTab ( _UI ( "Tab 3" ), 200 );
	pTabPanel->AddTab ( _UI ( "Tab 3" ), 200 );

	pTabPanel->SetRelativeX ( CWidget::eRelative::RELATIVE_SIZE );
	pTabPanel->SetRelativeY ( CWidget::eRelative::RELATIVE_POS );

	//pListView->SetRelativeX ( CWidget::eRelative::RELATIVE_SIZE );
	//pListView->SetRelativeY ( CWidget::eRelative::RELATIVE_SIZE );

	
	pTabPanel->AddControl ( 0, pButton );
	pTabPanel->AddControl ( 1, pButton2 );
	//pTabPanel->AddControl ( 1, pRadio );
	pTabPanel->AddControl ( 2, pListView );
	//pGui->AddWidget ( pTabPanel );
	//pWindow->AddControl ( pListView );
	pWindow2->AddControl ( pTabPanel );

	/*pGui->AddWidget ( pButton );
	pGui->AddWidget ( pButton2 );*/
	pGui->AddWidget ( pEditBox );
	pGui->AddWidget ( pWindow );
	pGui->AddWidget ( pWindow2 );
	pGui->AddWidget ( pListView );

	pGui->SetVisible ( true );
	//LeaveCriticalSection ( &cs_gui );

	gameProc = ( WNDPROC ) SetWindowLong ( *( HWND* ) 0x00C8CF88, GWL_WNDPROC, ( LONG ) DefWndProc );

	*( DWORD* ) 0x6194A0 = 0xC3;
}

VOID Draw ()
{
	auto ptr = ( DWORD* ) 0x00B6F5F0;
	int *as = ( int * ) ( *ptr + 0x598 );
	*as = 1;
	if (g_font)
	{
		TCHAR szchar[512]={_UI("{FfE7E00d}{00000000}{FFE7E00D}{00000000}çéàèãõñìíóò úùvamdsa{FFE7E00D}s{FFE7E00D}d{00000000}a{FFE7E00D}s{00000000}d{FFE7E00D}a{00000000}s{FFE7E00D}d{00000000}d{FFE7E00D}s{00000000}d{FFE7E00D}a{00000000}s{FFE7E00D}d{00000000}s{FFE7E00D}a{00000000}d{FFE7E00D}s{00000000}a{FFE7E00D}s{00000000}d{FFE7E00D}s{00000000}d{FFE7E00D}s {00000000}{FFE7E00D}{00000000}{FFE7E00D}fasdsdk jkazer um texto muito {ffe7e00D}loko da cabeça {FFE7E00D}porque eu quero fazer isso sim entendeu, va se {FFE7E00D}foder")};

		SIMPLEGUI_STRING stri= szchar;
		g_font->FormatText ( stri, 500 );
		g_font->CutString(800,stri);
		g_font->Print(220, 20, D3DCOLOR_RGBA(255, 255, 255, 255), stri.c_str(),  D3DFONT_COLORTABLE);
	}
	//pGui->GetRenderer ()->D3DCircle ( 300, 300, 200, D3DCOLOR_RGBA ( 255, 255, 255, 255 ),0, true );

	int posx = 0;
	if ( GetAsyncKeyState ( VK_LEFT ) )
		posx += 10;
	
	//EnterCriticalSection ( &cs_gui );
	if ( pGui )
		pGui->Draw ();
	//LeaveCriticalSection ( &cs_gui );
}

VOID OnLostDevice ()
{
	//EnterCriticalSection ( &cs_gui );
	if ( pGui )
		pGui->OnLostDevice ();
	//LeaveCriticalSection ( &cs_gui );
}

VOID OnResetDevice ()
{
	//EnterCriticalSection ( &cs_gui );
	if ( pGui )
		pGui->OnResetDevice ();
	//LeaveCriticalSection ( &cs_gui );
}