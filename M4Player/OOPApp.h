/***************************************************************
 * Name:      OOPApp.h
 * Purpose:   Defines Application Class
 * Author:    Ning (vanxining@139.com)
 * Created:   2009-12-19
 * Copyright: Ning
 **************************************************************/
#pragma once

class VdkLabel;
class VdkButton;
class VdkToggleButton;
class VdkScrollBar;
class VdkSlider;
class VdkListCtrl;
class VdkControl;
class VdkMenu;
class VdkMenuItem;

class MainPanel;
class PlayListPanel;
class LyricPanel;
class EqPanel;

class MapOfCtrlIdInfo;
class wxXmlSerializer;

class OOPLed;
class OOPLyric;
class OOPList;
class OOPLabel;
class OOPSong;
class OOPDesktopLyric;
class OOPWindow;
class OOPConf;
class OOPStickyWindowState;

class OOPTrayIcon;
class wxTaskBarIconEvent;

#include "VdkListCtrl.h" // for LcCellIter
#include "VdkBitmapArray.h"

#include "OOPDefs.h" // for enum OOPlayerPlayMode
#include "OOPSkin.h"
#include "StickyWindows.h"
#include "OOPScanner.h"
#include "OOPStopWatch.h"
#include "Lyric/OOPLyricParser.h"
#include "Lyric/OOPLyricMgr.h"
#include "OOPSongPtr.h"

#include <Music.hpp>
#include <wx/snglinst.h>

//////////////////////////////////////////////////////////////////////////

/*! �������Ϣ ID (��������) */
enum OOPWindowExtraMessageID {

	/// \brief ���贰��Ƥ��
	OOPEMC_RESET_WINDOW			= VEMC_USER + 0,
};

/*! �������õ�����������Ψһ��ʶö��ֵ */
enum OOPPanel {
	PANEL_MAIN, /*! ����� */
	PANEL_PLAYLIST, /*! �����б��� */
	PANEL_LYRIC, /*! ����㴰�� */
	PANEL_EQ, /*! ���������� */

	NUM_PANELS, /*! ������� */
};

//////////////////////////////////////////////////////////////////////////

/// \brief ����������Ҫ�õ���ָ��
class OOPPtrs
{
public:

	/// \brief ���캯��
	OOPPtrs() { Clear(); }

	/// \brief ��ʼ������ָ��
	void Clear() { memset( this, 0, sizeof( OOPPtrs ) ); }

protected:

	VdkButton* m_minimize;
	VdkButton* m_exit;
	VdkButton* m_minimode;

	VdkButton* m_play;
	VdkButton* m_pause;
	VdkButton* m_stop;
	VdkButton* m_prev;
	VdkButton* m_btnNext;

	VdkSlider* m_progress;
	VdkSlider* m_volume;
	VdkLabel* m_status;
	VdkLabel* m_stereo;
	VdkToggleButton* m_togglePlayList;
	VdkToggleButton* m_toggleLyric;
	VdkToggleButton* m_toggleEq;
	VdkToggleButton* m_toggleBrowser;

	VdkToggleButton* m_mute;
	VdkButton* m_openFile;

	OOPList* m_playList;
	OOPLyric* m_embeddedLyric;
	OOPLed* m_led;
	OOPLabel* m_rollingLabel;

	VdkMenu* m_MainMenu;
	VdkMenu* m_PlayListMenu;
	VdkMenu* m_LyricMenu;
};

/// \brief Ӧ�ó����࣬ͬʱҲ����������Ŀ�������
class OOPlayerApp : public wxApp, public OOPPtrs
{
public:

	/// \brief ���캯��
	OOPlayerApp();

	/// \brief �л����������ɲ��Ÿ�����״̬
	void EnablePlayer();

	/// \brief ��ͷ����һ�׸���
	void BeginSong(LcCellIter songIter, wxDC* pDC, wxDC* playListDC);

	/// \brief ��ͷ����һ�׸���
	void BeginSong(int index, wxDC* pDC, wxDC* playListDC);

	/// \brief ���þ���/������
	void ToggleMute(bool mute, wxDC* pDC);

	//////////////////////////////////////////////////////////////////////////
	// ��ȡ��������ǰ״̬

