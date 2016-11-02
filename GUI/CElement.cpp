#include "CGUI.h"

const SIZE GetControlMinSize ( CWidget::EControlType eType )
{
	SIZE size;

	switch ( eType )
	{
		case CWidget::TYPE_DROPDOWN:
		case CWidget::TYPE_EDITBOX:
		case CWidget::TYPE_BUTTON:
			size.cx = 80;
			size.cy = 20;
			break;
		case CWidget::TYPE_RADIOBUTTON:
		case CWidget::TYPE_CHECKBOX:
			size.cx = 80;
			size.cy = 15;
			break;
		case CWidget::TYPE_IMAGE:
		case CWidget::TYPE_BOX:
			size.cx = 80;
			size.cy = 80;
			break;
		case CWidget::TYPE_TEXTBOX:
		case CWidget::TYPE_LISTBOX:
		case CWidget::TYPE_LISTVIEW:
			size.cy = 100;
			break;
		case CWidget::TYPE_SCROLLBARHORIZONTAL:
		case CWidget::TYPE_PROGRESSBARHORIZONTAL:
		case CWidget::TYPE_TRACKBARHORIZONTAL:
			size.cx = 80;
			size.cy = 18;
			break;
		case CWidget::TYPE_SCROLLBARVERTICAL:
		case CWidget::TYPE_TRACKBARVERTICAL:
		case CWidget::TYPE_PROGRESSBARVERTICAL:
			size.cx = 18;
			size.cy = 80;
			break;
		case CWidget::TYPE_LABEL:
			size.cx = 20;
			break;
		case CWidget::TYPE_TABPANEL:
			size.cx = 200;
			size.cy = 200;
			break;
		case CWidget::TYPE_WINDOW:
			size.cx = 100;
			size.cy = 100;
			break;
		default:
			ZeroMemory ( &size, sizeof ( SIZE ) );
			break;
	}

	return size;
}

void CWidget::SetControl ( CDialog *pDialog, EControlType eType )
{
	if( !pDialog )
	{
		MessageBox ( 0, _UI ( "CControl::SetControl - Invalid 'pDialog' parameter" ), 0, 0 );
		return;
	}
	// Default state
	m_eState														= SControlColor::STATE_NORMAL;

	// Set default colors
	m_sControlColor.d3dColorSelectedFont							= D3DCOLOR_RGBA ( 0, 0, 0, 255 );
	m_sControlColor.d3dColorFont									= D3DCOLOR_RGBA ( 255, 255, 255, 255 );
	m_sControlColor.d3dColorShape									= D3DCOLOR_RGBA ( 0, 0, 0, 255 );
	m_sControlColor.d3dColorOutline									= D3DCOLOR_RGBA ( 0, 0, 0, 255 );
	m_sControlColor.d3dColorBoxBack									= D3DCOLOR_RGBA ( 40, 40, 40, 255 );
	m_sControlColor.d3dColorBoxSel									= D3DCOLOR_RGBA ( 100, 100, 100, 255 );

	m_sControlColor.d3dColorBox [ SControlColor::STATE_NORMAL ]		= D3DCOLOR_RGBA ( 80, 80, 80, 255 );
	m_sControlColor.d3dColorBox [ SControlColor::STATE_MOUSE_OVER ] = D3DCOLOR_XRGB ( 100, 100, 100, 255 );
	m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ]	= D3DCOLOR_XRGB ( 60, 60, 60, 255 );
	m_sControlColor.d3dColorBox [ SControlColor::STATE_DISABLED ]	= D3DCOLOR_XRGB ( 220, 220, 220, 255 );

	m_bEnabled														= true;
	m_bAntAlias														= true;
	m_bEnabledStateColor											= true;
	m_bVisible														= true;
	m_bUpdatedFont													= false;

	m_eType															= eType;
	m_pParent														= NULL;


	m_pDialog = pDialog;
	SetFont(															pDialog->GetFont ());
	m_pTexture														= NULL;

	m_minSize														= GetControlMinSize ( eType );

	m_eRelativeX = NO_RELATIVE;
	m_eRelativeY = NO_RELATIVE;

	m_sFontInfo.m_size.cx = 0;
	m_sFontInfo.m_size.cy = 0;
	//m_sFontInfo.m_bBold = false;
	//ZeroMemory(&m_sFontInfo.m_size, sizeof(SIZE));

	ZeroMemory ( &m_size, sizeof ( SIZE ) );
	ZeroMemory ( &m_oldParentSize, sizeof ( SIZE ) );

	ZeroMemory ( &m_pos, sizeof ( CVector ) );
	ZeroMemory ( &m_nonUpdatedPos, sizeof ( CVector ) );

	ZeroMemory ( &m_rContentBox, sizeof ( SControlRect ) );
}

