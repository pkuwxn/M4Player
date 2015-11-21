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

/// \brief VdkEdit 的风格属性集
enum VdkEditStyle {

    /// \brief 派生类风格属性集位序号
    VES_USER                    = VCS_USER + 10,

    /// \brief 发送文本框文本改变事件
    VES_TEXT_CHANGED_MSG        = 1 << (VCS_USER + 0),
};

/// \brief VdkEdit 的运行时状态属性集
enum VdkEditState {

    /// \brief 派生类状态位序号
    VEST_USER                   = VCST_USER + 10,

    /// \brief 光标正在显示
    ///
    /// 与之相反的状态是光标正好处于隐藏状态
    VEST_CURSOR_ON              = 1 << (VCST_USER + 0),

    /// \brief 正在撤销上一次的操作
    /// \note 主要是考虑到一次插入、删除多个字符与一次插入、
    /// 删除一个字符的处理有差异。在处理多个字符的情况时压制
    /// 单个字符撤销对象的生成。
    VEST_UNDO_REDOING           = 1 << (VCST_USER + 1),

    /// \brief 当前保存的静态风格是否位于全局常量数据区
    VEST_SSTYLE_GLOBAL_STATIC   = 1 << (VCST_USER + 2),
};

//////////////////////////////////////////////////////////////////////////

/// \brief 为 VdkLabel 设置初始化信息
class VdkEditInitializer : public
    VdkCtrlInitializer<VdkEditInitializer> {
public:

    typedef VdkEditInitializer Edit;

    /// \brief 构造函数
    VdkEditInitializer();

    /// \brief 初始文本
    Edit &text(const wxString &s) {
        Text = s;
        return *this;
    }

    /// \brief 文本对齐属性
    Edit &textAlign(align_type a) {
        TextAlign = a;
        return *this;
    }

    /// \brief 文本颜色
    Edit &textColor(const wxColour &color) {
        TextColor = color;
        return *this;
    }

    /// \brief 背景颜色
    Edit &bgColor(const wxColour &color) {
        BgColor = color;
        return *this;
    }

    /// \brief 输入验证器
    Edit &validator(const VdkTextValidator &v);

    /// \brief 是否允许编辑
    Edit &editable(bool b) {
        Editable = b;
        return *this;
    }

protected:

    wxString Text; ///< 初始文本
    align_type TextAlign; ///< 文本对齐属性
    wxColour TextColor; ///< 文本颜色
    wxColour BgColor; ///< 背景颜色
    VdkTextValidator *Validator; ///< 输入验证器
    bool Editable; ///< 是否允许编辑

    friend class VdkEdit;
};

//////////////////////////////////////////////////////////////////////////

class VdkEditStaticStyle;

/*! 可选的预定义静态背景定义 */
enum VdkEditStaticStyleID {

    VESS_BEGIN, /*! 内部使用 */

    VESS_MS_WORD_2003, /*! 背景为白色，选择的文本为背景为黑色 */
    VESS_MS_WINDOWS, /*! 背景为白色，选择的文本为背景为蓝色 */

    VESS_END, /*! 内部使用 */
};

