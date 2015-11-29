/***************************************************************
 * Name:      OOPLyricParser.cpp
 * Purpose:   解析 LRC/QRC/... 歌词文件
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2012-06-05
 **************************************************************/
#include "StdAfx.h"
#include "OOPLyricParser.h"

#include "wxUtil.h" // for IsEncodedInUtf8()、ReadAll()

#include <wx/wfstream.h> // for wxFFile
#include <wx/txtstrm.h> // for wxTextInputStream
#include <wx/tokenzr.h> // for wxStringTokenizer

#ifdef __WXMSW__
#   include <wx/msw/msvcrt.h>
#endif

//////////////////////////////////////////////////////////////////////////

LineInfo::LineInfo(const wxString &strLyric, int nStartTime)
    : m_strLyric(strLyric),
      m_nStartTime(nStartTime),
      m_nMilSeconds(0) {

}

void LineInfo::CalcYourTime(const LineInfo *next) {
    m_nMilSeconds = next->m_nStartTime - m_nStartTime;
}

int LineInfo::IsBefore(const LineInfo **p1, const LineInfo **p2) {
    return (*p1)->m_nStartTime - (*p2)->m_nStartTime;
}

//////////////////////////////////////////////////////////////////////////

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(_ListOfLineInfo);

//////////////////////////////////////////////////////////////////////////

OOPLyricParser::OOPLyricParser()
    : m_nTimeSum(0) {

}

OOPLyricParser::~OOPLyricParser() {
    Clear();
}

void OOPLyricParser::Clear() {
    m_lines.DeleteContents(true);
    m_lines.Clear();

    m_nTimeSum = 0;
}

bool OOPLyricParser::LoadFile(const wxString &strLrcFile, int nTimeSum) {
    wxASSERT(nTimeSum > 0);

    wxString lyric(LoadFile(strLrcFile));
    if (!lyric.empty()) {
        return Load(lyric, nTimeSum);
    }

    return false;
}

/*static*/
wxString OOPLyricParser::LoadFile(const wxString &strLrcFile) {
    if (wxFileExists(strLrcFile)) {
        wxFFile input(strLrcFile);
        if (!input.Error()) {
            wxString lyric;

            // 检测是否以 UTF-8 格式编码
            char buf[3];
            input.Read(buf, sizeof buf);
            // 重置文件指针
            input.Seek(0);

            if (IsEncodedInUtf8(buf)) {
                input.ReadAll(&lyric);
            } else {
                input.ReadAll(&lyric, wxConvAuto(wxFONTENCODING_GB2312));
            }

            return lyric;
        }
    }

    wxLogDebug(L"Cannot load LRC file.\n`%s`", strLrcFile);

    return wxEmptyString;
}

bool OOPLyricParser::Load(const wxString &lyric, int nTimeSum) {
    wxASSERT(nTimeSum > 0);

    Clear();

    if (!lyric.empty()) {
        wxStringTokenizer tokenizer(lyric, L"\r\n");
        while (tokenizer.HasMoreTokens()) {
            ParseLine(tokenizer.GetNextToken());
        }

        BuilidLines(nTimeSum);
    } else {
        wxLogError(L"歌词字符串为空");
    }

    return IsOk();
}

void OOPLyricParser::Load(wxInputStream &stream, int nTimeSum) {
    wxASSERT(nTimeSum > 0);
    wxASSERT(stream.IsOk());

    Clear();

    //==================================================

    wxTextInputStream text(stream);
    while (!stream.Eof()) {
        ParseLine(text.ReadLine());
    }

    BuilidLines(nTimeSum);
}

bool OOPLyricParser::IsOk() const {
    return !m_lines.empty();
}

bool OOPLyricParser::BuilidLines(int nTimeSum) {
    wxASSERT(nTimeSum > 0);

    if (!IsOk()) {
        wxLogDebug(L"未识别的歌词字符串");
        return false;
    }

    //-----------------------------------------------------

    m_lines.Sort(LineInfo::IsBefore);

    // 第一行开始前的停顿间隔，手动增加一个空行
    if (m_lines[0]->m_nStartTime > 0) {
        LineInfo *beforeFirstLine = new LineInfo(wxEmptyString, 0);
        m_lines.push_front(beforeFirstLine);
    }

    //-----------------------------------------------------
    // 提取各句显示的实际时间

    LineInfo *pInfo;
    int numLines = m_lines.size(), curr = 0;

    ListOfLineInfo::iterator i;
    for (i = m_lines.begin(); curr < numLines - 1; ++curr) {
        pInfo = *i;
        pInfo->CalcYourTime(*(++i));
    }

    m_nTimeSum = nTimeSum;

    // 对最后一行进行特殊处理
    pInfo = *i;
    pInfo->m_nMilSeconds = m_nTimeSum - pInfo->m_nStartTime;
    if (pInfo->m_nMilSeconds < 0) {
        // TODO: 会不会走到这里？
        enum {
            // 假如歌词长度比歌曲长度要长，那么最后一句歌词持续的时间
            // 默认为一小段时间
            DEFAULT_LAST_LINE_LAST = 1000,
        };

        pInfo->m_nMilSeconds = DEFAULT_LAST_LINE_LAST;
    }

    return true;
}

