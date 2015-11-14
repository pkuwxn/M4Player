/***************************************************************
 * Name:      IListViewPlaugin.h
 * Purpose:   VdkListView 的插件接口声明
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-3-3
 **************************************************************/
#pragma once

// 单选、多选 LvpClick
// 改变大小时根据百分比动态调整列宽 LvpColumnAutoAjuster
// 通过拖动改变项目排序 LvpDraggAndReorder
// 隔行变色 LvpCrossColor
// 鼠标滑过时高亮所在行 LvpHilighter
// 排序 LvpSorter
// 标题栏 LvpHeader

class VdkMouseEvent;
class VdkKeyEvent;
class VdkListView;

/// VdkListView 的插件接口声明
class IListViewPlaugin
{
public:

	typedef VdkListView ListView;

	/// 构造函数
	IListViewPlaugin(ListView* list)
		: m_list( list )
	{
		wxASSERT( m_list );
	}

	/// 虚析构函数
	virtual ~IListViewPlaugin() {}

public:

	/// 获取 VdkListView 对象
	ListView* GetListView() const { return m_list; }

	/// 是否可以被 VdkListView 安全删除
	///
	/// 假如插件具有多重继承的类层次结构，那么应该小心处理插件的生命周期。
	virtual bool CanDestoryByListView() const { return true; }

public:

    /// 擦除原有内容，绘制背景
    virtual void OnEraseRow(int row, wxDC& dc) {}
    
    /// 拖动事件
    /// @param rowAtPointer 当前鼠标指针下对应的项目
    virtual void OnDragg(int rowAtPointer, VdkMouseEvent& e) {}
    
    /// 鼠标双击事件
    virtual void OnDClick(int rowAtPointer, VdkMouseEvent& e) {}
    
    /// 鼠标左键按下
    virtual void OnClickDown(int rowAtPointer, VdkMouseEvent& e) {}
    /// 鼠标左键弹起
    virtual void OnClickUp(int rowAtPointer, VdkMouseEvent& e) {}
    
    /// 鼠标右键弹起
    virtual void OnRightUp(int rowAtPointer, VdkMouseEvent& e) {}
    
    /// 键盘按键事件
    virtual void OnKey(VdkKeyEvent& ke) {}
    
    /// 控件位置改变
    /// TODO:
    virtual void OnMove(int dx, int dy) {}
    
    /// 控件大小改变
    virtual void OnSize(int dx, int dy) {}

public:

	/// 数据适配器持有者通知列表框序号为@a row 的新项被加入
	virtual void OnItemAdd(size_t row) {}

	/// 数据适配器持有者通知列表框序号为@a row 的项被移除
	virtual void OnItemRemove(size_t row) {}

	/// 数据适配器持有者通知列表框所有数据项均已被移除
	virtual void OnClear() {}

private:

	ListView* m_list;
};