void CWidget::_SetFocus ( void )
{
	if ( m_pParent )
	{
		if ( m_pParent->GetType () == TYPE_WINDOW )
		{
			reinterpret_cast<CWindow*>( m_pParent )->SetFocussedWidget ( this );
		}
		else if ( m_pParent->GetType () == TYPE_TABPANEL )
		{
			reinterpret_cast<CTabPanel*>( m_pParent )->SetFocussedControl ( this );
		}
	}
	else
	{
		m_pDialog->SetFocussedWidget ( this );
	}
}

void CWidget::_ClearFocus ( void )
{
	if ( m_pParent )
	{
		if ( m_pParent->GetType () == TYPE_WINDOW )
		{
			reinterpret_cast< CWindow* >( m_pParent )->ClearControlFocus ();
		}
		else if ( m_pParent->GetType () == TYPE_TABPANEL )
		{
			reinterpret_cast< CTabPanel* >( m_pParent )->ClearControlFocus ();
		}
	}
	else
	{
		m_pDialog->ClearFocussedWidget ();
	}
}

void CWidget::SetColor ( SControlColor sColor )
{
	m_sControlColor = sColor;
}

SControlColor CWidget::GetColor ( SControlColor sColor )
{
	return m_sControlColor;
}

void CWidget::SetParent ( CWidget *pParent )
{
	if ( pParent && ( pParent->GetType () == EControlType::TYPE_TABPANEL || 
		 pParent->GetType () == EControlType::TYPE_WINDOW ) )
	{
		m_pParent = pParent;
	}
}

CWidget *CWidget::GetParent ( void )
{
	return m_pParent;
}

void CWidget::SetAction ( tAction pAction )
{
	m_pAction = pAction;
}

tAction CWidget::GetAction ( void )
{
	return m_pAction;
}

void CWidget::SetPos (Pos pos )
{
	SetPosX ( pos.m_nX );
	SetPosY ( pos.m_nY );
}

void CWidget::SetPos ( int nX, int nY )
{
	SetPos ( Pos ( nX, nY ) );
}

void CWidget::SetPosX(int nX)
{
	m_rBoundingBox.m_pos.m_nX	= nX;
	m_pos.m_nX					= nX;
	m_nonUpdatedPos.m_nX		= nX;
}

void CWidget::SetPosY(int nY)
{
	m_rBoundingBox.m_pos.m_nY	= nY;
	m_pos.m_nY					= nY;
	m_nonUpdatedPos.m_nY		= nY;
}

Pos *CWidget::GetPos ( void )
{
	return &m_pos;
}

Pos *CWidget::GetUpdatedPos ( void )
{
	return &m_rBoundingBox.m_pos;
}

int CWidget::GetWidth(void)
{
	return m_rBoundingBox.m_size.cx;
}

int CWidget::GetHeight(void)
{
	return m_rBoundingBox.m_size.cy;
}

void CWidget::SetWidth ( int iWidth )
{
	/*if(m_pParent )
	if ( m_rBoundingBox.m_size.cx >= m_pParent->GetWidth() )
		{
		iWidth =m_pParent->GetWidth () ;
		}*/
	m_realSize.cx = m_rBoundingBox.m_size.cx = m_size.cx = max ( m_minSize.cx, iWidth );
}

