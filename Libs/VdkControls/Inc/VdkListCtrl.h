/***************************************************************
 * Name:      VdkListCtrl.h
 * Purpose:   Code for VdkListCtrl implementation
 * Author:    Ning (vanxining@139.com)
 * Created:   2010-02-01
 * Copyright: Ning
 **************************************************************/
#pragma once
#include "VdkScrolledWindow.h"

#include <list>

//////////////////////////////////////////////////////////////////////////

/// \brief VdkListCtrl �ķ����
enum VdkListCtrlStyle {

	/// \brief �����������Ե���ʼ ID
	VLCS_USER				= VSWS_USER + 10,
	/// \brief �Ƿ������ѡ
	VLCS_MULTI_SELECT		= 1 << ( VSWS_USER + 0 ),
	/// \brief �Ƿ�ʹ�ý����б���ɫ
	VLCS_CROSS_COLORS		= 1 << ( VSWS_USER + 1 ),
	/// \brief �Ƿ����ɱ�ͷ
	VLCS_HEADER				= 1 << ( VSWS_USER + 2 ),
	/// \brief �Ƿ��������Ч��
	VLCS_HOVERING			= 1 << ( VSWS_USER + 3 ),
	/// \brief �Ƿ�����ѡ��ĳ��
	VLCS_NO_SELECT			= 1 << ( VSWS_USER + 4 ),
	/// \brief �Ƿ������϶�ĳ��
	VLCS_NO_DRAGG			= 1 << ( VSWS_USER + 5 ),
};

/// \brief ���� VdkListCtrl ĳ�е�����״̬
enum VdkLcHilightState {

	VDKLC_HS_NORMAL,	///< �ָ�����״̬
	VDKLC_HS_HILIGHT,	///< �����������
	VDKLC_HS_SELECTED	///< �ѱ�ѡ��
};

/// \brief VdkListCtrl ������ʱ���Լ�λ����
enum VdkListCtrlState {

	/// \brief �����������Ե���ʼ ID
	VLCST_USER				= VSWST_USER + 5,
	/// \brief ȫѡ��Ŀ
	VLCST_SELECT_ALL		= 1 << ( VSWST_USER + 0 ),
};

class VdkListCtrl;
struct VdkLcColumn;

/// \brief VdkListCtrl ��һ����Ԫ��
class VdkLcCell : public VdkClientDataOwner
{
public:

	/// \brief ���캯��
	VdkLcCell(VdkLcColumn* col,
			  const wxString& label,
			  void* clientData);

	/// \brief �������Ƶ�Ԫ���ı�
	/// 
	/// ����ֻ��Ҫʹ�����໯�ػ�������ɫ���ı���ɫ�����ݣ������ı�����
	/// ���� VDK �������Ե��ñ�������
	/// \note VdkLcCell ���Զ������ı����루����У���
	void DrawLabel(wxDC& dc, int x, int y) const;

	/// \brief ��Ԫ���ı��Ƿ�Ϊ��
	bool IsEmpty() const { return m_label.IsEmpty(); }

	/// \brief ��ȡ�ı���߾�
	int GetX_Padding() const;

	/// \brief ��ȡ��ǰ��Ԫ����ı�
	/// \param xPadding ��ȡ�ı���߾�
	wxString GetLabel(int* xPadding = NULL) const;

	/// \brief ���õ�Ԫ���ı�
	void SetLabel(const wxString& label);

	/// \brief ��ȡ��Ԫ����������
	VdkLcColumn* GetColumn() const { return m_col; }

private:

	// ���㵥Ԫ���ı�������ʼλ���Լ����ʱ����Ӧ����
	void CalcTextPos();

	// ͬ���������Լ��汾��ʹ֮���ı��еİ汾��ͬ
	void SyncDrawingVersion() const;

	// ���͵�ǰ�������Լ��汾
	//
	// ʵ�����ǽ�����Ϊ�л������Լ��汾��һ��
	void DecreaseDrawingVersion();

private:

