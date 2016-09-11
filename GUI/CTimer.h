#pragma once

#pragma comment (lib, "winmm.lib")
#include <windows.h>

class CTimer
{
public:


	CTimer ( void );
	~CTimer ( void );

	void Start ( float fSec );
	void Stop ( void );

	float TimeLeft ( void );
	bool Running ( void );

protected:
	DWORD dwStop;
};