void CWidget::SetHeight ( int iHeight )
{
	SIZE size;
	if ( m_pFont )
	{
		m_pFont->GetTextExtent ( GetText (), &size );
	}

	m_realSize.cy = m_rBoundingBox.m_size.cy = m_size.cy = max ( iHeight, max ( m_minSize.cy, size.cy ) );
	UpdateRects ();
}

void CWidget::SetSize ( SIZE size )
{
	SetSize ( size.cx, size.cy );
}

void CWidget::SetSize ( int iWidth, int iHeight )
{
	SetWidth ( iWidth );
	SetHeight ( iHeight );
	
}

void CWidget::SetMinSize ( int nMin, int nMax )
{
	SIZE size = GetControlMinSize ( m_eType );
	if ( size.cx > nMin )
		m_minSize.cx = nMin;

	if ( size.cy > nMax )
		m_minSize.cy = nMax;
}

void CWidget::SetMinSize ( SIZE size )
{
	SetMinSize ( size.cx, size.cy );
}

SIZE CWidget::GetMinSize ( void )
{
	return m_minSize;
}

bool CWidget::IsSizingX ( void )
{
	if ( m_rBoundingBox.m_size.cx != m_rContentBox.m_size.cx )
	{
		m_rContentBox.m_size.cx = m_rBoundingBox.m_size.cx;
		return true;
	}

	return false;
}

bool CWidget::IsSizingY ( void )
{
	if ( m_rBoundingBox.m_size.cy != m_rContentBox.m_size.cy )
	{
		m_rContentBox.m_size.cy = m_rBoundingBox.m_size.cy;
		return true;
	}

	return false;
}

bool CWidget::IsSizing ( void )
{
	return ( IsSizingX () ||
			 IsSizingY () );
}

bool CWidget::IsMovingX ( void )
{
	if ( m_pos.m_nY != m_rContentBox.m_pos.m_nY)
	{
		m_rContentBox.m_pos.m_nX =m_pos.m_nY;
		return true;
	}

	return false;
}

bool CWidget::IsMovingY ( void )
{
	if ( m_rBoundingBox.m_pos.m_nY != m_rContentBox.m_pos.m_nY)
	{
		m_rContentBox.m_pos.m_nY = m_rBoundingBox.m_pos.m_nY;
		return true;
	}

	return false;
}

bool CWidget::IsMoving ( void )
{
	return ( IsMovingX () ||
			 IsMovingY () );
}

SIZE CWidget::GetSize ( void )
{
	return m_rBoundingBox.m_size;
}

SIZE CWidget::GetRealSize ( void )
{
	return m_realSize;
}

bool CWidget::CanHaveFocus ( void )
{
	return ( m_bVisible && m_bEnabled );
}

bool CWidget::HasFocus ( void )
{
	return m_bHasFocus;
}

void CWidget::SetText(SIMPLEGUI_STRING sText, bool)
{
	m_sText = sText;
	//if (m_sText != sText)
	//{
	//	if (!m_sText.empty() || m_realSize.cx == 0)
	//	{
	//		SIZE size;
	//		if(m_pFont)
	//		m_pFont->GetTextExtent(sText.c_str(), &size);
	//		m_sFontInfo.m_size.cx = size.cx;

	//		// Update the control size, if the text was changed
	//		if (m_oldTextSize.cx != size.cx)
	//		{
	//			SetWidth(m_rBoundingBox.m_size.cx);
	//			m_oldTextSize.cx = size.cx;
	//		}
	//	}

	//	m_sText = sText;
	//}
}

const SIMPLEGUI_CHAR *CWidget::GetText ( void )
{
	return m_sText.c_str ();
}

void CWidget::Draw ( void )
{
	if ( !m_bEnabledStateColor )
		m_eState = SControlColor::STATE_NORMAL;
	else if ( !m_bEnabled )
		m_eState = SControlColor::STATE_DISABLED;
	else if ( m_bPressed  )
		m_eState = SControlColor::STATE_PRESSED;
	else if ( m_bMouseOver  )
		m_eState = SControlColor::STATE_MOUSE_OVER;
	else
		m_eState = SControlColor::STATE_NORMAL;
}

