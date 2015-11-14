/***************************************************************
 * Name:      VdkEdit.h
 * Purpose:   Code for VdkEdit declaration
 * Author:    vanxining (vanxining@139.com)
 * Created:   2011-03-14
 * Copyright: vanxining
 **************************************************************/
#pragma once
#include "VdkControl.h"
#include "VdkDC.h"
#include <wx/textentry.h>

#include <list>

//////////////////////////////////////////////////////////////////////////

class VdkEdit;
class VdkTextValidator;

/// \brief VdkEdit �ķ�����Լ�
enum VdkEditStyle {

	/// \brief �����������Լ�λ���
	VES_USER					= VCS_USER + 10,

	/// \brief �����ı����ı��ı��¼�
	VES_TEXT_CHANGED_MSG		= 1 << ( VCS_USER + 0 ),
};

/// \brief VdkEdit ������ʱ״̬���Լ�
enum VdkEditState {

	/// \brief ������״̬λ���
	VEST_USER					= VCST_USER + 10,

	/// \brief ���������ʾ
	///
	/// ��֮�෴��״̬�ǹ�����ô�������״̬
	VEST_CURSOR_ON				= 1 << ( VCST_USER + 0 ),

	/// \brief ���ڳ�����һ�εĲ���
	/// \note ��Ҫ�ǿ��ǵ�һ�β��롢ɾ������ַ���һ�β��롢
	/// ɾ��һ���ַ��Ĵ����в��졣�ڴ������ַ������ʱѹ��
	/// �����ַ�������������ɡ�
	VEST_UNDO_REDOING			= 1 << ( VCST_USER + 1 ),

	/// \brief ��ǰ����ľ�̬����Ƿ�λ��ȫ�ֳ���������
	VEST_SSTYLE_GLOBAL_STATIC	= 1 << ( VCST_USER + 2 ),
};

//////////////////////////////////////////////////////////////////////////

/// \brief Ϊ VdkLabel ���ó�ʼ����Ϣ
class VdkEditInitializer : public 
	VdkCtrlInitializer< VdkEditInitializer >
{
public:

	typedef VdkEditInitializer Edit;

	/// \brief ���캯��
	VdkEditInitializer();

	/// \brief ��ʼ�ı�
	Edit& text(const wxString& s) { Text = s; return *this; }

	/// \brief �ı���������
	Edit& textAlign(align_type a) { TextAlign = a; return *this; }

	/// \brief �ı���ɫ
	Edit& textColor(const wxColour& color) { TextColor = color; return *this; }

	/// \brief ������ɫ
	Edit& bgColor(const wxColour& color) { BgColor = color; return *this; }

	/// \brief ������֤��
	Edit& validator(const VdkTextValidator& v);

	/// \brief �Ƿ�����༭
	Edit& editable(bool b) { Editable = b; return *this; }

protected:

	wxString Text; ///< ��ʼ�ı�
	align_type TextAlign; ///< �ı���������
	wxColour TextColor; ///< �ı���ɫ
	wxColour BgColor; ///< ������ɫ
	VdkTextValidator* Validator; ///< ������֤��
	bool Editable; ///< �Ƿ�����༭

	friend class VdkEdit;
};

//////////////////////////////////////////////////////////////////////////

class VdkEditStaticStyle;

/*! ��ѡ��Ԥ���徲̬�������� */
enum VdkEditStaticStyleID {

	VESS_BEGIN, /*! �ڲ�ʹ�� */

	VESS_MS_WORD_2003, /*! ����Ϊ��ɫ��ѡ����ı�Ϊ����Ϊ��ɫ */
	VESS_MS_WINDOWS, /*! ����Ϊ��ɫ��ѡ����ı�Ϊ����Ϊ��ɫ */

	VESS_END, /*! �ڲ�ʹ�� */
};

