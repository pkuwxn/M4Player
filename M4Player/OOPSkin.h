/***************************************************************
 * Name:      OOPSkin.h
 * Purpose:   管理 OOPlayer 的皮肤读取、切换等内容
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-02-18
 * Copyright: Ning
 **************************************************************/
#pragma once

/// \brief 管理 OOPlayer 的皮肤读取、切换等内容
class OOPSkin
{
public:

	/// \brief 构造函数
	OOPSkin() : m_selected( NULL ) {}

	/// \brief 析构函数
	~OOPSkin();

	/// \brief 获取所有皮肤
	void GetAllSkins();

public:

	class SkinInfo;
	WX_DECLARE_LIST( SkinInfo, ListOfSkinInfo );
	typedef OOPSkin::ListOfSkinInfo::const_iterator SkinIter;

	/// \brief 获取皮肤列表迭代器的起始点
	SkinIter begin() const { return m_skins.begin(); }

	/// \brief 获取皮肤列表迭代器的终止点
	SkinIter end() const { return m_skins.end(); }

	/// \brief 获取已被选用的皮肤
	SkinInfo* GetSelectedSkin() const { return m_selected; }

	/// \brief 设置已被选用的皮肤
	void SetSelectedSkin(SkinInfo* skin) { m_selected = skin; }

private:

	ListOfSkinInfo m_skins;
	SkinInfo* m_selected;

public:

	/// \brief 皮肤信息结构
	class SkinInfo {
	public:

		/// \brief 构造函数
		SkinInfo(const wxString& strName, const wxString& strRootPath);

		/// \brief 获取皮肤名字
		wxString name() const { return m_strName; }

		/// \brief 获取皮肤名字根目录名
		wxString folderName() const;

		/// \brief 获取皮肤根目录的路径
		///
		/// 路径末尾已加上“/”。
		wxString rootPath() const { return m_strRootPath; }

		/// \brief 获取皮肤作者
		wxString author() const { return m_author; }

		/// \brief 获取皮肤作者个人主页
		wxString url() const { return m_url; }

		/// \brief 获取皮肤作者邮箱
		wxString email() const { return m_email; }

		/// \brief 获取皮肤掩码色
		wxColour maskColor() const { return m_maskColor; }

		/// \brief 当前皮肤缩略图是否已然被读入
		bool isThumbNailOk() const { return m_bmpThumbnail.IsOk(); }

		/// \brief 获取皮肤缩略图
		wxBitmap thumbNail(const wxSize& sz) const;

	private:

		// 得到某个节点的内容
		wxString GetNode(const wxString& xmlData, const wxString& nodeName);

		//////////////////////////////////////////////////////////////////////////

		wxString m_strName; // 皮肤名字
		wxString m_strRootPath; // 皮肤根目录的路径
		wxString m_author; // 皮肤作者
		wxString m_url; // 皮肤官方网站
		wxString m_email; // 皮肤作者E-Mail
		wxColour m_maskColor; // 皮肤掩码色

		mutable wxBitmap m_bmpThumbnail; // 皮肤缩略图
	};

	/// \brief 获取指定序号的皮肤
	OOPSkin::SkinInfo* GetSkin(int index);
};