bool CWidget::MsgProc ( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return false;
}

bool CWidget::HandleKeyboard ( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return false;
}

bool CWidget::HandleMouse ( UINT uMsg, CVector pos, WPARAM wParam, LPARAM lParam )
{
	return false;
}

void CWidget::SetFont(const SIMPLEGUI_CHAR *szFontName, DWORD dwHeight, bool bBold)
{
	if ( !m_pDialog /*|| szFontName == NULL*/ )
	{
		MessageBox ( 0, _UI ( "CControl::SetFont - Invalid 'm_pDialog' " ), 0, 0 );
		return;
	}

	SAFE_DELETE(m_pFont);

	m_pDialog->LoadFont(szFontName, dwHeight, bBold, &m_pFont);

	if (!m_pFont)
	{
		MessageBox ( 0, _UI ( "CControl::SetFont - error for loading font" ), 0, 0 );
		return;
	}

	////SIMPLEGUI_STRCPY(m_sFontInfo.m_szFontName, szFontName);
	m_sFontInfo.m_bBold = bBold;

	if (m_sFontInfo.m_dwHeight != dwHeight)
	{
		SIZE size;
		m_pFont->GetTextExtent(_UI("Y"), &size);
		m_sFontInfo.m_size.cy = size.cy;

		SetHeight(m_size.cy);
		m_sFontInfo.m_dwHeight = dwHeight;
	}
}

SFontInfo CWidget::GetFontInfo(void)
{
	return SFontInfo();
}

void CWidget::SetFont(SFontInfo sInfo)
{
	//SetFont(sInfo.m_szFontName, sInfo.m_dwHeight, sInfo.m_bBold);
}

void CWidget::SetFont ( CD3DFont *pFont )
{
	SAFE_DELETE ( m_pFont );

	m_pFont = pFont;

	if ( !pFont )
	{
		MessageBox ( 0, _UI ( "CControl::SetFont - Invalid 'pFont' parameter" ), 0, 0 );
		return;
	}

	//m_sFontInfo.m_bBold = bBold;

	/*if (m_sFontInfo.m_dwHeight != dwHeight)
	{*/
	if (m_size.cy)
	{
		SIZE size;
		m_pFont->GetTextExtent(_UI("Y"), &size);
		m_sFontInfo.m_size.cy = 123;

		SetHeight(m_size.cy);
	}
		//m_sFontInfo.m_dwHeight = dwHeight;
	//}
}

CD3DFont *CWidget::GetFont ( void )
{
	return m_pFont;
}

void CWidget::SetStateColor ( D3DCOLOR d3dColor, SControlColor::SControlState eState )
{
	m_sControlColor.d3dColorBox [ eState ] = d3dColor;
}

CWidget::eRelative CWidget::GetRelativeX ( void )
{
	return m_eRelativeX;
}

CWidget::eRelative CWidget::GetRelativeY ( void )
{
	return m_eRelativeY;
}

void CWidget::SetRelativeX ( eRelative eRelativeType )
{
	m_eRelativeX = eRelativeType;
}

void CWidget::SetRelativeY ( eRelative eRelativeType )
{
	m_eRelativeY = eRelativeType;
}

void CWidget::SetEnabled ( bool bEnabled )
{
	m_bEnabled = bEnabled;
}

bool CWidget::IsEnabled ( void )
{
	return m_bEnabled;
}

void CWidget::SetVisible ( bool bVisible )
{
	m_bVisible = bVisible;
}

bool CWidget::IsVisible ( void )
{
	return m_bVisible;
}

void CWidget::SetEnabledStateColor ( bool bEnable )
{
	m_bEnabledStateColor = bEnable;
}

void CWidget::SetAntAlias ( bool bAntAlias )
{
	m_bAntAlias = bAntAlias;
}

CWidget::EControlType CWidget::GetType ( void )
{
	return m_eType;
}

void CWidget::OnFocusIn ( void )
{
	m_bHasFocus = true;
}