	/// \brief ��ȡ���׸�Ľ���
	double GetProgress() const;

	/// \brief �Ƿ����ڲ���
	bool IsPlaying() const;

	/// \brief �Ƿ���Ȼ��ͣ
	bool IsPaused() const;

	/// \brief �Ƿ���Ȼֹͣ����
	bool IsStopped() const;

	/// \brief �Ƿ���
	bool IsMute() const;

	/// \brief �������Ƿ��ڿɲ��ŵ�״̬
	bool IsPlayable() const;

	/// \brief ��ȡ���ڲ��ŵĸ����ĳ���[����:ms(����)]
	unsigned int GetCurrSongLength() const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief ����������
	///
	/// �����ڹ�����Ϻ�Ӧ���ñ�����
	void SetMainPanel(MainPanel* panel, wxDC* pDC);

	/// \brief ���������ڵĿؼ� ID
	void GenerateMainPanelCtrlIds(MapOfCtrlIdInfo& ids);

	/// \brief ֻ������������ʾ�����ʾ�����Ӵ���
	void OnMainPanelFirstShown(VdkWindowFirstShownEvent&);

	/// \brief ������ʾ��ǰ���Ÿ�����Ϣ
	void RollLabel(wxDC* pDC, bool startAtOnce);

	/// \brief ��ʾ�����б���
	void ShowPlayListPanel(VdkVObjEvent&);

	/// \brief �ر�/���ز����б���
	void DismissPlayListPanel();

	/// \brief �ڲ����б��ڵİ����¼�
	/// \return �Ƿ���ֹ�����¼�����
	bool OnPlayListKeys(wxKeyEvent& e);

	/// \brief ��ʾ����㴰��
	void ShowLyricPanel(VdkVObjEvent&);

	/// \brief �ر�/���ظ�ʴ���
	void DismissLyricPanel();

	/// \brief ��ʼ�������
	void InitLyric();

	/// \brief ��ʾ����������
	void ShowEqPanel(VdkVObjEvent&);

	/// \brief �ر�/���ؾ���������
	void DismissEqPanel();
	
	/// \brief �û��������ƶ���壬���¸���״̬
	void HidePanel(OOPWindow* win);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ȡƤ���б������
	OOPSkin& GetSkinList() { return m_skinList; }

	/// \brief ��̬����
	void SelectSkin(OOPSkin::SkinInfo* info) {
		SelectSkin( *info );
	}

	/// \brief ��̬����
	/// \param strSkinFolder Ҫ�л�����Ƥ���ĸ�Ŀ¼
	void SelectSkin(OOPSkin::SkinInfo& info);

	/// \brief ��Ӧ�����¼�
	void OnSelectSkin(VdkVObjEvent&);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ȡ��������
	StickyWindows& GetWindowManager() { return m_windowManager; }

	/// \brief ������и��� StickyWindows ����
	void AddToWindowManager(VdkWindow* win) {
		m_windowManager.AddWindow( win );
	}

	/// \brief ������и��� StickyWindows ����
	void RemoveFromWindowManager(VdkWindow* win) {
		m_windowManager.RemoveWindow( win );
	}

	/// \brief \a win �Ƿ���Ȼճ����������
	bool IsStickyToMain(VdkWindow* win) {
		return m_windowManager.IsStickyToMain( win );
	}

	/// \brief �����ѱ����Ƥ����������ļ�����������������
	///
	/// ����ֵ����Ϊ�գ���ʱ��һ���ǵ�һ��Ӧ�����Ƥ����
	OOPStickyWindowState* GetPanelStickyState(const wxString& uuid);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ���ɲ��Ŵ������˵�
	void GenerateMainMenu();

	/// \brief ��䲥���б��ڵĲ��ŵ���ģʽ�л��˵�
	/// \param ownedByPLP �Ƿ�Ϊ�����б�����ӵ��
	void FillPlayModesMenu(VdkMenu* menu, bool ownedByPLP = false);

	/// \brief ��䲥���б�
	void GeneratePlayList(OOPList* playList);

	/// \brief ���ɸ�ʴ��ڲ˵�
	void GenerateLyricMenu();

	/// \brief ��ȡ�����б���Ŀ�ı����ʽ
	wxString GetPlayListTitleFormat() const;