//////////////////////////////////////////////////////////////////////////
/// \brief 自画的文本框控件
///
/// 规避极度麻烦的自画控件与原生控件混合使用时的焦点转移问题。
class VdkEdit : public VdkControl,
    public wxTextEntryBase,
    public wxTimer, // 因为事件处理机制的限制，必须公有继承
    public VdkDcPostListener {
public:

    /// \brief 默认构造函数
    VdkEdit();

    /// \brief 析构函数
    ~VdkEdit();

    /// \brief XRC 动态创建
    virtual void Create(wxXmlNode *node);

    /// \brief 实际构建函数
    void Create(const VdkEditInitializer &init_data);

    //////////////////////////////////////////////////////////////////////////

    /// \brief 验证用户输入的文本是否符合要求
    bool Validate() const;

    /// \brief 设置输入验证器
    void SetValidator(const VdkTextValidator &v);

    /// \brief 获取当前文本值
    wxString GetValue() const {
        return DoGetValue();
    }

    /// \brief 设置当前文本值
    void SetValue(const wxString &strText, wxDC *pDC = NULL);

    /// \brief 获取文本框内的字符数量
    int GetLength() const {
        return m_text.size();
    }

    /// \brief 在指定位置\a pos 处插入文本
    /// \param pos 若不为 wxNOT_FOUND ，则会撤销任何已选择的文本，
    /// 并将光标移动到最后一个被插入的字符后面。\n
    /// 若为 wxNOT_FOUND ，则在当前插入点处插入文本，替换掉已选择文本。
    void Insert(const wxString &strText, long pos, wxDC *pDC = NULL);

    /// \breif 向当前位置添加文本
    ///
    /// 会使已选择的文本置空。
    virtual void WriteText(const wxString &text);

    /// \brief 删除[\a from, \a to)区间的文本
    /// \note 本函数会撤销任何已选择的文本，并将光标移动到\a to 指定的字符。
    /// \attention 区间不包含\a to 。
    virtual void Remove(long from, long to) {
        Remove(from, to, NULL);
    }

    /// \brief 删除文本，提供选项是否重画控件
    void Remove(long from, long to, wxDC *pDC);

    /// \brief 复制已选择文本
    virtual void Copy();

    /// \brief 撤销上次操作
    void Undo(wxDC *pDC);

    /// \brief 重做上次操作
    virtual void Redo() {
        Redo(NULL);
    }

    /// \brief 重做上次操作
    void Redo(wxDC *pDC);

    /// \brief 能否撤销上次操作
    virtual bool CanUndo() const;

    /// \brief 能否重做上次操作
    virtual bool CanRedo() const;

    /// \brief 清空文本
    void Clear() {
        Clear(NULL);
    }

    /// \brief 清空文本
    void Clear(wxDC *pDC);

    // insertion point
    // ---------------

    /// \brief 设置当前光标位置
    virtual void SetInsertionPoint(long pos) {
        SetInsertionPoint(pos, NULL);
    }

    /// \brief 将光标移动到末尾
    void MoveToEnd(wxDC *pDC);

    /// \brief 设置当前光标位置
    void SetInsertionPoint(long pos, wxDC *pDC);

    /// \brief 获取当前光标位置
    virtual long GetInsertionPoint() const;

    /// \brief 获取上次光标位置
    virtual long GetLastPosition() const;

    /// \brief 选择所有文本
    virtual void SelectAll() {
        SelectAll(NULL);
    }

    /// \brief 选择所有文本
    void SelectAll(wxDC *pDC);

    /// \brief 将光标向前/后移动
    void Forward(long step);

    /// \brief 将光标向前/后移动
    void Backward(long step);

    /// \brief 选取[\a from, \a to]区间的文本
    void SetSelection(long from, long to, wxDC *pDC);

    /// \brief 获取选取区间的上下限
    virtual void GetSelection(long *from, long *to) const;

    /// \brief 获取当前已被选取的文本
    virtual wxString GetStringSelection() const;

    // status
    // ------

    /// \brief 是否允许编辑
    virtual bool IsEditable() const;

    /// \brief 设置可允许编辑状态
    virtual void SetEditable(bool editable) {
        SetEditable(editable, NULL);
    }

    /// \brief 设置可允许编辑状态
    void SetEditable(bool editable, wxDC *pDC);

    // 界面绘制
    // ------

    /// \brief 设置静态风格
    void SetStaticStyle(VdkEditStaticStyle *sstyle);

    /// \brief 设置静态风格
    void SetStaticStyle(VdkEditStaticStyleID id);

    /// \brief 获取当前正使用的静态风格
    VdkEditStaticStyle *GetStaticStyle() const;

public:

    /// \brief 是否能够复制
    virtual bool CanCopy() const;

    /// \brief 是否能够剪切
    virtual bool CanCut() const;

    /// \brief 是否能够粘贴
    virtual bool CanPaste() const;

protected:

    /// \brief 获取文本内容
    virtual wxString DoGetValue() const;

    /// \brief 获取原生句柄
    ///
    /// override this to return the associated window, it will be used for event
    /// generation and also by generic hints implementation
    virtual wxWindow *GetEditableWindow() {
        return NULL;
    }

private:

    // 剪切已选择文本
    virtual void Cut() {
        DoCut(NULL);
    }
    // 粘贴剪贴板中的文本
    virtual void Paste() {
        DoPaste(NULL);
    }

    // 撤销上次操作
    virtual void Undo() {
        Undo(NULL);
    }

    // 选取[\a from, \a to]区间的文本
    virtual void SetSelection(long from, long to) {
        SetSelection(from, to, NULL);
    }

    // 是否正在执行“撤销”、“重做”
    bool IsOnUndoRedo() const;

private:

    // 绘制闪动光标
    void DrawCursor(wxDC &dc);

    // 立即显示光标
    void ShowCursorAtOnce(wxDC &dc, bool stopTimer = false);

    // 移动光标到新的位置(并更新在屏幕上的位置)
    void MoveCursorOnScreen(int cursorPos);

    // 是否应该闪动光标
    bool ShouldFlashCursor();

    // 执行提交前绘图操作
    virtual void OnPost(wxDC &dc);

    // 继续 Timer 的执行
    void ResumeTimer();

    // 发送文本改变事件
    void SendTextChangedMsg(wxDC *pDC);

    //////////////////////////////////////////////////////////////////////////

    // 绘制控件
    virtual void DoDraw(wxDC &dc);

    // 继承自 wxTimer ，闪动光标
    virtual void Notify();

    // 处理鼠标事件
    virtual void DoHandleMouseEvent(VdkMouseEvent &e);

    // 接收、处理通知信息
    virtual void DoHandleNotify(const VdkNotify &notice);

    // 处理键盘按键事件
    virtual void DoHandleKeyEvent(VdkKeyEvent &e);

    // 处理弹出菜单事件
    void OnMenuEvent(VdkVObjEvent &e);

    //////////////////////////////////////////////////////////////////////////

    // 根据当前字体刷新字体高度
    int RefrshFontHeight();

    // 适配 DC
    void PrepareDC(wxDC &dc);

    // 还原已适配的 DC
    void RestoreDC(wxDC &dc);

    // 绘制文本
    void DrawCaption(wxDC &dc);

    // 清除所有已绘制的内容并根据控件状态绘制边框
    void ClearBgAndDrawBorder(wxDC &dc);

    // 获取文本显示范围的宽度
    int GetTextAreaSize() const;

    // 获取剪切区域的宽度
    //
    // 注意与 \link GetTextAreaSize \endlink 的不同：
    // 本函数还需考虑光标位于最后时需要显示空间。
    int GetCanvasWidth() const;

    // 初始化右键菜单
    void ImplementMenu();

    // 弹出右键菜单
    // \a x 以父窗口左上角为计量起始坐标
    // \a y 以父窗口左上角为计量起始坐标
    // \note 一般而言，DoHandleMouseEvent 得到的指针坐标即为
    // 以父窗口左上角为计量起始坐标。
    void PopupMenu(int x, int y);

    // 粘贴并重画
    void DoPaste(wxDC *pDC);

    // 剪切并重画
    void DoCut(wxDC *pDC);

    // 保存当前状态，用于撤销操作
    void SaveCurrentState();

private:

    bool m_editable; // 是否允许编辑
    VdkTextValidator *m_validator; // 输入验证器
    VdkEditStaticStyle *m_sstyle; // 文本框的静态风格

    align_type m_TextAlign; // 文本对齐属性

    int m_cursorSlice; // 光标闪动时间间隔
    int m_cursorSize; // 光标竖线的宽度
    int m_textPadding; // 文本之间的间隙

    VdkCtrlId m_menuId0; // 菜单项 ID 的起始 ID ，即“撤销/重做”

    //////////////////////////////////////////////////////////////////////////
    // 控件绘图属性

    int m_fontHeight; // 字体高度
    int m_yFix; // 使文本和光标居中显示

    wxRect m_dcClippingBoxOrigin; // 用于恢复 DC 原来的剪切区域
    wxPoint m_dcOrigin; // 用于恢复 DC 原来的物理起始坐标

    //////////////////////////////////////////////////////////////////////////
    // 动态更改菜单项状态

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

        VEM_ITEM_COUNT = 7, // 菜单项总数
    };

    VdkMenuItem *m_menuItems[VEM_ITEM_COUNT]; // 与文本框相关联的菜单项

