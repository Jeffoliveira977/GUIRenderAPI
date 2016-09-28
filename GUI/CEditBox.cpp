#include "CGUI.h"

#define MinState( a, b, c, d ) a < b ? c : d 

//-----------------------------------------------------------------------------
// CUniBuffer class for the edit control
//-----------------------------------------------------------------------------

// Static member initialization
HINSTANCE               CUniBuffer::s_hDll = NULL;
HRESULT ( WINAPI*CUniBuffer::_ScriptApplyDigitSubstitution )( const SCRIPT_DIGITSUBSTITUTE*, SCRIPT_CONTROL*,
															  SCRIPT_STATE* ) = Dummy_ScriptApplyDigitSubstitution;
HRESULT ( WINAPI*CUniBuffer::_ScriptStringAnalyse )( HDC, const void*, int, int, int, DWORD, int, SCRIPT_CONTROL*,
													 SCRIPT_STATE*, const int*, SCRIPT_TABDEF*, const BYTE*,
													 SCRIPT_STRING_ANALYSIS* ) = Dummy_ScriptStringAnalyse;
HRESULT ( WINAPI*CUniBuffer::_ScriptStringCPtoX )( SCRIPT_STRING_ANALYSIS, int, BOOL, int* ) = Dummy_ScriptStringCPtoX;
HRESULT ( WINAPI*CUniBuffer::_ScriptStringXtoCP )( SCRIPT_STRING_ANALYSIS, int, int*, int* ) = Dummy_ScriptStringXtoCP;
HRESULT ( WINAPI*CUniBuffer::_ScriptStringFree )( SCRIPT_STRING_ANALYSIS* ) = Dummy_ScriptStringFree;
const SCRIPT_LOGATTR*   ( WINAPI*CUniBuffer::_ScriptString_pLogAttr )( SCRIPT_STRING_ANALYSIS ) =
Dummy_ScriptString_pLogAttr;
const int*              ( WINAPI*CUniBuffer::_ScriptString_pcOutChars )( SCRIPT_STRING_ANALYSIS ) =
Dummy_ScriptString_pcOutChars;


//--------------------------------------------------------------------------------------
void CUniBuffer::Initialize ()
{
	if ( s_hDll ) // Only need to do once
		return;

	s_hDll = LoadLibrary ( UNISCRIBE_DLLNAME );
	if ( s_hDll )
	{
		FARPROC Temp;
		GETPROCADDRESS ( s_hDll, ScriptApplyDigitSubstitution, Temp );
		GETPROCADDRESS ( s_hDll, ScriptStringAnalyse, Temp );
		GETPROCADDRESS ( s_hDll, ScriptStringCPtoX, Temp );
		GETPROCADDRESS ( s_hDll, ScriptStringXtoCP, Temp );
		GETPROCADDRESS ( s_hDll, ScriptStringFree, Temp );
		GETPROCADDRESS ( s_hDll, ScriptString_pLogAttr, Temp );
		GETPROCADDRESS ( s_hDll, ScriptString_pcOutChars, Temp );
	}
}


//--------------------------------------------------------------------------------------
void CUniBuffer::Uninitialize ()
{
	if ( s_hDll )
	{
		PLACEHOLDERPROC ( ScriptApplyDigitSubstitution );
		PLACEHOLDERPROC ( ScriptStringAnalyse );
		PLACEHOLDERPROC ( ScriptStringCPtoX );
		PLACEHOLDERPROC ( ScriptStringXtoCP );
		PLACEHOLDERPROC ( ScriptStringFree );
		PLACEHOLDERPROC ( ScriptString_pLogAttr );
		PLACEHOLDERPROC ( ScriptString_pcOutChars );

		FreeLibrary ( s_hDll );
		s_hDll = NULL;
	}
}

//--------------------------------------------------------------------------------------
bool CUniBuffer::SetBufferSize ( int nNewSize )
{
	// If the current size is already the maximum allowed,
	// we can't possibly allocate more.
	if ( m_nBufferSize == 0xFFFF )
		return false;

	int nAllocateSize = ( nNewSize == -1 || nNewSize < m_nBufferSize * 2 ) ? ( m_nBufferSize ? m_nBufferSize *
																			   2 : 256 ) : nNewSize * 2;

	// Cap the buffer size at the maximum allowed.
	if ( nAllocateSize > 0xFFFF )
	{
		nAllocateSize = 0xFFFF;
	}

	WCHAR* pTempBuffer = new WCHAR [ nAllocateSize ];
	if ( !pTempBuffer )
		return false;

	ZeroMemory ( pTempBuffer, sizeof ( WCHAR ) * nAllocateSize );

	if ( m_pwszBuffer )
	{
		CopyMemory ( pTempBuffer, m_pwszBuffer, m_nBufferSize * sizeof ( WCHAR ) );
		delete [] m_pwszBuffer;
	}

	m_pwszBuffer = pTempBuffer;
	m_nBufferSize = nAllocateSize;
	return true;
}


