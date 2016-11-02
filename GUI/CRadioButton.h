#pragma once

#include "CGUI.h"

class CRadioButton : public CWidget
{
public:
	CRadioButton ( CDialog *pDialog );
	~CRadioButton ( void );

	void SetGroup(UINT uGroup);
	UINT GetGroup(void);

	void Draw ( void );

	bool IsChecked(void);
	void SetChecked(bool bChecked);

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );

	void SetText(SIMPLEGUI_STRING sText, bool = false);

	void SetWidth(int nWidth);
	void SetHeight(int nHeight);

	void UpdateRects ( void );
	bool ContainsPoint (Pos pos );
private:
	
	UINT m_uGroup;

	SControlRect m_rText;
	SControlRect m_rBox;

	static std::map<CWidget*, std::map<UINT, CWidget*>> mADD;

};

