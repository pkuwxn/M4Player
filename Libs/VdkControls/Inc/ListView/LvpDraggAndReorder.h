/***************************************************************
 * Name:      LvpDraggAndReorder.h
 * Purpose:   ʵ�� VdkListView ͨ���϶��ı���Ŀ����
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-3
 **************************************************************/
#pragma once
#include "LvpClick.h"
#include <wx/vector.h>

/// ʵ�� VdkListView ͨ���϶��ı���Ŀ����
class LvpDraggAndReorder : public LvpClick
{
public:

	/// �кŸ�����
	/// 
	/// ����һЩ�кţ���������и��٣�ʹ֮���϶�ĳ��֮��
	/// ������Ŀ���к���Ȼ�������б��б�����ȷ��
	class RowTracker
	{
	public:

		/// �������϶��¼�
		class MoveListener
		{
		public:

			/// �к�Ϊ@a row ���д�@a row �ϵ�@a dst
			virtual void OnRowMove(int row, int dst) = 0;
		};

		/// ���캯��
		RowTracker(MoveListener& listener);

		/// ���������ע����к�
		void Clear();

		/// ע��һ��
		void AddRow(int& row);

		/// ���к�Ϊ@a row ���д�@a row �ϵ�@a dst
		void Move(int row, int dst);

	private:

		wxVector< int* > m_rows;
		MoveListener& m_listener;
	};
    
	/// ���캯��
	LvpDraggAndReorder(ListView* list, SelectMode sm, 
					   const wxBrush& selected, 
					   const wxPen& draggTarget,
					   RowTracker::MoveListener& listener);

private:

    virtual void OnDragg(int rowAtPointer, VdkMouseEvent& e);
    virtual void OnClickUp(int rowAtPointer, VdkMouseEvent& e);

	virtual void OnItemAdd(size_t row);
	virtual void OnItemRemove(size_t row);
	virtual void OnClear();

private:

	int m_lastDraggTarget; // ���϶���Ŀ��������ʱ��ǰ������Ŀ��
	RowTracker m_rowTracker; // �кŸ�����

	wxPen m_draggTarget; // �϶���Ŀʱ��ʾĿ��λ�õ�������ɫ
};
