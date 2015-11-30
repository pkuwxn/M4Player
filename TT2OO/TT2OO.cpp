
#include "TT2OO.h"

// for _access
#ifdef _WIN32
#   include <io.h>
#else
#   include <unistd.h>
#   define _access access
#endif

#include <stdarg.h> // for va_start

#include <iostream>
#include <sstream>

//////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#   define PATH_SEP '\\'
#else
#   define PATH_SEP '/'
#endif

string g_strDir;
string g_strMaskColor;
bool g_silent = true;

int main(int argc, char **args) {
    for (int i = 1; i < argc; i++) {
        if ((strcmp(args[i], "-v") == 0) || (strcmp(args[i], "--verbose") == 0)) {
            g_silent = false;
        } else {
            g_strDir = args[i];
        }
    }

    if (g_strDir.empty()) {
        cout << "请输入皮肤目录：";
        getline(cin , g_strDir);
    }

    if (g_strDir.find_last_of(PATH_SEP) != g_strDir.length() - 1) {
        g_strDir += PATH_SEP;
    }

    //////////////////////////////////////////////////////////////////////////

    int id;
    string strFile;
    vector<const char *> vFiles;
    vFiles.push_back("skin.xml");
    vFiles.push_back("Skin.xml");

    id = CheckPossibleFileNames(g_strDir, vFiles);
    if (id != -1) {
        strFile = g_strDir + vFiles[id];
    } else {
        cout << g_strDir << "似乎不是一个正确的 TTPlayer 皮肤目录" << endl;
        exit(-1);
    }

    TiXmlDocument *docSkin(LoadXmlDoc(strFile));

    // 皮肤作者信息
    TiXmlDocument doc;
    doc.LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", "yes"));

    //////////////////////////////////////////////////////////////////////////

    TiXmlElement *root = docSkin->RootElement();
    TiXmlElement *el = new TiXmlElement("Skin");
    doc.LinkEndChild(el);

    TiXmlElement *elAttri;
    TiXmlText *elAttriText;

    TiXmlAttribute *attri = root->FirstAttribute();
    while (attri) {
        if (strcmp(attri->Name(), "version")) {
            elAttri = new TiXmlElement(attri->Name());
            elAttriText = new TiXmlText(attri->Value());
            elAttri->LinkEndChild(elAttriText);

            el->LinkEndChild(elAttri);
        }

        attri = attri->Next();
    }

    if (!g_silent) {
        doc.Print();
    }

    string strIntroDoc(g_strDir + "Skin_OOP.xml");
    doc.SaveFile(strIntroDoc.c_str());

    g_strMaskColor = root->Attribute("transparent_color");

    //////////////////////////////////////////////////////////////////////////

    map<string, const char *> vcmap;

    vcmap.insert(make_pair("progress", "VdkSlider"));
    vcmap.insert(make_pair("volume", "VdkSlider"));
    vcmap.insert(make_pair("balance", "VdkSlider"));
    vcmap.insert(make_pair("surround", "VdkSlider"));
    vcmap.insert(make_pair("preamp", "VdkSlider"));
    vcmap.insert(make_pair("eqfactor", "VdkSlider"));

    vcmap.insert(make_pair("scrollbar", "VdkScrollBar"));

    vcmap.insert(make_pair("stereo", "VdkLabel"));
    vcmap.insert(make_pair("status", "VdkLabel"));

    vcmap.insert(make_pair("browser", "VdkToggleButton"));
    vcmap.insert(make_pair("lyric", "VdkToggleButton"));
    vcmap.insert(make_pair("equalizer", "VdkToggleButton"));
    vcmap.insert(make_pair("playlist", "VdkToggleButton"));
    vcmap.insert(make_pair("mute", "VdkToggleButton"));
    vcmap.insert(make_pair("ontop", "VdkToggleButton"));

    vcmap.insert(make_pair("title", "VdkStaticImage"));

    vcmap.insert(make_pair("visual", "Visual"));
    vcmap.insert(make_pair("icon", "OOPIcon"));
    vcmap.insert(make_pair("led", "OOPLed"));
    vcmap.insert(make_pair("info", "OOPLabel"));
    vcmap.insert(make_pair("toolbar", "OOPToolBar"));
    vcmap.insert(make_pair("mini_border", "OOPBorder"));

    //////////////////////////////////////////////////////////////////////////

    // 主窗口有些按钮的名字与子窗口的其他类型控件有冲突
    TiXmlElement *child = root->FirstChild()->ToElement();
    ParsePanel(child, vcmap);

    //////////////////////////////////////////////////////////////////////////

    vcmap["lyric"] = "OOPLyric"; // TODO:
    vcmap["playlist"] = "OOPList";
    vcmap["enabled"] = "VdkToggleButton"; // in EqPanel

    int count = 0;
    while (ParsePanel(child, vcmap)) {
        ++count;
    }
    if (count < 3) {
        cout << "皮肤文件似乎有错。\n";
    }

    if (g_silent) {
        cout << "转换成功!\n";
    }
}

