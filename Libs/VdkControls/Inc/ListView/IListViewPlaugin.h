/***************************************************************
 * Name:      IListViewPlaugin.h
 * Purpose:   VdkListView �Ĳ���ӿ�����
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-3
 **************************************************************/
#pragma once

// ��ѡ����ѡ LvpClick
// �ı��Сʱ���ݰٷֱȶ�̬�����п� LvpColumnAutoAjuster
// ͨ���϶��ı���Ŀ���� LvpDraggAndReorder
// ���б�ɫ LvpCrossColor
// ��껬��ʱ���������� LvpHilighter
// ���� LvpSorter
// ������ LvpHeader

class VdkMouseEvent;
class VdkKeyEvent;
class VdkListView;

/// VdkListView �Ĳ���ӿ�����
class IListViewPlaugin
{
public:

	typedef VdkListView ListView;

	/// ���캯��
	IListViewPlaugin(ListView* list)
		: m_list( list )
	{
		wxASSERT( m_list );
	}

	/// ����������
	virtual ~IListViewPlaugin() {}

public:

	/// ��ȡ VdkListView ����
	ListView* GetListView() const { return m_list; }

	/// �Ƿ���Ա� VdkListView ��ȫɾ��
	///
	/// ���������ж��ؼ̳е����νṹ����ôӦ��С�Ĵ��������������ڡ�
	virtual bool CanDestoryByListView() const { return true; }

public:

    /// ����ԭ�����ݣ����Ʊ���
    virtual void OnEraseRow(int row, wxDC& dc) {}
    
    /// �϶��¼�
    /// @param rowAtPointer ��ǰ���ָ���¶�Ӧ����Ŀ
    virtual void OnDragg(int rowAtPointer, VdkMouseEvent& e) {}
    
    /// ���˫���¼�
    virtual void OnDClick(int rowAtPointer, VdkMouseEvent& e) {}
    
    /// ����������
    virtual void OnClickDown(int rowAtPointer, VdkMouseEvent& e) {}
    /// ����������
    virtual void OnClickUp(int rowAtPointer, VdkMouseEvent& e) {}
    
    /// ����Ҽ�����
    virtual void OnRightUp(int rowAtPointer, VdkMouseEvent& e) {}
    
    /// ���̰����¼�
    virtual void OnKey(VdkKeyEvent& ke) {}
    
    /// �ؼ�λ�øı�
    /// TODO:
    virtual void OnMove(int dx, int dy) {}
    
    /// �ؼ���С�ı�
    virtual void OnSize(int dx, int dy) {}

public:

	/// ����������������֪ͨ�б�����Ϊ@a row ���������
	virtual void OnItemAdd(size_t row) {}

	/// ����������������֪ͨ�б�����Ϊ@a row ����Ƴ�
	virtual void OnItemRemove(size_t row) {}

	/// ����������������֪ͨ�б��������������ѱ��Ƴ�
	virtual void OnClear() {}

private:

	ListView* m_list;
};
