#pragma once

/*!\brief 一个抽象了具有的列表项 GUI 控件的虚基类（不能实例化）
**
** 具体而言，就是这个控件是类似菜单、VdkListBox 一类的单列
** 控件，一行就是一项。 VdkListCtrl 不算，因为 VdkListCtrl
** 是多列结构，无法很好的用一行行的方法表示出来；即使能，开
** 销也远比使用一列列来表示要大得多。
** \note 类似于 wxItemContainer
**/
template< typename T >
class VdkListItemContainer
{
public:

	/// \brief 获取指定位置的菜单项
	virtual T* GetItem(int index) const
	{
		if( index < GetItemCount()	)
			return m_items[index];

		return NULL;
	}

	/// \brief 获取列表中项目的总数
	int GetItemCount() const { return m_items.size(); }

protected:

	/// \brief 构造函数
	/// \param fixed 项目是否固定高度
	VdkListItemContainer(bool fixed = false, int lastY = 0)
		: m_fixed(fixed), m_lastY(lastY), m_lastHeight(0)
	{

	}

	/// \brief 在列表末尾插入一项
	void Append(T* item, int h)
	{
		m_lastY += m_lastHeight;
		m_lastHeight = h;

		m_items.push_back(item);
		m_startCoords.push_back(m_lastY);
	}

	/// \brief 在制定的\a index 处插入一项
	///
	/// 新插入的一项的序号将会是\a index
	void Insert(int index, T* item, int h = 0)
	{
		// 假如用户实质是想在末尾添加一项
		if( index == GetItemCount() )
		{
			Append(item, h);
			return;
		}

		wxASSERT( index >= 0 && index<GetItemCount() );

		if( m_fixed )
			h = m_lastHeight;
		else
			wxASSERT(h>0);

		m_startCoords.insert(m_startCoords.begin()+index, m_startCoords[index]);
		WalkCoords(index+1, h);
		m_items.insert(m_items.begin()+index, item);

		m_lastY += h;
	}

	/// \brief 删除\a index 项
	void Delete(int index)
	{
		int h(m_lastHeight);
		if( !m_fixed )
		{
			if( index < GetItemCount() - 1 )
				h = m_startCoords[index + 1] - m_startCoords[index];
		}

		WalkCoords(index + 1, -h);
	}

	/// \brief 修改某项的高度
	/// \param dt 新旧高度之间的差距
	void SetItemHeight(int index, int dt)
	{
		if( m_fixed )
			return;

		WalkCoords(index + 1, dt);
		if( index+1 == GetItemCount() )
			m_lastHeight += index;
		else
			m_lastY += dt;
	}

	/// \brief 根据给定的位置\a y 查找符合要求的项目
	/// \return 找不到时返回-1
	int FindIndex(int y)
	{
		coords::iterator i, beg( m_startCoords.begin() );
		for( i = beg; i + 1 != m_startCoords.end(); ++i )
		{
			// 刚好触线不能说找到，否则右键菜单会默认选中第一项
			if( *i == y )
				return -1;

			if( *i < y && *(i + 1) > y )
			{
				if( TestFind( *m_items[ i - beg ] ) )
					return i - beg;
				else
					break;
			}
		}

		if( i + 1 == m_startCoords.end() && y < m_lastY + m_lastHeight && 
			TestFind( *m_items[ GetItemCount() - 1 ] ) )
			return GetItemCount() - 1;

		return -1;
	}

	/// \brief 修改自\a index 起及其后所有项的坐标
	/// \param dt 新旧高度之间的差距
	void WalkCoords(int index, int dt)
	{
		coords::iterator i( m_startCoords.begin() + index );
		for(; i != m_startCoords.end(); ++i )
			*i += dt;
	}

	/// \brief 测试找到的项目是否符合要求
	virtual bool TestFind(const T& item) = 0;

	//////////////////////////////////////////////////////////////////////////

	WX_DECLARE_LIST(T, item_list);

	typedef wxArrayInt coords;
	typedef typename item_list::const_iterator ItemIter;

	bool					m_fixed;		///< 各项是否固定高度

	item_list				m_items;		///< 所有条目
	coords					m_startCoords;	///< 各项的起始 y 坐标
	int						m_lastY;		///< 最后一项的 y 坐标
	int						m_lastHeight;	///< 最后一项的高度
};

template< typename T >
void VdkListItemContainer<T>::wxitem_listNode::DeleteData()
{
	delete (T *)GetData();
}