	VdkLcColumn*		m_col; // ��������
	wxString			m_label; // ����

	int					m_xFix; // �ı���߾�
	// �����ⳬ��ʱ��ʾ���ı�(һ����ĩβ��...)
	wxString			m_labelOverflow;

	// ��ǰ��ͼ�汾
	mutable int			m_drawingVersion;
};

typedef std::list< VdkLcCell* > ListOfLcCell;
typedef ListOfLcCell::iterator LcCellIter;

//////////////////////////////////////////////////////////////////////////

/*!\brief ���� VdkListCtrl ��һ��
 *
 * ע���һ�е����⺬�塣
**/
struct VdkLcColumn {

	/// \brief Ĭ�Ϲ��캯��
	VdkLcColumn();

	/// \brief ��\a rhs ���Ʊ���
	///
	/// �������еĸ��
	void Clone(const VdkLcColumn &rhs);

	/// \brief �����л���ʱʹ�õ�����
	void SetFont(const wxFont& font);

	/// \brief �б�ؼ��ı��С������п�
	void UpdateWidth(size_t w);

	/// \brief ��ȡ��ǰ��ͼ�汾
	/// 
	/// ����ϣ��ʵ������һ��Ч������ĳЩ��ͼ���Ա仯ʱ����Ԫ��Ҫ����
	/// ���¼�������������ڻ���ʱ�ż�ʱ�޸ġ�
	int GetDrawingVersion() const { return m_drawingVersion; }

	/// \brief ��ߵ�ǰ��ͼ�汾
	void IncreaseDrawingVersion() { m_drawingVersion++; }

	//////////////////////////////////////////////////////////////////////////

	//! ���б�ؼ���С�ı�ʱ���зֵõĸñ���������80%��
	int					percentage;
	int					leftPadding; ///< �ı���߾�
	align_type			textAlign;	///< �е��ı�����
	wxColour			textColor;	///< ���е��ı���ɫ
	wxFont				font; ///< ���л���ʱ���õ�����

	wxString			heading; ///< �еı���
	int					x; ///< ��������� VdkListCtrl ��x����ʼ����
	int					width; ///< �еĿ��
	int					height; ///< ��ÿ��Ԫ��ĸ߶�

	ListOfLcCell		cells; ///< �еĸ���

private:

	int					m_drawingVersion;

	DECLARE_NO_COPY_CLASS( VdkLcColumn )
};

typedef std::list< VdkLcColumn* > ListOfLcColumn;
typedef ListOfLcColumn::iterator LcColIter;

/// \brief �б�����ݼ�
class LcDataSet
{
public:

	/// ����������
	virtual ~LcDataSet();

	/// ɾ����������(�к���)
	virtual void RemoveAllColumns();

	/// ɾ������
	virtual void RemoveAllRows();

public:

	/*! �б��ĸ��� */
	ListOfLcColumn cols;
};

WX_DEFINE_SORTED_ARRAY_INT( int, ArrayOfSortedInts );

//////////////////////////////////////////////////////////////////////////

/// \brief Ϊ VdkLcColumn ���ó�ʼ����Ϣ
///
/// \attention �����������壡
class VdkLcColumnInitializer
{
public:

	/// \brief ���캯��
	/// \param lst �����ṩ��������б��ؼ�������ȷ����������
	VdkLcColumnInitializer(VdkListCtrl* lst);

	/// \brief ����Ҫ�����е����
	///
	/// �ɹ�����һ�к����е���ž���\a i
	VdkLcColumnInitializer& index(int i) { Index = i; return *this; }

	/// \brief ���б�ؼ���С�ı�ʱ���зֵõĸñ���
	/// \param p 0 <= p <= 100
	/// \note �Ⲣ���Ǵ�����ռ VdkListCtrl �ܿ�ȵ� \a p%��\n
	/// ���еĳ�ʼ��Ȼ���Ҫ��\a width ��ָ����
	VdkLcColumnInitializer& percentage(int p) {
		Percentage = p;
		return *this;
	}

