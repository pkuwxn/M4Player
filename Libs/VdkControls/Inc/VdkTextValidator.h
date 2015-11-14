/***************************************************************
 * Name:      VdkTextValidator.h
 * Purpose:   ���� VdkTextValidator ��
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2011-7-31
 **************************************************************/
#pragma once

/// \brief wxTextValidator ��ֻ֧��ԭ���ؼ�
class VdkTextValidator : public wxTextValidator
{
public:

	/// \brief ���ƹ��캯��
	VdkTextValidator(const VdkTextValidator& v);

	/// \brief ���캯��
	VdkTextValidator(long style = wxFILTER_NONE, 
					 wxString* valPtr = NULL);

	/// \brief �����Լ�
	virtual wxObject* Clone() const;

	/// \brief �����Լ�
	bool Copy(const VdkTextValidator& v);

	/// \brief ��ָ֤���ַ����Ƿ����Ҫ��
	/// \param str Ҫ��֤���ַ���
	/// \param parent ��������Ի���ʱ�Ի���ĸ�����
	virtual bool Validate(const wxString& str, wxWindow* parent);
};