bool ParsePanel(TiXmlElement *&ttpPanel, const map<string, const char *> &vcmap) {
    if (!ttpPanel) {
        return false;
    }

    //////////////////////////////////////////////////////////////////////////

    TiXmlDocument doc;
    doc.LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", "yes"));

    TiXmlElement *el(new TiXmlElement("resource"));
    el->SetAttribute("xmlns", "http://www.wxwidgets.org/wxxrc");
    el->SetAttribute("version", "2.3.0.1");
    doc.LinkEndChild(el);

    //////////////////////////////////////////////////////////////////////////

    TiXmlElement *oopCtrl;
    TiXmlElement *ttpCtrl;
    TiXmlAttribute *attri;

    // 要保存的XML文档名字
    string strName;

    //////////////////////////////////////////////////////////////////////////

    TiXmlElement *elForm = new TiXmlElement("object");
    elForm->SetAttribute("class", "VdkWindow");
    elForm->SetAttribute("name", ttpPanel->Value());

    strName = string(ttpPanel->Value());

    // 面板的参数
    attri = ttpPanel->FirstAttribute();
    while (attri) {
        if (strcmp(attri->Name(), "position") == 0) {
            Pos2Rect("rect", "position", elForm, ttpPanel);
        } else if (strcmp(attri->Name(), "resize_rect") == 0) {
            // 连 resize_rect 也是两点式
            Pos2Rect("resize_rect", "resize_rect", elForm, ttpPanel);
        } else {
            InsertChildNode(elForm, attri->Name(), attri->Value());
        }

        attri = attri->Next();
    }

    el->LinkEndChild(elForm);

    //////////////////////////////////////////////////////////////////////////
    // 各个控件的参数

    map<string, const char *>::const_iterator mi;
    ttpCtrl = ttpPanel->FirstChildElement();

    while (ttpCtrl) {
        oopCtrl = new TiXmlElement("object");

        oopCtrl->SetAttribute("name", ttpCtrl->Value());
        ParseAttri(ttpCtrl, oopCtrl); // 在这里处理控件定义

        mi = vcmap.find(ttpCtrl->Value());
        if (mi != vcmap.end()) {
            // 将 VdkScrollBar 作为 VdkListBox 的一份子
            if (strcmp(mi->second, "VdkScrollBar") == 0) {
                delete oopCtrl;
                ttpCtrl = ttpCtrl->NextSiblingElement();
                continue;
            }

            oopCtrl->SetAttribute("class", mi->second);

            // 播放列表和歌词秀有特殊的配置文件
            if (strcmp(mi->second, "OOPList") == 0) {
                string strAlign("sync_x_y");

                // OOPList 的对齐属性为 sync_x_y
                TiXmlElement *align = oopCtrl->FirstChildElement("align");
                if (align) {
                    strAlign = align->GetText();
                    strAlign += "+sync_x_y";

                    // 避免重复的结点
                    oopCtrl->RemoveChild(align);
                }

                InsertChildNode(oopCtrl, "align", strAlign.c_str());

                // 整合特殊配置文件
                string strFileName(g_strDir + "PlayList.xml");
                TiXmlDocument *docSpecical(LoadXmlDoc(strFileName, true));
                MergeSpecicalConf(docSpecical, oopCtrl);
                delete docSpecical;

                // 整合滚动条
                TiXmlElement *child, *parent(ttpCtrl->Parent()->ToElement());
                for (child = parent->FirstChildElement();
                     child; child = child->NextSiblingElement()) {
                    if (strcmp(child->Value(), "scrollbar") == 0) {
                        break;
                    }
                }

                if (child) {
                    TiXmlElement *sb(new TiXmlElement("object"));
                    sb->SetAttribute("class", "VdkScrollBar");

                    sb->SetAttribute("name", child->Value());
                    ParseAttri(child, sb);

                    // 对齐属性为 sync_y+right
                    InsertChildNode(sb, "align", "sync_y+right", true);

                    // 按钮的状态数为 3
                    InsertChildNode(sb, "thumb_expl", "3");

                    oopCtrl->LinkEndChild(sb);
                }
            } // END if( strcmp(mi->second, "OOPList") == 0 )
            else if (strcmp(mi->second, "OOPLyric") == 0) {
                const char *panelName = elForm->Attribute("name");
                if (strcmp(panelName, "mini_window") == 0) {
                    oopCtrl->SetAttribute("class", "VdkToggleButton");
                } else {
                    string strFileName(g_strDir + "Lyric.xml");
                    TiXmlDocument *docSpecical(LoadXmlDoc(strFileName, true));
                    MergeSpecicalConf(docSpecical, oopCtrl);
                    delete docSpecical;
                }
            } else if (strcmp(mi->second, "VdkLabel") == 0) {
                RenameOOPAttri(oopCtrl, "align", "text-align");
            }
            // 标题位图都是水平居中的，但<align>的值不符规范（TTPlayer默认center为水平居中）
            else if (mi->first == "title") {
                TiXmlElement *el(oopCtrl->FirstChildElement("align"));
                if (el) {
                    if (strcmp(el->GetText(), "center") == 0) {
                        if (el->FirstChild()) {
                            TiXmlNode *chd = el->FirstChild();
                            chd->SetValue("center_x");
                        }
                    }

                }
            }
            // 均衡器窗口的两个滚动条需要特殊对待
            else if (mi->first == "preamp" || mi->first == "eqfactor") {
                InsertChildNode(oopCtrl, "vertical", "true");
            }

        } // END if( mi != vcmap.end() )
        else {
            oopCtrl->SetAttribute("class", "VdkButton");
        }

        elForm->LinkEndChild(oopCtrl);

        ttpCtrl = ttpCtrl->NextSiblingElement();
    }

    TiXmlNode *next(ttpPanel->NextSibling());
    if (next) {
        ttpPanel = next->ToElement();
    } else {
        ttpPanel = NULL;
    }

    if (!g_silent) {
        doc.Print();
    }

    strName = g_strDir + strName + ".xml";
    doc.SaveFile(strName.c_str());

    return true;
}