	/// \brief �����ı���߾�
	VdkLcColumnInitializer& leftPadding(int p) {
		LeftPadding = p;
		return *this;
	}

	/// \brief ������ʾ��ͷ�ؼ�ʱ��ͷ��˵������
	VdkLcColumnInitializer& heading(const wxString& h) {
		Heading = h; return *this;
	}

	/// \brief �����еĳ�ʼ�п�
	VdkLcColumnInitializer& width(int w) { Width = w; return *this; }

	/// \brief �������ı��Ķ�������
	VdkLcColumnInitializer& textAlign(align_type a) {
		TextAlign = a;
		return *this;
	}

	/// \brief �������ı����ı���ɫ
	VdkLcColumnInitializer& textColor(const wxColour& color) {
		TextColor = color;
		return *this;
	}

	/// \brief �������ı�������
	///
	/// \attention �����������壡
	VdkLcColumnInitializer& font(const wxFont& f) {
		Font = f;
		return *this;
	}

private:

	int Index;
	int Percentage;
	int LeftPadding; // �ı���߾�
	wxString Heading;
	int Width;
	align_type TextAlign;
	wxColour TextColor;
	wxFont Font;

	friend class VdkListCtrl;
};

//////////////////////////////////////////////////////////////////////////

/*!\brief �б�ؼ�
 *
 * VdkListCtrl ���¼�ֻ��˫���б���ʱ�Żᷢ�ͣ��û�ѡ�е����к�
 * ����ͨ�� VdkVObjEvent �� SetClientData() ��Ա��ȡ��
 * \attention �ػ������ؼ���Ҫֱ�ӵ��� VdkControl::Draw(),
 * ��Ӧ�õ��� VdkScrolled::RefreshState ��
**/
class VdkListCtrl : public VdkScrolledWindow
{
public:

	/// \brief ���캯��
	/// \param style �б��ķ�����
	VdkListCtrl(long style = VCS_BORDER_SIMPLE | VLCS_HOVERING);

	/// \brief ��������
	~VdkListCtrl();

	/// \brief XRC ��̬����
	virtual void OnXrcCreate(wxXmlNode* node);

	/// \brief �����ؼ�
	void Create(VdkWindow* parent,
				const wxString& strName,
				const wxRect& rc);

	/// \brief ��\a o ��¡
	void Clone(VdkListCtrl* o, VdkControl* parent);

	/////////////////////////////////////////////////////////////////////////

	/// \brief ���ð󶨵����ݼ�
	/// \param dataSet Ҫ�󶨵����ݼ�
	/// \attention VdkListCtrl ����ӹ����ݼ�ָ�룬�û������ֶ�����
	void Attach(LcDataSet* dataSet);

	/// \brief �����Ѱ󶨵����ݼ�
	LcDataSet* Datach();

	/// \brief ���б��в���һ��
	void InsertColumn(const VdkLcColumnInitializer& init_data);

	/// \brief ���б�ĩβ����һ��
	/// 
	/// \return ����һ����ʱ������������ÿ�����Ϊ�²����е�ÿһ����Ԫ��
	/// ÿ���ڵ����һ����Ԫ��ָ������� VdkLcCell::GetCellClientData() ��
	/// ��ȡ��
	VdkLcCell* Append(const wxString& label, bool updateSize);

	/// \brief ���б��в���һ��
	/// \param index �ɹ�����һ�к����е���ž���\a index
	/// \param label ��һ�еı���
	/// \param updateSize �Ƿ������������⻭���Ĵ�С
	/// \note �����б��ж��У�����������б�����ʹ��
	/// \link SetCellLabel() \endlink ��
	VdkLcCell* InsertRow(size_t index, const wxString& label, bool updateSize);

