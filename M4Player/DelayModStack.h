/***************************************************************
 * Name:      DelayModStack.h
 * Purpose:   延迟修改栈
 * Author:    Wang Xiaoning (vanxining@139.com)
 * Created:   2012-03-10
 **************************************************************/
#pragma once
#include <loki/Singleton.h>
#include <wx/vector.h>
#include "OOPSongPtr.h"

/// 延迟修改栈
///
/// 假如歌曲文件被占用，而用户又要求对歌曲文件进行修改，如文件更名、保存
/// 修改后的标签，那么便需要使用这个机制来将修改推迟到一个合适的时机（如
/// 停止播放当前歌曲时）。
class DelayModStack {
public:

    /* 需要延迟进行的修改操作 */
    enum ModAction {
        MA_RENAME_FILE, /*! 重命名歌曲文件 */
        MA_SAVE_TAGS, /*! 保存修改后的标签 */
    };

    /// 添加一首歌曲到栈里
    ///
    /// 需要延迟进行的修改操作为重命名歌曲文件
    bool Add(OOPSongPtr song);

    /// 添加一首歌曲到栈里
    ///
    /// 需要延迟进行的修改操作为重命名歌曲文件。
    bool Add(OOPSongPtr song, const wxString &newPath);

    /// 尝试提交指定歌曲的延迟修改信息
    bool Commit(OOPSongPtr song);

    /// 尝试提交所有已保存的歌曲的延迟修改信息
    void CommitAll();

    /// 清空栈
    void Clear();

private:

    DelayModStack() {}
    ~DelayModStack() {}
    DelayModStack(const DelayModStack &) {}
    DelayModStack &operator = (const DelayModStack &) {
        return *this;
    }
    DelayModStack *operator & () {
        return this;
    }

    friend struct Loki::CreateUsingNew<DelayModStack>;

private:

    typedef wxVector<OOPSongPtr> Stack;
    Stack m_stack;

    //-------------------------------

    struct DelayRenameAction {
        OOPSongPtr song;
        wxString newPath;
    };

    typedef wxVector<DelayRenameAction> RenameStack;
    RenameStack m_renameStack;

    friend class RenameSessionFinder;
};

typedef Loki::SingletonHolder<DelayModStack> SingleDelayModStack;
