#pragma once

class VdkControl;
class wxXmlNode;

/// ���XML���ڵ�������Ƿ������Ҫ��ģ������������@a errNo
void CheckXmlConf(wxXmlNode* xmlNode, const wxChar* szNode, int errNo);
