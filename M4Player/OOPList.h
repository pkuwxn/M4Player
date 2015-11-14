#pragma once
#include "VdkListCtrl.h"

class wxInputStream;
class wxOutputStream;
class OOPSong;
class OOPListEntry;
class OOPSongPtr;

/// \brief OOPlayer ר���б�ؼ�
class OOPList : public VdkListCtrl
{
public:

	/// \brief ���캯��
	OOPList();

	/// \brief ��������
	~OOPList();

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���б������һ�׸���
	/// \param numRows ��ǰ�б��е���Ŀ�����������óɹ�����Ŀ��Ӧ������ 1 ��
	///				   ��Ҫ��ϣ������ÿ�μ����б���Ŀ��(һ��ʱ�临�Ӷ�ΪO(n))��
	void AddToList(OOPSong& song, size_t numRows);

	/// \brief ���ݵ�ǰ��Ŀ����������еĿ��
	void UpdateSnColumnWidth(int numRowsOld, int numRowsCurr = wxNOT_FOUND);

	/// \brief ��ȡ��@a index ��ӵ� OOPSong �����
	OOPSongPtr GetSongPtr(int index);

	/// \brief ��ȡ�ڵ�����@a i ��ָ�����ӵ� OOPSong �����
	OOPSongPtr GetSongPtr(const LcCellIter& i);

	/// \brief ��ȡ�ڵ�����@a i ��ָ�����ӵ� OOPListEntry �����
	OOPListEntry* GetListEntry(const LcCellIter& i);

	/// \brief ɾ���� \a index ��
	///
	/// ������ GUI ��
	void RemoveSong(int index);

	/// \brief ɾ���� \a index ��
	///
	/// ������ GUI ��
	void RemoveSong(const LcCellIter& i);

	/// \brief �Ƴ����е�Ԫ���и��ŵ��û��Զ�����Ϣ( OOPSong �����)
	void RemoveAllSongPtrs();

	//////////////////////////////////////////////////////////////////////////

	/// \brief �Զ����Ʒ�ʽ���沥���б�
	void Serialize(wxInputStream& fin);

	/// \brief �Զ����Ʒ�ʽ���沥���б�
	void Serialize(wxOutputStream& fout);

	//////////////////////////////////////////////////////////////////////////

	/// \brief �����ƶ�������ѡ��һ�����򷽷������򲥷��б�
	void SortList(VdkCtrlId cmd);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ǰ�Ƿ�����ѱ���������
	bool HasLocked() { return m_locked != end(); }

	/// \brief ��ȡ���ڲ��ŵĸ����� ID
	/// \note ���ﲻ�ܼ� const
	int GetLockedIndex();

	/// \brief ��ȡ���ڲ��ŵĸ���
	LcCellIter GetLocked() { return m_locked; }

	/// \brief ��ȡ���ڲ��ŵĸ����Ĺ�������
	/// \note ���ﲻ�ܼ� const
	OOPSongPtr GetLockedSong();

	/// \brief �������ڲ��ŵĸ���
	/// \param index �����ֵ��Ч
	void SetLocked(int index, wxDC* pDC);

	/// \brief �������ڲ��ŵĸ���
	/// \param locked �����ֵ��Ч
	void SetLocked(const LcCellIter& locked, wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	/// \brief �����б�ĵ�һ�׸���
	/// \note ���ﲻ�ܼ�const���η�
	LcCellIter begin();

	/// \brief �����б�����һ�׸���
	/// \note ���ﲻ�ܼ�const���η�
	LcCellIter end();

protected:

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void OnNotify(const VdkNotify& notice);

private:

	// �Ƴ���Ԫ���и��ŵ��û��Զ�����Ϣ( OOPSong �����)
	void RemoveSongPointer(const LcCellIter& i);

	// �Ƴ���Ԫ���и��ŵ��û��Զ�����Ϣ( OOPSong �����)
	void RemoveSongPointer(int i);

private:

	// ��������¼�
	virtual void OnMouseEvent(VdkMouseEvent& e);

	// ��Ӧ�����¼�
	virtual void OnKeyEvent(VdkKeyEvent& e);

	// XRC ��̬����
	virtual void OnXrcCreate(wxXmlNode* node);

	// ��������ɫѡ����
	void CreateGradientBrush();

	// ���Ƶ�Ԫ���ı�
	virtual VdkCusdrawReturnFlag
		DoDrawCellText(const VdkLcCell* cell,
					   int col_index,
					   int index,
					   wxDC& dc,
					   VdkLcHilightState state );

	// ���������ƣ��б���
	virtual VdkCusdrawReturnFlag DoEraseRow
		(const LcCellIter& it, int index, wxDC& dc);

	// ��Ӧ��յ�ǰ�������¼�
	virtual void DoClear(wxDC* pDC = NULL);

private:

	// @brief ���б������һ�׸���
	// @param title ����ָ��һ���ض����б������
	// @param numRows ��ǰ�б��е���Ŀ�����������óɹ�����Ŀ��Ӧ������ 1 ��
	//				  ��Ҫ��ϣ������ÿ�μ����б���Ŀ��(һ��ʱ�临�Ӷ�Ϊ
	//				  O(n) )��
	void AddToList(OOPListEntry& entry, const wxString& title, size_t numRows);

	// ��ȡѡ����ı߿���ı���ɫ
	wxColour GetSelectedColour() const;

	// ���µ� @a row �е�ʱ����ʾ
	void UpdateTime(int row, unsigned int len);

	// ����ָ����Ԫ���ʱ����ʾ
	void UpdateTime(VdkLcCell* cell, unsigned int len);

	// ����ĳЩ����ʾ����Щ�п��ܾ�Ϊ�գ�-1��
	void RedrawRows(const wxArrayInt& rows, wxDC& dc);

	//////////////////////////////////////////////////////////////////////////

	wxColour m_textColor; // �ı���ɫ
	wxColour m_snColor; // ��ŵ��ı���ɫ
	wxColour m_timeColor; // ʱ����ʾ���ı���ɫ
	wxColour m_lockedColor; // �����ڲ��š���Ŀ��������ɫ
	wxColour m_selectedColor; // ����ɫ��(����ѡ��һ����Ŀ��ļ���)

	LcCellIter m_locked; // ���֡����ڲ��š�����Ŀ

	DECLARE_CLONEABLE_VOBJECT( OOPList )
};
