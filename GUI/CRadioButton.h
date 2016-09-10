#pragma once

#include "CGUI.h"

class CRadioButton : public CControl
{
public:
	CRadioButton ( CDialog *pDialog );
	~CRadioButton ( void );

	void SetGroup ( UINT uGroup )
	{
		m_uGroup = uGroup;
	}

	UINT GetGroup ( void )
	{
		return m_uGroup;
	}

	void Draw ( void );

	bool GetChecked ( void )
	{
		return ( mADD [ m_pParent ] [ m_uGroup ] == this );
	}

	void SetChecked ( bool bChecked )
	{
		mADD [ m_pParent ] [ m_uGroup ] = bChecked ? this : NULL;
		SendEvent ( EVENT_CONTROL_SELECT, bChecked );
	}

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );

	void UpdateRects ( void );
	bool ContainsRect ( CPos pos );
private:
	
	UINT m_uGroup;
	SControlRect m_rText;

	static std::map<CControl*, std::map<UINT, CControl*>> mADD;

};