	//////////////////////////////////////////////////////////////////////////

	/// \brief �õ�����֧�ֵ��ļ�����
	///
	/// �����ڡ��򿪡��Ի�����д��ļ���
	static wxString GetSupportedFileTypes();

	/// \brief �����ļ����Ƿ���ϲ�����֧�ֵ��ļ�����
	static bool TestFileType(const wxString& strFileExtName);

private:

	// ��ʱ������
	void CreateDesktopLyric();

    // ��ʼ��Ӧ�ó���
    virtual bool OnInit();

	// �����ѷ���Ķ���
	virtual int OnExit();

	// ��ʼ�������ļ�
	bool InitConf();

	// ����Ӧ�ó������������Ϣ
	void SaveConf();

	// �������浱ǰ������崰�ڵĿɼ���(��ֹ�˳�����ʱ����)
	void SavePanelVisibilities();

	// ��������������Ϣ��������Ϣ��(��������)
	void UpdateConf();

	// ���沥���б�
	void SavePlayList(const wxString& path);
	// ���벥���б�
	void LoadPlayList(const wxString& path);

	// ���浱ǰѡ���Ƥ��
	void SaveCurrentSkin(OOPSkin::SkinInfo& info);

	// ��ȡӦ�ó��������ļ���·��
	wxString GetAppConfFilePath() const;

	// ��ȡĬ�ϲ����б���ļ�·��
	wxString GetPlayListPath() const;

	// ���������Ӵ��ں�һ����ȫ����ʾ����
	void ShowAllPanelsOnReady();

	//////////////////////////////////////////////////////////////////////////

	// ����ָ��·���ĸ���
	// @return �Ƿ�ɹ���ʼ���ţ���Ϊ�ļ�������һ�����١��ĸ����ļ�
	bool DoPlay();
	void DoPause(); // ��ͣ
	void DoStop(); // ֹͣ����
	void DoMute(bool mute); // ����

	//////////////////////////////////////////////////////////////////////////

	// ���������š�������ͣ������ť��ʵ�����
	// @param hilight �Ƿ����Ҫ��ʾ�İ�ť
	void ShowHidePlayPause(wxDC* pDC);

	// ����״̬�ı������ı�
	//
	// �϶�����������ʱ��������ı���ֵ�������ܴ���ʹ�� VdkLabelGhost��
	void SetStatusLabel(wxString text = wxEmptyString, wxDC* pDC = NULL);

	//////////////////////////////////////////////////////////////////////////

	void Quit(VdkVObjEvent&);
	void Minimize(VdkVObjEvent&);

	// �л�������ʾ����Ļ�ϵĴ��ڵ���ʾ���
	bool ToggleAllOnScreenWindow();

	void Pause(VdkVObjEvent&);
	void Play(VdkVObjEvent&);

	void Stop(VdkVObjEvent&);
	void OnMenuPlayCtrl(VdkVObjEvent&); // �˵����Ʋ��š���ͣ��ֹͣ

	void OnProgress(VdkVObjEvent&); // �϶���������
	void StepProgress(VdkVObjEvent&); // OOPLed �ص�
	void RollTaskbarTitle(); // ��������������������

	//////////////////////////////////////////////////////////////////////////
	// ����ͼ��

	// ��֪��Ϊʲô�� MSW ���������˴�������ͼ��Ĵ��룬������ʾ���ú���
	// ����׼���ڴ��ڿ���ʱ�����洴��
	void OnIdleCreateTrayIcon(wxIdleEvent&);

    // ��������ͼ��
    void CreateTrayIcon();

	void UpdateTrayIcon(); // ��������ͼ�����ʾ�ı�
	// ����ͼ���¼�
	void OnTrayIconLeftClick(wxTaskBarIconEvent&);
	void OnTrayIconRightDown(wxTaskBarIconEvent&);

	// ���յ�ǰ���ø�������ͼ��˵���״̬(ѡ�С��ı��ȵ�)
	void OnUpdateTrayMenuUI(wxUpdateUIEvent& e);

	//////////////////////////////////////////////////////////////////////////

	void OnLyricDragg(VdkVObjEvent&); // �϶����

