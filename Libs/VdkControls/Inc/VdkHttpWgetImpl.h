/***************************************************************
 * Name:      VdkHttpWgetImpl.h
 * Purpose:   使用 wget 来实现 VdkHTTP 的接口
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-03-17
 **************************************************************/
#pragma once
#include "VdkHTTP.h"

/// 使用 wget 来实现 VdkHTTP 的接口
class VdkHttpWgetImpl : public VdkHTTP {
public:

    /// 构造函数
    VdkHttpWgetImpl(wxFontEncoding defaultEncoding);

    /// 虚析构函数
    virtual ~VdkHttpWgetImpl();

    /// HTTP Get
    virtual bool Get(const wxString &url, wxString &result);

    /// 启用/禁用 GZip
    virtual void EnableGzip(GZipMode gzm);

    /// GZip 是否已然启用
    virtual bool IsGzipEnabled() const;

private:

    GZipMode m_gzm;
};
