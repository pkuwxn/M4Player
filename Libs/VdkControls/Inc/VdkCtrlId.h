/***************************************************************
 * Name:      VdkCtrlId.h
 * Purpose:   VdkCtrlId 的定义，用于减少依赖、免于重复定义
 * Author:    Wang Xiaoning (m4player@foxmail.com)
 * Created:   2010-12-03
 * Copyright: Wang Xiaoning
 **************************************************************/
#pragma once

typedef int VdkCtrlId;
const VdkCtrlId VC_ID_ANY = -1;

/// \brief 获取唯一的控件 ID
/// \param advanced 在此基础上为本次调用保留多少个 ID
VdkCtrlId VdkGetUniqueId(int advanced = 1);
