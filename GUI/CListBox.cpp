#include "CGUI.h"

#define TEXTBOX_TEXTSPACE 18

CListBox::CListBox ( CDialog *pDialog )
{
	SetControl ( pDialog, TYPE_LISTBOX );

	m_sControlColor.d3dColorFont = D3DCOLOR_RGBA ( 0, 0, 0, 255 );
	m_sControlColor.d3dColorSelectedFont = D3DCOLOR_RGBA ( 255, 255, 255, 255 );

	m_nSelected = m_iIndex = m_nOldIndex =-1;

	m_pEntryList = new CEntryList ( pDialog );

	if ( !m_pEntryList )
		MessageBox ( 0, _UI ( "CListBox::CListBox: Error for creating CEntryList" ), _UI ( "GUIAPI.asi" ), 0 );

	m_pEntryList->GetScrollbar ()->SetControl ( this );
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
	
	if ( !m_pEntryList->GetSize() )
		return;

	CScrollBarVertical *pScrollbarVer = m_pEntryList->GetScrollbar()->GetVerScrollbar ();
	CScrollBarHorizontal *pScrollbarHor = m_pEntryList->GetScrollbar ()->GetHorScrollbar ();

	SControlRect rRect = m_rText;
	rRect.m_pos.m_nX += pScrollbarHor->GetTrackPos() + 4;

	// Draw all contexts
	for ( int i = pScrollbarVer->GetTrackPos (); i < pScrollbarVer->GetTrackPos () + pScrollbarVer->GetPageSize (); i++ )
	{
		if ( i < ( int ) m_pEntryList->GetSize () )
		{
			SIZE fontSize = m_pEntryList->GetTextSize ();
			if ( i != pScrollbarVer->GetTrackPos () )
				rRect.m_pos.m_nY += fontSize.cy;

			SEntryItem *pEntry = m_pEntryList-> GetEntryByIndex ( i );

			// Check for a valid 'pEntry' pointer and if text is not NULL
			if ( pEntry &&
				 pEntry->m_sText.c_str () != NULL )
			{
				if ( m_pEntryList->IsEntrySelected ( pEntry ) ||
					 m_iIndex == i )
				{
					SControlRect rBoxSel = rRect;
					rBoxSel.m_pos.m_nX -= 17;
					rBoxSel.m_pos.m_nY += 1;
					rBoxSel.m_size.cx = rRect.m_size.cx + pScrollbarHor->GetTrackPos ();
					rBoxSel.m_size.cy = fontSize.cy - 1;
					
					D3DCOLOR d3dColor = m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ];
					if ( m_pEntryList->IsEntrySelected ( pEntry ) )
						d3dColor = m_sControlColor.d3dColorBoxSel;

					m_pDialog->DrawBox ( rBoxSel, d3dColor, 0, false );
					m_pDialog->DrawFont ( rRect, m_sControlColor.d3dColorSelectedFont, pEntry->m_sText.c_str (), 0, m_pFont );
				}
				else
				{
					m_pDialog->DrawFont ( rRect, m_sControlColor.d3dColorFont, pEntry->m_sText.c_str (), D3DFONT_COLORTABLE, m_pFont );
				}

				if ( m_bMultiSelection )
				{
					CVector pos = rRect.m_pos;
					pos.m_nX -= 16;
					pos.m_nY += 2;

					SIZE size = fontSize;
					size.cy -= 4;
					size.cx = size.cy;

					D3DCOLOR d3dColor = m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ];
					m_pDialog->DrawBox ( SControlRect ( pos, size ), m_sControlColor.d3dColorBox [ m_eState ], m_sControlColor.d3dColorOutline );

					if ( m_pEntryList->IsEntrySelected ( pEntry ) )
					{
						size.cx = size.cy -= 2;

						m_pDialog->DrawBox ( SControlRect ( pos+2, size ), m_sControlColor.d3dColorShape, 0 );
					}
				}
			}
		}
	}

	m_pEntryList->GetScrollbar()->OnDraw ();
}

void CListBox::SetMultiSelection ( bool bMultiSelection )
{
	m_bMultiSelection = bMultiSelection;
	m_pEntryList->SetMultiSelection ( bMultiSelection );
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
	CWidget::OnClickLeave ();

	m_pEntryList->GetScrollbar ()->OnClickLeave ();
}

bool CListBox::OnClickEvent ( void )
{
	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	return ( CWidget::OnClickEvent () ||
			 pScrollbar->OnClickEvent () );
}

void CListBox::OnFocusIn ( void )
{
	CWidget::OnFocusIn ();

	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	if ( pScrollbar )
		pScrollbar->OnFocusIn ();
}

void CListBox::OnFocusOut ( void )
{
	CWidget::OnFocusOut ();

	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	if ( pScrollbar )
		pScrollbar->OnFocusOut ();
}

void CListBox::OnMouseEnter ( void )
{
	CWidget::OnMouseEnter ();

	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	if ( pScrollbar )
		pScrollbar->OnMouseEnter ();
}

void CListBox::OnMouseLeave ( void )
{
	CWidget::OnMouseLeave ();
	m_iIndex = -1;

	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	if ( pScrollbar )
		pScrollbar->OnMouseLeave ();
}

