/***************************************************************
 * Name:      DelayModStack.h
 * Purpose:   �ӳ��޸�ջ
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-10
 **************************************************************/
#pragma once
#include <loki/Singleton.h>
#include <wx/vector.h>
#include "OOPSongPtr.h"

/// �ӳ��޸�ջ
///
/// ��������ļ���ռ�ã����û���Ҫ��Ը����ļ������޸ģ����ļ�����������
/// �޸ĺ�ı�ǩ����ô����Ҫʹ��������������޸��Ƴٵ�һ�����ʵ�ʱ������
/// ֹͣ���ŵ�ǰ����ʱ����
class DelayModStack
{
public:

	/* ��Ҫ�ӳٽ��е��޸Ĳ��� */
	enum ModAction {
		MA_RENAME_FILE, /*! �����������ļ� */
		MA_SAVE_TAGS, /*! �����޸ĺ�ı�ǩ */
	};

	/// ���һ�׸�����ջ��
	///
	/// ��Ҫ�ӳٽ��е��޸Ĳ���Ϊ�����������ļ�
	bool Add(OOPSongPtr song);

	/// ���һ�׸�����ջ��
	///
	/// ��Ҫ�ӳٽ��е��޸Ĳ���Ϊ�����������ļ���
	bool Add(OOPSongPtr song, const wxString& newPath);

	/// �����ύָ���������ӳ��޸���Ϣ
	bool Commit(OOPSongPtr song);

	/// �����ύ�����ѱ���ĸ������ӳ��޸���Ϣ
	void CommitAll();

	/// ���ջ
	void Clear();

private:

	DelayModStack() {}
	~DelayModStack() {}
	DelayModStack(const DelayModStack&) {}
	DelayModStack& operator = (const DelayModStack&) { return *this; }
	DelayModStack* operator & () { return this; }

	friend struct Loki::CreateUsingNew< DelayModStack >;

private:
	
	typedef wxVector< OOPSongPtr > Stack;
	Stack m_stack;

	//-------------------------------

	struct DelayRenameAction {
		OOPSongPtr song;
		wxString newPath;
	};

	typedef wxVector< DelayRenameAction > RenameStack;
	RenameStack m_renameStack;

	friend class RenameSessionFinder;
};

typedef Loki::SingletonHolder< DelayModStack > SingleDelayModStack;