//--------------------------------------------------------------------------------------
// Uniscribe -- Analyse() analyses the string in the buffer
//--------------------------------------------------------------------------------------
HRESULT CUniBuffer::Analyse ()
{
	if ( m_Analysis )
		_ScriptStringFree ( &m_Analysis );

	SCRIPT_CONTROL ScriptControl; // For uniscribe
	SCRIPT_STATE ScriptState;   // For uniscribe
	ZeroMemory ( &ScriptControl, sizeof ( ScriptControl ) );
	ZeroMemory ( &ScriptState, sizeof ( ScriptState ) );
	_ScriptApplyDigitSubstitution ( NULL, &ScriptControl, &ScriptState );

	if ( !m_pFontNode )
		return E_FAIL;

	HDC hDC = m_pFontNode->GetHDC ();
	HRESULT hr = _ScriptStringAnalyse ( hDC,
										m_pwszBuffer,
										lstrlenW ( m_pwszBuffer ) + 1,  // NULL is also analyzed.
										lstrlenW ( m_pwszBuffer ) * 3 / 2 + 16,
										-1,
										SSA_BREAK | SSA_GLYPHS | SSA_FALLBACK | SSA_LINK,
										0,
										&ScriptControl,
										&ScriptState,
										NULL,
										NULL,
										NULL,
										&m_Analysis );
	if ( SUCCEEDED ( hr ) )
	{
		m_bAnalyseRequired = false;  // Analysis is up-to-date
	}
	return hr;
}


//--------------------------------------------------------------------------------------
CUniBuffer::CUniBuffer ( int nInitialSize )
{
	CUniBuffer::Initialize ();  // ensure static vars are properly init'ed first

	m_nBufferSize = 0;
	m_pwszBuffer = NULL;
	m_bAnalyseRequired = true;
	m_Analysis = NULL;
	m_pFontNode = NULL;

	if ( nInitialSize > 0 )
	{
		SetBufferSize ( nInitialSize );
	}
}


//--------------------------------------------------------------------------------------
CUniBuffer::~CUniBuffer ()
{
	delete [] m_pwszBuffer;
	if ( m_Analysis )
	{
		_ScriptStringFree ( &m_Analysis );
	}
}


//--------------------------------------------------------------------------------------
WCHAR& CUniBuffer::operator[]( int n )  // No param checking
{
	// This version of operator[] is called only
	// if we are asking for write access, so
	// re-analysis is required.
	m_bAnalyseRequired = true;
	return m_pwszBuffer [ n ];
}


//--------------------------------------------------------------------------------------
void CUniBuffer::Clear ()
{
	*m_pwszBuffer = L'\0';
	m_bAnalyseRequired = true;
}


//--------------------------------------------------------------------------------------
// Inserts the char at specified index.
// If nIndex == -1, insert to the end.
//--------------------------------------------------------------------------------------
bool CUniBuffer::InsertChar ( int nIndex, WCHAR wChar )
{
	assert ( nIndex >= 0 );

	if ( nIndex < 0 || nIndex > lstrlenW ( m_pwszBuffer ) )
		return false;  // invalid index

					   // Check for maximum length allowed
	if ( GetTextSize () + 1 >= DXUT_MAX_EDITBOXLENGTH )
		return false;

	if ( lstrlenW ( m_pwszBuffer ) + 1 >= m_nBufferSize )
	{
		if ( !SetBufferSize ( -1 ) )
			return false;  // out of memory
	}

	assert ( m_nBufferSize >= 2 );

	// Shift the characters after the index, start by copying the null terminator
	WCHAR* dest = m_pwszBuffer + lstrlenW ( m_pwszBuffer ) + 1;
	WCHAR* stop = m_pwszBuffer + nIndex;
	WCHAR* src = dest - 1;

	while ( dest > stop )
	{
		*dest-- = *src--;
	}

	// Set new character
	m_pwszBuffer [ nIndex ] = wChar;
	m_bAnalyseRequired = true;

	return true;
}


//--------------------------------------------------------------------------------------
// Removes the char at specified index.
// If nIndex == -1, remove the last char.
//--------------------------------------------------------------------------------------
bool CUniBuffer::RemoveChar ( int nIndex )
{
	if ( !lstrlenW ( m_pwszBuffer ) ||
		 nIndex < 0 ||
		 nIndex >= lstrlenW ( m_pwszBuffer ) )
		return false;  // Invalid index

	MoveMemory ( m_pwszBuffer + nIndex, m_pwszBuffer + nIndex + 1, sizeof ( WCHAR ) * ( lstrlenW ( m_pwszBuffer ) - nIndex ) );
	m_bAnalyseRequired = true;
	return true;
}


