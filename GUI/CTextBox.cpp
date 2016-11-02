#include "CGUI.h"

#define TEXTBOX_TEXTSPACE 18

CLogBox::CLogBox ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_TEXTBOX );

	/*m_sControlColor.d3dColorFont = D3DCOLOR_RGBA ( , 0, 0, 255 );
	m_sControlColor.d3dColorSelectedFont = D3DCOLOR_RGBA ( 255, 255, 255, 255 );
	*/

	m_bGenerateLog = false;

	m_pEntryList = new CEntryList ( pDialog );

	if ( !m_pEntryList )
		MessageBox ( 0, _UI ( "CLogBox::CLogBox: Error for creating CEntryList" ), _UI ( "GUIAPI.asi" ), 0 );

	m_pEntryList->GetScrollbar ()->SetControl ( this );

	m_pLogFile = new CLogFile ( _UI ( "Log.txt" ) );
}

CLogBox::~CLogBox ( void )
{
	SAFE_DELETE ( m_pEntryList );
	SAFE_DELETE ( m_pLogFile );
}

void CLogBox::Draw ( void )
{
	if ( !m_bVisible ||
		!m_pFont )
	{
		return;
	}
	
	m_pDialog->DrawBox ( m_rBoundingBox, m_sControlColor.d3dColorBoxBack, m_sControlColor.d3dColorOutline, m_bAntAlias );
	m_pEntryList->Render ( m_rBoundingBox, 0, 0, 0, m_sControlColor.d3dColorFont, -1 );
}

void CLogBox::SetSortedList ( bool bSort )
{
	m_pEntryList->SetSortedList ( bSort );
}

void CLogBox::AddText ( E_LOGTYPE eType, const SIMPLEGUI_CHAR *szText, ... )
{
	va_list arglist;
	va_start ( arglist, szText );

	time_t now;
	time ( &now );

	SIMPLEGUI_CHAR szStr [ MAX_PATH ], szFormat [ MAX_PATH ];

	SIMPLEGUI_FTIME ( szStr, 128, _UI ( "[%d.%m %H:%M:%S] {FF750000}" ), localtime ( &now ) );
	SIMPLEGUI_STRCAT ( szStr, GetModeType ( eType ) );
	SIMPLEGUI_STRCAT ( szStr, _UI ( "{00000000}: " ) );
	wvsprintf ( szFormat, szText, arglist );
	SIMPLEGUI_STRCAT ( szStr, szFormat );

	m_pEntryList->AddEntry ( new SEntryItem ( szStr ) );

	if ( m_pLogFile && m_bGenerateLog )
		m_pLogFile->Log ( GetModeType ( eType ), szText, arglist );

	va_end ( arglist );
}

void CLogBox::RemoveText ( UINT uIndex )
{
	m_pEntryList->RemoveEntry ( m_pEntryList->GetEntryByIndex ( uIndex ) );
}

const SIMPLEGUI_CHAR *CLogBox::GetText(UINT uIndex)
{
	SEntryItem *pEntry = m_pEntryList->GetEntryByIndex(uIndex);

	return pEntry ? 
		pEntry->m_sText.c_str() :
		NULL;
}

void CLogBox::ResetList ( void )
{
	m_pEntryList->ResetList ();
}

bool CLogBox::OnKeyDown ( WPARAM wParam )
{
	CScrollBarVertical *pScrollbarVer = m_pEntryList->GetScrollbar ()->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pEntryList->GetScrollbar ()->GetHorScrollbar ();

	switch ( wParam )
	{
		case VK_LEFT:
		{
			pScrollbarHor->Scroll ( -pScrollbarHor->GetStepSize () );
			return true;
		}
		case VK_UP:
		{
			pScrollbarVer->Scroll ( -m_pEntryList->GetTextSize ().cy );
			return true;
		}
		case VK_RIGHT:
		{
			pScrollbarHor->Scroll ( pScrollbarHor->GetStepSize () );
			return true;
		}
		case VK_DOWN:
		{
			pScrollbarVer->Scroll ( m_pEntryList->GetTextSize ().cy );
			return true;
		}
	}

	return false;
}

bool CLogBox::OnMouseButtonDown ( sMouseEvents e )
{
	if ( !CanHaveFocus () )
		return false;

	if ( m_pEntryList->GetScrollbar ()->OnMouseButtonDown ( e ) )
		return true;

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		// First acquire focus
		_SetFocus ();
		
		if ( m_rBoundingBox.ContainsPoint ( e.pos ) )
		{
			// Pressed while inside the control
			m_bPressed = true;
			return true;
		}
	}

	return false;
}

bool CLogBox::OnMouseButtonUp ( sMouseEvents e )
{
	if ( m_pEntryList->GetScrollbar ()->OnMouseButtonUp ( e ) )
		return true;

	if ( m_bPressed )
	{
		m_bPressed = false;
		return true;
	}

	return false;
}

bool CLogBox::OnMouseWheel ( int zDelta )
{
	CScrollBarVertical *pScrollbarVer = m_pEntryList->GetScrollbar ()->GetVerScrollbar ();

	pScrollbarVer->Scroll ( -zDelta * pScrollbarVer->GetStepSize () );
	return true;
}

bool CLogBox::OnMouseMove ( CVector pos )
{
	if ( m_pEntryList->GetScrollbar ()->OnMouseMove ( pos ) )
		return true;

	return false;
}

void CLogBox::OnClickLeave ( void )
{
	CWidget::OnClickLeave ();

	m_pEntryList->GetScrollbar ()->OnClickLeave ();
}

bool CLogBox::OnClickEvent ( void )
{
	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	return ( CWidget::OnClickEvent () ||
			 pScrollbar->OnClickEvent () );
}

void CLogBox::OnFocusIn ( void )
{
	CWidget::OnFocusIn ();

	m_pEntryList->GetScrollbar ()->OnFocusIn ();
}

void CLogBox::OnFocusOut ( void )
{
	CWidget::OnFocusOut ();

	m_pEntryList->GetScrollbar ()->OnFocusOut ();
}

void CLogBox::OnMouseEnter ( void )
{
	CWidget::OnMouseEnter ();

	 m_pEntryList->GetScrollbar ()->OnMouseEnter ();
}

void CLogBox::OnMouseLeave ( void )
{
	CWidget::OnMouseLeave ();

	m_pEntryList->GetScrollbar ()->OnMouseLeave ();
}

bool CLogBox::CanHaveFocus ( void )
{
	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	return ( CWidget::CanHaveFocus () ||
			 pScrollbar->CanHaveFocus () );
}

void CLogBox::UpdateRects ( void )
{
	CWidget::UpdateRects ();

	m_pEntryList->UpdateScrollbars ( m_rBoundingBox );
}

bool CLogBox::ContainsPoint ( CVector pos )
{
	if ( !CanHaveFocus () )
		return false;

	return m_pEntryList->ContainsRects ( m_rBoundingBox, pos );
}

void CLogBox::GenerateLogFile ( bool bGenerateLog )
{
	m_bGenerateLog = bGenerateLog;
}

const SIMPLEGUI_CHAR *CLogBox::GetModeType ( E_LOGTYPE eType )
{
	switch ( eType )
	{
		case CLogBox::LOG_NONE:
			break;
		case CLogBox::LOG_DEBUG:
			return _UI ( "DEBUG" );
		case CLogBox::LOG_INFO:
			return _UI ( "INFO" );
		case CLogBox::LOG_ERROR:
			return _UI ( "ERROR" );
		case CLogBox::LOG_WARNING:
			return _UI ( "WARNING" );
		default:
			break;
	}

	return NULL;
}