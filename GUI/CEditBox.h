#pragma once

#include "CGUI.h"
class CUniBuffer
{
public:
	CUniBuffer ( int nInitialSize = 1 );
	~CUniBuffer ();

	static void WINAPI      Initialize ();
	static void WINAPI      Uninitialize ();

	int                     GetBufferSize ()
	{
		return m_nBufferSize;
	}
	bool                    SetBufferSize ( int nSize );
	int                     GetTextSize ()
	{
		return SIMPLEGUI_STRLEN ( m_pwszBuffer );
	}
	const TCHAR* GetBuffer ()
	{
		return m_pwszBuffer;
	}
	const TCHAR& operator[]( int n ) const
	{
		return m_pwszBuffer [ n ];
	}
	TCHAR& operator[]( int n );
	CD3DFont* GetFontNode ()
	{
		return m_pFontNode;
	}
	void                    SetFontNode ( CD3DFont* pFontNode )
	{
		m_pFontNode = pFontNode;
	}
	void                    Clear ();

	bool                    InsertChar ( int nIndex, WCHAR wChar ); // Inserts the char at specified index. If nIndex == -1, insert to the end.
	bool                    RemoveChar ( int nIndex );  // Removes the char at specified index. If nIndex == -1, remove the last char.
	bool                    InsertString ( int nIndex, const TCHAR* pStr, int nCount = -1 );  // Inserts the first nCount characters of the string pStr at specified index.  If nCount == -1, the entire string is inserted. If nIndex == -1, insert to the end.
	bool                    SetText ( TCHAR* wszText );

	// Uniscribe
	HRESULT                 CPtoX ( int nCP, BOOL bTrail, int* pX );
	HRESULT                 XtoCP ( int nX, int* pCP, int* pnTrail );
	void                    GetPriorItemPos ( int nCP, int* pPrior );
	void                    GetNextItemPos ( int nCP, int* pPrior );

private:
	HRESULT                 Analyse ();      // Uniscribe -- Analyse() analyses the string in the buffer

	TCHAR* m_pwszBuffer;    // Buffer to hold text
	int m_nBufferSize;   // Size of the buffer allocated, in characters

						 // Uniscribe-specific
	CD3DFont* m_pFontNode;          // Font node for the font that this buffer uses
	bool m_bAnalyseRequired;            // True if the string has changed since last analysis.
	SCRIPT_STRING_ANALYSIS m_Analysis;  // Analysis for the current string

private:
	// Empty implementation of the Uniscribe API
	static HRESULT WINAPI   Dummy_ScriptApplyDigitSubstitution ( const SCRIPT_DIGITSUBSTITUTE*, SCRIPT_CONTROL*,
																 SCRIPT_STATE* )
	{
		return E_NOTIMPL;
	}
	static HRESULT WINAPI   Dummy_ScriptStringAnalyse ( HDC, const void*, int, int, int, DWORD, int, SCRIPT_CONTROL*,
														SCRIPT_STATE*, const int*, SCRIPT_TABDEF*, const BYTE*,
														SCRIPT_STRING_ANALYSIS* )
	{
		return E_NOTIMPL;
	}
	static HRESULT WINAPI   Dummy_ScriptStringCPtoX ( SCRIPT_STRING_ANALYSIS, int, BOOL, int* )
	{
		return E_NOTIMPL;
	}
	static HRESULT WINAPI   Dummy_ScriptStringXtoCP ( SCRIPT_STRING_ANALYSIS, int, int*, int* )
	{
		return E_NOTIMPL;
	}
	static HRESULT WINAPI   Dummy_ScriptStringFree ( SCRIPT_STRING_ANALYSIS* )
	{
		return E_NOTIMPL;
	}
	static const SCRIPT_LOGATTR* WINAPI Dummy_ScriptString_pLogAttr ( SCRIPT_STRING_ANALYSIS )
	{
		return NULL;
	}
	static const int* WINAPI Dummy_ScriptString_pcOutChars ( SCRIPT_STRING_ANALYSIS )
	{
		return NULL;
	}

	// Function pointers
	static                  HRESULT ( WINAPI* _ScriptApplyDigitSubstitution )( const SCRIPT_DIGITSUBSTITUTE*,
																			   SCRIPT_CONTROL*, SCRIPT_STATE* );
	static                  HRESULT ( WINAPI* _ScriptStringAnalyse )( HDC, const void*, int, int, int, DWORD, int,
																	  SCRIPT_CONTROL*, SCRIPT_STATE*, const int*,
																	  SCRIPT_TABDEF*, const BYTE*,
																	  SCRIPT_STRING_ANALYSIS* );
	static                  HRESULT ( WINAPI* _ScriptStringCPtoX )( SCRIPT_STRING_ANALYSIS, int, BOOL, int* );
	static                  HRESULT ( WINAPI* _ScriptStringXtoCP )( SCRIPT_STRING_ANALYSIS, int, int*, int* );
	static                  HRESULT ( WINAPI* _ScriptStringFree )( SCRIPT_STRING_ANALYSIS* );
	static const SCRIPT_LOGATTR* ( WINAPI*_ScriptString_pLogAttr )( SCRIPT_STRING_ANALYSIS );
	static const int* ( WINAPI*_ScriptString_pcOutChars )( SCRIPT_STRING_ANALYSIS );

	static HINSTANCE s_hDll;  // Uniscribe DLL handle

};

class CEditBox : public CControl
{

public:
	CEditBox ( CDialog *pDialog );
	~CEditBox ();

	void Draw ();

	void OnClickLeave ( void );

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );
	bool OnMouseMove ( CPos pos );

	bool OnKeyDown ( WPARAM wParam );           
	bool OnKeyCharacter ( WPARAM wParam );

	void            SetText ( SIMPLEGUI_STRING sString, bool bSelected = false );
	const TCHAR*         GetText ();
	// Returns text length in chars excluding NULL.
	int             GetTextLength ();
	
	HRESULT         GetTextCopy ( __out_ecount ( bufferCount )  TCHAR* strDest,
								  UINT bufferCount );
	void            ClearText ();
	void UpdateRects ( void );
	bool ContainsRect ( CPos pos );
private:
	void            PlaceCaret ( int nCP );
	void            DeleteSelectionText ();
	void            CopyToClipboard ();
	void            PasteFromClipboard ();

	CTimer m_timer;
	bool m_bMouseDrag;
	CUniBuffer m_Buffer;     // Buffer to hold text
	int m_nBorder;      // Border of the window
	int m_nSpacing;     // Spacing between the text and the edge of border
	SControlRect m_rText;       // Bounding rectangle for the text
	double m_dfBlink;      // Caret blink time in milliseconds
	double m_dfLastBlink;  // Last timestamp of caret blink
	bool m_bCaretOn;     // Flag to indicate whether caret is currently visible
	int m_nCaret;       // Caret position, in characters
	bool m_bInsertMode;  // If true, control is in insert mode. Else, overwrite mode.
	int m_nSelStart;    // Starting position of the selection. The caret marks the end.
	int m_nFirstVisible;// First visible character in the edit control
};