//--------------------------------------------------------------------------------------
// Inserts the first nCount characters of the string pStr at specified index.
// If nCount == -1, the entire string is inserted.
// If nIndex == -1, insert to the end.
//--------------------------------------------------------------------------------------
bool CUniBuffer::InsertString ( int nIndex, const WCHAR* pStr, int nCount )
{
	if ( nIndex < 0 )
		return false;

	if ( nIndex > lstrlenW ( m_pwszBuffer ) )
		return false;  // invalid index

	if ( -1 == nCount )
	{
		nCount = lstrlenW ( pStr );
	}

	// Check for maximum length allowed
	if ( GetTextSize () + nCount >= DXUT_MAX_EDITBOXLENGTH )
		return false;

	if ( lstrlenW ( m_pwszBuffer ) + nCount >= m_nBufferSize )
	{
		if ( !SetBufferSize ( lstrlenW ( m_pwszBuffer ) + nCount + 1 ) )
			return false;  // out of memory
	}

	MoveMemory ( m_pwszBuffer + nIndex + nCount, m_pwszBuffer + nIndex, sizeof ( WCHAR ) * ( lstrlenW ( m_pwszBuffer ) - nIndex + 1 ) );
	CopyMemory ( m_pwszBuffer + nIndex, pStr, nCount * sizeof ( WCHAR ) );
	m_bAnalyseRequired = true;

	return true;
}


//--------------------------------------------------------------------------------------
bool CUniBuffer::SetText ( TCHAR* wszText )
{
	assert ( wszText != NULL );

	int nRequired = int ( wcslen ( wszText ) + 1 );

	// Check for maximum length allowed
	if ( nRequired >= DXUT_MAX_EDITBOXLENGTH )
		return false;

	while ( GetBufferSize () < nRequired )
	{
		if ( !SetBufferSize ( -1 ) )
			break;
	}

	// Check again in case out of memory occurred inside while loop.
	if ( GetBufferSize () >= nRequired )
	{
		wcscpy_s ( m_pwszBuffer, GetBufferSize (), wszText );
		m_bAnalyseRequired = true;
		return true;
	}
	else
		return false;
}


//--------------------------------------------------------------------------------------
HRESULT CUniBuffer::CPtoX ( int nCP, BOOL bTrail, int* pX )
{
	assert ( pX );
	*pX = 0;  // Default

	HRESULT hr = S_OK;
	if ( m_bAnalyseRequired )
	{
		hr = Analyse ();
	}

	if ( SUCCEEDED ( hr ) )
	{
		hr = _ScriptStringCPtoX ( m_Analysis, nCP, bTrail, pX );
	}
	return hr;
}


//--------------------------------------------------------------------------------------
HRESULT CUniBuffer::XtoCP ( int nX, int* pCP, int* pnTrail )
{
	assert ( pCP && pnTrail );
	*pCP = 0; 
	*pnTrail = FALSE;  // Default

	HRESULT hr = S_OK;
	if ( m_bAnalyseRequired )
	{
		hr = Analyse ();
	}

	if ( SUCCEEDED ( hr ) )
	{
		hr = _ScriptStringXtoCP ( m_Analysis, nX, pCP, pnTrail );
	}

	// If the coordinate falls outside the text region, we
	// can get character positions that don't exist.  We must
	// filter them here and convert them to those that do exist.
	if ( *pCP == -1 && *pnTrail == TRUE )
	{
		*pCP = 0; *pnTrail = FALSE;
	}
	else if ( *pCP > lstrlenW ( m_pwszBuffer ) && *pnTrail == FALSE )
	{
		*pCP = lstrlenW ( m_pwszBuffer ); *pnTrail = TRUE;
	}

	return hr;
}


//--------------------------------------------------------------------------------------
void CUniBuffer::GetPriorItemPos ( int nCP, int* pPrior )
{
	*pPrior = nCP;  // Default is the char itself

	if ( m_bAnalyseRequired )
	{
		if ( FAILED ( Analyse () ) )
			return;
	}

	const SCRIPT_LOGATTR* pLogAttr = _ScriptString_pLogAttr ( m_Analysis );
	if ( !pLogAttr )
		return;

	if ( !_ScriptString_pcOutChars ( m_Analysis ) )
		return;

	int nInitial = *_ScriptString_pcOutChars ( m_Analysis );
	if ( nCP - 1 < nInitial )
	{
		nInitial = nCP - 1;
	}
	
	for ( int i = nInitial; i > 0; --i )
	{
		if ( pLogAttr [ i ].fWordStop ||       // Either the fWordStop flag is set
			 ( !pLogAttr [ i ].fWhiteSpace &&  // Or the previous char is whitespace but this isn't.
			 pLogAttr [ i - 1 ].fWhiteSpace ) )
		{
			*pPrior = i;
			return;
		}
	}

	// We have reached index 0.  0 is always a break point, so simply return it.
	*pPrior = 0;
}


