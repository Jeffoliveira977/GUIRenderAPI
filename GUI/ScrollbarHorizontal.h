#pragma once

#include "CGUI.h"

class CScrollBarHorizontal : public CScrollbar, public CControl
{
public:
	CScrollBarHorizontal ( CDialog *pDialog );

	void Draw ( void );

	void UpdateRects ( void );
	bool ContainsRect ( CPos pos );

	bool HandleMouse ( UINT uMsg, CPos pos, WPARAM wParam, LPARAM lParam );

	bool OnMouseButtonDown ( CPos pos );
	bool OnMouseButtonUp ( CPos pos );
	bool OnMouseMove ( CPos pos );

	void OnClickLeave ( void );
	bool OnClickEvent ( void );

private:

	// ARROWSTATE indicates the state of the arrow buttons.
	// CLEAR            No arrow is down.
	// CLICKED_UP       Up arrow is clicked.
	// CLICKED_DOWN     Down arrow is clicked.
	// HELD_UP          Up arrow is held down for sustained period.
	// HELD_DOWN        Down arrow is held down for sustained period.
	enum ARROWSTATE
	{
		CLEAR,
		CLICKED_UP,
		CLICKED_DOWN,
		CLICKED_THUMB,
		HELD_UP,
		HELD_DOWN,
	};

	SControlRect m_rUpButton;
	SControlRect m_rDownButton;
	SControlRect m_rThumb;

	bool m_bShowThumb;
	bool m_bDrag;
	int nThumbOffset;
	CTimer m_timer;
	ARROWSTATE m_Arrow; // State of the arrows
};