//////////////////////////////////////////////////////////////////////////
/// \brief �Ի����ı���ؼ�
///
/// ��ܼ����鷳���Ի��ؼ���ԭ���ؼ����ʹ��ʱ�Ľ���ת�����⡣
class VdkEdit : public VdkControl,
				public wxTextEntryBase,
				public wxTimer, // ��Ϊ�¼�������Ƶ����ƣ����빫�м̳�
				public VdkDcPostListener
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkEdit();

	/// \brief ��������
	~VdkEdit();

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ʵ�ʹ�������
	void Create(const VdkEditInitializer& init_data);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��֤�û�������ı��Ƿ����Ҫ��
	bool Validate() const;

	/// \brief ����������֤��
	void SetValidator(const VdkTextValidator& v);

	/// \brief ��ȡ��ǰ�ı�ֵ
	wxString GetValue() const { return DoGetValue(); }

	/// \brief ���õ�ǰ�ı�ֵ
	void SetValue(const wxString& strText, wxDC* pDC = NULL);

	/// \brief ��ȡ�ı����ڵ��ַ�����
	int GetLength() const { return m_text.size(); }

	/// \brief ��ָ��λ��\a pos �������ı�
	/// \param pos ����Ϊ wxNOT_FOUND ����᳷���κ���ѡ����ı���
	/// ��������ƶ������һ����������ַ����档\n
	/// ��Ϊ wxNOT_FOUND �����ڵ�ǰ����㴦�����ı����滻����ѡ���ı���
	void Insert(const wxString& strText, long pos, wxDC* pDC = NULL);

	/// \breif ��ǰλ������ı�
	///
	/// ��ʹ��ѡ����ı��ÿա�
	virtual void WriteText(const wxString& text);

	/// \brief ɾ��[\a from, \a to)������ı�
	/// \note �������᳷���κ���ѡ����ı�����������ƶ���\a to ָ�����ַ���
	/// \attention ���䲻����\a to ��
	virtual void Remove(long from, long to) {
		Remove( from, to, NULL );
	}

	/// \brief ɾ���ı����ṩѡ���Ƿ��ػ��ؼ�
	void Remove(long from, long to, wxDC* pDC);

	/// \brief ������ѡ���ı�
	virtual void Copy();

	/// \brief �����ϴβ���
	void Undo(wxDC* pDC);

	/// \brief �����ϴβ���
	virtual void Redo() { Redo( NULL ); }

	/// \brief �����ϴβ���
	void Redo(wxDC* pDC);

	/// \brief �ܷ����ϴβ���
	virtual bool CanUndo() const;

	/// \brief �ܷ������ϴβ���
	virtual bool CanRedo() const;

	/// \brief ����ı�
	void Clear() { Clear( NULL ); }

	/// \brief ����ı�
	void Clear(wxDC* pDC);

	// insertion point
	// ---------------

	/// \brief ���õ�ǰ���λ��
	virtual void SetInsertionPoint(long pos) {
		SetInsertionPoint( pos, NULL );
	}

	/// \brief ������ƶ���ĩβ
	void MoveToEnd(wxDC* pDC);

	/// \brief ���õ�ǰ���λ��
	void SetInsertionPoint(long pos, wxDC* pDC);

	/// \brief ��ȡ��ǰ���λ��
	virtual long GetInsertionPoint() const;

	/// \brief ��ȡ�ϴι��λ��
	virtual long GetLastPosition() const;

	/// \brief ѡ�������ı�
	virtual void SelectAll() { SelectAll( NULL ); }

	/// \brief ѡ�������ı�
	void SelectAll(wxDC* pDC);

	/// \brief �������ǰ/���ƶ�
	void Forward(long step);

	/// \brief �������ǰ/���ƶ�
	void Backward(long step);

	/// \brief ѡȡ[\a from, \a to]������ı�
	void SetSelection(long from, long to, wxDC* pDC);

	/// \brief ��ȡѡȡ�����������
	virtual void GetSelection(long *from, long *to) const;

	/// \brief ��ȡ��ǰ�ѱ�ѡȡ���ı�
	virtual wxString GetStringSelection() const;

	// status
	// ------

	/// \brief �Ƿ�����༭
	virtual bool IsEditable() const;

	/// \brief ���ÿ�����༭״̬
	virtual void SetEditable(bool editable) {
		SetEditable( editable, NULL );
	}

	/// \brief ���ÿ�����༭״̬
	void SetEditable(bool editable, wxDC* pDC);

	// �������
	// ------

	/// \brief ���þ�̬���
	void SetStaticStyle(VdkEditStaticStyle* sstyle);

	/// \brief ���þ�̬���
	void SetStaticStyle(VdkEditStaticStyleID id);

	/// \brief ��ȡ��ǰ��ʹ�õľ�̬���
	VdkEditStaticStyle* GetStaticStyle() const;

public:

	/// \brief �Ƿ��ܹ�����
	virtual bool CanCopy() const;

	/// \brief �Ƿ��ܹ�����
	virtual bool CanCut() const;

	/// \brief �Ƿ��ܹ�ճ��
	virtual bool CanPaste() const;

