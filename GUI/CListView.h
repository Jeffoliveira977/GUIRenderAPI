#pragma once

#include "CGUI.h"

class CListView : public CControl
{
public:
	CListView											( CDialog *pDialog );
	~CListView											( void );

	void					AddColumn					( const SIMPLEGUI_CHAR *szColumnName, int nWidth );
	void					RemoveColumn				( UINT nColumnId );
	void					RemoveAllColumns			( void );

	void					SetColumnName				( UINT nColumnId, const SIMPLEGUI_CHAR *szColumnName );
	const SIMPLEGUI_CHAR*	GetColumnName				( UINT nColumnId );

	const SEntryItem*		GetColumnItemByRow			( UINT nColumnId, UINT nRow );

	void					SetColumnWidth				( UINT nColumnId, int nWidth );
	int						GetColumnWidth				( UINT nColumnId );

	void					AddColumnItem				( UINT nColumnId, const SIMPLEGUI_CHAR *szItem, const SIMPLEGUI_CHAR *szValue = _UI ( "" ) );
	void					AddColumnItem				( UINT nColumnId, SEntryItem *pEntry );

	void					SetColumnItemName			( UINT nColumnId, UINT nIndex, const SIMPLEGUI_CHAR *szItem );
	void					RemoveColumnItem			( UINT nColumnId, UINT nIndex );

	void					RemoveAllItemsFromColumn	( UINT nColumnId );
	void					RemoveAllItems				( void );

	size_t					GetNumOfColumns				( void );
	size_t					GetNumOfItemsFromColumn		( UINT nColumnId );
	size_t					GetNumOfItems				( void );

	int						GetAllColumnsWidth			( void );

	const SEntryItem*		GetSelectedItem				( UINT nColumnId );

	int						GetColumnIdAtArea			( CPos pos );
	int						GetColumnIdAtAreaBorder		( CPos pos );
	int						GetColumnOffset				( UINT nColumnId );

	void					SetSortable					( bool bSortable );
	void					SetTitleSizable				( bool bSizable );
	void					SetTitleMovable				( bool bMovable );

	const SEntryItem*		FindItemInRow				( UINT nRow );

	void					Draw						( void );

	void					MoveColumn					( UINT nColumnId, UINT nPosition );
	void					SortColumn					( UINT nColumnId );

	void					OnClickLeave				( void );
	bool					OnClickEvent				( void );

	void					OnFocusIn					( void );
	void					OnFocusOut					( void );

	void					OnMouseEnter				( void );
	void					OnMouseLeave				( void );

	bool					CanHaveFocus				( void );

	bool					HandleMouse					( UINT uMsg, CPos pos, WPARAM wParam, LPARAM lParam );
	bool					HandleKeyboard				( UINT uMsg, WPARAM wParam, LPARAM lParam );

	void					UpdateRects					( void );
	bool					ContainsRect				( CPos pos );

private:

	struct SListViewColumn
	{
		std::vector<SEntryItem*>		m_sItem;
		SIMPLEGUI_STRING				m_sColumnName;
		int								m_nWidth;
	};

	int					m_nDragX;
	int					m_nOverColumnId;
	int					m_nId;
	int					m_nIndex;
	int					m_nSelected;

	bool				m_bSizable;
	bool				m_bMovable;
	bool				m_bSortable;

	bool				m_bSizing;
	bool				m_bMoving;
	bool				m_bSorting;


	UINT				m_nRowSize;
	CD3DFont			*m_pTitleFont;
	CScrollablePane		*m_pScrollbar;

	SControlRect		m_rColumnArea;
	SControlRect		m_rListBoxArea;

	static UINT									m_nColumnSort;
	std::vector<SListViewColumn>				m_vColumnList;
	typedef std::map<UINT, SEntryItem*>			ColumnItem;

	static bool				ColumnItemLess				( ColumnItem a, ColumnItem b )
	{
		SEntryItem* pEntrya = a [ m_nColumnSort ];
		SEntryItem* pEntryb = b [ m_nColumnSort ];

		if ( !pEntrya || !pEntryb )
			return false;

		return ( pEntrya->m_sText < pEntryb->m_sText );
	}

	static bool				ColumnItemGreater			( ColumnItem a, ColumnItem b )
	{
		SEntryItem* pEntrya = a [ m_nColumnSort ];
		SEntryItem* pEntryb = b [ m_nColumnSort ];

		if ( !pEntrya || !pEntryb )
			return false;

		return ( pEntrya->m_sText > pEntryb->m_sText );
	}
};