//--------------------------------------------------------------------------------------
void CUniBuffer::GetNextItemPos ( int nCP, int* pPrior )
{
	*pPrior = nCP;  // Default is the char itself

	HRESULT hr = S_OK;
	if ( m_bAnalyseRequired )
		hr = Analyse ();

	if ( FAILED ( hr ) )
		return;

	const SCRIPT_LOGATTR* pLogAttr = _ScriptString_pLogAttr ( m_Analysis );
	if ( !pLogAttr )
		return;

	if ( !_ScriptString_pcOutChars ( m_Analysis ) )
		return;

	int nInitial = *_ScriptString_pcOutChars ( m_Analysis );
	if ( nCP + 1 < nInitial )
	{
		nInitial = nCP + 1;
	}

	int i = nInitial;
	int limit = *_ScriptString_pcOutChars ( m_Analysis );
	while ( limit > 0 && i < limit - 1 )
	{
		if ( pLogAttr [ i ].fWordStop )      // Either the fWordStop flag is set
		{
			*pPrior = i;
			return;
		}
		else if ( pLogAttr [ i ].fWhiteSpace &&  // Or this whitespace but the next char isn't.
				  !pLogAttr [ i + 1 ].fWhiteSpace )
		{
			*pPrior = i + 1;  // The next char is a word stop
			return;
		}

		++i;
		limit = *_ScriptString_pcOutChars ( m_Analysis );
	}

	// We have reached the end. It's always a word stop, so simply return it.
	*pPrior = *_ScriptString_pcOutChars ( m_Analysis ) - 1;
}

//--------------------------------------------------------------------------------------
CEditBox::CEditBox ( CDialog *pDialog )
{
	SetControl ( pDialog, EControlType::TYPE_EDITBOX );

	m_bCaretOn = m_bInsertMode = true;
	m_nFirstVisible = m_nCaret = m_nSelStart = 0;

	m_timer.Start ( 0.8 );
	m_sControlColor.d3dColorBox [ SControlColor::STATE_PRESSED ] = D3DCOLOR_RGBA ( 200, 200, 200, 255 );
}

//--------------------------------------------------------------------------------------
CEditBox::~CEditBox ( void )
{
	ClearText ();
}

//--------------------------------------------------------------------------------------
const TCHAR* CEditBox::GetText ( void )
{
	return m_Buffer.GetBuffer ();
}

//--------------------------------------------------------------------------------------
int CEditBox::GetTextLength ( void )
{
	return m_Buffer.GetTextSize ();
}

//--------------------------------------------------------------------------------------
// PlaceCaret: Set the caret to a character position, and adjust the scrolling if
//             necessary.
//--------------------------------------------------------------------------------------
void CEditBox::PlaceCaret ( int nCP )
{
	assert ( nCP >= 0 && nCP <= m_Buffer.GetTextSize () );
	m_nCaret = nCP;

	// Obtain the X offset of the character.
	int nX1st, nX, nX2;
	m_Buffer.CPtoX ( m_nFirstVisible, FALSE, &nX1st );  // 1st visible char
	m_Buffer.CPtoX ( nCP, FALSE, &nX );  // LEAD
	
	// If nCP is the NULL terminator, get the leading edge instead of trailing.
	if ( nCP == m_Buffer.GetTextSize () )
	{
		nX2 = nX;
	}
	else
	{
		m_Buffer.CPtoX ( nCP, TRUE, &nX2 );  // TRAIL
	}

	// If the left edge of the char is smaller than the left edge of the 1st visible char,
	// we need to scroll left until this char is visible.
	if ( nX < nX1st )
	{
		// Simply make the first visible character the char at the new caret position.
		m_nFirstVisible = nCP;
	}
	else
	{	// If the right of the character is bigger than the offset of the control's
		// right edge, we need to scroll right to this character.
		if ( nX2 > nX1st + m_rText.size.cx )
		{
			// Compute the X of the new left-most pixel
			int nXNewLeft = nX2 - m_rText.size.cx;

			// Compute the char position of this character
			int nCPNew1st, nNewTrail;
			m_Buffer.XtoCP ( nXNewLeft, &nCPNew1st, &nNewTrail );

			// If this coordinate is not on a character border,
			// start from the next character so that the caret
			// position does not fall outside the text rectangle.
			int nXNew1st;
			m_Buffer.CPtoX ( nCPNew1st, FALSE, &nXNew1st );
			if ( nXNew1st < nXNewLeft )
			{
				++nCPNew1st;
			}

			m_nFirstVisible = nCPNew1st;
		}
	}
}