void CWidget::OnFocusOut ( void )
{
	m_bHasFocus = false;
}

bool CWidget::OnMouseOver ( void )
{
	return m_bMouseOver;
}

void CWidget::OnMouseEnter ( void )
{
	m_bMouseOver = true;
}

void CWidget::OnMouseLeave ( void )
{
	m_bMouseOver = false;
}

void CWidget::OnClickEnter ( void )
{
	m_bPressed = true;
}

void CWidget::OnClickLeave ( void )
{
	m_bPressed = false;
}

bool CWidget::OnClickEvent ( void )
{
	return m_bPressed;
}

bool CWidget::OnMouseButtonDown ( sMouseEvents e )
{
	return false;
}

bool CWidget::OnMouseButtonUp ( sMouseEvents e )
{
	return false;
}

bool CWidget::OnMouseMove ( CVector pos )
{
	return false;
}
bool CWidget::OnMouseWheel ( int nDelta )
{
	return false;
}

bool CWidget::OnKeyDown ( WPARAM wParam )
{
	return false;
}

bool CWidget::OnKeyUp ( WPARAM wParam )
{
	return false;
}

bool CWidget::OnKeyCharacter ( WPARAM wParam )
{
	return false;
}

bool CWidget::InjectKeyboard ( sKeyEvents e )
{
	if ( e.uMsg == WM_KEYDOWN )
	{
		if ( OnKeyDown ( e.wKey ) )
			return true;
	}
	else if ( e.uMsg == WM_KEYUP )
	{
		if ( OnKeyUp ( e.wKey ) )
			return true;
	}
	else if ( e.uMsg == WM_CHAR )
	{
		if ( OnKeyCharacter ( e.wKey ) )
			return true;
	}

	return false;
}

bool CWidget::InjectMouse ( sMouseEvents e )
{
	if ( e.eMouseMessages == sMouseEvents::ButtonDown )
	{
		if ( OnMouseButtonDown ( e ) )
			return true;
	}
	else if ( e.eMouseMessages == sMouseEvents::ButtonUp )
	{
		if ( OnMouseButtonUp ( e ) )
			return true;
	}
	else if ( e.eMouseMessages == sMouseEvents::MouseMove )
	{
		if ( OnMouseMove ( e.pos ) )
			return true;
	}
	else if ( e.eMouseMessages == sMouseEvents::MouseWheel )
	{
		if ( OnMouseWheel ( e.nDelta ) )
			return true;
	}

	return false;
}

bool CWidget::SendEvent ( eEVentControl event, int params )
{
	if ( !m_pAction )
		return false;

	m_pAction ( this, event, params );
	return true;
}

void CWidget::EnterScissorRect ( void )
{
	m_rScissor = m_rBoundingBox;

	sCissor.SetScissor ( m_pDialog->GetDevice (), m_rScissor.GetRect () );
}

void CWidget::EnterScissorRect ( SControlRect rRect )
{
	m_rScissor = m_rBoundingBox;

	if ( m_pParent )
	{
		int nDragOffSet;
		if ( rRect.m_pos.m_nY + rRect.m_size.cy < m_rScissor.m_pos.m_nY + m_rScissor.m_size.cy - 1 )
		{
			m_rScissor.m_size.cy = rRect.m_pos.m_nY + rRect.m_size.cy - m_rScissor.m_pos.m_nY - 1;
		}

		if ( m_rScissor.m_pos.m_nY < rRect.m_pos.m_nY + 1 )
		{
			nDragOffSet = rRect.m_pos.m_nY + 1 - m_rScissor.m_pos.m_nY;
			m_rScissor.m_pos.m_nY += nDragOffSet;
			m_rScissor.m_size.cy = m_rScissor.m_size.cy - nDragOffSet;
		}

		if ( rRect.m_pos.m_nX + rRect.m_size.cx < m_rScissor.m_pos.m_nX + m_rScissor.m_size.cx + 1 )
		{
			m_rScissor.m_size.cx = rRect.m_pos.m_nX + rRect.m_size.cx - m_rScissor.m_pos.m_nX - 1;
		}

		if ( m_rScissor.m_pos.m_nX < rRect.m_pos.m_nX + 1 )
		{
			nDragOffSet = rRect.m_pos.m_nX - m_rScissor.m_pos.m_nX + 1;
			m_rScissor.m_pos.m_nX += nDragOffSet;
			m_rScissor.m_size.cx = m_rScissor.m_size.cx - nDragOffSet;
		}
	}

	sCissor.SetScissor ( m_pDialog->GetDevice (), m_rScissor.GetRect () );
}

