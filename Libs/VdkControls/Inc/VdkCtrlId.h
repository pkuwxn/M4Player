/***************************************************************
 * Name:      VdkCtrlId.h
 * Purpose:   VdkCtrlId �Ķ��壬���ڼ��������������ظ�����
 * Author:    vanxining (vanxining@139.com)
 * Created:   2010-12-03
 * Copyright: vanxining
 **************************************************************/
#pragma once

typedef int VdkCtrlId;
const VdkCtrlId VC_ID_ANY = -1;

/// \brief ��ȡΨһ�Ŀؼ� ID
/// \param advanced �ڴ˻�����Ϊ���ε��ñ������ٸ� ID
VdkCtrlId VdkGetUniqueId(int advanced = 1);
