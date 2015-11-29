/***************************************************************
 * Name:      VdkListView.h
 * Purpose:   类 Android 的列表控件，能动态获取数据
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-03-03
 **************************************************************/
#pragma once
#include "VdkScrolledWindow.h"
class IListViewPlaugin;

/// 类 Android 的列表控件，能动态获取数据
class VdkListView : public VdkScrolledWindow {
public:

    /// 数据适配器
    class Adapter {
    public:

        /// 虚析构函数
        virtual ~Adapter() {}

        /// 数据集是否为空
        virtual bool IsEmpty() const = 0;

        /// 获取数据集里的数据条目总数
        virtual size_t GetRowCount() const = 0;

        /// 获取数据集里的列总数
        virtual size_t GetColumnCount() const = 0;

        /// 获取某个单元格内的文本
        virtual wxString GetCell(size_t row, size_t col) const = 0;

    public:

        enum {
            MATCH_PARENT = -1, ///< 未指定的列宽度，未实现
        };

        /// 一列的风格定义
        struct ColumnStyle {
            wxString heading; ///< 列的标题
            int width; ///< 该列的当前宽度
            /// 当列表控件大小改变时本列分得的该变量（如80%）
            int extendingPercentage;
            align_type textAlign;   ///< 列的文本对齐
        };

        /// 获取列风格定义
        virtual ColumnStyle &GetColumnStyle(size_t col) = 0;
    };

    //////////////////////////////////////////////////////////////////////////

    /// 构造函数
    /// @param style 列表框的风格组合
    VdkListView(long style = 0);

    /// 析构函数
    ~VdkListView();

    /// XRC 动态创建
    virtual void OnXrcCreate(wxXmlNode *node);

    /// 创建控件
    void Create(VdkWindow *parent,
                const wxString &strName,
                const wxRect &rc);

    //////////////////////////////////////////////////////////////////////////

    /*! 适配器的生命周期管理模式 */
    enum AdapterLifeTimePolicy {
        ALTP_HELD, ///< VdkListView 接管适配器，在 VdkListView 删除时也删除适配器
        ALTP_STATIC, ///< 外部静态变量
    };

    /// 设置数据适配器
    /// @param adapter 适配器指针，不可以设为空
    /// @param altp 适配器的生命周期管理模式
    void SetAdapter(Adapter *adapter, AdapterLifeTimePolicy altp);

    /// 更新虚拟画布大小
    void UpdateSize(wxDC *pDC);

    /// 获取已绑定的适配器
    Adapter *GetAdapter() const {
        return m_adapter;
    }

    /// 删除已绑定的适配器对象(以释放占用的内存)
    void DestroyAdapter() {
        wxDELETE(m_adapter);
    }

    //////////////////////////////////////////////////////////////////////////

    /// 数据适配器持有者通知列表框序号为@a row 的新项被加入
    void NotifyItemAdd(size_t row);

    /// 数据适配器持有者通知列表框序号为@a row 的项被移除
    void NotifyItemRemove(size_t row);

    /// 数据适配器持有者通知列表框所有数据项均已被移除
    void NotifyClear();

    //////////////////////////////////////////////////////////////////////////

    typedef IListViewPlaugin *PluginPtr;

    /// 添加一个插件
    void AddPlugin(PluginPtr plugin);

    /// 移除插件时对插件对象执行的操作
    enum DestoryPolicy {
        PDP_LEAVE_IT_AS_IS, ///< 保留指针
        PDP_DESTORY, ///< 顺便删除插件对象
    };

    /// 移除指定插件
    void RemovePlugin(const wxString &name, DestoryPolicy dp);

    /// 删除所有插件对象
    void DestroyAllPlugins();

public:

    /// 当前作用域下最大可以显示的条目数
    int GetShownItems() const {
        return m_shownItems;
    }

    /// 得到行高
    int GetRowHeight() const {
        return m_rowHeight;
    }

    /// 设置行高
    ///
    /// 提供的数值不能小于字体的高度。
    void SetRowHeight(int height, wxDC *pDC = NULL);

    /// 获取绘制单元格时左侧距本列开始坐标的距离
    static size_t GetCellLeftPadding();

public:

    /// 存取默认背景画刷
    wxBrush GetBackgroundBrush() const {
        return m_background;
    }
    void SetBackgroundBrush(const wxBrush &brush) {
        m_background = brush;
    }

    // 更新行状态（予以重画）
    void UpdateRow(int index, wxDC &dc);

protected:

    /// 处理鼠标事件
    virtual void OnMouseEvent(VdkMouseEvent &e);

    /// 响应用户按键事件
    virtual void OnKeyEvent(VdkKeyEvent &ke);

    /// 绘制控件
    virtual void OnDraw(wxDC &dc);

    /// 擦除（绘制）行背景
    void EraseRow(int index, wxDC &dc);

    /// 绘制一行的文本
    ///
    /// 本函数因还原被选择或高亮的项而生。
    /// @attention 本函数效率比较低下，谨慎使用！
    /// 推荐用途:\link UpdateRow \endlink
    void DrawRowText(int row, wxDC &dc);

    /// 在坐标(@a x, @a y)处开始绘制单元格(@a col, @a row)的文本
    ///
    /// 会自动处理诸如左填充、对齐、长文本截断等问题。
    void DrawCellText(wxDC &dc, size_t row, size_t col, int x, int y);

    /// 处理 X 轴坐标文本对齐、长文本截断
    /// @param col 文本所属列
    /// @param text 要显示的文本，过长时会被截断
    /// @return 文本绘图坐标 x 坐标的修正值
    int CalTextXAxisProp(wxDC &dc, size_t col, wxString &text);

protected:

    /// 从某一竖直坐标\a y 判断指针下的列表项 ID
    /// @return 点击点是否严格处于可视区域之内，否则点击了控件
    /// 最后的空白处
    bool GetIndex(int y, int *rowAtPointer);

private:

    // （重新）计算当前作用域下最大可以显示的条目数
    void CalcShownItems();

    // 接收、处理通知信息
    // 派生自 VdkScrolledWindow
    virtual void OnNotify(const VdkNotify &notice);

    /// @brief 在此擦除（绘制）行背景
    /// @return 指示 VdkListCtrl 的后续行为
    /// @see VdkCusdrawReturnFlag
    virtual VdkCusdrawReturnFlag DoEraseRow(int index, wxDC &dc) {
        return VCCDRF_DODEFAULT;
    }

    enum DataSetChangeAction {
        DSCA_ADD,
        DSCA_REMOVE,
        DSCA_CLEAR,
    };

    // 处理数据集变化
    void OnDataSetChange(DataSetChangeAction dsca, size_t row);

private:

    long m_style;
    Adapter *m_adapter;
    bool m_held;

    int m_rowHeight; // 行高
    int m_shownItems; // 当前的 VdkListCtrl 能容纳多少行

    // TODO:
    // 在正常的垂直显示范围之下还要绘制多少条项目，
    // 多用于允许自动滚动的滚动窗口，否则往往会使得当前屏的
    // 第一行移出画布上沿后，而最后一行的下一行却还没进入
    // 用户的视野。
    int m_nShownItemsAddIn;

    wxBrush m_background; // 默认背景画刷
    wxVector<PluginPtr> m_plugins;

    DECLARE_DYNAMIC_VOBJECT
};