	// ���õ�ǰ���Ž���
	bool SetPlayingOffset(sf::Time offset);

	// �������ڲ��ŵĸ����� OOPSong ����
	//
	// ��������ǩ��Ϣ��δ���룬��Ὣ����롣
	void UpdateCurrSong();

	// �������б�ɾ����Ŀ����
	void HandleDelete(VdkVObjEvent&);

	// �Ƿ�����ɾ���ļ�
	enum DelAction {
		DA_DEL_FROM_LIST_ONLY,
		DA_DEL_PHYSICALLY,
	};

	// �������б�ɾ����Ŀ����
	// ���һ���Ӻ�����ֹ���ֶԲ����б��ڷ������� VdkDC �����
	void DoHandleDelete(DelAction da, wxDC& dc, wxDC& playListDC);

	// ��Ӧ�����б��ڿ�ݲ˵��ġ����š���
	void OnPlaySel(VdkVObjEvent& e);

	// ��Ӧ��ɾ�������ļ����˵���
	void OnDelAll(VdkVObjEvent& e);

	// ɾ���ظ��Ĳ����б���Ŀ
	void RemoveDuplicatedItems(VdkVObjEvent&);
	// ɾ������Ĳ����б���Ŀ(�жϱ�׼����������Ϊ0)
	void RemoveWrongItems(VdkVObjEvent&);

	// ɾ���б������и���
	void DeleteAllSongs(wxDC* pDC, wxDC* playListDC);

	// �Ƿ���յ�ǰ��ʵ���ʾ
	enum ClearLyricOpt {
		CL_CLEAR, // ���
		CL_LEAVE, // ����
	};

	// ֹͣ����
	// @param clearLyric �Ƿ���յ�ǰ��ʵ���ʾ
	void DoStop(wxDC* pDC, wxDC* playListDC, ClearLyricOpt clearLyric);

	// �л������������ɲ��Ÿ�����״̬
	//
	// �������б���û�и��������ܲ��š�
	void StopPlayer(wxDC* pDC);

	void DeleteSel(VdkVObjEvent&); // ɾ��ѡ����Ŀ
	// �������б��ѡ���¼�
	void SelectAllNoneReverse(VdkVObjEvent&);

	// �첽�������Բ�������ĸ���������˳������֪ͨ�¼�
	void OnSongFinished(wxCommandEvent&);

	void OnVolume(VdkVObjEvent&); // ����������
	void DoSetVolume(); // ����������ʵ��Ӧ�õ�����������

	void OnMute(VdkVObjEvent&); // ���������ϵ�������������ť
	void OnMenuVolumeCtrl(VdkVObjEvent&); // �˵���������

	// �˵����Ʋ�����һ�ס���һ��
	void OnMenuPrevNext(VdkVObjEvent&);
	void OnPrevNext(VdkVObjEvent&); // ��һ�ס���һ��

	enum PrevNextAction {
		PNA_PREV,
		PNA_NEXT,
	};

	// ͳһ�����л���һ�ס���һ����Ϊ
	void HandlePrevNext(PrevNextAction pna, wxDC* pDC, wxDC* playListDC);

	void OnPlayListDClick(VdkVObjEvent&); // ˫���б���Ŀ
	void OnPlayModes(VdkVObjEvent&); // �л������б������ʽ
	void SwitchPlayMode(OOPlayerPlayMode mode);

	// ѯ���û������沥���б�
	void SaveListByUser(VdkVObjEvent&);
	// ѯ���û�����һ���µĲ����б�
	void OpenListByUser(VdkVObjEvent&);
	// ѯ���û�Ҫ�����Ĳ����б�·��(�½�/��)
	// �����û�ָ����·��
	wxString AskPathToList(const wxString& alert, long dlgStyle);
	// ���� path �Ƿ��뵱ǰ��Ĭ�ϲ����б��ļ���·����ͬ
	bool IsPlayListPathOk(const wxString& path);

	// ���һ���µĲ����б�
	void AddPlayList(VdkVObjEvent&);
	// ���򲥷��б�
	void SortPlayList(VdkVObjEvent&);
	// �����û��û�������Ҳ����б�
	void OnSearchPlaylist(VdkVObjEvent&);
	//  ������ǩ��Ϣ�Ѹ���
	void OnSongInfoUpdated(wxCommandEvent& e);
	// ���ļ�·��
	void OnLocateFile(VdkVObjEvent&);