void CEditBox::CopyToClipboard ()
{
	// Copy the selection text to the clipboard
	if ( m_nCaret != m_nSelStart && OpenClipboard ( NULL ) )
	{
		EmptyClipboard ();

		HGLOBAL hBlock = GlobalAlloc ( GMEM_MOVEABLE, sizeof ( TCHAR ) * ( m_Buffer.GetTextSize () + 1 ) );
		if ( hBlock )
		{
			TCHAR* pwszText = ( TCHAR* ) GlobalLock ( hBlock );
			if ( pwszText )
			{
				int nFirst = __min ( m_nCaret, m_nSelStart );
				int nLast = __max ( m_nCaret, m_nSelStart );

				if ( nLast - nFirst > 0 )
				{
					CopyMemory ( pwszText, m_Buffer.GetBuffer () + nFirst, ( nLast - nFirst ) * sizeof ( TCHAR ) );
				}
				pwszText [ nLast - nFirst ] = L'\0';  // Terminate it
				GlobalUnlock ( hBlock );
			}
			SetClipboardData ( CF_UNICODETEXT, hBlock );
		}
		CloseClipboard ();

		// We must not free the object until CloseClipboard is called.
		if ( hBlock )
		{
			GlobalFree ( hBlock );
		}
	}
}

void CEditBox::PasteFromClipboard ()
{
	DeleteSelectionText ();

	if ( OpenClipboard ( NULL ) )
	{
		HANDLE handle = GetClipboardData ( CF_UNICODETEXT );
		if ( handle )
		{
			// Convert the ANSI string to Unicode, then
			// insert to our buffer.
			TCHAR* pwszText = ( TCHAR* ) GlobalLock ( handle );
			if ( pwszText )
			{
				// Copy all characters up to null.
				if ( m_Buffer.InsertString ( m_nCaret, pwszText ) )
				{
					PlaceCaret ( m_nCaret + SIMPLEGUI_STRLEN ( pwszText ) );
				}
				m_nSelStart = m_nCaret;
				GlobalUnlock ( handle );
			}
		}
		CloseClipboard ();
	}
}

//--------------------------------------------------------------------------------------
void CEditBox::ClearText ()
{
	PlaceCaret ( 0 );
	m_Buffer.Clear ();
	m_nFirstVisible = 0;
	m_nSelStart = 0;
}


//--------------------------------------------------------------------------------------
void CEditBox::SetText ( SIMPLEGUI_STRING sString, bool bSelected )
{
	if ( m_pFont )
		m_pFont->RemoveColorTableFromString ( sString );

	m_Buffer.SetText ( ( TCHAR* ) sString.c_str () );
	m_nFirstVisible = 0;

	// Move the caret to the end of the text
	PlaceCaret ( m_Buffer.GetTextSize () );
	m_nSelStart = bSelected ? 0 : m_nCaret;
}

//--------------------------------------------------------------------------------------
HRESULT CEditBox::GetTextCopy ( __out_ecount ( bufferCount ) TCHAR* strDest,
									UINT bufferCount )
{
	assert ( strDest );
	SIMPLEGUI_STRCPY_S ( strDest, bufferCount, m_Buffer.GetBuffer () );

	return S_OK;
}


//--------------------------------------------------------------------------------------
void CEditBox::DeleteSelectionText ()
{
	int nFirst = __min ( m_nCaret, m_nSelStart );
	int nLast = __max ( m_nCaret, m_nSelStart );

	// Update caret and selection
	PlaceCaret ( nFirst );
	m_nSelStart = m_nCaret;

	// Remove the characters
	for ( int i = nFirst; i < nLast; ++i )
		m_Buffer.RemoveChar ( nFirst );
}

