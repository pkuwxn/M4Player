/***************************************************************
 * Name:      OOPFileSystem.h
 * Purpose:   ���� OOPlayer ��·��
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-04-19
 **************************************************************/
#pragma once

/// ���� OOPlayer ��·��
class OOPFileSystem
{
public:

	/// ��ȡ��·��
	///
	/// ·���ַ���ĩβ�Ѽ�·���ָ�����
	static wxString GetRootPath();

	/// ��ȡƤ���ļ���Ŀ¼
	///
	/// ·���ַ���ĩβ�Ѽ�·���ָ�����
	static wxString GetSkinRootDir();

	/// ��ȡӦ�ó�����ԴĿ¼
	///
	/// ·���ַ���ĩβ�Ѽ�·���ָ�����
	static wxString GetAppResDir();
};
