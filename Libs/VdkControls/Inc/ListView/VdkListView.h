/***************************************************************
 * Name:      VdkListView.h
 * Purpose:   �� Android ���б�ؼ����ܶ�̬��ȡ����
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-03
 **************************************************************/
#pragma once
#include "VdkScrolledWindow.h"
class IListViewPlaugin;

/// �� Android ���б�ؼ����ܶ�̬��ȡ����
class VdkListView : public VdkScrolledWindow
{
public:

	/// ����������
	class Adapter
	{
	public:

		/// ����������
		virtual ~Adapter() {}

		/// ���ݼ��Ƿ�Ϊ��
		virtual bool IsEmpty() const = 0;

		/// ��ȡ���ݼ����������Ŀ����
		virtual size_t GetRowCount() const = 0;

		/// ��ȡ���ݼ����������
		virtual size_t GetColumnCount() const = 0;

		/// ��ȡĳ����Ԫ���ڵ��ı�
		virtual wxString GetCell(size_t row, size_t col) const = 0;

	public:

		enum {
			MATCH_PARENT = -1, ///< δָ�����п�ȣ�δʵ��
		};

		/// һ�еķ����
		struct ColumnStyle
		{
			wxString heading; ///< �еı���
			int width; ///< ���еĵ�ǰ���
			/// ���б�ؼ���С�ı�ʱ���зֵõĸñ�������80%��
			int extendingPercentage;
			align_type textAlign;	///< �е��ı�����
		};

		/// ��ȡ�з����
		virtual ColumnStyle& GetColumnStyle(size_t col) = 0;
	};

	//////////////////////////////////////////////////////////////////////////

	/// ���캯��
	/// @param style �б��ķ�����
	VdkListView(long style = 0);

	/// ��������
	~VdkListView();
	
	/// XRC ��̬����
	virtual void OnXrcCreate(wxXmlNode* node);

	/// �����ؼ�
	void Create(VdkWindow* parent,
				const wxString& strName,
				const wxRect& rc);

	//////////////////////////////////////////////////////////////////////////

	/*! ���������������ڹ���ģʽ */
	enum AdapterLifeTimePolicy {
		ALTP_HELD, ///< VdkListView �ӹ����������� VdkListView ɾ��ʱҲɾ��������
		ALTP_STATIC, ///< �ⲿ��̬����
	};

	/// ��������������
	/// @param adapter ������ָ�룬��������Ϊ��
	/// @param altp ���������������ڹ���ģʽ
	void SetAdapter(Adapter* adapter, AdapterLifeTimePolicy altp);

	/// �������⻭����С
	void UpdateSize(wxDC* pDC);

	/// ��ȡ�Ѱ󶨵�������
	Adapter* GetAdapter() const { return m_adapter; }

	/// ɾ���Ѱ󶨵�����������(���ͷ�ռ�õ��ڴ�)
	void DestroyAdapter() { wxDELETE( m_adapter ); }

	//////////////////////////////////////////////////////////////////////////

	/// ����������������֪ͨ�б�����Ϊ@a row ���������
	void NotifyItemAdd(size_t row);

	/// ����������������֪ͨ�б�����Ϊ@a row ����Ƴ�
	void NotifyItemRemove(size_t row);

	/// ����������������֪ͨ�б��������������ѱ��Ƴ�
	void NotifyClear();

	//////////////////////////////////////////////////////////////////////////

	typedef IListViewPlaugin * PluginPtr;

	/// ���һ�����
	void AddPlugin(PluginPtr plugin);

	/// �Ƴ����ʱ�Բ������ִ�еĲ���
	enum DestoryPolicy {
		PDP_LEAVE_IT_AS_IS, ///< ����ָ��
		PDP_DESTORY, ///< ˳��ɾ���������
	};

	/// �Ƴ�ָ�����
	void RemovePlugin(const wxString& name, DestoryPolicy dp);

	/// ɾ�����в������
	void DestroyAllPlugins();

public:

	/// ��ǰ����������������ʾ����Ŀ��
	int GetShownItems() const { return m_shownItems; }

	/// �õ��и�
	int GetRowHeight() const { return m_rowHeight; }

	/// �����и�
	///
	/// �ṩ����ֵ����С������ĸ߶ȡ�
	void SetRowHeight(int height, wxDC* pDC = NULL);

	/// ��ȡ���Ƶ�Ԫ��ʱ���౾�п�ʼ����ľ���
	static size_t GetCellLeftPadding();

public:

	/// ��ȡĬ�ϱ�����ˢ
	wxBrush GetBackgroundBrush() const { return m_background; }
	void SetBackgroundBrush(const wxBrush& brush) { m_background = brush; }

	// ������״̬�������ػ���
	void UpdateRow(int index, wxDC& dc);

protected:

	/// ��������¼�
	virtual void OnMouseEvent(VdkMouseEvent& e);

	/// ��Ӧ�û������¼�
	virtual void OnKeyEvent(VdkKeyEvent& ke);

	/// ���ƿؼ�
	virtual void OnDraw(wxDC& dc);

	/// ���������ƣ��б���
	void EraseRow(int index, wxDC& dc);

	/// ����һ�е��ı�
	///
	/// ��������ԭ��ѡ���������������
	/// @attention ������Ч�ʱȽϵ��£�����ʹ�ã�
	/// �Ƽ���;:\link UpdateRow \endlink
	void DrawRowText(int row, wxDC& dc);

	/// ������(@a x, @a y)����ʼ���Ƶ�Ԫ��(@a col, @a row)���ı�
	///
	/// ���Զ�������������䡢���롢���ı��ضϵ����⡣
	void DrawCellText(wxDC& dc, size_t row, size_t col, int x, int y);

	/// ���� X �������ı����롢���ı��ض�
	/// @param col �ı�������
	/// @param text Ҫ��ʾ���ı�������ʱ�ᱻ�ض�
	/// @return �ı���ͼ���� x ���������ֵ
	int CalTextXAxisProp(wxDC& dc, size_t col, wxString& text);

protected:

	/// ��ĳһ��ֱ����\a y �ж�ָ���µ��б��� ID
	/// @return ������Ƿ��ϸ��ڿ�������֮�ڣ��������˿ؼ�
	/// ���Ŀհ״�
	bool GetIndex(int y, int* rowAtPointer);

private:

	// �����£����㵱ǰ����������������ʾ����Ŀ��
	void CalcShownItems();

	// ���ա�����֪ͨ��Ϣ
	// ������ VdkScrolledWindow
	virtual void OnNotify(const VdkNotify& notice);

	/// @brief �ڴ˲��������ƣ��б���
	/// @return ָʾ VdkListCtrl �ĺ�����Ϊ
	/// @see VdkCusdrawReturnFlag
	virtual VdkCusdrawReturnFlag DoEraseRow(int index, wxDC& dc)
	{
		return VCCDRF_DODEFAULT;
	}

	enum DataSetChangeAction {
		DSCA_ADD,
		DSCA_REMOVE,
		DSCA_CLEAR,
	};

	// �������ݼ��仯
	void OnDataSetChange(DataSetChangeAction dsca, size_t row);

private:

	long m_style;
	Adapter* m_adapter;
	bool m_held;

	int m_rowHeight; // �и�
	int m_shownItems; // ��ǰ�� VdkListCtrl �����ɶ�����

	// TODO:
	// �������Ĵ�ֱ��ʾ��Χ֮�»�Ҫ���ƶ�������Ŀ��
	// �����������Զ������Ĺ������ڣ�����������ʹ�õ�ǰ����
	// ��һ���Ƴ��������غ󣬶����һ�е���һ��ȴ��û����
	// �û�����Ұ��
	int m_nShownItemsAddIn;

	wxBrush m_background; // Ĭ�ϱ�����ˢ
	wxVector< PluginPtr > m_plugins;

	DECLARE_DYNAMIC_VOBJECT
};