void CEditBox::Draw ()
{
	CControl::Draw ();

	if ( m_bHasFocus )
	{
		m_eState = SControlColor::STATE_PRESSED;
	}

	// Left and right X cordinates of the selection region
	int nSelStartX = 0, nCaretX = 0;

	// Compute the X coordinates of the first visible character.
	int nXFirst;
	m_Buffer.CPtoX ( m_nFirstVisible, FALSE, &nXFirst );

	// Compute the X coordinates of the selection rectangle
	m_Buffer.CPtoX ( m_nCaret, FALSE, &nCaretX );
	if ( m_nCaret != m_nSelStart )
	{
		m_Buffer.CPtoX ( m_nSelStart, FALSE, &nSelStartX );
	}
	else
	{
		nSelStartX = nCaretX;
	}

	m_pDialog->DrawBox ( m_rBoundingBox, m_sControlColor.d3dColorBox [ m_eState ], m_sControlColor.d3dColorOutline );

	m_Buffer.SetFontNode ( m_pFont );
	PlaceCaret ( m_nCaret );  // Call PlaceCaret now that we have the font info (node),

	SIMPLEGUI_STRING str = m_Buffer.GetBuffer () + m_nFirstVisible;

	// Render the selection rectangle
	if ( m_nCaret != m_nSelStart && m_bHasFocus )
	{
		RECT rcSelection;
		int nSelLeftX = nCaretX;
		int	nSelRightX = nSelStartX;

		// Swap if left is bigger than right
		if ( nSelLeftX > nSelRightX )
		{
			int nTemp = nSelLeftX;
			nSelLeftX = nSelRightX;
			nSelRightX = nTemp;
		}

		SetRect ( &rcSelection, nSelLeftX, m_rText.GetRect ().top, nSelRightX, m_rText.GetRect ().bottom );
		OffsetRect ( &rcSelection, m_rText.GetRect ().left - nXFirst, 0 );
		IntersectRect ( &rcSelection, &m_rText.GetRect (), &rcSelection );

		SControlRect rSelecion ( rcSelection );
		m_pDialog->DrawBox ( rSelecion, D3DCOLOR_RGBA ( 40, 40, 40, 255 ), 0, false );

		// Insert color key to string
		str.insert ( MinState ( m_nSelStart, m_nCaret, max ( m_nSelStart - m_nFirstVisible, 0 ), min ( m_nSelStart - m_nFirstVisible, str.size () ) ),
					 MinState ( m_nSelStart, m_nCaret, _UI ( "{FFFFFFFF}" ), _UI ( "{00000000}" ) ) ); // Insert at begin selection

		str.insert ( MinState ( m_nSelStart, m_nCaret, min ( m_nCaret + 10 - m_nFirstVisible, str.size () ), m_nCaret - m_nFirstVisible ),
					 MinState ( m_nSelStart, m_nCaret, _UI ( "{00000000}" ), _UI ( "{FFFFFFFF}" ) ) ); // Insert at end selection
	}

	SIZE size;
	m_pFont->GetTextExtent ( GetText (), &size );
	m_pDialog->DrawFont ( SControlRect ( m_rText.pos.GetX (), m_rText.pos.GetY () + m_rText.size.cy / 2 - ( size.cy / 2 ) ),
						  m_sControlColor.d3dColorSelectedFont, str.c_str (), D3DFONT_COLORTABLE, m_pFont );

	if ( !m_timer.Running () )
	{
		m_bCaretOn = !m_bCaretOn;
		m_timer.Start ( 0.6f );
	}

	// Render the caret if this control has the focus
	if ( m_bHasFocus && m_bCaretOn )
	{
		int nX = m_rText.pos.GetX () - nXFirst + nCaretX;

		CD3DRender *pRender = m_pDialog->GetRenderer ();

		// If we are in overwrite mode, adjust the caret rectangle
		// to fill the entire character.
		str = m_Buffer.GetBuffer ();
		if ( !m_bInsertMode &&
			 m_nCaret == m_nSelStart &&
			 m_nCaret < str.size () )
		{
			TCHAR szStr [ 2 ] = { str [ m_nCaret ] , 0 };
			GetTextExtentPoint32 ( m_pFont->GetHDC (), szStr, 1, &size );
			pRender->D3DBox ( nX, m_rText.pos.GetY (), size.cx - 1, m_rText.size.cy, 0, D3DCOLOR_RGBA ( 20, 20, 20, 200 ), 0, false );
		}
		else
		{
			pRender->D3DLine ( nX - 1, m_rText.pos.GetY (), nX - 1, m_rText.pos.GetY () + m_rText.size.cy, 0xFF000000, true );
		}
	}
}

void CEditBox::OnClickLeave ( void )
{
	CControl::OnClickLeave ();
	m_bMouseDrag  = false;
}

bool CEditBox::OnMouseButtonDown ( sMouseEvents e )
{
	if ( !CanHaveFocus () )
		return false;

	if ( e.eButton == sMouseEvents::LeftButton )
	{
		// Determine the character corresponding to the coordinates.
		int nCP, nTrail, nX1st;
		m_Buffer.CPtoX ( m_nFirstVisible, FALSE, &nX1st );  // X offset of the 1st visible char
		if ( SUCCEEDED ( m_Buffer.XtoCP ( e.pos.GetX () - m_rText.pos.GetX () + nX1st, &nCP, &nTrail ) ) )
		{
			// Cap at the NULL character.
			if ( nTrail && nCP < m_Buffer.GetTextSize () )
			{
				PlaceCaret ( nCP + 1 );
			}
			else
			{
				PlaceCaret ( nCP );
			}

			m_nSelStart = m_nCaret;
		}

		if ( m_rBoundingBox.InControlArea ( e.pos ) )
		{
			// Pressed while inside the control
			m_bPressed = m_bMouseDrag = true;

			if ( m_pParent  )
			{
				m_pParent->SetFocussedControl ( this );
			}
			return true;
		}
	}

	return false;
}

