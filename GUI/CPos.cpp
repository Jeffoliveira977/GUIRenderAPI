#include "CGUI.h"

CVector::CVector(POINT point)
{
	m_nX = point.x;
	m_nY = point.y;
}

CVector::CVector( CVector *pPos )
{
	m_nX = pPos->m_nX;
	m_nY = pPos->m_nY;
}

CVector::CVector( int nX, int nY )
{
	m_nX = nX;
	m_nY = nY;
}

CVector::CVector()
{
	m_nX = 0;
	m_nY = 0;
}

CVector::~CVector()
{
}

CVector CVector::operator*(CVector otherPos)
{
	return  CVector(m_nX * otherPos.m_nX, m_nY * otherPos.m_nY);
}

CVector CVector::operator/(CVector otherPos)
{
	return CVector(m_nX / otherPos.m_nX, m_nY / otherPos.m_nY);
}

CVector CVector::operator*(int pos)
{
	return CVector(m_nX * pos, m_nY * pos);
}

CVector CVector::operator/(int pos)
{
	return CVector(m_nX / pos, m_nY / pos);
}

CVector CVector::operator*=(CVector otherPos)
{
	m_nX *= otherPos.m_nX;
	m_nY *= otherPos.m_nY;

	return *this;
}

CVector CVector::operator/=(CVector otherPos)
{
	m_nX /= otherPos.m_nX;
	m_nY /= otherPos.m_nY;

	return *this;
}

CVector CVector::operator*=(int pos)
{
	m_nX *= pos;
	m_nY *= pos;

	return *this;
}

CVector CVector::operator/=(int pos)
{
	m_nX /= pos;
	m_nY /= pos;

	return *this;
}

CVector CVector::operator + ( CVector otherPos )
{
	return CVector(m_nX + otherPos.m_nX, m_nY + otherPos.m_nY);
}

CVector CVector::operator - ( CVector otherPos )
{
	return CVector(m_nX - otherPos.m_nX, m_nY - otherPos.m_nY);
}

CVector CVector::operator + ( int pos)
{
	return CVector (m_nX + pos, m_nY + pos);
}

CVector CVector::operator - ( int pos)
{
	return CVector (m_nX - pos, m_nY - pos);
}

CVector CVector::operator+=(CVector otherPos)
{
	m_nX += otherPos.m_nX;
	m_nY += otherPos.m_nY;

	return *this;
}

CVector CVector::operator-=(CVector otherPos)
{
	m_nX -= otherPos.m_nX;
	m_nY -= otherPos.m_nY; 
	
	return *this;
}

CVector CVector::operator+=(int pos)
{
	m_nX += pos;
	m_nY += pos;

	return *this;
}

CVector CVector::operator-=(int pos)
{
	m_nX -= pos;
	m_nY -= pos;

	return *this;
}

bool CVector::operator == (CVector otherPos)
{
	return (m_nX == otherPos.m_nX &&
		m_nY == otherPos.m_nY);
}

bool CVector::operator!=(CVector otherPos)
{
	return !operator==(otherPos);
}


