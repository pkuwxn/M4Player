/***************************************************************
 * Name:      OOPConf.h
 * Purpose:   OOPlayer ������������Ϣ
 * Author:    Wang Xiao Ning (vanxining@139.com)
 * Created:   2011-05-07
 **************************************************************/
#pragma once

 /// \brief ճ������״̬�־û�
class OOPStickyWindowState
{
public:

    /// \brief ���캯��
    OOPStickyWindowState();

    /// \brief �Ƿ���ճ����������
    bool IsStickyToMain() const { return m_stickyToMain; }

    /// \brief ��ȡ����Ĵ���������
    wxRect GetRect() const { return m_rect; }

    /// \brief ��յ�ǰֵ�����ڻ���ʱ��״̬��ΪĬ��ֵ
    void Reset();

private:

    // �Ƿ��Ѿ�ճ����������
    bool m_stickyToMain;

    // ���������򣬿�������������������ϽǶ��ԣ�Ҳ�п�������Ļ����
    wxRect m_rect;

    friend class OOPWindow;
    friend bool FromString(const wxString &raw, OOPStickyWindowState &out);
};

/// \brief OOPlayer ��Ӧ�ó������������Ϣ
class OOPConf
{
public:

	/// \brief ���캯��
	OOPConf();

    /// \brief ��������
    bool Load(const wxString& path);

    /// \brief ��������
    bool Save(const wxString& path);

    /// \brief �������ճ������״̬
    void ClearStickyWindowStates();

public:

	int m_currSong; /*! ��ǰ���ڲ��ŵĸ��� */
	int m_playMode; /*! ��ǰ�����б����ģʽ */
	wxString m_skinFolderName; /*! ��ǰƤ��Ŀ¼�� */
	bool m_playing; /*! �ر�ʱ�Ƿ����ڲ��� */
	int m_currPos; /*! ��ǰ���Ž��� */

	/*! ��ʹ�� Ubuntu Unity shell ����Ҳ��ʾ����ͼ�� */
	bool m_showTaskBarIconUnderUnity;
	bool m_playListShown; /*! �����б����Ƿ�ɼ� */
	bool m_lyricShown; /*! ����㴰���Ƿ�ɼ� */
	bool m_eqShown; /*! EQ �����Ƿ�ɼ� */

	double m_volume; /*! ��ǰ���� */
	bool m_mute; /*! �Ƿ��� */

    /*! �����ڵ�λ�ü�ճ��״̬ */
    OOPStickyWindowState mainPanelWndState;
    OOPStickyWindowState playlistPanelWndState;
    OOPStickyWindowState lyricPanelWndState;
    OOPStickyWindowState eqPanelWndState;

private:

    enum {
        NUM_PANELS = 4,
    };

private:

    bool Player(const wxXmlNode *node);
    wxXmlNode *Player();
};
