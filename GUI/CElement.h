#pragma once

#include "CPos.h"

class CDialog;

typedef CVector Pos;
typedef CVector Size;

enum eEVentControl
{
	EVENT_CONTROL_KEY_DOWN,
	EVENT_CONTROL_CLICKED,
	EVENT_CONTROL_GOT_FOCUS,
	EVENT_CONTROL_LOST_FOCUS,
	EVENT_CONTROL_CHANGE,
	EVENT_CONTROL_SELECT,
	EVENT_CONTROL_MOUSE_OVER,
	EVENT_CONTROL_MOUSE_OUT,
	EVENT_CONTROL_END,
	EVENT_CONTROL_DBCLICK
};

typedef void ( __cdecl *tAction )( CWidget*, eEVentControl, int );

struct SControlRect
{
	Pos m_pos;
	SIZE m_size;

	SControlRect ( void )
	{}

	SControlRect (Pos pos )
	{
		m_pos = pos;
	}

	SControlRect(int iX, int iY)
	{
		m_pos.m_nX = iX;
		m_pos.m_nY = iY;
	}

	SControlRect ( SIZE size )
	{
		m_size = size;
	}

	SControlRect (Pos pos, SIZE size )
	{
		m_pos = pos;
		m_size = size;
	}

	SControlRect ( int nX, int nY, int nWidth, int nHeight )
	{
		m_pos.m_nX = nX;
		m_pos.m_nY = nY;
		m_size.cx = nWidth;
		m_size.cy = nHeight;
	}

	SControlRect(RECT rect)
	{
		m_pos.m_nX = rect.left;
		m_pos.m_nY = rect.top;
		m_size.cx = rect.right - rect.left;
		m_size.cy = rect.bottom - rect.top;
	}

	SControlRect GetIntersection ( const SControlRect& rect ) const
	{
		if ( m_pos.m_nX + m_size.cx > rect.m_pos.m_nX  &&
			 m_pos.m_nX < rect.m_pos.m_nX + rect.m_size.cx  &&
			 m_pos.m_nY + m_size.cy > rect.m_pos.m_nY  &&
			 m_pos.m_nY < rect.m_pos.m_nY + rect.m_size.cy )
		{
			SControlRect ret;

			// fill in ret with the intersection
			ret.m_pos.m_nX = ( m_pos.m_nX > rect.m_pos.m_nX ) ? m_pos.m_nX : rect.m_pos.m_nX;
			ret.m_size.cx = ( m_pos.m_nX + m_size.cx < rect.m_pos.m_nX + rect.m_size.cx ) ? m_size.cx : rect.m_size.cx;
			ret.m_pos.m_nY = ( m_pos.m_nY > rect.m_pos.m_nY ) ? m_pos.m_nY : rect.m_pos.m_nY;
			ret.m_size.cy = ( m_pos.m_nY + m_size.cy < rect.m_pos.m_nY + rect.m_size.cy ) ? m_size.cy : rect.m_size.cy;

			return ret;
		}
		else
		{
			return SControlRect ();
		}
	}

	BOOL ContainsPoint ( Pos pos )
	{
		return ( pos.m_nX >= m_pos.m_nX &&
				 pos.m_nX <= m_pos.m_nX + m_size.cx &&
				 pos.m_nY >= m_pos.m_nY &&
				 pos.m_nY <= m_pos.m_nY + m_size.cy );
	}

	RECT GetRect ( void )
	{
		RECT rect;
		SetRect ( &rect, m_pos.m_nX, m_pos.m_nY, m_pos.m_nX + m_size.cx, m_pos.m_nY + m_size.cy);
		return rect;
	}


};

struct SControlColor
{
	enum SControlState
	{
		STATE_NORMAL,
		STATE_MOUSE_OVER,
		STATE_PRESSED,
		STATE_DISABLED
	};

	D3DCOLOR d3dColorWindowTitle;
	D3DCOLOR d3dCOlorWindowBack;

	D3DCOLOR d3dColorBoxSel;
	D3DCOLOR d3dColorBoxBack;
	D3DCOLOR d3dColorBox [ 4 ];

