#pragma once

#include "CGUI.h"

class CProgressBarVertical : public CWidget
{
public:
	CProgressBarVertical ( CDialog *pDialog );

	void Draw ( void );

	void UpdateRects ( void );
	bool ContainsPoint ( CVector pos );

	bool OnMouseButtonDown ( sMouseEvents e );
	bool OnMouseButtonUp ( sMouseEvents e );
	bool OnMouseMove ( CVector pos );

	void SetValue ( float fValue );
	float GetValue ( void );

	void SetMaxValue ( float fMax );
	float GetMaxValue ( void );

	void SetStep ( float fStep );
	float GetStep ( void );

	float ValueFromPos ( int iX );
private:
	float m_fValue;
	float m_fMax;
	float m_fStep;

	bool m_bDrag; 

	CTimer m_timer;
	SControlRect m_rProgress;
};

