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
		m_mButtonID [ this ] [ m_pParent ] [ m_uGroup ] = m_nCount[ m_pParent ];
	}

	UINT GetGroup ( void )
	{
		return m_uGroup;
	}

	void Draw ( void );

	bool GetChecked ( void )
	{
		return m_bChecked;
	}

	void SetChecked ( bool bChecked )
	{
		m_mID  [ m_uGroup ] = m_mButtonID [ this ] [ m_pParent ] [ m_uGroup ];
		SendEvent ( EVENT_CONTROL_SELECT, m_bChecked = bChecked );
	}

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );

	void UpdateRects ( void );
	bool ContainsRect ( CPos pos );
private:
	
	UINT m_uGroup;
	SControlRect m_rText;
	bool m_bChecked;

	typedef std::map<CControl*, std::map<UINT, UINT>> RadioParent;
	static  std::map<CControl*,UINT>m_nCount;
	static std::map<UINT, UINT> m_mID;
	static std::map<CRadioButton*, RadioParent> m_mButtonID;
};

