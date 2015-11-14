#pragma once

class VdkControl;
class wxXmlNode;

/// 检查XML根节点的名字是否符合所要求的，若不符合输出@a errNo
void CheckXmlConf(wxXmlNode* xmlNode, const wxChar* szNode, int errNo);
