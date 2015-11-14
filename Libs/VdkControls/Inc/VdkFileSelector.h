#pragma once
#include "VdkControl.h"

class VdkEdit;

/// \brief ���� HTML �еġ�<file>����ǩ
class VdkFileSelector : public VdkCtrlHandler
{
public:

	/// \brief �򿪵�����
	enum SEL_TYPE {

		ST_DIR, ///< ���ļ���
		ST_FILE ///< ���ļ��С�
	};

	/// \brief Ĭ�Ϲ��캯��
	VdkFileSelector();

	/// \brief XRC ��̬����
	virtual void Create(wxXmlNode* node);

	/// \brief ִ��ʵ�ʹ�������
	void Create(VdkWindow* Window, const wxString& strName, const wxRect& rc);

	//////////////////////////////////////////////////////////////////////////

	/// \brief ��ȡ�õ����ļ�/�ļ���·��
	wxString GetPath() const;

	/// \brief ���õ�ǰ��ʾ·��
	void SetPath(const wxString& strPath);

	/// \brief ���á��򿪡�ͨ�öԻ�����ʾ���ļ�����
	void SetExt(const wxString& strDefaultFileName, const wxString& strExtPrompt);

	/// \brief ��ǰ�򿪵������ǡ��ļ������ǡ��ļ��С�
	SEL_TYPE GetType() const { return m_type; }

	/// \brief ���õ�ǰ�򿪵�����
	void SetType(SEL_TYPE type) { m_type = type; }

	/// \brief ���á��򿪡�ͨ�öԻ���ı���
	void SetAlertMessage(const wxString& alertMsg) { m_alertMsg = alertMsg; }

	/// \brief ��ȡ���򿪡�ͨ�öԻ���ı���
	wxString GetAlertMessage() const { return m_alertMsg; }

private:

	// ��ʾ���򿪡�ͨ�öԻ���(��Ϣת����)
	void OnSelectorRed(VdkVObjEvent&);

	// ��ʾ���򿪡�ͨ�öԻ���
	void OnSelector(wxCommandEvent&);

	//////////////////////////////////////////////////////////////////////////

	VdkEdit*		m_edit;

	SEL_TYPE		m_type;
	wxString		m_alertMsg;

	wxString		m_extPrompt;
	wxString		m_defaultFileName;

	DECLARE_DYNAMIC_VOBJECT
};