void CWidget::LeaveScissorRect ( void )
{
	sCissor.RestoreScissor ();
}

SControlRect CWidget::GetRect ( void )
{
	return m_rBoundingBox;
}

void CWidget::LinkPos ( Pos pos )
{
	Pos newPos = m_nonUpdatedPos + pos;

	if ( m_pParent &&
		 m_eType != TYPE_WINDOW )
	{
		SIZE parentRealSize = m_pParent->GetRealSize ();
		SIZE parentSize = m_pParent->GetSize ();
		Pos  parentPos = *m_pParent->GetPos ();

		//if ( m_realSize.cx >= parentRealSize.cx )
		//{
		//	SetWidth ( parentRealSize.cx );
		//}

		int nTitle = 0;
		if ( m_pParent->GetType () == TYPE_WINDOW )
		{
			nTitle = static_cast< CWindow* >( m_pParent )->GetTitleBarHeight ();
		}
		else if ( m_pParent->GetType () == TYPE_TABPANEL )
		{
			nTitle = static_cast< CTabPanel* >( m_pParent )->GetTabSizeY ();
		}

		//int nParentHeight = parentRealSize.cy - nTitle;
		//if ( m_realSize.cy >= nParentHeight )
		//{
		//	//SetHeight ( nParentHeight );
		//	//m_rBoundingBox.m_size.cy = m_size.cy = nParentHeight;
		//}

		int nScrollSize = 0;

		if ( m_eRelativeX == eRelative::RELATIVE_POS )
		{
			CScrollablePane *pScrollbar = m_pParent->GetScrollbar ();

			nScrollSize = pScrollbar && pScrollbar->IsVerScrollbarNeeded () ?
				pScrollbar->GetVerScrollbar ()->GetWidth () : 0;

			newPos.m_nX += parentSize.cx - nScrollSize - parentRealSize.cx;
			if ( newPos.m_nX < parentPos.m_nX ||
				 m_pos.m_nX < 0 )
			{
				newPos.m_nX = pos.m_nX;
			}
			else if ( m_oldParentSize.cx != parentSize.cx )
			{
				if ( m_oldParentSize.cx )
					m_pos.m_nX = newPos.m_nX - parentPos.m_nX;

				m_oldParentSize.cx = parentSize.cx;
			}
		}

		int nControlAreaX = parentPos.m_nX + m_nonUpdatedPos.m_nX + m_size.cx;
		int nWindowAreaX = parentPos.m_nX + parentRealSize.cx;

		if ( m_eRelativeX != NO_RELATIVE &&
			 m_oldPos.m_nX != m_nonUpdatedPos.m_nX &&
			 nControlAreaX > nWindowAreaX )
		{
			m_nonUpdatedPos.m_nX -= nControlAreaX - nWindowAreaX;
			m_pos.m_nX = m_nonUpdatedPos.m_nX;
			m_oldPos.m_nX = m_nonUpdatedPos.m_nX;
		}

		if ( m_eRelativeY == eRelative::RELATIVE_POS )
		{
			CScrollablePane *pScrollbar = m_pParent->GetScrollbar ();

			nScrollSize = pScrollbar && pScrollbar->IsHorScrollbarNeeded () ?
				pScrollbar->GetHorScrollbar ()->GetHeight () :
				0;

			newPos.m_nY += parentSize.cy - nScrollSize - parentRealSize.cy;

			if ( newPos.m_nY < parentPos.m_nY + nTitle ||
				 m_pos.m_nY < nTitle )
			{
				newPos.m_nY = pos.m_nY;
			}
			else if ( m_oldParentSize.cy != parentSize.cy )
			{
				if ( m_oldParentSize.cy )
					m_pos.m_nY = newPos.m_nY - parentPos.m_nY;

				m_oldParentSize.cy = parentSize.cy;
			}
		}

		int nControlAreaY = parentPos.m_nY + m_nonUpdatedPos.m_nY + m_size.cy;
		int nWindowAreaY = parentPos.m_nY- nTitle + parentRealSize.cy;

		if ( m_eRelativeY != NO_RELATIVE &&
			 m_oldPos.m_nY != m_nonUpdatedPos.m_nY &&
			 nControlAreaY > nWindowAreaY )
		{
			m_nonUpdatedPos.m_nY -= nControlAreaY - nWindowAreaY;
			m_pos.m_nY = m_nonUpdatedPos.m_nY;
			m_oldPos.m_nY = m_nonUpdatedPos.m_nY;
		}
	}

	m_rBoundingBox.m_pos = newPos;
	UpdateRects ();
}