	/// \brief �������⻭���Ĵ�С
	///
	/// �������Ż��������������������ÿ����һ�ж����ô���
	/// �����\link VdkScrolledWindow::SetVirtualSize \endlink ��
	void UpdateSize(wxDC* pDC);

	/// \brief ��յ�ǰ������
	///
	/// ��ǰ�ĸ����б��ֲ��䡣��ֻӰ���С�
	void Clear(wxDC* pDC);

	/// \brief ɾ���� \a index ��
	void RemoveRow(int index, bool updateSize, wxDC* pDC);

	/// \brief �����б�
	/// \a dX ��ҪԽ����������������
	/// \a dY ��ҪԽ����������������
	bool ScrollList(int dX, int dY, wxDC* pDC = NULL);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ѡ�е� \a index ��
	/// \param index �� 0 Ϊ��㡣
	void Select(int index, wxDC* pDC) {	Select( index, index + 1, pDC ); }

	/// \brief ѡ�� [\a first, \a last) ��
	/// \param first �� 0 Ϊ��㡣
	/// \param last �����������Ϊ wxNOT_FOUND����ѡ����ʼ��\a first 
	/// �����һ�
	void Select(int first, int last, wxDC* pDC);

	/// \brief ȫѡ��Ŀ
	void SelectAll(wxDC* pDC);

	/// \brief �����ѡ����б���
	void SelectNone(wxDC* pDC);

	/// \brief ����ѡ��
	void SelectReverse(wxDC* pDC);

	/// \brief ��ȡ������ѡ�����Ŀ
	///
	/// ������Ϊ��ѡ��Ŀ����ţ��������С�
	const ArrayOfSortedInts& GetSelectedItems() const {
		return m_selItems;
	}

	/// \brief ��ȡ���ѡ�е��еĵ�һ�еĵ�����
	LcCellIter GetLastSel() const;

	/// \brief ��ȡ���ѡ�е��е����
	int GetLastSelIndex() const;

	/// \brief �Ƿ�ѡ����������Ŀ
	bool IsAllSel() const;

	/// \brief ָ�����Ƿ��ѱ�ѡ��
	bool IsSelected(int index) const;

	/// \brief ���ص�ǰ�Ƿ�����Ŀ��ѡ��
	bool HasSelected() const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ȡ��һ�е�\a index �е�Ԫ�������
	wxString GetString(int index);

	/// \brief ��ȡ��\a row �С���\a col �е�Ԫ�������
	wxString GetCellLabel(int row, int col);

	/// \brief ����ָ����ָ����С����ı�
	void SetCellLabel(int row, int col, const wxString& label);

	/// \brief ����ָ����Ԫ����ı�
	void SetCellLabel(LcCellIter cell, const wxString& label);

	/// \brief ����ָ����ָ����С����û��Զ���ָ��
	void SetCellClientData(int row, int col, void* clientData);

	/// \brief ��ȡָ����ָ����С����û��Զ���ָ��
	void* GetCellClientData(int row, int col);

	//////////////////////////////////////////////////////////////////////////

	/// \brief �б��Ƿ�Ϊ��
	///
	/// ��û���κ��У�ӵ���е�û������Ȼ�����ա�
	bool IsEmpty() const;

	/// \brief �õ�����
	int GetItemCount() const { return GetRowCount(); }

	/// \brief �õ�����
	int GetRowCount() const;

	/// \brief �õ�����
	int GetColumnCount() const;

	/// \brief ����ĳһ�е��ı�ǰ��ɫ
	void SetColumnTextColor(int index, const wxColour& color);

	/// \brief ��ȡ��ʼ���
	virtual void GetViewStart(int* x, int* y) const;

	/// \brief ��ȡ�����ܵ���ʼ���
	virtual void GetMaxViewStart(int* x, int* y) const;

	/// \brief �õ��и�
	int GetRowHeight() const { return m_rowHeight; }

