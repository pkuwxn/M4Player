#pragma once

class VdkControl;
class wxXmlNode;

/// 检查XML根节点的名字是否符合所要求的，若不符合输出 @a errNo
void CheckXmlConf(wxXmlNode *xmlNode, const wxChar *szNode, int errNo);

/// 返回一个路径字符串的窄版本
///
/// 对于 Windows，编码是 OEM（GB2312），对于 Linux 则总是 UTF-8
wxScopedCharBuffer NarrowedPath(const wxString &path);