private:

    // 一个字符(享元模式)
    class Character {
    public:

        // 获取字符唯一句柄
        static Character *GetInstance(wxChar ch);

        // 删除所有已分配的堆对象
        static void DeleteAllInstances();

        // 绘制字符
        void Draw(wxDC &dc, int x, int y);

        // 获取保存的字符
        wxChar get() const {
            return m_char;
        }

    private:

        // 构造函数
        Character(wxChar ch) : m_char(ch) {}

    private:

        wxChar m_char;

    private:

        WX_DECLARE_HASH_MAP(wxChar, Character *,
                            wxIntegerHash, wxIntegerEqual, CharacterPool);
        typedef CharacterPool::iterator CharIter;

        static CharacterPool ms_pool;
    };

    // 一个字符(添加了 VdkEdit 的额外信息)
    class EditChar {
    public:

        // 构造函数
        EditChar(wxChar ch, wxDC &dc);

        // 构造函数
        EditChar(wxChar ch, wxCoord width, wxCoord height);

        // 绘制字符
        void Draw(wxDC &dc, wxCoord x, wxCoord y);

        // 响应字体改变事件
        // \return 返回新的宽度。
        int UpdateSize(wxDC &dc);

        // 获取保存的字符
        wxChar get() const {
            return m_char->get();
        }

        // 获取字符宽度
        wxCoord GetWidth() const {
            return m_width;
        }

        // 获取字符高度
        wxCoord GetHeight() const {
            return m_height;
        }

    private:

        Character *m_char;

        wxCoord m_width;
        wxCoord m_height;
    };

    typedef std::list<EditChar *> Characters;
    typedef Characters::iterator CharIter;
    typedef Characters::const_iterator CharIter_Const;

    //////////////////////////////////////////////////////////////////////////

    // 返回字符迭代器的开头
    CharIter begin() {
        return m_text.begin();
    }

    // 返回字符迭代器的结尾的超越
    CharIter end() {
        return m_text.end();
    }

    // 返回字符迭代器的开头
    CharIter_Const begin() const {
        return m_text.begin();
    }

    // 返回字符迭代器的结尾的超越
    CharIter_Const end() const {
        return m_text.end();
    }

    // 获取最后一个有意义字符的迭代器
    CharIter GetLastChar();

    // 获取指定位置字符的迭代器
    CharIter GetCharIterator(long pos);

    // 获取字符迭代器的序号
    long IndexOf(const CharIter &i);

    // 获取指定区间的起止迭代器
    bool GetRangeInterators(long from, long to, CharIter &first, CharIter &last);

    //////////////////////////////////////////////////////////////////////////

    // 在当前光标后插入一个字符
    // \param 是否保存当前状态用于“撤销”、“重做”。
    // \note 一般而言，假如只是插入单个字符一般都需要，
    // 但大批量插入字符时也需要每次插入一个字符，
    // 这时候只需要在批量插入的函数里保存状态。
    void InsertChar(wxChar ch, bool saveUndo);

    // 删除当前光标前的字符
    void Backspace();

    // 删除当前光标后的字符
    void Delete();

    // 获取字符宽度
    void GetCharSize(wxChar ch, wxCoord *w, wxCoord *h);

    // 获取指定 EditChar 相对于左边距的偏移
    // \note 不包括文本左边距
    int GetCharOffset(const CharIter &ch);

    // 恢复字符
    //
    // 包括清除可能的光标残留影像。
    void RecoverChar(const CharIter &i, wxDC &dc);

    // 设置当前光标位置
    //
    // 如需重画整个控件，请请勿提供\a pDC ，而是在本函数之后调用
    // \link VdkControl::Draw \endlink。
    // \param pos 光标位于哪个字符之后，逻辑位置
    // \param cursorPos 光标的显示坐标，不包括文本左边距，假如为 wxNOT_FOUND
    // 则自动计算
    // \param pDC 只会重画光标，不会重画整个控件
    void SetInsertionPoint(const CharIter &pos, int cursorPos, wxDC *pDC);

    // 选取[\a from, \a to)区间的文本
    // \param to 不包括此字符。
    void SetSelection(const CharIter &from, const CharIter &to, wxDC *pDC);

    // 选择连续性的文本
    void SelectSuccessiveText(wxDC *pDC);

    // 字符类型
    enum CharType {

        CT_NONE, // 未初始化的类型
        CT_SPACE, // 空白符
        CT_ALPHA, // 英文字符、数字
        CT_NON_ENGLISH, // 非英文字符，但可以表示出来
        CT_PUNCTUATE, // 标点符号
    };

    // 获取字符类型
    CharType GetCharType(const CharIter &ch);

    //////////////////////////////////////////////////////////////////////////

    // 测试当前物理起始坐标是否合乎要求(上下限)
    void TestViewStart();

    // 维护最大可能物理起始坐标的正确性
    ///
    // 一般用于在最后一屏内容的区域里删除字符后更新起始坐标。
    void TestMaxViewStart();

    // 将已选择文本扩展到指定\a i 字符
    // \param cursorPos 不包括文本左边距，假如为 wxNOT_FOUND 则自动计算
    void DraggTo(const CharIter &i, int cursorPos, wxDC *pDC);

    // 是否存在已选择的文本
    bool IsSelected() const;

    // 重置选择终点(无任何选择)
    void ResetSelEnd();

    // 取消选择任何选项
    // \param i 同时将光标移动到\a i 前面
    // \param cursorPos 光标绘制位置
    void SelectNone(const CharIter &i, int cursorPos, wxDC *pDC);

    // 删除所有已选择的文本
    void RemoveSelected(wxDC *pDC);

    // 获取已选择文本的起始区间的迭代器表示
    void GetSelRange(CharIter &first, CharIter &last) const;

    // 删除指定区间内的文本
    //
    // 不包括\a last 指定的字符。执行后会将光标移动到\a last 。
    // \param from 可以提供起始字符的序号，提高保存撤销信息的效率。
    void DoRemove(const CharIter &first, const CharIter &last, long from, wxDC *pDC);