bool CEditBox::OnMouseButtonUp ( sMouseEvents e )
{
	m_bMouseDrag = false;
	return false;
}

bool CEditBox::OnMouseMove ( CPos pos )
{
	if ( m_bMouseDrag )
	{
		// Determine the character corresponding to the coordinates.
		int nCP, nTrail, nX1st;
		m_Buffer.CPtoX ( m_nFirstVisible, FALSE, &nX1st );  // X offset of the 1st visible char
		if ( SUCCEEDED ( m_Buffer.XtoCP ( pos.GetX () - m_rText.pos.GetX () + nX1st, &nCP, &nTrail ) ) )
		{
			// Cap at the NULL character.
			if ( nTrail && nCP < m_Buffer.GetTextSize () )
			{
				PlaceCaret ( nCP + 1 );
			}
			else
			{
				PlaceCaret ( nCP );
			}
			return true;
		}
	}

	return false;
}

bool CEditBox::OnKeyDown ( WPARAM wParam )
{
	if ( !CanHaveFocus () )
		return false;

	bool bHandled = false;

	switch ( wParam )
	{
		case VK_TAB:
			// We don't process Tab in case keyboard input is enabled and the user
			// wishes to Tab to other controls.
			break;

		case VK_HOME:
		{
			PlaceCaret ( 0 );

			// Shift is not down. Update selection
			// start along with the caret.
			if ( GetKeyState ( VK_SHIFT ) >= 0 )
			{
				m_nSelStart = m_nCaret;
			}

			bHandled = true;
			break;
		}

		case VK_END:
		{
			PlaceCaret ( m_Buffer.GetTextSize () );

			// Shift is not down. Update selection
			// start along with the caret.
			if ( GetKeyState ( VK_SHIFT ) >= 0 )
			{
				m_nSelStart = m_nCaret;
			}

			bHandled = true;
			break;
		}

		case VK_INSERT:
		{
			if ( GetKeyState ( VK_CONTROL ) < 0 )
			{
				// Control Insert. Copy to clipboard
				CopyToClipboard ();
			}
			else if ( GetKeyState ( VK_SHIFT ) < 0 )
			{
				// Shift Insert. Paste from clipboard
				PasteFromClipboard ();
			}
			else
			{
				// Toggle caret insert mode
				m_bInsertMode = !m_bInsertMode;
			}
			break;
		}

		case VK_DELETE:
		{// Check if there is a text selection.
			if ( m_nCaret != m_nSelStart )
			{
				DeleteSelectionText ();
				SendEvent ( EVENT_CONTROL_CHANGE, true );
			}
			else
			{
				// Deleting one character
				if ( m_Buffer.RemoveChar ( m_nCaret ) )
				{
					SendEvent ( EVENT_CONTROL_CHANGE, true );
				}
			}

			bHandled = true;
			break;
		}

		case VK_LEFT:
		{
			if ( GetKeyState ( VK_CONTROL ) < 0 )
			{
				// Control is down. Move the caret to a new item
				// instead of a character.
				m_Buffer.GetPriorItemPos ( m_nCaret, &m_nCaret );
				PlaceCaret ( m_nCaret );
			}
			else if ( m_nCaret > 0 )
			{
				PlaceCaret ( m_nCaret - 1 );
			}

			// Shift is not down. Update selection
			// start along with the caret.
			if ( GetKeyState ( VK_SHIFT ) >= 0 )
			{
				m_nSelStart = m_nCaret;
			}

			bHandled = true;
			break;
		}

		case VK_RIGHT:
		{
			if ( GetKeyState ( VK_CONTROL ) < 0 )
			{
				// Control is down. Move the caret to a new item
				// instead of a character.
				m_Buffer.GetNextItemPos ( m_nCaret, &m_nCaret );
				PlaceCaret ( m_nCaret );
			}
			else if ( m_nCaret < m_Buffer.GetTextSize () )
			{
				PlaceCaret ( m_nCaret + 1 );
			}

			// Shift is not down. Update selection
			// start along with the caret.
			if ( GetKeyState ( VK_SHIFT ) >= 0 )
			{
				m_nSelStart = m_nCaret;
			}

			bHandled = true;
			break;
		}

		case VK_UP:
		case VK_DOWN:
		{
			// Trap up and down arrows so that the dialog
			// does not switch focus to another control.
			bHandled = true;
			break;
		}

		default:
			bHandled = wParam != VK_ESCAPE;  // Let the application handle Esc.
	}

	return bHandled;
}