	D3DCOLOR d3dColorOutline;
	D3DCOLOR d3dColorShape;

	D3DCOLOR d3dColorSelectedFont;
	D3DCOLOR d3dColorFont;
};

struct sMouseEvents
{
	enum eMouseButton
	{
		LeftButton,
		RightButton,
		MiddleButton
	};

	enum eMouseMessages
	{
		ButtonDown,
		ButtonUp,
		MouseMove,
		MouseWheel
	};

	INT				nDelta;
	CVector			pos;

	eMouseMessages	eMouseMessages;
	eMouseButton	eButton;

	/*sMouseEvents(){}
	sMouseEvents ( eMouseButton eButton, CPos m_pos, INT nDelta )
	{
		this->eButton = eButton;
		this->m_pos = m_pos;
		this->nDelta = nDelta;
	}*/
};

struct sKeyEvents
{
	WPARAM wKey;
	UINT uMsg;

	/*sKeyEvents () {}
	sKeyEvents ( UINT uMsg, WPARAM wKey )
	{
		this->uMsg = uMsg;
		this->wKey = wKey;
	}*/
};

struct sControlEvents
{
	sKeyEvents keyEvent;
	sMouseEvents mouseEvent;

};
struct SFontInfo
{
	//SIMPLEGUI_CHAR  m_szFontName[128];
	DWORD			m_dwHeight;
	bool			m_bBold;
	SIZE			m_size;
};

class CWidget
{
public:

	enum eRelative
	{
		RELATIVE_POS,
		RELATIVE_SIZE,
		NO_RELATIVE
	};

	enum EControlType
	{
		TYPE_BUTTON,
		TYPE_CHECKBOX,
		TYPE_RADIOBUTTON,
		TYPE_DROPDOWN,
		TYPE_EDITBOX,
		TYPE_IMAGE,
		TYPE_BOX,
		TYPE_LISTBOX,
		TYPE_SCROLLBARHORIZONTAL,
		TYPE_SCROLLBARVERTICAL,
		TYPE_TRACKBARHORIZONTAL,
		TYPE_TRACKBARVERTICAL,
		TYPE_PROGRESSBARHORIZONTAL,
		TYPE_PROGRESSBARVERTICAL,
		TYPE_LABEL,
		TYPE_TEXTBOX,
		TYPE_LISTVIEW,
		TYPE_TABPANEL,
		TYPE_WINDOW
	};

	void EnterScissorRect ( void );
	void EnterScissorRect ( SControlRect rRect);
	void LeaveScissorRect ( void );

	SControlRect GetRect ( void );

	void SetColor ( SControlColor sColor );
	SControlColor GetColor ( SControlColor sColor );

	void SetControl ( CDialog *pGui, EControlType eType );

	void SetParent ( CWidget *pParent );
	CWidget *GetParent ( void );

	void _SetFocus ( void );
	void _ClearFocus ( void );

	virtual SIZE GetRealSize ( void );

	void SetAction ( tAction pAction );
	tAction GetAction ( void );

	void SetPos ( int nX, int nY );
	void SetPos ( Pos pos );

	void SetPosX ( int nX );
	void SetPosY ( int nY );

	CVector *GetPos ( void );
	CVector *GetUpdatedPos ( void );

	virtual void SetWidth ( int iWidth );
	int GetWidth ( void );

	virtual void SetHeight ( int iHeight );
	int GetHeight ( void );

	virtual void SetSize ( SIZE size );
	virtual void SetSize ( int iWidth, int iHeight );

	void SetMinSize ( int nMin, int nMax );
	void SetMinSize ( SIZE size );
	SIZE GetMinSize ( void);

	virtual bool IsSizingX ( void );
	virtual bool IsSizingY ( void );

	virtual bool IsSizing ( void );

	virtual bool IsMovingX ( void );
	virtual bool IsMovingY ( void );
	virtual bool IsMoving ( void );

	virtual SIZE GetSize ( void );

	virtual bool CanHaveFocus ( void );
	bool HasFocus ( void );