void Pos2Rect(const char *nodeName, 
              const char *attriName,
              TiXmlElement *oopCtrl, 
              TiXmlElement *ttpCtrl) {
    static string strPosition;
    strPosition = ttpCtrl->Attribute(attriName);

    string num;
    string::iterator i(strPosition.begin());

    int coords[4];

    for (int k = 0; k < 4; k++) {
        while (i != strPosition.end() && *i != ',') {
            num.push_back(*i);
            ++i;
        }

        if (i != strPosition.end()) {
            ++i;    // 跳过','
        }

        coords[k] = atoi(num.c_str());
        num.clear();
    }

    //---------------------------------------------------
    // 对不正确的矩形，默认为空

    if (coords[2] < coords[0]) {
        coords[2] = coords[0];
    }

    if (coords[3] < coords[1]) {
        coords[3] = coords[1];
    }

    //---------------------------------------------------

    TiXmlElement *elAttri;
    TiXmlText *elAttriText;
    ostringstream ss;

    ss  << coords[0] << ',' << coords[1] << ','
        << coords[2] - coords[0] << ',' << coords[3] - coords[1];
    elAttri = new TiXmlElement(nodeName);
    elAttriText = new TiXmlText(ss.str().c_str());
    oopCtrl->LinkEndChild(elAttri);
    elAttri->LinkEndChild(elAttriText);
}

int CheckPossibleFileNames(const string &strDir, int nAlt, ...) {
    va_list lpParam;
    va_start(lpParam, nAlt);

    string strFile;
    for (int i = 0; i < nAlt; i++) {
        strFile = strDir + va_arg(lpParam, char *);

        if (_access(strFile.c_str(), 0) != -1) {
            return i;
        }
    }

    return nAlt;
}

int CheckPossibleFileNames(const string &strDir,
                           const vector<const char *> &szFiles) {
    string strFile;
    int nCount = szFiles.size();

    for (int i = 0; i < nCount; i++) {
        strFile = strDir + szFiles[i];

        if (_access(strFile.c_str(), 0) != -1) {
            return i;
        }
    }

    return -1;
}

void InsertChildNode(TiXmlElement *parent,
                     const char *szName, 
                     const char *szValue, 
                     bool linkToFirst) {
    TiXmlElement *elAttri = new TiXmlElement(szName);
    TiXmlText *elAttriText = new TiXmlText(szValue);
    elAttri->LinkEndChild(elAttriText);

    if (linkToFirst) {
        TiXmlNode *firstChild = parent->FirstChild();
        if (firstChild) {
            parent->InsertBeforeChild(firstChild, *elAttri);
            delete elAttri;

            return;
        }
    }

    parent->LinkEndChild(elAttri);
}

TiXmlDocument *LoadXmlDoc(string &strFile, bool utf8) {
    TiXmlDocument *docSkin(new TiXmlDocument);
    if (!utf8) {
        char *szData(ReadAll(strFile.c_str()));

        if (szData == NULL) {
            delete docSkin;
            return NULL;
        }

        char *szUtf8Data;
        szUtf8Data = Ansi2Utf8(szData);

        docSkin->Parse(szUtf8Data);

        DELA(szData);
        DELA(szUtf8Data);
    } else {
        docSkin->LoadFile(strFile.c_str());
    }

    if (docSkin->Error()) {
        delete docSkin;
        return NULL;
    }

    return docSkin;
}