void OOPLyricParser::ParseLine(const wxString &line) {
    wxString strLine(line);
    if (!strLine.Trim(true).Trim(false).empty()) {
        strLine.Replace(L"：", L":");
    } else {
        return;
    }

    wxString strTime, strNum, strLyric;
    wxString::size_type startPos = 0, endPos = -1;

    endPos = strLine.Find(L']', true);
    // 即使歌词内容为空也继续，因为可能仅仅添加一个空行用以分隔
    if (endPos != wxString::npos) {
        strLyric = strLine.Mid(endPos + 1);
    } else { // 无效行(没有时间标签或其它用“[”“]”括起来的行)
        return;
    }

    endPos = -1;
    while (true) {
        startPos = strLine.find(L'[', startPos);

        // 时间标签可能是连续的
        if ((startPos == wxString::npos) || (startPos != endPos + 1)) {
            break;
        }

        endPos = strLine.find(L']', startPos);
        if (endPos != wxString::npos) {
            startPos++;
            strTime = strLine.Mid(startPos, endPos - startPos);
        } else { // TODO: 格式错误
            break;
        }

        // 格式错误
        if ((strTime.length() != 8) && (strTime.length() != 5)) {
            break;
        }

        // 检测时间标签的合法性
        const static wchar_t *sLegalChars = L"0123456789:.";

        bool ok = true;
        wxString::size_type len = strTime.length();
        for (wxString::size_type i = 0; i < len; i++) {
            if (!wcschr(sLegalChars, strTime[i])) {
                ok = false;
                break;
            }
        }

        if (!ok) {
            break;
        }

        //-----------------------------------------------------
        // 00:32.04

        long nStartTime = 0, nNum;

        strNum = strTime.Mid(0, 2);
        strNum.ToLong(&nNum);
        nStartTime += nNum * 60 * 1000;

        strNum = strTime.Mid(3, 2);
        strNum.ToLong(&nNum);
        nStartTime += nNum * 1000;

        // #(00:32.04) == 8
        if (strTime.length() == 8) {
            strNum = strTime.Mid(6, 2);
            strNum.ToLong(&nNum);
            nStartTime += nNum * 10;
        }

        LineInfo *info = new LineInfo(strLyric, nStartTime);
        m_lines.Append(info);

        startPos = endPos;

    } // END while( true )
}

wxString OOPLyricParser::GetLyric() const {
    wxString lyricAll;

    LineIter it(begin()), e(end());
    for (; it != e; ++it) {
        lyricAll += (*it)->m_strLyric + L"\r\n";    // TODO: 使用系统原生换行符
    }

    return lyricAll;
}

size_t OOPLyricParser::GetMaxTextWidth(const wxFont &font) const {
    size_t m_nMaxTextWidth = 0;

    wxMemoryDC mdc;
    mdc.SetFont(font);

    LineIter it(begin()), e(end());
    for (; it != e; ++it) {
        wxCoord w;
        mdc.GetTextExtent((*it)->m_strLyric, &w, NULL);

        if (size_t(w) > m_nMaxTextWidth) {
            m_nMaxTextWidth = w;
        }
    }

    return m_nMaxTextWidth;
}

ListOfLineInfo::size_type OOPLyricParser::IndexOf(LineIter it) const {
    return m_lines.IndexOf(*it);
}

LineIter OOPLyricParser::GetLine(ListOfLineInfo::size_type index) const {
    wxASSERT(index < GetLinesCount());

    LineIter it(m_lines.begin());
    for (size_t i = 0; i < index; i++) {
        ++it;
    }

    return it;
}

LineIter OOPLyricParser::WhichLine(size_t timeOffset) const {
    if (IsOk()) {
        LineIter i(begin()), j(i);
        for (++j; j != end(); ++i, ++j) {
            if (((*i)->GetStartTime() <= timeOffset) &&
                    ((*j)->GetStartTime() > timeOffset)) {
                return i;
            }
        }

        // 对最后一行作特殊处理
        if ((*i)->GetStartTime() <= timeOffset) {
            return i;
        }
    }

    return end();
}
