#pragma once
#include "CGUI.h"

class CScrollbar /*: public CControl*/
{
public:
	/*CScrollbar ();
	~CScrollbar ();*/

	int GetMinSize ( void );
	int GetEndSize ( void );

	int GetTrackPos ( void );
	void SetTrackPos ( int nPosition );

	int GetPageSize ( void );
	void SetPageSize ( int nPageSize );

	void SetStepSize ( int nStep );
	int GetStepSize ( void );

	bool IsThumbShowing ( void );

	void SetTrackRange ( int nStart, int nEnd );

	// Scroll by nDelta items (plus or minus)
	void Scroll ( int nDelta );

	// Ensure that item nIndex is displayed, scroll if necessary
	void ShowItem ( int nIndex );

	int GetThumbPos ( void );
	int GetThumbSize ( void );
protected:
	void SetBackPos ( int nPos );
	void SetBackSize ( int nSize );

	void UpdateThumbRect ( void );
	void Cap ( void );  // Clips position at boundaries. Ensures it stays within legal range.

	int m_nThumbPos;
	int m_nThumbSize;

	int m_nBackPos;
	int m_nBackSize;

	int m_nStep;	  // Step m_size used for increase / decrease button clicks.
	int m_nPosition;  // Position of the first displayed item
	int m_nPageSize;  // How many items are displayable in one page

	int m_nStart;     // First item
	int m_nEnd;       // The index after the last item

	bool m_bShowThumb;
};

