#pragma once

/*!\brief һ�������˾��е��б��� GUI �ؼ�������ࣨ����ʵ������
**
** ������ԣ���������ؼ������Ʋ˵���VdkListBox һ��ĵ���
** �ؼ���һ�о���һ� VdkListCtrl ���㣬��Ϊ VdkListCtrl
** �Ƕ��нṹ���޷��ܺõ���һ���еķ�����ʾ��������ʹ�ܣ���
** ��ҲԶ��ʹ��һ��������ʾҪ��öࡣ
** \note ������ wxItemContainer
**/
template< typename T >
class VdkListItemContainer
{
public:

	/// \brief ��ȡָ��λ�õĲ˵���
	virtual T* GetItem(int index) const
	{
		if( index < GetItemCount()	)
			return m_items[index];

		return NULL;
	}

	/// \brief ��ȡ�б�����Ŀ������
	int GetItemCount() const { return m_items.size(); }

protected:

	/// \brief ���캯��
	/// \param fixed ��Ŀ�Ƿ�̶��߶�
	VdkListItemContainer(bool fixed = false, int lastY = 0)
		: m_fixed(fixed), m_lastY(lastY), m_lastHeight(0)
	{

	}

	/// \brief ���б�ĩβ����һ��
	void Append(T* item, int h)
	{
		m_lastY += m_lastHeight;
		m_lastHeight = h;

		m_items.push_back(item);
		m_startCoords.push_back(m_lastY);
	}

	/// \brief ���ƶ���\a index ������һ��
	///
	/// �²����һ�����Ž�����\a index
	void Insert(int index, T* item, int h = 0)
	{
		// �����û�ʵ��������ĩβ���һ��
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

	/// \brief ɾ��\a index ��
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

	/// \brief �޸�ĳ��ĸ߶�
	/// \param dt �¾ɸ߶�֮��Ĳ��
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

	/// \brief ���ݸ�����λ��\a y ���ҷ���Ҫ�����Ŀ
	/// \return �Ҳ���ʱ����-1
	int FindIndex(int y)
	{
		coords::iterator i, beg( m_startCoords.begin() );
		for( i = beg; i + 1 != m_startCoords.end(); ++i )
		{
			// �պô��߲���˵�ҵ��������Ҽ��˵���Ĭ��ѡ�е�һ��
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

	/// \brief �޸���\a index ����������������
	/// \param dt �¾ɸ߶�֮��Ĳ��
	void WalkCoords(int index, int dt)
	{
		coords::iterator i( m_startCoords.begin() + index );
		for(; i != m_startCoords.end(); ++i )
			*i += dt;
	}

	/// \brief �����ҵ�����Ŀ�Ƿ����Ҫ��
	virtual bool TestFind(const T& item) = 0;

	//////////////////////////////////////////////////////////////////////////

	WX_DECLARE_LIST(T, item_list);

	typedef wxArrayInt coords;
	typedef typename item_list::const_iterator ItemIter;

	bool					m_fixed;		///< �����Ƿ�̶��߶�

	item_list				m_items;		///< ������Ŀ
	coords					m_startCoords;	///< �������ʼ y ����
	int						m_lastY;		///< ���һ��� y ����
	int						m_lastHeight;	///< ���һ��ĸ߶�
};

template< typename T >
void VdkListItemContainer<T>::wxitem_listNode::DeleteData()
{
	delete (T *)GetData();
}