bool CListBox::CanHaveFocus ( void )
{
	CScrollablePane *pScrollbar = m_pEntryList->GetScrollbar ();

	return ( CWidget::CanHaveFocus () ||
			 pScrollbar->CanHaveFocus () );
}

bool CListBox::OnKeyDown ( WPARAM wParam )
{
	CScrollBarVertical *pScrollbarVer = m_pEntryList->GetScrollbar ()->GetVerScrollbar ();

	bool bHandled = false;
	switch ( wParam )
	{
		case VK_UP:
		{
			if ( m_nSelected > 0 )
			{
				m_nSelected--;
			}
			else
			{
				m_nSelected = ( int ) m_pEntryList->GetSize () - 1;
			}

			if ( m_pEntryList->IsEntrySelectedByIndex ( m_nSelected ) )
			{
				m_pEntryList->SetSelectedEntryByIndex ( m_nSelected + 1, false );
			}
			else
			{
				m_pEntryList->SetSelectedEntryByIndex ( m_nSelected, true );
			}

			bHandled = true;
			break;
		}

		case VK_DOWN:
		{

			if ( m_nSelected + 1 < ( int ) m_pEntryList->GetSize () )
			{
				m_nSelected++;
			}
			else
			{
				m_nSelected = 0;
			}

			if ( m_pEntryList->IsEntrySelectedByIndex ( m_nSelected ) )
			{
				m_pEntryList->SetSelectedEntryByIndex ( m_nSelected - 1, false );
			}
			else
			{
				m_pEntryList->SetSelectedEntryByIndex ( m_nSelected, true );
			}

			bHandled = true;
			break;
		}
	}

	if ( bHandled )
	{
		if ( !GetAsyncKeyState ( VK_SHIFT ) && m_bMultiSelection )
			m_pEntryList->ClearSelection ();

		SendEvent ( EVENT_CONTROL_SELECT, m_nSelected );
		pScrollbarVer->ShowItem ( m_nSelected );

		SControlRect rText = m_rText;
		rText.m_pos.m_nX -= (m_bMultiSelection ? 18 : 0);
		rText.m_size.cx = rText.m_size.cx + (m_bMultiSelection ? 0 : -20);
		rText.m_size.cy = TEXTBOX_TEXTSPACE - 2;

		m_iIndex = m_pEntryList->GetItemAtPos ( rText, m_pDialog->GetMouse ()->GetPos () );
		return true;
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

bool CListBox::OnMouseButtonUp ( sMouseEvents e )
{
	// Let the scroll bar handle it first.
	if ( m_pEntryList->GetScrollbar ()->OnMouseButtonUp ( e ) )
		return true;

	if ( m_bPressed )
	{
		m_bPressed = false;

		if ( m_rBoundingBox.ContainsPoint ( e.pos ) )
		{
			if ( m_iIndex != -1 )
			{
				m_nSelected = m_iIndex;
				m_pEntryList->SetSelectedEntryByIndex ( m_nSelected, !m_pEntryList->IsEntrySelectedByIndex(m_nSelected) );
			}

			SendEvent ( EVENT_CONTROL_SELECT, true );
			return true;
		}
	}

	return false;
}

bool CListBox::OnMouseMove ( CVector pos )
{
	// Let the scroll bar handle it first.
	if ( m_pEntryList->GetScrollbar ()->OnMouseMove ( pos ) )
		return true;

	CScrollBarVertical *pScrollbarVer = m_pEntryList->GetScrollbar ()->GetVerScrollbar ();

	if ( GetAsyncKeyState ( VK_LBUTTON ) && !m_bHasFocus )
		return false;

	SControlRect rText = m_rText;
	rText.m_pos.m_nX -= (m_bMultiSelection ? 18 : 0);
	rText.m_size.cx = rText.m_size.cx + ( m_bMultiSelection ? 0 : -20 );
	rText.m_size.cy = TEXTBOX_TEXTSPACE - 2;

	m_iIndex = m_pEntryList->GetItemAtPos ( rText, pos );
	if ( m_iIndex != -1 )
		return true;

	return false;
}

bool CListBox::OnMouseWheel ( int zDelta )
{
	UINT uLines;
	SystemParametersInfo ( SPI_GETWHEELSCROLLLINES, 0, &uLines, 0 );
	m_pEntryList->GetScrollbar ()->GetVerScrollbar ()->Scroll ( -zDelta * uLines );
	m_iIndex += -zDelta * uLines;

	return true;
}

void CListBox::UpdateRects ( void )
{
	CWidget::UpdateRects ();

	m_rText = m_rBoundingBox;
	m_rText.m_pos.m_nX += (m_bMultiSelection ? 14 : 0);
	m_rText.m_size.cx -= (m_bMultiSelection ? 14 : 0);

	m_pEntryList->UpdateScrollbars ( m_rText );
}

bool CListBox::ContainsPoint ( CVector pos )
{
	if ( !CanHaveFocus () )
		return false;

	return m_pEntryList->ContainsRects ( m_rBoundingBox, pos );
}