	/// \brief �����и�
	///
	/// �ṩ����ֵ����С������ĸ߶ȡ�
	void SetRowHeight(int height, wxDC* pDC = NULL);

	/// \brief �����п�
	///
	/// ��\a adjustFrom ����\a w ��ȸ�\a adjust ��
	/// \return �����Ƿ�ɹ���
	bool AjustCollumn(size_t w, size_t adjust, size_t adjustFrom);

	/// \brief ��ָ��\a cell �õ���ǰ��Ӧ���к�
	/// \see GetColumn0Iterator
	int IndexOf(const VdkLcCell* cell) const;

	/// \brief ��ȡָ����ָ����С����
	VdkLcCell* GetCell(size_t row, size_t col) const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief �������õ�ν�ʺ���
	typedef bool (*CellComparer)(const VdkLcCell*, const VdkLcCell*);

	/// \brief �����б��
	void Sort(int col, CellComparer comp);

protected:

	/// \brief ��������¼�
	virtual void OnMouseEvent(VdkMouseEvent& e);

	/// \brief ���ƿؼ�
	virtual void OnDraw(wxDC& dc);

	/// \brief ���ա�����֪ͨ��Ϣ
	virtual void OnNotify(const VdkNotify& notice);

	/// \brief ��Ӧ�û������¼�
	virtual void OnKeyEvent(VdkKeyEvent& vke);

	//////////////////////////////////////////////////////////////////////////

	/// \brief �ڴ˲��������ƣ��б���
	/// \param it ��Ҫ���������еĵ�һ�еĵ�����
	/// \param index ��Ҫ���������е����
	/// \return ָʾ VdkListCtrl �ĺ�����Ϊ
	/// \see VdkCusdrawReturnFlag
	/// \attention Ϊ������ܣ���ĳ��״̬��Ϣ�����е����������棬�����
	/// �����ʱӦ����\a it �����в��ԣ���ʹ���к������棬��Ӧ��\a index
	/// �����ԡ�
	virtual VdkCusdrawReturnFlag DoEraseRow
		(const LcCellIter& it, int index, wxDC& dc);

	/// \brief �ڴ˻���ÿһ����Ԫ��
	///
	/// Ҳ����ֻ�Ǹ��Ļ�ͼ�����ĵĸ������ԣ�������Ӧֵ���ɡ�
	virtual VdkCusdrawReturnFlag DoDrawCellText(const VdkLcCell* cell,
		int col_index, int index, wxDC& dc, VdkLcHilightState state);

	//////////////////////////////////////////////////////////////////////////
	// �����⼸������������������

	/// \brief ��ȡѡ�����ʼ�� ID
	int GetSelStart() const { return m_selStart; }

	/// \brief ��ȡ�϶���Ŀʱ��Ŀ��λ�� ID
	int GetLastDraggTarget() const { return m_lastDraggTarget; }

	/// \brief ��ĳһ��ֱ����\a y �ж�ָ���µ��б��� ID
	/// \param index ��Χ�����ڿ�������֮��
	/// \param indexMayOverflow ��Χ���ܻᳬ����������һ���У�һ�������ж��û�
	/// ��������Ƿ�λ�����Ŀհ������С�
	void GetIndex(int y, int& index, int& indexMayOverflow);

	/// \brief ��ȡָ����Ԫ��(\a col, \a row)�ĵ�����
	///
	/// ��Ϊ\a index �䶯�ܴ󣬱��������൱����ȫ��m_selItems ����
	/// ID ʵ��������ʵ���������飬��֮��ʷԭ�򣩣����б�������
	/// �������Ǳ��ֲ���ġ�����������������ȱ����������
	/// �����������ĵ���׷�� index �ĸ��ֱ䶯��
	LcCellIter GetCellIterator(int col, int row);

	/// \brief ��ȡָ����Ԫ��(\a col, \a row)�ĵ�����
	LcCellIter GetCellIterator(VdkLcColumn* col, int row);