void CWidget::UpdateRects ( void )
{
	m_rBoundingBox.m_size = m_size;

	if ( m_pParent )
	{
		SIZE size = m_pParent->GetSize ();

		if ( m_realSize.cx >= size.cx )
		{
			m_rBoundingBox.m_size.cx = m_size.cx = size.cx;
			//SetWidth ( parentRealSize.cx );
		}

		int nTitle = 0;
		if ( m_pParent->GetType () == TYPE_WINDOW )
		{
			nTitle = static_cast< CWindow* >( m_pParent )->GetTitleBarHeight ();
		}
		else if ( m_pParent->GetType () == TYPE_TABPANEL )
		{
			nTitle = static_cast< CTabPanel* >( m_pParent )->GetTabSizeY ();
		}

		int nParentHeight = size.cy - nTitle;
		if ( m_realSize.cy >= nParentHeight )
		{
			m_rBoundingBox.m_size.cy = m_size.cy = nParentHeight;
		}

		if ( m_eRelativeX == eRelative::RELATIVE_SIZE )
		{
			int asd = 0;
			CScrollablePane *scrollbar = m_pParent->GetType () == TYPE_WINDOW ? static_cast< CWindow* >( m_pParent )->GetScrollbar () : NULL;
			//if ( scrollbar )
			asd = scrollbar&& scrollbar->IsVerScrollbarNeeded () ? scrollbar->GetVerScrollbar ()->GetWidth () : 0;

			//size.cx = m_realSize.cx - asd;
			m_rBoundingBox.m_size.cx = max ( m_minSize.cx, m_size.cx + ( m_pParent->GetSize ().cx - m_pParent->GetRealSize ().cx ) );
		}

		if ( m_eRelativeY == eRelative::RELATIVE_SIZE )
		{
			int asd = 0;
			CScrollablePane *scrollbar = m_pParent->GetType () == TYPE_WINDOW ? static_cast< CWindow* >( m_pParent )->GetScrollbar () : NULL;
			/*if ( scrollbar )
			asd =  scrollbar->IsHorScrollbarNeeded () ? scrollbar->GetHorScrollbar ()->GetHeight () : 0;*/
			/*int nControlAreaY = m_pParent->GetPos ()->GetY () + m_nonUpdatedPos.GetY () + m_size.cy;
			int nWindowAreaY = m_pParent->GetPos ()->GetY () + m_pParent->GetRealSize ().cy;
			if ( nControlAreaY > nWindowAreaY )*/
			//size.cy = m_realSize.cy - asd;
			m_rBoundingBox.m_size.cy = max ( m_minSize.cy, m_size.cy + ( m_pParent->GetSize ().cy - m_pParent->GetRealSize ().cy ) );
		}
	}
}

bool CWidget::ContainsPoint ( Pos pos )
{
	if ( !CanHaveFocus () )
		return false;

	return m_rBoundingBox.ContainsPoint ( pos );
}