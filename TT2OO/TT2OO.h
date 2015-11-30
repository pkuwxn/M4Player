#pragma once

#include <tinyxml.h>

#include <map>
#include <vector>
#include <string>

using namespace std;

TiXmlDocument *LoadXmlDoc(string &strFile, bool utf8 = false);
bool ParsePanel(TiXmlElement *&Panel, const map<string, const char *> &vcmap);

void ParseAttri(TiXmlElement *src, TiXmlElement *dest);

void Pos2Rect(const char *nodeName, 
              const char *attriName,
              TiXmlElement *Panel, 
              TiXmlElement *elCtrl);

void InsertChildNode(TiXmlElement *parent,
                     const char *szName, 
                     const char *szValue, 
                     bool linkToFirst = false);

void MergeSpecicalConf(TiXmlDocument *docSpecical, TiXmlElement *parent);

/// \brief 主要是为了适配 Linux
///
/// Linux 文件名区分大小写
int CheckPossibleFileNames(const string &strDir, int nAlt, ...);

/// \brief 主要是为了适配 Linux
///
/// Linux 文件名区分大小写
int CheckPossibleFileNames(const string &strDir,
                           const vector<const char *> &szFiles);

/// \brief 将 TTPlayer 的属性名标准化
///
/// 这是全局性的标准化，例如 TTP 的“font_size”一定对应
/// OOP 的“font-size”。
/// \see RenameOOPAttri
void RenameAttri(string &attriName);

/// \brief 重命名一个属性
///
/// 例如将<align>改名为<text-align>。
/// \see RenameAttri
void RenameOOPAttri(TiXmlElement *oopCtrl,
                    const string &attriName, 
                    const string &newName);

wchar_t *Ansi2Unicode(const char *szAnsi);
char *Ansi2Utf8(const char *szAnsi);

template<class T>
void DELA(T *&p) {
    if (p) {
        delete [] p;
        p = NULL;
    }
}

/// \brief 读取文本文件的所有内容
char *ReadAll(const char *szFileName);