	virtual void SetText ( SIMPLEGUI_STRING sString, bool = false );
	const SIMPLEGUI_CHAR *GetText ( void );

	void SendMsg ( eEVentControl, int );

	virtual void Draw ( void );

	virtual bool MsgProc ( UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual bool HandleKeyboard ( UINT uMsg, WPARAM wParam, LPARAM lParam );
	virtual bool HandleMouse ( UINT uMsg, CVector pos, WPARAM wParam, LPARAM lParam );

	SFontInfo GetFontInfo(void);
	void SetFont(SFontInfo sInfo);
	void SetFont ( const SIMPLEGUI_CHAR *szFontName, DWORD dwHeight, bool bBold = false );
	void SetFont ( CD3DFont *pFont );

	CD3DFont *GetFont ( void );

	virtual CScrollablePane *GetScrollbar ( void ) { return NULL; }

	void SetStateColor ( D3DCOLOR d3dColor, SControlColor::SControlState eState );

	virtual void OnFocusIn ( void );
	virtual void OnFocusOut ( void );

	virtual bool OnMouseOver ( void );
	virtual void OnMouseEnter ( void );
	virtual void OnMouseLeave ( void );

	virtual void OnClickEnter ( void );
	virtual void OnClickLeave ( void );
	virtual	bool OnClickEvent ( void );

	virtual bool OnMouseButtonDown ( sMouseEvents e );
	virtual bool OnMouseButtonUp ( sMouseEvents e );
	virtual bool OnMouseMove ( CVector pos );
	virtual bool OnMouseWheel ( int zDelta );

	virtual bool OnKeyDown ( WPARAM wParam );
	virtual bool OnKeyUp ( WPARAM wParam );
	virtual bool OnKeyCharacter ( WPARAM wParam );

	virtual bool InjectKeyboard ( sKeyEvents e );
	virtual bool InjectMouse ( sMouseEvents e );

	bool SendEvent ( eEVentControl event, int params );


public:
	void LinkPos ( CVector pos );

public:
	eRelative GetRelativeX ( void );
	eRelative GetRelativeY ( void );

	void SetRelativeX ( eRelative eRelativeType );
	void SetRelativeY ( eRelative eRelativeType );

	void SetEnabled ( bool bEnabled = true );
	bool IsEnabled ( void );

	void SetVisible ( bool bVisible = true );
	bool IsVisible ( void );

	void SetEnabledStateColor ( bool bEnable = true );
	void SetAntAlias ( bool bAntAlias = true );

	EControlType GetType ( void );

	virtual void UpdateRects ( void );
	virtual bool ContainsPoint ( CVector pos );
protected:
	bool bins;
	SIZE m_oldParentSize;
	CScissor sCissor;
	CVector m_oldPos;
	SIZE m_size;
	SIZE m_minSize;

	SIZE m_realSize;
	SIZE m_oldTextSize;

	SControlRect m_rScissor;
	SControlRect m_rScissorCpy;

	SControlRect m_rContentBox;
	SControlRect m_rBoundingBox;

	CD3DTexture *m_pTexture;
	CD3DFont* m_pFont;
	CWidget* m_pParent;
	CDialog *m_pDialog;

	SFontInfo m_sFontInfo;

	SIMPLEGUI_STRING m_sText;

	SControlColor m_sControlColor;

	EControlType m_eType;
	tAction m_pAction;

	SControlColor::SControlState m_eState;

	eRelative m_eRelativeX;
	eRelative m_eRelativeY;

	CRITICAL_SECTION cs;
	CVector m_pos;
	CVector m_nonUpdatedPos;
	
	bool m_bRelativePosX;
	bool m_bRelativePosY;

	bool m_bRelativeSizeX;
	bool m_bRelativeSizeY;

	bool m_bAntAlias;
	bool m_bPressed;
	bool m_bMouseOver;

	bool m_bHasFocus;

	bool m_bEnabled;
	bool m_bVisible;

	bool m_bEnabledStateColor;
	bool m_bUpdatedFont;
};