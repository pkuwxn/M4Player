/***************************************************************
 * Name:      OOPSkin.h
 * Purpose:   ���� OOPlayer ��Ƥ����ȡ���л�������
 * Author:    Ning (vanxining@139.com)
 * Created:   2011-02-18
 * Copyright: Ning
 **************************************************************/
#pragma once

/// \brief ���� OOPlayer ��Ƥ����ȡ���л�������
class OOPSkin
{
public:

	/// \brief ���캯��
	OOPSkin() : m_selected( NULL ) {}

	/// \brief ��������
	~OOPSkin();

	/// \brief ��ȡ����Ƥ��
	void GetAllSkins();

public:

	class SkinInfo;
	WX_DECLARE_LIST( SkinInfo, ListOfSkinInfo );
	typedef OOPSkin::ListOfSkinInfo::const_iterator SkinIter;

	/// \brief ��ȡƤ���б����������ʼ��
	SkinIter begin() const { return m_skins.begin(); }

	/// \brief ��ȡƤ���б����������ֹ��
	SkinIter end() const { return m_skins.end(); }

	/// \brief ��ȡ�ѱ�ѡ�õ�Ƥ��
	SkinInfo* GetSelectedSkin() const { return m_selected; }

	/// \brief �����ѱ�ѡ�õ�Ƥ��
	void SetSelectedSkin(SkinInfo* skin) { m_selected = skin; }

private:

	ListOfSkinInfo m_skins;
	SkinInfo* m_selected;

public:

	/// \brief Ƥ����Ϣ�ṹ
	class SkinInfo {
	public:

		/// \brief ���캯��
		SkinInfo(const wxString& strName, const wxString& strRootPath);

		/// \brief ��ȡƤ������
		wxString name() const { return m_strName; }

		/// \brief ��ȡƤ�����ָ�Ŀ¼��
		wxString folderName() const;

		/// \brief ��ȡƤ����Ŀ¼��·��
		///
		/// ·��ĩβ�Ѽ��ϡ�/����
		wxString rootPath() const { return m_strRootPath; }

		/// \brief ��ȡƤ������
		wxString author() const { return m_author; }

		/// \brief ��ȡƤ�����߸�����ҳ
		wxString url() const { return m_url; }

		/// \brief ��ȡƤ����������
		wxString email() const { return m_email; }

		/// \brief ��ȡƤ������ɫ
		wxColour maskColor() const { return m_maskColor; }

		/// \brief ��ǰƤ������ͼ�Ƿ���Ȼ������
		bool isThumbNailOk() const { return m_bmpThumbnail.IsOk(); }

		/// \brief ��ȡƤ������ͼ
		wxBitmap thumbNail(const wxSize& sz) const;

	private:

		// �õ�ĳ���ڵ������
		wxString GetNode(const wxString& xmlData, const wxString& nodeName);

		//////////////////////////////////////////////////////////////////////////

		wxString m_strName; // Ƥ������
		wxString m_strRootPath; // Ƥ����Ŀ¼��·��
		wxString m_author; // Ƥ������
		wxString m_url; // Ƥ���ٷ���վ
		wxString m_email; // Ƥ������E-Mail
		wxColour m_maskColor; // Ƥ������ɫ

		mutable wxBitmap m_bmpThumbnail; // Ƥ������ͼ
	};

	/// \brief ��ȡָ����ŵ�Ƥ��
	OOPSkin::SkinInfo* GetSkin(int index);
};