bool CEditBox::OnKeyCharacter ( WPARAM wParam )
{
	if ( !CanHaveFocus () )
		return false;

	bool bHandled = false;
	switch ( ( WCHAR ) wParam )
	{
		// Backspace
		case VK_BACK:
		{
			// If there's a selection, treat this
			// like a delete key.
			if ( m_nCaret != m_nSelStart )
			{
				DeleteSelectionText ();
				SendEvent ( EVENT_CONTROL_CHANGE, true );
			}
			else if ( m_nCaret > 0 )
			{
				// Move the caret, then delete the char.
				PlaceCaret ( m_nCaret - 1 );
				m_nSelStart = m_nCaret;
				m_Buffer.RemoveChar ( m_nCaret );
				SendEvent ( EVENT_CONTROL_CHANGE, true );
			}

			bHandled = true;
			break;
		}

		case 24:        // Ctrl-X Cut
		case VK_CANCEL: // Ctrl-C Copy
		{
			CopyToClipboard ();

			// If the key is Ctrl-X, delete the selection too.
			if ( ( WCHAR ) wParam == 24 )
			{
				DeleteSelectionText ();
				SendEvent ( EVENT_CONTROL_CHANGE, true );
			}
			bHandled = true;
			break;
		}

		// Ctrl-V Paste
		case 22:
		{
			PasteFromClipboard ();
			SendEvent ( EVENT_CONTROL_CHANGE, true );

			bHandled = true;
			break;
		}

		// Ctrl-A Select All
		case 1:
		{
			if ( m_nSelStart == m_nCaret )
			{
				m_nSelStart = 0;
				PlaceCaret ( m_Buffer.GetTextSize () );
			}

			bHandled = true;
			break;
		}

		case VK_RETURN:
		{
			// Invoke the callback when the user presses Enter.
			SendEvent ( EVENT_CONTROL_CHANGE, true );
			bHandled = true;
			break;
		}
			// Junk characters we don't want in the string
		case 26:  // Ctrl Z
		case 2:   // Ctrl B
		case 14:  // Ctrl N
		case 19:  // Ctrl S
		case 4:   // Ctrl D
		case 6:   // Ctrl F
		case 7:   // Ctrl G
		case 10:  // Ctrl J
		case 11:  // Ctrl K
		case 12:  // Ctrl L
		case 17:  // Ctrl Q
		case 23:  // Ctrl W
		case 5:   // Ctrl E
		case 18:  // Ctrl R
		case 20:  // Ctrl T
		case 25:  // Ctrl Y
		case 21:  // Ctrl U
		case 9:   // Ctrl I
		case 15:  // Ctrl O
		case 16:  // Ctrl P
		case 27:  // Ctrl [
		case 29:  // Ctrl ]
		case 28:  // Ctrl \ 
			break;

		default:
		{
			// If there's a selection and the user
			// starts to type, the selection should
			// be deleted.
			if ( m_nCaret != m_nSelStart )
			{
				DeleteSelectionText ();
			}

			// If we are in overwrite mode and there is already
			// a char at the caret's position, simply replace it.
			// Otherwise, we insert the char as normal.
			if ( !m_bInsertMode && m_nCaret < m_Buffer.GetTextSize () )
			{
				m_Buffer [ m_nCaret ] = ( WCHAR ) wParam;
				PlaceCaret ( m_nCaret + 1 );
				m_nSelStart = m_nCaret;
			}
			else
			{
				// Insert the char
				if ( m_Buffer.InsertChar ( m_nCaret, ( WCHAR ) wParam ) )
				{
					PlaceCaret ( m_nCaret + 1 );
					m_nSelStart = m_nCaret;
				}
			}

			SendEvent ( EVENT_CONTROL_CHANGE, true );
			bHandled = true;
		}
	}

	return bHandled;
}

void CEditBox::UpdateRects ( void )
{
	CControl::UpdateRects ();

	m_rText = m_rBoundingBox;
	m_rText.pos.SetX ( m_rText.pos.GetX () + 4 );
	m_rText.pos.SetY ( m_rText.pos.GetY () + 2 );
	m_rText.size.cx -= 6;
	m_rText.size.cy -= 3;
}

bool CEditBox::ContainsRect ( CPos pos )
{
	if ( !CanHaveFocus () )
		return false;

	return ( m_rBoundingBox.InControlArea ( pos ) || 
			 m_rText.InControlArea ( pos ) );
}
