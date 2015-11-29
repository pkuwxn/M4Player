/***************************************************************
 * Name:      VdkBitmapArray.h
 * Purpose:   VdkBitmapArray 的头文件
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2011-01-10
 * Copyright: Wang Xiaoning
 **************************************************************/
#pragma once
#include "VdkDefs.h"

/// \brief 位图数组的管理对象类
///
/// 编写该类的目的是避免生成过多小位图以节省资源。
class VdkBitmapArray {
public:

    /// \brief 构造函数
    VdkBitmapArray();

    /// \brief 构造函数
    /// \param strPath 位图所在的磁盘路径
    /// \param cols 分割的份数
    VdkBitmapArray(const wxString &strPath, unsigned cols, unsigned rows);

    /// \brief 构造函数
    /// \param bm 所用位图
    /// \param cols 分割的份数
    VdkBitmapArray(const wxBitmap &bm, unsigned cols, unsigned rows) {
        Set(bm, cols, rows);
    }

    /// \brief 复制构造函数
    VdkBitmapArray(const VdkBitmapArray &other);

    /// \brief 复制赋值函数
    VdkBitmapArray &operator = (const VdkBitmapArray &rhs);

    /// \brief 从另一个对象复制
    void Assign(const VdkBitmapArray &rhs);

    /// \brief 数组是否已然初始化/位图存在
    bool IsOk() const {
        return m_bmp.IsOk();
    }

    /// \brief 设置数组所用位图
    /// \param bm 所用位图
    /// \param cols 分割的份数
    void Set(const wxBitmap &bm, unsigned cols, unsigned rows);

    /// \brief 设置数组所用位图
    /// \param strPath 位图所在的磁盘路径
    /// \param cols 分割的份数
    void Set(const wxString &strPath, unsigned cols, unsigned rows);

    /// \brief 将(\a row, \a col )处的小位图复制到 DC 的(\a x, \a y)处
    void BlitTo(wxDC &dc, unsigned col, unsigned row, wxCoord x, wxCoord y);

    /// \brief 更改数组位图大小
    ///
    /// 当前只支持更改只有一行的位图数组的大小。
    bool Resize(int newwidth, int newheight,
                VdkResizeableBitmapType type, const wxRect &rcTile);

    //////////////////////////////////////////////////////////////////////////

    /// \brief 获取原始位图
    wxBitmap GetRawBitmap() const {
        return m_bmp;
    }

    /// \brief 获取真正意义上的位图数组
    wxBitmap *Explode() const;

    /// \brief 得到小位图的宽度
    unsigned GetWidth() const;

    /// \brief 得到小位图的高度
    unsigned GetHeight() const;

    /// \brief 得到分割的份数
    unsigned GetExplNum() const {
        return m_cols;
    }

    /// \brief 得到行数
    unsigned GetRows() const {
        return m_rows;
    }

    /// \brief 得到列数
    unsigned GetColumns() const {
        return m_cols;
    }

private:

    wxBitmap m_bmp;
    unsigned m_cols, m_rows;
};

//////////////////////////////////////////////////////////////////////////

/// \brief 位图数组的指定小位图，使之具有 wxBitmap 的类似性质
class VdkBitmapArrayId {
public:

    /// \brief 默认构造函数，构建一个空白伪位图
    VdkBitmapArrayId() : m_ba(NULL), m_x(-1), m_y(-1) {}

    /// \brief 构造函数
    /// \param ba 小位图所属的位图数组
    /// \param index 小位图的序号
    VdkBitmapArrayId(VdkBitmapArray &ba, int x, int y)
        : m_ba(&ba), m_x(x), m_y(y) {}

    /// \brief 复制赋值函数
    VdkBitmapArrayId &operator = (const VdkBitmapArrayId &another) {
        m_ba = another.m_ba;
        m_x = another.m_x;
        m_y = another.m_y;

        return *this;
    }

    /// \brief 将小位图复制到指定 DC 的(\a x, \a y)处
    void BlitTo(wxDC &dc, wxCoord x, wxCoord y) const;

    /// \brief 小位图是否有效，可以被复制
    bool IsOk() const {

        return m_ba &&
               m_ba->IsOk() &&
               m_x >=0 &&
               m_x < static_cast<int>(m_ba->GetColumns()) &&
               m_y >=0 &&
               m_y < static_cast<int>(m_ba->GetRows());
    }

private:

    VdkBitmapArray *m_ba;
    int m_x, m_y;
};