protected:

	/// \brief ��ȡ�ı�����
	virtual wxString DoGetValue() const;

	/// \brief ��ȡԭ�����
	///
	/// override this to return the associated window, it will be used for event
	/// generation and also by generic hints implementation
	virtual wxWindow* GetEditableWindow() { return NULL; }

private:

	// ������ѡ���ı�
	virtual void Cut() { DoCut( NULL ); }
	// ճ���������е��ı�
	virtual void Paste() { DoPaste( NULL ); }

	// �����ϴβ���
	virtual void Undo() { Undo( NULL ); }

	// ѡȡ[\a from, \a to]������ı�
	virtual void SetSelection(long from, long to) {
		SetSelection( from, to, NULL );
	}

	// �Ƿ�����ִ�С�����������������
	bool IsOnUndoRedo() const;

private:

	// �����������
	void DrawCursor(wxDC& dc);

	// ������ʾ���
	void ShowCursorAtOnce(wxDC& dc, bool stopTimer = false);

	// �ƶ���굽�µ�λ��(����������Ļ�ϵ�λ��)
	void MoveCursorOnScreen(int cursorPos);

	// �Ƿ�Ӧ���������
	bool ShouldFlashCursor();

	// ִ���ύǰ��ͼ����
	virtual void OnPost(wxDC& dc);

	// ���� Timer ��ִ��
	void ResumeTimer();

	// �����ı��ı��¼�
	void SendTextChangedMsg(wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////

	// ���ƿؼ�
	virtual void DoDraw(wxDC& dc);

	// �̳��� wxTimer ���������
	virtual void Notify();

	// ��������¼�
	virtual void DoHandleMouseEvent(VdkMouseEvent& e);

	// ���ա�����֪ͨ��Ϣ
	virtual void DoHandleNotify(const VdkNotify& notice);

	// ������̰����¼�
	virtual void DoHandleKeyEvent(VdkKeyEvent& e);

	// �������˵��¼�
	void OnMenuEvent(VdkVObjEvent& e);

	//////////////////////////////////////////////////////////////////////////

	// ���ݵ�ǰ����ˢ������߶�
	int RefrshFontHeight();

	// ���� DC
	void PrepareDC(wxDC& dc);

	// ��ԭ������� DC
	void RestoreDC(wxDC& dc);

	// �����ı�
	void DrawCaption(wxDC& dc);

	// ��������ѻ��Ƶ����ݲ����ݿؼ�״̬���Ʊ߿�
	void ClearBgAndDrawBorder(wxDC& dc);

	// ��ȡ�ı���ʾ��Χ�Ŀ��
	int GetTextAreaSize() const;

	// ��ȡ��������Ŀ��
	// 
	// ע���� \link GetTextAreaSize \endlink �Ĳ�ͬ��
	// ���������迼�ǹ��λ�����ʱ��Ҫ��ʾ�ռ䡣
	int GetCanvasWidth() const;

	// ��ʼ���Ҽ��˵�
	void ImplementMenu();

	// �����Ҽ��˵�
	// \a x �Ը��������Ͻ�Ϊ������ʼ����
	// \a y �Ը��������Ͻ�Ϊ������ʼ����
	// \note һ����ԣ�DoHandleMouseEvent �õ���ָ�����꼴Ϊ
	// �Ը��������Ͻ�Ϊ������ʼ���ꡣ
	void PopupMenu(int x, int y);

	// ճ�����ػ�
	void DoPaste(wxDC* pDC);

	// ���в��ػ�
	void DoCut(wxDC* pDC);

	// ���浱ǰ״̬�����ڳ�������
	void SaveCurrentState();

private:

	bool m_editable; // �Ƿ�����༭
	VdkTextValidator* m_validator; // ������֤��
	VdkEditStaticStyle* m_sstyle; // �ı���ľ�̬���

	align_type m_TextAlign; // �ı���������

	int m_cursorSlice; // �������ʱ����
	int m_cursorSize; // ������ߵĿ��
	int m_textPadding; // �ı�֮��ļ�϶

	VdkCtrlId m_menuId0; // �˵��� ID ����ʼ ID ����������/������

	//////////////////////////////////////////////////////////////////////////
	// �ؼ���ͼ����

	int m_fontHeight; // ����߶�
	int m_yFix; // ʹ�ı��͹�������ʾ

	wxRect m_dcClippingBoxOrigin; // ���ڻָ� DC ԭ���ļ�������
	wxPoint m_dcOrigin; // ���ڻָ� DC ԭ����������ʼ����

	//////////////////////////////////////////////////////////////////////////
	// ��̬���Ĳ˵���״̬

	enum VdkEditMenuId {

		VEM_MENU_ENTITY = -1,
		VEM_UNDO,
		VEM_REDO,
		VEM_CUT,
		VEM_COPY,
		VEM_PASTE,
		VEM_DELETE,
		VEM_SELECT_ALL,

		//---------------------------------

		VEM_ITEM_COUNT = 7, // �˵�������
	};

	VdkMenuItem* m_menuItems[VEM_ITEM_COUNT]; // ���ı���������Ĳ˵���

private:

	// һ���ַ�(��Ԫģʽ)
	class Character
	{
	public:

		// ��ȡ�ַ�Ψһ���
		static Character* GetInstance(wxChar ch);

		// ɾ�������ѷ���ĶѶ���
		static void DeleteAllInstances();

		// �����ַ�
		void Draw(wxDC& dc, int x, int y);

		// ��ȡ������ַ�
		wxChar get() const { return m_char; }

	private:

		// ���캯��
		Character(wxChar ch) : m_char( ch ) {}

	private:

		wxChar m_char;

	private:

		WX_DECLARE_HASH_MAP( wxChar, Character*,
			wxIntegerHash, wxIntegerEqual, CharacterPool );
		typedef CharacterPool::iterator CharIter;

		static CharacterPool ms_pool;
	};

	// һ���ַ�(����� VdkEdit �Ķ�����Ϣ)
	class EditChar
	{
	public:

		// ���캯��
		EditChar(wxChar ch, wxDC& dc);

		// ���캯��
		EditChar(wxChar ch, wxCoord width, wxCoord height);

		// �����ַ�
		void Draw(wxDC& dc, wxCoord x, wxCoord y);

		// ��Ӧ����ı��¼�
		// \return �����µĿ�ȡ�
		int UpdateSize(wxDC& dc);

		// ��ȡ������ַ�
		wxChar get() const { return m_char->get(); }

		// ��ȡ�ַ����
		wxCoord GetWidth() const { return m_width; }

		// ��ȡ�ַ��߶�
		wxCoord GetHeight() const { return m_height; }

	private:

		Character* m_char;

		wxCoord m_width;
		wxCoord m_height;
	};

	typedef std::list< EditChar* > Characters;
	typedef Characters::iterator CharIter;
	typedef Characters::const_iterator CharIter_Const;

	//////////////////////////////////////////////////////////////////////////

	// �����ַ��������Ŀ�ͷ
	CharIter begin() { return m_text.begin(); }

	// �����ַ��������Ľ�β�ĳ�Խ
	CharIter end() { return m_text.end(); }

	// �����ַ��������Ŀ�ͷ
	CharIter_Const begin() const { return m_text.begin(); }

	// �����ַ��������Ľ�β�ĳ�Խ
	CharIter_Const end() const { return m_text.end(); }

	// ��ȡ���һ���������ַ��ĵ�����
	CharIter GetLastChar();

	// ��ȡָ��λ���ַ��ĵ�����
	CharIter GetCharIterator(long pos);

	// ��ȡ�ַ������������
	long IndexOf(const CharIter& i);

	// ��ȡָ���������ֹ������
	bool GetRangeInterators(long from, long to, CharIter& first, CharIter& last);

	//////////////////////////////////////////////////////////////////////////

	// �ڵ�ǰ�������һ���ַ�
	// \param �Ƿ񱣴浱ǰ״̬���ڡ�������������������
	// \note һ����ԣ�����ֻ�ǲ��뵥���ַ�һ�㶼��Ҫ��
	// �������������ַ�ʱҲ��Ҫÿ�β���һ���ַ���
	// ��ʱ��ֻ��Ҫ����������ĺ����ﱣ��״̬��
	void InsertChar(wxChar ch, bool saveUndo);

	// ɾ����ǰ���ǰ���ַ�
	void Backspace();

	// ɾ����ǰ������ַ�
	void Delete();

	// ��ȡ�ַ����
	void GetCharSize(wxChar ch, wxCoord* w, wxCoord* h);

	// ��ȡָ�� EditChar �������߾��ƫ��
	// \note �������ı���߾�
	int GetCharOffset(const CharIter& ch);

	// �ָ��ַ�
	//
	// ����������ܵĹ�����Ӱ��
	void RecoverChar(const CharIter& i, wxDC &dc);

	// ���õ�ǰ���λ��
	//
	// �����ػ������ؼ����������ṩ\a pDC �������ڱ�����֮�����
	// \link VdkControl::Draw \endlink��
	// \param pos ���λ���ĸ��ַ�֮���߼�λ��
	// \param cursorPos ������ʾ���꣬�������ı���߾࣬����Ϊ wxNOT_FOUND 
	// ���Զ�����
	// \param pDC ֻ���ػ���꣬�����ػ������ؼ�
	void SetInsertionPoint(const CharIter& pos, int cursorPos, wxDC* pDC);

	// ѡȡ[\a from, \a to)������ı�
	// \param to ���������ַ���
	void SetSelection(const CharIter& from, const CharIter& to, wxDC* pDC);

	// ѡ�������Ե��ı�
	void SelectSuccessiveText(wxDC* pDC);

	// �ַ�����
	enum CharType {

		CT_NONE, // δ��ʼ��������
		CT_SPACE, // �հ׷�
		CT_ALPHA, // Ӣ���ַ�������
		CT_NON_ENGLISH, // ��Ӣ���ַ��������Ա�ʾ����
		CT_PUNCTUATE, // ������
	};

	// ��ȡ�ַ�����
	CharType GetCharType(const CharIter& ch);

	//////////////////////////////////////////////////////////////////////////

	// ���Ե�ǰ������ʼ�����Ƿ�Ϻ�Ҫ��(������)
	void TestViewStart();

	// ά��������������ʼ�������ȷ��
	///
	// һ�����������һ�����ݵ�������ɾ���ַ��������ʼ���ꡣ
	void TestMaxViewStart();

	// ����ѡ���ı���չ��ָ��\a i �ַ�
	// \param cursorPos �������ı���߾࣬����Ϊ wxNOT_FOUND ���Զ�����
	void DraggTo(const CharIter& i, int cursorPos, wxDC* pDC);

	// �Ƿ������ѡ����ı�
	bool IsSelected() const;

	// ����ѡ���յ�(���κ�ѡ��)
	void ResetSelEnd();

	// ȡ��ѡ���κ�ѡ��
	// \param i ͬʱ������ƶ���\a i ǰ��
	// \param cursorPos ������λ��
	void SelectNone(const CharIter& i, int cursorPos, wxDC* pDC);

	// ɾ��������ѡ����ı�
	void RemoveSelected(wxDC* pDC);

	// ��ȡ��ѡ���ı�����ʼ����ĵ�������ʾ
	void GetSelRange(CharIter& first, CharIter& last) const;

	// ɾ��ָ�������ڵ��ı�
	//
	// ������\a last ָ�����ַ���ִ�к�Ὣ����ƶ���\a last ��
	// \param from �����ṩ��ʼ�ַ�����ţ���߱��泷����Ϣ��Ч�ʡ�
	void DoRemove(const CharIter& first, const CharIter& last, long from, wxDC* pDC);

private:

	//////////////////////////////////////////////////////////////////////////
	// �ؼ�״̬

	Characters m_text; // ��ǰ�ı�

	int m_cursorPos; // ��굱ǰλ�ã������ؼ���������ַ������ǵ����ȿ��ַ���
	CharIter m_cursorChar; // ��������ַ�

	int m_selEndPos; // ѡ��Χ�Ľ�������
	CharIter m_selEndChar; // ѡ��Χ�Ľ�����ĵ�һ���ַ����� STL �ĸ�����ͬ

	int m_total; // �ַ����ܳ���
	int m_xViewStart; // һ����ʾ����ʱ��ʼ��������

	//////////////////////////////////////////////////////////////////////////
	// �����༭��ʵ��

	// Ϊ�������������״̬��Ϣ
	class Action
	{
	public:

		// ���캯��
		Action(VdkEdit* edit, long insertPoint)
			: m_edit( edit ), m_insertPoint( insertPoint )
		{

		}

		// ����������
		virtual ~Action() {}

		// ��������
		virtual void Exec(wxDC* pDC) = 0;

		// �ظ�����
		virtual void UnExec(wxDC* pDC) = 0;

	protected:

		VdkEdit* m_edit; // �ı���ؼ�
		long m_insertPoint; // ���������Ĳ����
	};

	int m_maxUndo; // ���ɡ��������Ĳ���

	typedef wxVector< Action* > Actions;
	typedef Actions::iterator ActionIter;

	Actions m_actions; // �������������������б�
	int m_nextUndo; // ��һ�Ρ���������ִ�еĲ���

	//////////////////////////////////////////////////////////////////////////

	// Ϊ����ɾ���ַ��Ĳ�������״̬��Ϣ
	class InsRmv : public Action
	{
	public:

		// ����������
		enum ActionType {
			IRT_INSERT, // �����ı�
			IRT_DEL,	// ʹ��ɾ����ɾ�������ַ�
			IRT_BACK,	// ʹ���˸��ɾ�������ַ�
			IRT_REMOVE,	// ɾ������ַ�
		};

		// ���캯��
		InsRmv(VdkEdit* edit, long insertPoint, wxString operand, ActionType type)
			: Action( edit, insertPoint ), m_operand( operand ), m_type( type )
		{
			wxASSERT( m_operand.length() > 0 );
		}

		// ��������
		virtual void Exec(wxDC* pDC);

		// �ظ�����
		virtual void UnExec(wxDC* pDC);

	private:

		// ��������
		void UndoIns(wxDC* pDC);

		// ����ɾ��
		void UndoDel(wxDC* pDC);

		// �������Ϊ�Ƿ�ɾ���ַ�
		bool IsDelOperation() const {
			return m_type != IRT_INSERT;
		}

		//////////////////////////////////////////////////////////////////////////

		wxString m_operand; // ���������ַ�
		ActionType m_type; // ����������
	};

	// ���á���������Ϊ����
	void SetUndoAction(Action* action);

	// ����ɾ���ַ�ʱ�ġ���������Ϣ
	//
	// ��Ϊɾ���֡��˸񡱺͡�ɾ������
	void SaveDeleteCharState(const CharIter& insertPoint, InsRmv::ActionType type);

	//////////////////////////////////////////////////////////////////////////

	static size_t ms_editCount; // ����ֵ��Ϊ 0 ʱɾ���ѷ���� Character �Ѷ���

	DECLARE_CLONEABLE_VOBJECT( VdkEdit )
};