	/// \brief ��ȡ�е�Ԫ�����һ���ĺ��
	/// \attention ע��õ�����һ��������ĵ�������
	LcCellIter GetColumnEnd(int col) const ;

	/// \brief �����£����㵱ǰ����������������ʾ����Ŀ��
	void CalcShownItems();

	/// \brief ��ǰ����������������ʾ����Ŀ��
	int GetShownItems() const { return m_shownItems; }

	/// \brief �����������Ĵ�ֱ��ʾ��Χ֮�»�Ҫ��ʾ����Ŀ��
	void SetShownItemsAddIn(int addin) { m_nShownItemsAddIn = addin; }

	/// \brief ��ȡ�������Ĵ�ֱ��ʾ��Χ֮�»�Ҫ��ʾ����Ŀ��
	int GetShownItemsAddin() const { return m_nShownItemsAddIn; }

	/// \brief ��ȡ��������ʼ�����к�
	/// 
	/// ��ĳЩ��̬�����£���Ҫ�����������ʱ��������һ�������У�
	/// ����һ������һ�����ص��ƶ�����ʱ��Ҫ�� yStep ���õ��㹻С����1px����
	/// ��ô VdkScrolledWindow::GetViewStart()  �õ���ֵ�Ͳ��ǵ�ǰ��
	/// ��ʼ�����кţ���Ҫʹ���������������������
	void FixViewStart(int* x, int* y) const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief ʹ������δ֪ DC ���ػ��ؼ����ݣ������¹�����״̬
	///
	/// ���ƿؼ���� DC ����ʼ��������Ͳü����򶼲����ţ������߱��������趨��
	void RefreshContent(wxDC* pDC);

	/// \brief ������״̬�������ػ���
	void UpdateRow(int index, wxDC& dc);

	/// \brief ������״̬�������ػ���
	/// \param it Ҫ���µ��еĵ�һ�еĵ�����
	void UpdateRow(const LcCellIter& it, wxDC& dc);

	/// \brief ���������ƣ��б���
	/// \param it ��Ҫ���������еĵ�һ�еĵ�����
	/// \param index ��Ҫ���������е���ţ���Ϊ wxNOT_FOUND ������Զ�����
	/// ��һ������\a it �Զ����м���
	/// \return ���ر������е��к�
	int EraseRow(const LcCellIter& it, int index, wxDC& dc);

	/// \brief ����һ�е��ı�
	///
	/// ��������ԭ���������������
	/// \attention ������Ч�ʱȽϵ��£�����ʹ�ã��Ƽ���;��
	/// \link UpdateRow \endlink ��
	void DrawRowText(int index, wxDC& dc);

private:

	// ���б��в���һ��
	// @param index �ɹ�����һ�к����е���ž���\a index ����Ϊ -1��
	// �����б�ĩβ���һ�С�
	VdkLcCell* DoInsertRow(int index, const wxString& label, bool updateSize);

	// ��յ�ǰ������
	//
	// ������ɸ�д������֪������¼������������ڵ�һʱ�䱻
	// ���á�
	virtual void DoClear(wxDC* pDC = NULL) {}

	// �ڴ˻���һ�У����Կ����еı���
	//
	// ÿ�λ���һ��
	// \param beg ��Ҫ���Ƶĵ�һ��
	// \param end ��Ҫ���Ƶ����һ��
	// \attention \a end �� STL һ����ָ����Чֵ����һ��λ�ã�ѭ����
	// ����ʹ�� <= ��
	void DrawColumn(const VdkLcColumn& col, 
					int col_index, 
					wxDC& dc, 
					unsigned beg, 
					unsigned end);

	// �������ѡ����
	// 
	// ��Ҫ��Ϊ��ֻ����һ���޸� m_lastSelected ��������ڡ�
	void UpdateLastSelected(const LcCellIter& iter);