private:

    //////////////////////////////////////////////////////////////////////////
    // 控件状态

    Characters m_text; // 当前文本

    int m_cursorPos; // 光标当前位置，以像素计算而不是字符（考虑到不等宽字符）
    CharIter m_cursorChar; // 光标后面的字符

    int m_selEndPos; // 选择范围的结束坐标
    CharIter m_selEndChar; // 选择范围的结束后的第一个字符，与 STL 的概念相同

    int m_total; // 字符串总长度
    int m_xViewStart; // 一行显示不下时起始绘制坐标

    //////////////////////////////////////////////////////////////////////////
    // 撤销编辑的实现

    // 为“撤销”保存的状态信息
    class Action {
    public:

        // 构造函数
        Action(VdkEdit *edit, long insertPoint)
            : m_edit(edit), m_insertPoint(insertPoint) {

        }

        // 虚析构函数
        virtual ~Action() {}

        // 撤销操作
        virtual void Exec(wxDC *pDC) = 0;

        // 回复操作
        virtual void UnExec(wxDC *pDC) = 0;

    protected:

        VdkEdit *m_edit; // 文本框控件
        long m_insertPoint; // 撤销操作的插入点
    };

    int m_maxUndo; // 最大可“撤销”的步数

    typedef wxVector<Action *> Actions;
    typedef Actions::iterator ActionIter;

    Actions m_actions; // “撤销”、“重做”列表
    int m_nextUndo; // 下一次“撤销”所执行的操作

    //////////////////////////////////////////////////////////////////////////

    // 为插入删除字符的操作保存状态信息
    class InsRmv : public Action {
    public:

        // 操作的类型
        enum ActionType {
            IRT_INSERT, // 插入文本
            IRT_DEL,    // 使用删除键删除单个字符
            IRT_BACK,   // 使用退格键删除单个字符
            IRT_REMOVE, // 删除多个字符
        };

        // 构造函数
        InsRmv(VdkEdit *edit, long insertPoint, wxString operand, ActionType type)
            : Action(edit, insertPoint), m_operand(operand), m_type(type) {
            wxASSERT(m_operand.length() > 0);
        }

        // 撤销操作
        virtual void Exec(wxDC *pDC);

        // 回复操作
        virtual void UnExec(wxDC *pDC);

    private:

        // 撤销插入
        void UndoIns(wxDC *pDC);

        // 撤销删除
        void UndoDel(wxDC *pDC);

        // 保存的行为是否删除字符
        bool IsDelOperation() const {
            return m_type != IRT_INSERT;
        }

        //////////////////////////////////////////////////////////////////////////

        wxString m_operand; // 被操作的字符
        ActionType m_type; // 操作的类型
    };

    // 设置“撤销”行为对象
    void SetUndoAction(Action *action);

    // 保存删除字符时的“撤销”信息
    //
    // 因为删除分“退格”和“删除”。
    void SaveDeleteCharState(const CharIter &insertPoint, InsRmv::ActionType type);

    //////////////////////////////////////////////////////////////////////////

    static size_t ms_editCount; // 当此值降为 0 时删除已分配的 Character 堆对象

    DECLARE_CLONEABLE_VOBJECT(VdkEdit)
};