	// ����ļ��Ĳ˵��ص�����
	// ���罫�ص������ŵ��������лᵼ�� this ָ�벻��ȷ
	void OnAddFileFolders(VdkVObjEvent&);

	//////////////////////////////////////////////////////////////////////////
	// ���

	// ��ʼ�µĸ�����ػỰ
	void NewLyricDownloadSession();

	// Ϊ���ڲ��ŵĸ������ظ��
	//
	// ��ʹ�ø����Ѿ������˸�ʣ���������Ȼ�����¼��ء�
	// @attention ��Ҫֱ�ӵ��������������Ӧ�õ���
	// \link UpdateLyricState \endlink ��
	bool LoadCurrSongLyric();

	// ���ݲ�������ǰ״̬Ϊ�����ؼ�ָ����ȷ�Ķ���(��ͣ�������ȵ�)
	void UpdateLyricState();

	// ��ո�ʣ�����ʾԤ���彻��������ı�
	void ClearLyric();

	// �����������
	void OnSearchLyric(VdkVObjEvent&);
	// �û�ѡ����һ�����������ĸ��
	void OnLyricLoaded(wxCommandEvent& e);

	/// ���Ƹ�ʵ�������
	void OnCopyLyric(VdkVObjEvent&);

	/// �������
	void OnAssociateLyric(VdkVObjEvent&);

	/// ���¼��ظ��
	void OnReloadLyric(VdkVObjEvent&);

	/// �������
	void OnRemoveLyric(VdkVObjEvent&);

	//////////////////////////////////////////////////////////////////////////
	// ����Ӧ�ó���ǰ״̬

	// ����һ��״̬��־λ
	void SetAddinState(int state) { m_state |= state; }

	// �Ƴ����趨��ָ��״̬��־λ
	void RemoveState(int state) { m_state &= ~state; }

	// ȡ��ָ��״̬��־λ
	void ReverseState(int state) { m_state ^= state; }

	// ����ָ��״̬��־λ
	bool TestState(int state) const { return (m_state & state) != 0; }

	//////////////////////////////////////////////////////////////////////////
	// ����

	// ����վ����
	void OnWebsiteLinks(VdkVObjEvent&);

	// �򿪡����ڡ��Ի���
	void OnAbout(VdkVObjEvent&);

	// �򿪡���Ҫ�������Ի���
	void OnFeedback(VdkVObjEvent&);

private:

	wxSingleInstanceChecker* m_checker;

	OOPConf* m_conf; // Ӧ�ó�������

	long m_state; // ��ǰӦ�ó���״̬

	sf::Music m_music;
	MusicEndingDetector m_endingDetector;
	OOPStopWatch m_stopWatch;
	OOPSongPtr m_currSong;

	OOPLyricParser m_lyricParser;
	OOPDesktopLyric* m_desktopLyric;
	OOPLyricMgr m_lyric;

	// ���ںͲ˵��ľ��������Ӧ�ó��������������ǲ����
	MainPanel* m_mainPanel;
	PlayListPanel* m_playListPanel;
	LyricPanel* m_lyricPanel;
	EqPanel* m_eqPanel;

	VdkBitmapArray m_mainMenuBitmaps;

	VdkMenuItem* m_playPauseItem;
	VdkMenuItem* m_stopItem;
	VdkMenuItem* m_muteItem;
	VdkMenuItem* m_prevItem;
	VdkMenuItem* m_nextItem;
	VdkMenu* m_playModes1; // ����ģʽ�˵�
	VdkMenu* m_playModes2; // ����ģʽ�˵�(�����б���)

	// ��ǰ�����б�ĵ�����ʽ
	OOPlayerPlayMode m_playMode;

	//////////////////////////////////////////////////////////////////////////

	// ���� OOPlayer ��Ƥ���б�
	OOPSkin m_skinList;

	// WinAMP ��ʽ��ճ������
	StickyWindows m_windowManager;

	// ����ͼ��
	OOPTrayIcon* m_trayIcon;

	DECLARE_EVENT_TABLE()
};