	// �����û�ָ���ġ����б�ؼ���С�ı�ʱ���зֵõ�
	// �ı������ܺ��Ƿ�С�ڻ����100%
	// \param percentage ����ı������ܺʹ���100%��VdkListCtrl ��Դ�
	// ������������
	int TestWidthArragements(int percentage);

	// ����ס Shift ��ʱ�������µ������İ����¼�
	//
	// ģ���������������¼�������������
	void OnShiftUpDownKeys(int sel, wxDC& dc);

	// ʹ�� Ctrl + Home/End �ƶ����б�ͷ���β
	void OnCtrlPlusHomeEnd(VdkKeyEvent& e);

	//////////////////////////////////////////////////////////////////////////

	class ColumnListAdapter;

	// �б�ؼ��ı�ͷ�ؼ�
	class VdkLcHeader : public VdkControl
	{
	public:

		// ���캯��
		VdkLcHeader(VdkWindow* win,
					const wxString& strName,
					const wxRect& rc,
					VdkListCtrl* listctrl,
					ColumnListAdapter& cols);

	private:

		// ���ƿؼ�
		virtual void DoDraw(wxDC& dc);

	private:

		int					m_yFix;
		VdkListCtrl*		m_listctrl;
		ColumnListAdapter&	m_cols;

		wxBitmap			m_bmBkGnd;
	};

	int m_rowHeight; // �и�
	int m_shownItems; // ��ǰ�� VdkListCtrl �����ɶ�����
	// �������Ĵ�ֱ��ʾ��Χ֮�»�Ҫ��ʾ��������Ŀ��
	// �����������Զ������Ĺ������ڣ�����������ʹ�õ�ǰ����
	// ��һ���Ƴ��������غ󣬶����һ��ȴ��û�����û�����Ұ��
	int m_nShownItemsAddIn;

	VdkLcHeader* m_pHeader; // �Ƿ���ʾ�����ı�ͷ

	//////////////////////////////////////////////////////////////////////////
	// ������ VdkListCtrl �ĵ�ǰ״̬��Ϣ

	int m_lastHilighted;
	int m_selStart; // ����ѡ��ʱָ���ĵ�һ��� ID
	int m_lastDraggTarget; // ���϶���Ŀ��������ʱ��ǰ������Ŀ��
	LcCellIter m_lastSelected; // ���ѡ�е�һ��

	ArrayOfSortedInts m_selItems;

private:

	// �������������
	class ColumnListAdapter
	{
	public:

		// ���캯��
		ColumnListAdapter(LcDataSet* dataSet);

		// ���ð󶨵����ݼ�
		void Attach(LcDataSet* dataSet);

		// �����Ѱ󶨵����ݼ�
		LcDataSet* Datach();

		// �Ƿ�����ȷ�󶨿��õ����ݼ�
		bool IsOk() const;

	public:

		typedef ListOfLcColumn::value_type value_type;

		typedef LcColIter iterator;
		typedef ListOfLcColumn::const_iterator const_iterator;

		LcColIter begin();
		LcColIter end();

		ListOfLcColumn::const_iterator begin() const;
		ListOfLcColumn::const_iterator end() const;

		bool empty() const;
		ListOfLcColumn::size_type size() const;

		LcColIter insert(LcColIter it, VdkLcColumn* col);
		void push_back(VdkLcColumn* col);

	private:

		LcDataSet* m_dataSet;
	};

	ColumnListAdapter m_cols;

protected:

	// VdkListCtrl �ķ���壬������ɴ�ȡ
	wxPen m_draggTarget; // �϶���Ŀʱ��ʾĿ��λ�õ�������ɫ
	wxBrush m_hilighted; // ������ı�����ˢ
	wxBrush m_selected; // ѡ����ı�����ˢ
	wxBrush m_crossBrush1; // ������ɫ 1
	wxBrush m_crossBrush2; // ������ɫ 2

	DECLARE_CLONEABLE_VOBJECT( VdkListCtrl )
};