/// \brief ��ʽ����������
VdkEdit& operator << (VdkEdit& edit, int num);

//////////////////////////////////////////////////////////////////////////

/// \brief VdkEdit �ľ�̬���
class VdkEditStaticStyle
{
public:

	/// \brief Ĭ�Ϲ��캯��
	VdkEditStaticStyle();

	/// \brief �����ı���ɫ
	VdkEditStaticStyle& textColor(const wxColour& color) {
		m_TextColor = color;
		return *this;
	}

	/// \brief ���ñ�����ˢ
	VdkEditStaticStyle& bgBrush(const wxBrush& brush) {
		m_BgBrush = brush;
		return *this;
	}

	/// \brief ������ѡ���ı��ı�����ˢ
	VdkEditStaticStyle& selTextBgBrush(const wxBrush& brush) {
		m_SelTextBgBrush = brush;
		return *this;
	}

	/// \brief ���ò��ɱ༭ʱ�ı�����ˢ
	VdkEditStaticStyle& uneditableBgBrush(const wxBrush& brush) {
		m_UneditableBgBrush = brush;
		return *this;
	}

	/// \brief ���ñ߿���ʽ
	VdkEditStaticStyle& normalBorder(const wxPen& pen) {
		m_NormalBorder = pen;
		return *this;
	}

