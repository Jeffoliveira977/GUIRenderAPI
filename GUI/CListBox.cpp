#include "CGUI.h"

#define TEXTBOX_TEXTSPACE 18

CListBox::CListBox ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_LISTBOX );

	m_sControlColor.d3dColorFont = D3DCOLOR_RGBA ( 0, 0, 0, 255 );
	m_sControlColor.d3dColorSelectedFont = D3DCOLOR_RGBA ( 255, 255, 255, 255 );

	m_nSelected = m_iIndex = -1;

	m_pEntryList = new CEntryList ( pDialog );

	if ( !m_pEntryList )
		MessageBox ( 0, _UI ( "CListBox::CListBox: Error for creating CEntryList" ), _UI ( "GUIAPI.asi" ), 0 );

	m_pEntryList->GetScrollbar ()->AddControl ( this );
}

CListBox::~CListBox ( void )
{
	SAFE_DELETE ( m_pEntryList );
}

void CListBox::Draw ( void )
{
	if ( !m_bVisible ||
		 !m_pFont )
	{
		return;
	}

	if ( !m_pEntryList ) 
		return;

	m_pDialog->DrawBox ( m_rBoundingBox, m_sControlColor.d3dColorBoxBack, m_sControlColor.d3dColorOutline, m_bAntAlias );
	m_pEntryList->Render ( m_rBoundingBox, m_sControlColor.d3dColorBoxSel, m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ], m_sControlColor.d3dColorSelectedFont, m_sControlColor.d3dColorFont, m_iIndex );
}

void CListBox::SetSortedList ( bool bSort )
{
	m_pEntryList->SetSortedList ( bSort );
}

void CListBox::AddItem (  SEntryItem *pEntry )
{
	m_pEntryList->AddEntry ( pEntry );
}

void CListBox::AddItem ( const SIMPLEGUI_CHAR *szText, const SIMPLEGUI_CHAR *szValue )
{
	m_pEntryList->AddEntry ( new SEntryItem ( szText ) );
}

void CListBox::RemoveItem ( SEntryItem *pEntry )
{
	m_pEntryList->RemoveEntry ( pEntry );
}

void CListBox::RemoveItem ( UINT uIndex )
{
	m_pEntryList->RemoveEntry ( m_pEntryList->GetEntryByIndex ( uIndex ) );
}

SEntryItem *CListBox::GetSelectedItem ( void )
{
	return m_pEntryList->GetSelectedEntry ();
}

void CListBox::OnClickLeave ( void )
{
	CControl::OnClickLeave ();

	m_pEntryList->GetScrollbar ()->OnClickLeave ();
}

bool CListBox::OnClickEvent ( void )
{
	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	return ( CControl::OnClickEvent () ||
			 pScrollbar->OnClickEvent () );
}

void CListBox::OnFocusIn ( void )
{
	CControl::OnFocusIn ();

	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	if ( pScrollbar )
		pScrollbar->OnFocusIn ();
}

void CListBox::OnFocusOut ( void )
{
	CControl::OnFocusOut ();

	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	if ( pScrollbar )
		pScrollbar->OnFocusOut ();
}

void CListBox::OnMouseEnter ( void )
{
	CControl::OnMouseEnter ();

	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	if ( pScrollbar )
		pScrollbar->OnMouseEnter ();
}

void CListBox::OnMouseLeave ( void )
{
	CControl::OnMouseLeave ();

	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	if ( pScrollbar )
		pScrollbar->OnMouseLeave ();
}

bool CListBox::CanHaveFocus ( void )
{
	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	return ( CControl::CanHaveFocus () ||
			 pScrollbar->CanHaveFocus () );
}