void MergeSpecicalConf(TiXmlDocument *docSpecical, TiXmlElement *parent) {
    // 这个特殊配置文件不存在
    if (!docSpecical) {
        return;
    }

    TiXmlAttribute *attri(docSpecical->RootElement()->
                          FirstChild()->
                          ToElement()->
                          FirstAttribute());
    while (attri) {
        InsertChildNode(parent, attri->Name(), attri->Value());
        attri = attri->Next();
    }
}

void ParseAttri(TiXmlElement *src, TiXmlElement *dest) {
    TiXmlAttribute *attri(src->FirstAttribute());
    while (attri) {
        if (strcmp(attri->Name(), "position") == 0) {
            Pos2Rect("rect", "position", dest, src);
        } else {
            string attriName(attri->Name()), attriValue(attri->Value());

            RenameAttri(attriName);
            if (attriName == "font-size") { // 不知道为什么TT的字体要大3号
                int size = atoi(attriValue.c_str());
                if (size == 13) {
                    size = 12;
                }

                char temp[4];
                sprintf(temp, "%d", size -= 3);

                attriValue = temp;
            }

            InsertChildNode(dest, attriName.c_str(), attriValue.c_str());
        }

        attri = attri->Next();
    }
}

void RenameAttri(string &attriName) {
    static map<string, string> mapping;
    if (mapping.size() == 0) {
        mapping["font_size"] = "font-size";
        mapping["font"] = "font-family";
        mapping["color"] = "font-color";
    }

    map<string, string>::iterator i = mapping.find(attriName);
    if (i != mapping.end()) {
        attriName = i->second;
    }
}

void RenameOOPAttri(TiXmlElement *oopCtrl,
                    const string &attriName, 
                    const string &newName) {
    TiXmlElement *el(oopCtrl->FirstChildElement(attriName.c_str()));

    if (el) {
        TiXmlElement *newEl = new TiXmlElement(newName.c_str());
        TiXmlNode *chd = el->FirstChild();
        if (chd) {
            newEl->InsertEndChild(*chd);
        }

        oopCtrl->RemoveChild(el);
        oopCtrl->LinkEndChild(newEl);
    }
}

char *ReadAll(const char *szFileName) {
    FILE *f = fopen(szFileName, "r");
    if (!f) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    size_t fsz = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = new char[fsz + 1];
    size_t read = fread(buf, 1, fsz, f);
    buf[read] = 0;

    return buf;
}

//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32

#include <Windows.h>

wchar_t *_Ansi2Unicode(const char *szAnsi) {
    int nWideLen = MultiByteToWideChar
        (CP_ACP, 0, szAnsi, (DWORD) strlen(szAnsi), 0, 0);
    wchar_t *szWideForm = new wchar_t[nWideLen + 1];

    MultiByteToWideChar
        (CP_ACP, 0, szAnsi, (DWORD) strlen(szAnsi), szWideForm, nWideLen);

    szWideForm[nWideLen] = 0;
    return szWideForm;
}

char *Ansi2Utf8(const char *szAnsi) {
    // trans2wide
    wchar_t *unicode(_Ansi2Unicode(szAnsi));
    int nWideLen = wcslen(unicode);

    // wide2utf8
    int nUtf8Len = WideCharToMultiByte
        (CP_UTF8, 0, unicode, nWideLen, 0,  0, NULL, NULL);

    char *szUtf8 = new char[nUtf8Len + 1];
    memset(szUtf8, 0, nUtf8Len + 1);

    WideCharToMultiByte
        (CP_UTF8, 0, unicode, nWideLen, szUtf8, nUtf8Len, NULL, NULL);

    delete [] unicode;
    return szUtf8;
}

#else // !_WIN32

#include <iconv.h>
#include <memory.h>

int code_convert(const char *from_charset,
                 const char *to_charset,
                 size_t *pInBytesLeft, size_t *pOutBytesLeft,
                 const char *inbuf, char *outbuf) {
    iconv_t cd;
    cd = iconv_open(to_charset, from_charset);
    if (cd == 0) {
        return -1;
    }

    memset(outbuf, 0, *pOutBytesLeft);
    char *dummy = const_cast<char *>(inbuf);
    if (iconv(cd, &dummy, pInBytesLeft, &outbuf, pOutBytesLeft) == (size_t) -1) {
        return -1;
    }

    iconv_close(cd);
    return 0;
}

char *Ansi2Utf8(const char *szAnsi) {
    size_t nInBytes = strlen(szAnsi);
    size_t nOutBytes = nInBytes * 3 + 1;
    char *outbuf = new char[nOutBytes];

    code_convert("gbk", "utf-8", &nInBytes, &nOutBytes, szAnsi, outbuf);

    return outbuf;
}

#endif