	/// \brief ���ñ߿���ʽ
	VdkEditStaticStyle& innerBorder(const wxPen& pen) {
		m_InnerBorder = pen;
		return *this;
	}

	/// \brief ���ñ߿���ʽ
	VdkEditStaticStyle& outerBorder(const wxPen& pen) {
		m_OuterBorder = pen;
		return *this;
	}

	typedef void (*Resetter)();

	/// \brief �������ü���������
	VdkEditStaticStyle& resetter(Resetter rst) {
		m_Resetter = rst;
		return *this;
	}

	/// \brief ���ü�����һ
	void decRef() {
		
		if( m_Resetter )
			m_Resetter();
		else
			delete this;
	}

private:

	wxColour m_TextColor; // �ı���ɫ

	// �߿���ʽ(��ɫ��������״��)
	wxPen m_NormalBorder; // ����״̬�±߽�
	wxPen m_InnerBorder; // ����ʱ�ڱ߽�
	wxPen m_OuterBorder; // ����ʱ��߽�

	wxBrush m_BgBrush; // ������ˢ
	wxBrush m_SelTextBgBrush; // ��ѡ���ı��ı�����ˢ
	wxBrush m_UneditableBgBrush; // ���ɱ༭ʱ�ı�����ˢ

	Resetter m_Resetter; // ���ü���������

	friend class VdkEdit;
};
