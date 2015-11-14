/***************************************************************
 * Name:      OOPFileSystem.h
 * Purpose:   管理 OOPlayer 的路径
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-04-19
 **************************************************************/
#pragma once

/// 管理 OOPlayer 的路径
class OOPFileSystem
{
public:

	/// 获取根路径
	///
	/// 路径字符串末尾已加路径分隔符。
	static wxString GetRootPath();

	/// 获取皮肤文件根目录
	///
	/// 路径字符串末尾已加路径分隔符。
	static wxString GetSkinRootDir();

	/// 获取应用程序资源目录
	///
	/// 路径字符串末尾已加路径分隔符。
	static wxString GetAppResDir();
};
