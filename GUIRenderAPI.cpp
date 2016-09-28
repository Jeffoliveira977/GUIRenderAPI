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
		//EnterCriticalSection ( &cs_gui );

		pGui->MsgProc ( uMsg, wParam, lParam );

		/*CMouse *pMouse = pGui->GetMouse ();
		if ( pMouse )
		pMouse->HandleMessage ( uMsg, wParam, lParam );

		CKeyboard *pKey = pGui->GetKeyboard ();
		if ( pKey )
		pKey->HandleMessage ( uMsg, wParam, lParam );*/

		//LeaveCriticalSection ( &cs_gui );
	}

	return CallWindowProc ( gameProc, hWnd, uMsg, wParam, lParam );
}


int RandU ( int nMin, int nMax )
{
	return nMin + ( int ) ( ( double ) rand () / ( RAND_MAX + 1 ) * ( nMax - nMin + 1 ) );
}

VOID Init ()
{
	IDirect3DDevice9* pDevice = *( IDirect3DDevice9** ) 0x00C97C28;
	//EnterCriticalSection ( &cs_gui );

	pGui = new CDialog ( pDevice );

	D3DVIEWPORT9 vp;
	pDevice->GetViewport ( &vp );
	DWORD s_iWidth = vp.Width;
	DWORD s_iHeight = vp.Height;

	pGui->LoadFont ( _UI ( "Tahoma" ), 10, false );
	auto fServBrowser1 = pGui->AddWindow ( s_iWidth / 2 + 800, s_iHeight / 2, 750, 500, _UI ( "SERVER BROWSER" ), 1 );
	fServBrowser1->AddControl ( fServBrowser );
	// Create Servers Brouser
	fServBrowser = pGui->AddWindow ( s_iWidth / 2, s_iHeight / 2, 750, 500, _UI ( "SERVER BROWSER" ),1 );

	

	sbTab [ 0 ] = pGui->AddButton ( fServBrowser, 0, 1110, 200, 20, _UI ( "Internet" ) );

	sbTab [ 1 ] = pGui->AddButton ( fServBrowser, 169, 0, 200, 20, _UI ( "LAN" ) );
	sbTab [ 2 ] = pGui->AddButton ( fServBrowser, 318, 0, 200, 20, _UI ( "VIP" ) );
	sbTab [ 3 ] = pGui->AddButton ( fServBrowser, 467, 0, 200, 20, _UI ( "Favourite" ) );

	sbConnect = pGui->AddButton ( fServBrowser, 560, 380, 80, 20, _UI ( "Connect" ) );
	sbRefresh = pGui->AddButton ( fServBrowser, 660, 380, 80, 20, _UI ( "Refresh" ) );
	sbAddToFav = pGui->AddButton ( fServBrowser, 600, 405, 120, 20, _UI ( "Add to favourites" ) );
	
	sbFltNotFull = pGui->AddCheckBox ( fServBrowser, 150, 410, 100, true, _UI ( "Add to favourites" ) );

	sbFltNotEmpty = pGui->AddCheckBox ( fServBrowser, 480, 385, 100, true, _UI ( "Add to favourites" ) );
	sbFltNoPassword = pGui->AddCheckBox ( fServBrowser, 480, 410, 100, true, _UI ( "Add to favourites" ) );

	
	auto pScroll1 = pGui->AddScrollBar ( fServBrowser, 300, 200, 18, 200, -20, 100, 10, 1 );
	pScroll1->SetEnabled ( false );
	auto  pRadio2 = pGui->AddRadioButton ( fServBrowser, 0, 160, 220, 120, _UI ( "Refresh" ) );
	auto  pRadios2 = pGui->AddRadioButton ( fServBrowser, 0, 160, 280, 120, _UI ( "Refresh" ) );

	auto  pRadio12 = pGui->AddRadioButton ( fServBrowser, 1, 220, 220, 120, _UI ( "Refresh" ) );
	auto  pRadios12 = pGui->AddRadioButton ( fServBrowser, 1, 220, 280, 1120, _UI ( "Refresh" ) );
	


	auto fServBrowser2 = pGui->AddWindow ( s_iWidth / 2, s_iHeight / 2 + 600, 750, 500, _UI ( "SERVER BROWSER" ) );
	auto fServBrowser3 = pGui->AddWindow ( 800, s_iHeight / 2 + 600, 750, 500, _UI ( "SERVER BROWSER" ),1 );


	CButton *sbTab2 [ 4 ];
	sbTab2 [ 0 ] = pGui->AddButton ( fServBrowser1, 20, 220, 200, 20, _UI ( "Internet" ) );
	sbTab2 [ 1 ] = pGui->AddButton ( fServBrowser1, 169, 0, 200, 20, _UI ( "LAN" ) );
	sbTab2 [ 2 ] = pGui->AddButton ( fServBrowser1, 318, 0, 200, 20, _UI ( "VIP" ) );
	sbTab2 [ 3 ] = pGui->AddButton ( fServBrowser1, 467, 0, 200, 20, _UI ( "Favourite" ) );
	sbTab [ 0 ]->SetRelativeX ( CControl::RELATIVE_POS );
	sbTab [ 0 ]->SetRelativeY ( CControl::RELATIVE_POS );

	auto sbConnect2 = pGui->AddButton ( fServBrowser1, 560, 380, 80, 20, _UI ( "Connect" ) );
	auto sbRefresh2 = pGui->AddButton ( fServBrowser1, 660, 380, 80, 20, _UI ( "Refresh" ) );
	auto sbAddToFav2 = pGui->AddButton ( fServBrowser1, 600, 405, 120, 20, _UI ( "Add to favourites" ) );

	//sbAddToFav2->SetRelativePosX ( true );
	//sbAddToFav2->SetRelativePosY ( true );
	//sbAddToFav2->SetRelativeSizeX ( true );
	//sbAddToFav2->SetRelativeSizeY ( true );
	auto sbFltNotFull2 = pGui->AddCheckBox ( fServBrowser1, 150, 410, 150, true, _UI ( "Add to favourites" ) );

	auto sbFltNotEmpty2 = pGui->AddCheckBox ( fServBrowser1, 480, 385, 40, 0, _UI ( "Add to favourites" ) );
	auto sbFltNoPassword2 = pGui->AddCheckBox ( fServBrowser1, 480, 420, 150, true, _UI ( "Add to favourites" ) );


	auto  pRadio22 = pGui->AddRadioButton ( fServBrowser1, 0, 600, 220, 120, _UI ( "Refresh" ) );
	auto  pRadios22 = pGui->AddRadioButton ( fServBrowser1, 0, 600, 280, 120, _UI ( "Refresh" ) );

	auto  pRadio122 = pGui->AddRadioButton ( fServBrowser1, 1, 220, 220, 115, _UI ( "Refresh" ) );
	auto  pRadios122 = pGui->AddRadioButton ( fServBrowser1, 1, 220, 280, 115, _UI ( "Refresh" ) );

		auto ptrack = pGui->AddTrackBarVertical ( fServBrowser1, 300, 200, 20, 400, -10, 1000, 990 );
	auto pScroll = pGui->AddScrollBar ( fServBrowser1, 300, 200, 18, 200, 0, 100, 10, 1 );
	pScroll->SetTrackRange ( 0, 300 );
	auto pDrop = pGui->AddDropDown ( fServBrowser1, 500, 200, 200, 20, _UI ( "WWWWWWWWWWWWW") );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASDaasdasdsasdas{FF750000} asd as asd asdasdas asd" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) ); pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) ); pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	pDrop->AddItem ( _UI ( "ASD" ) );
	

	auto pTextBox = pGui->AddListBox ( fServBrowser1, 20, 350, 350, 108 );
	for ( size_t i = 0; i < 40; i++ )
	{
		TCHAR szzx [ 30 ];
		SIMPLEGUI_SPRINTF ( szzx,_UI("%i"), i );
		pTextBox->AddItem ( szzx );

	}
	pTextBox->SetMultiSelection ( false );
	

	auto pProgressBar = pGui->AddProgressBarHorizontal ( fServBrowser1, 100, 100, 200, 20, 200, 180 );


	auto pScrolls = pGui->AddScrollBarHorizontal ( fServBrowser1, 300, 200, 200, 18, 0, 20, 15, 199 );

	
	auto pLabel = pGui->AddLabel ( fServBrowser1, 400, 400, 300, 200, _UI ( "Great Text asd sChanched" ));

	

	auto psd = pGui->AddEditBox ( fServBrowser3, 400, 0, 800, 0,_UI ("sasd{FF750000}asaaddssddsadssaddsasd") );
	auto psd1 = pGui->AddEditBox ( fServBrowser3, 0, 40, 200, 0, _UI ( "sasd{FF750000}asaaddssddsadssaddsasd" ) );
	auto psd2 = pGui->AddEditBox ( fServBrowser3, 0, 80, 200, 0, _UI ( "sasd{FF750000}asaaddssddsadssaddsasd" ) );
	psd->SetRelativeX ( CControl::RELATIVE_POS );
	psd->SetRelativeY ( CControl::RELATIVE_SIZE );
	//psd->SetRelativeY ( CControl::RELATIVE_POS );
	//auto dsds = pGui->AddTabPanel ( fServBrowser3, 860, 420, 300, 200 );
	//dsds->AddTab ( _UI ( "Tab 1" ), 100 );
	//dsds->AddTab ( _UI ( "Tab 2" ), 60 );
	//dsds->AddTab ( _UI ( "Tab 3" ), 100 );
	//dsds->AddTab ( _UI ( "Tab 4" ), 120 );
	//dsds->AddTab ( _UI ( "Tab 5" ), 100 );
	//dsds->AddTab ( _UI ( "Tab 6" ), 100 );
	//dsds->AddTab ( _UI ( "Tab 7" ), 120 );
	//dsds->AddTab ( _UI ( "Tab 8" ), 100 );
	//dsds->SetRelativeX ( CControl::RELATIVE_POS );
	//dsds->SetRelativeY ( CControl::RELATIVE_SIZE );
	auto sds = pGui->AddTabPanel ( fServBrowser3, 120, 120, 400, 1300 );
	sds->AddTab ( _UI ( "Tab 1" ), 100 );
	sds->AddTab ( _UI ( "Tab 2" ), 60 );
	sds->AddTab ( _UI ( "Tab 3" ), 100 );
	sds->AddTab ( _UI ( "Tab 4" ), 120 );
	sds->AddTab ( _UI ( "Tab 5" ), 100 );
	sds->AddTab ( _UI ( "Tab 6" ), 100 );
	sds->AddTab ( _UI ( "Tab 7" ), 120 );
	sds->AddTab ( _UI ( "Tab 8" ), 100 );
	sds->SetRelativeX ( CControl::RELATIVE_POS );
	sds->SetRelativeY ( CControl::RELATIVE_SIZE );

	auto list = pGui->AddListView ( fServBrowser, 100, 100, 500, 100, _UI ( "as" ) );
	//list->SetRelativeSizeX ( true );

	CListView *pListView = new CListView ( pGui );
	pListView->SetPos ( CPos ( 0, 200 ) );
	pListView->SetSize ( 500, 220 );
	pListView->SetText ( _UI ( "Refresh" ) );

	pListView->AddColumn ( _UI ( "ASDAS" ), 100 );
	pListView->AddColumn ( _UI ( "ASDAS" ), 100 );
	pListView->AddColumn ( _UI ( "ASDAS" ), 100 );
	pListView->AddColumn ( _UI ( "ASDAS" ), 100 );

	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 1, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 2, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 3, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 3, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 1, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 1, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 1, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 1, _UI ( "ASD" ) );
	pListView->AddColumnItem ( 0, _UI ( "ASD" ) );

	CDropDown *pDropd = new CDropDown ( pGui );
	pDropd->SetPos ( CPos ( 500, 200 ) );
	pDropd->SetSize ( 200, 20 );
	pDropd->SetText ( _UI ( "Refresh" ) );
	//pDropd->SetRelativeX ( CControl::RELATIVE_SIZE );

	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASDaasdasdsasdas{FF750000} asd as asd asdasdas asd" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) ); pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) ); pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );
	pDropd->AddItem ( _UI ( "ASD" ) );


	CButton* pButton = new CButton ( pGui );

	CEditBox *pEdit = new CEditBox ( pGui );


	CRadioButton *pRadio221 = new CRadioButton ( pGui );
	pRadio221->SetPos ( CPos ( 0, 410 ) );
	pRadio221->SetSize ( 220, 0 );
	pRadio221->SetGroup ( 0 );
	pRadio221->SetText ( _UI ( "Refresh" ) );

	CRadioButton *pRadio3 = new CRadioButton ( pGui );
	pRadio3->SetPos ( CPos ( 260, 280 ) );
	pRadio3->SetSize ( 220, 0 );
	pRadio3->SetGroup ( 0 );
	pRadio3->SetText ( _UI ( "Refresh" ) );

	CRadioButton *pRadio222 = new CRadioButton ( pGui );
	pRadio222->SetPos ( CPos ( 220, 220 ) );
	pRadio222->SetSize ( 220, 0 );
	pRadio222->SetGroup ( 1 );
	pRadio222->SetText ( _UI ( "Refresh" ) );

	CRadioButton *pRadio23 = new CRadioButton ( pGui );
	pRadio23->SetPos ( CPos ( 220, 280 ) );
	pRadio23->SetSize ( 220, 0 );
	pRadio23->SetGroup ( 1 );
	pRadio23->SetText ( _UI ( "Refresh" ) );

	sds->AddControl ( 0, pRadio23 );
	sds->AddControl ( 0, pRadio222 );
	sds->AddControl ( 0, pRadio3 );
	sds->AddControl ( 0, pRadio221 );
	sds->AddControl ( 1, pDropd );
	sds->AddControl ( 2, pListView );
	if ( pEdit )
	{
		pEdit->SetPos ( CPos ( 150, 0 ) );
		pEdit->SetSize ( 100, 0 );
		pEdit->SetText ( _UI ( "sasd{FF750000}asaaddssddsadssaddsasd" ) );
	}

	if ( pButton )
	{
		pButton->SetPos ( CPos (250, 200 ) );
		pButton->SetSize ( 100, 100 );
		pButton->SetText ( _UI ( "ASDSA" ) );
 
	}

	sds->AddControl ( 1, pEdit );
	sds->AddControl ( 2, pButton );

	pGui->SetVisible ( true );
	//LeaveCriticalSection ( &cs_gui );

	gameProc = ( WNDPROC ) SetWindowLong ( *( HWND* ) 0x00C8CF88, GWL_WNDPROC, ( LONG ) DefWndProc );

	*( DWORD* ) 0x6194A0 = 0xC3;
}

VOID Draw ()
{
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