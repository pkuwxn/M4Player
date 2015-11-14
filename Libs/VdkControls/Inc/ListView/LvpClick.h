/***************************************************************
 * Name:      LvpClick.h
 * Purpose:   ʵ�� VdkListView ��ѡ����ѡ
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-3
 **************************************************************/
#pragma once
#include "IListViewPlaugin.h"

/// ʵ�� VdkListView ��ѡ����ѡ
class LvpClick : public IListViewPlaugin
{
public:

	//! ��ѡ/��ѡ
	enum SelectMode {
		SM_SINGLE, ///< ��ѡ
		SM_MULTI, ///< ��ѡ
	};

	/// ���캯��
	LvpClick(ListView* list, SelectMode sm, const wxBrush& selected);

	/// ѡ�е�@a index ��
	///
	/// ��ǰ����ѡ��ᱻ��ա�
	void Select(int index, wxDC* pDC);

	/// ѡ��[@a index, end) �����е�������Ŀ
	///
	/// ��ǰ����ѡ��ᱻ��գ��ҽ��ڶ�ѡģʽ����Ч��
	/// @attention ����Ϊ�ҿ����䡣
	void Select(int beg, int end, wxDC* pDC);

	/// ѡ��ָ�������е�������Ŀ
	///
	/// ��ǰ����ѡ��ᱻ��ա�
	void Select(const wxArrayInt& items, wxDC* pDC);

	/// �����ѡ����б���
	///
	/// �����ػ��ؼ���������ʽ���� VdkListView::RefreshState() ��
	void SelectNone();

	/// ��ȡѡ��ģʽ
	SelectMode GetSelectMode() const {
		return m_multiSel ? SM_MULTI : SM_SINGLE;
	}

	/// ����ѡ��ģʽ
	///
	/// �������һ����ⲿֱ�ӵ��ã��ڲ�ʵ�ֻ���ʱ���� pDC �Ļ�ͼ��ʼ�㣬
	/// ���ڷ���ʱ�ָ���
	/// �����ã�����յ�ǰ����ѡ��
	void SetSelectMode(SelectMode sm, wxDC* pDC);

	/// ��ȡѡ����ı�����ˢ
	wxBrush GetSelectedBrush() const { return m_selected; }
	void SetSelectedBrush(const wxBrush& brush) { m_selected = brush; }

public:

	/// Interface definition for a callback to be invoked when an item in this 
	/// ListView has been clicked. 
	class OnItemClickListener
	{
	public:

		/// Callback method to be invoked when an item in this ListView 
		/// has been clicked. 
		/// @param parent The ListView where the click happened.
		/// @param index The row id of the item that was clicked.
		/// @param dc The device context of this ListView.
		virtual void OnItemClick(LvpClick* parent, int index, wxDC& dc) = 0;
	};

	/// ������Ŀ���������
	void SetOnItemClickListener(OnItemClickListener* listener) {
		m_clickListener = listener;
	}

public:

	WX_DEFINE_SORTED_ARRAY_INT( int, ArrayOfSortedInts );
	/// ��ȡ��ѡ�е���
	const ArrayOfSortedInts& GetSelItems() const { return m_selItems; }
    
protected:

    virtual void OnEraseRow(int row, wxDC& dc);
    virtual void OnDragg(int rowAtPointer, VdkMouseEvent& e) {}
    virtual void OnDClick(int rowAtPointer, VdkMouseEvent& e);
    virtual void OnClickDown(int rowAtPointer, VdkMouseEvent& e);
    virtual void OnClickUp(int rowAtPointer, VdkMouseEvent& e);
    virtual void OnRightUp(int rowAtPointer, VdkMouseEvent& e) {}

	virtual void OnItemAdd(size_t row);
	virtual void OnItemRemove(size_t row);
	virtual void OnClear();

private:

    virtual void OnKey(VdkKeyEvent& e);
	void OnPageDown(VdkKeyEvent& e);
	void OnPageUp(VdkKeyEvent& e);
	void OnDown(VdkKeyEvent& e);
	void OnUp(VdkKeyEvent& e);
	void OnCtrlPlusHomeEnd(VdkKeyEvent& e);

	void OnItemAddRemove(size_t row, bool add);

private:

	// ����ס Shift ��ʱ�������µ������İ����¼�
	//
	// ģ���������������¼�������������
	void OnShiftUpDownKeys(int currSel, wxDC& dc);

	/// �Ƿ���ȫѡ��״̬��
	bool IsAllSelected();

	// �����ѡ����б���ڲ�����
	void SelectNone(wxDC* pDC);

	// �����������ѡ��
	// 
	// ��Ҫ��Ϊ��ֻ����һ���޸� m_lastSelected ��������ڡ�
	void UpdateVeryLastSel(int row);

protected:

	/// ��ȡ����ѡ��ʱָ���ĵ�һ��� ID
	int& GetSelStart() { return m_selStart; }

	/// ��ȡ�������ѡ��
	int& GetVeryLastClicked() { return m_veryLastClicked; }

	/// ��ȡ��ѡ�е���
	ArrayOfSortedInts& GetSelItems() { return m_selItems; }

private:

	bool m_multiSel; // �Ƿ����ö�ѡ

	wxBrush m_selected; // ѡ����ı�����ˢ
	int m_selStart; // ����ѡ��ʱָ���ĵ�һ��� ID
	int m_veryLastClicked; // �������ѡ����ڡ��ϡ����¡����Ĵ���

	ArrayOfSortedInts m_selItems;

	//============================================

	OnItemClickListener* m_clickListener;
};