bool CListBox::OnKeyDown ( WPARAM wParam )
{
	CScrollBarVertical *pScrollbarVer = m_pEntryList->GetScrollbar ()->GetVerScrollbar ();

	switch ( wParam )
	{
		case VK_LEFT:
		case VK_UP:
		{
			if ( m_iIndex > 0 )
			{
				m_iIndex--;
				m_nSelected = m_iIndex;
				pScrollbarVer->ShowItem ( m_nSelected );

				SendEvent ( EVENT_CONTROL_SELECT, m_nSelected );
			}
			else
			{
				m_nSelected = m_iIndex = m_pEntryList->GetSize () - 1;
				pScrollbarVer->Scroll ( m_pEntryList->GetSize () - 1 );
			}
			
			m_pEntryList->SetSelectedEntryByIndex ( m_nSelected, true );
			return true;
		}

		case VK_RIGHT:
		case VK_DOWN:
		{
			if ( m_iIndex + 1 < ( int ) m_pEntryList->GetSize () )
			{
				m_iIndex++;
				m_nSelected = m_iIndex;
			}
			else
			{
				m_nSelected = m_iIndex = 0;
			}

			pScrollbarVer->ShowItem ( m_nSelected );
			SendEvent ( EVENT_CONTROL_SELECT, m_nSelected );
			m_pEntryList->SetSelectedEntryByIndex ( m_nSelected, true );

			return true;
		}
	}
	return false;
}

bool CListBox::OnMouseButtonDown ( sMouseEvents e )
{
	if ( !CanHaveFocus () )
		return false;

	// Let the scroll bar handle it first.
	if ( m_pEntryList->GetScrollbar ()->OnMouseButtonDown ( e ) )
		return true;

	// First acquire focus
	if ( e.eButton == sMouseEvents::LeftButton )
	{
		m_pParent->SetFocussedControl ( this ); 
		
		if ( m_rBoundingBox.InControlArea ( e.pos ) )
		{
			// Pressed while inside the control
			m_bPressed = true;
			return true;
		}
	}

	return false;
}

bool CListBox::OnMouseButtonUp ( sMouseEvents e )
{
	// Let the scroll bar handle it first.
	if ( m_pEntryList->GetScrollbar ()->OnMouseButtonUp ( e ) )
		return true;

	if ( m_bPressed )
	{
		m_bPressed = false;

		if ( m_rBoundingBox.InControlArea ( e.pos ) )
		{
			if ( m_iIndex != -1 )
			{
				m_nSelected = m_iIndex;
				m_pEntryList->SetSelectedEntryByIndex ( m_nSelected, true );
			}

			SendEvent ( EVENT_CONTROL_SELECT, true );
			return true;
		}
	}

	return false;
}

bool CListBox::OnMouseMove ( CPos pos )
{
	// Let the scroll bar handle it first.
	if ( m_pEntryList->GetScrollbar ()->OnMouseMove ( pos ) )
		return true;

	CScrollBarVertical *pScrollbarVer = m_pEntryList->GetScrollbar ()->GetVerScrollbar ();
	m_iIndex = -1;

	if ( GetAsyncKeyState ( VK_LBUTTON ) && !m_bHasFocus )
		return false;

	SControlRect rText = m_rBoundingBox;
	rText.pos.SetX ( rText.pos.GetX () + 4 );
	rText.size.cx -= ( pScrollbarVer->GetWidth () + 3 );
	rText.size.cy = TEXTBOX_TEXTSPACE - 2;

	for ( int i = pScrollbarVer->GetTrackPos (); i < pScrollbarVer->GetTrackPos () + pScrollbarVer->GetPageSize (); i++ )
	{
		if ( i < ( int ) m_pEntryList->GetSize () )
		{
			if ( i != pScrollbarVer->GetTrackPos () )
				rText.pos.SetY ( rText.pos.GetY () + m_pEntryList->GetTextSize ().cy );

			SEntryItem *pEntry = m_pEntryList->GetEntryByIndex ( i );

			// Check for a valid 'pEntry' pointer and if text is not NULL
			// and determine which item has been selected
			if ( pEntry &&
				 pEntry->m_sText.c_str () != NULL &&
				 rText.InControlArea ( pos ) )
			{
				m_iIndex = i;
				return true;
			}
		}
	}

	return false;
}

bool CListBox::OnMouseWheel ( int zDelta )
{
	UINT uLines;
	SystemParametersInfo ( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
	m_pEntryList->GetScrollbar ()->GetVerScrollbar ()->Scroll ( -zDelta * uLines );

	return true;
}

void CListBox::UpdateRects ( void )
{
	CControl::UpdateRects ();

	m_pEntryList->UpdateScrollbars ( m_rBoundingBox );
}

bool CListBox::ContainsRect ( CPos pos )
{
	if ( !CanHaveFocus () )
		return false;

	return m_pEntryList->ContainsRects ( m_rBoundingBox, pos );
}
