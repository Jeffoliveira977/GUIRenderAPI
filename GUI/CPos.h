#pragma once

class CVector
{
public:
	int m_nX, m_nY;

	CVector(POINT point);
	CVector( CVector *pos );
	CVector( int iX, int iY );
	CVector();

	~CVector();

	CVector operator * (CVector otherPos);
	CVector operator / (CVector otherPos);

	CVector operator *= (CVector otherPos);
	CVector operator /= (CVector otherPos);

	CVector operator * (int pos);
	CVector operator / (int pos);

	CVector operator *= (int pos);
	CVector operator /= (int pos);

	CVector operator + ( CVector otherPos );
	CVector operator - ( CVector otherPos );

	CVector operator += (CVector otherPos);
	CVector operator -= (CVector otherPos);

	CVector operator + ( int nIncPos );
	CVector operator - ( int nDecPos);

	CVector operator += (int pos);
	CVector operator -= (int pos);

	bool operator == ( CVector otherPos );
	bool operator != ( CVector otherPos );
};