/// \brief 流式操作符函数
VdkEdit &operator << (VdkEdit &edit, int num);

//////////////////////////////////////////////////////////////////////////

/// \brief VdkEdit 的静态风格
class VdkEditStaticStyle {
public:

    /// \brief 默认构造函数
    VdkEditStaticStyle();

    /// \brief 设置文本颜色
    VdkEditStaticStyle &textColor(const wxColour &color) {
        m_TextColor = color;
        return *this;
    }

    /// \brief 设置背景画刷
    VdkEditStaticStyle &bgBrush(const wxBrush &brush) {
        m_BgBrush = brush;
        return *this;
    }

    /// \brief 设置已选择文本的背景画刷
    VdkEditStaticStyle &selTextBgBrush(const wxBrush &brush) {
        m_SelTextBgBrush = brush;
        return *this;
    }

    /// \brief 设置不可编辑时的背景画刷
    VdkEditStaticStyle &uneditableBgBrush(const wxBrush &brush) {
        m_UneditableBgBrush = brush;
        return *this;
    }

    /// \brief 设置边框样式
    VdkEditStaticStyle &normalBorder(const wxPen &pen) {
        m_NormalBorder = pen;
        return *this;
    }

    /// \brief 设置边框样式
    VdkEditStaticStyle &innerBorder(const wxPen &pen) {
        m_InnerBorder = pen;
        return *this;
    }

    /// \brief 设置边框样式
    VdkEditStaticStyle &outerBorder(const wxPen &pen) {
        m_OuterBorder = pen;
        return *this;
    }

    typedef void (*Resetter)();

    /// \brief 设置引用计数管理器
    VdkEditStaticStyle &resetter(Resetter rst) {
        m_Resetter = rst;
        return *this;
    }

    /// \brief 引用计数减一
    void decRef() {

        if (m_Resetter) {
            m_Resetter();
        } else {
            delete this;
        }
    }

private:

    wxColour m_TextColor; // 文本颜色

    // 边框样式(颜色、线条点状等)
    wxPen m_NormalBorder; // 正常状态下边界
    wxPen m_InnerBorder; // 高亮时内边界
    wxPen m_OuterBorder; // 高亮时外边界

    wxBrush m_BgBrush; // 背景画刷
    wxBrush m_SelTextBgBrush; // 已选择文本的背景画刷
    wxBrush m_UneditableBgBrush; // 不可编辑时的背景画刷

    Resetter m_Resetter; // 引用计数管理函数

    friend class VdkEdit;
};
