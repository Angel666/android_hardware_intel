/*************************************************************************/ /*!
@File           pvrversion.h
@Title          Version numbers and strings.
@Copyright      Copyright (c) Imagination Technologies Ltd. All Rights Reserved
@Description    Version numbers and strings for PVR Consumer services
                components.
@License        Strictly Confidential.
$Log:
*/ /**************************************************************************/

#ifndef _PVRVERSION_H_
#define _PVRVERSION_H_

#define PVR_STR(X) #X
#define PVR_STR2(X) PVR_STR(X)

#define PVRVERSION_MAJ               1
#define PVRVERSION_MIN               8
#define PVRVERSION_BRANCH            18

#define PVRVERSION_FAMILY           "sgxddk"
#define PVRVERSION_BRANCHNAME       "1.8"
#define PVRVERSION_BUILD             789263
#define PVRVERSION_BSCONTROL        "intel_android_sgx_ogles1_ogles2_GPL"

#define PVRVERSION_STRING           "intel_android_sgx_ogles1_ogles2_GPL sgxddk 18 1.8@" PVR_STR2(PVRVERSION_BUILD)
#define PVRVERSION_STRING_SHORT     "1.8@" PVR_STR2(PVRVERSION_BUILD)

#define COPYRIGHT_TXT               "Copyright (c) Imagination Technologies Ltd. All Rights Reserved."

#define PVRVERSION_BUILD_HI          78
#define PVRVERSION_BUILD_LO          9263
#define PVRVERSION_STRING_NUMERIC    PVR_STR2(PVRVERSION_MAJ) "." PVR_STR2(PVRVERSION_MIN) "." PVR_STR2(PVRVERSION_BUILD_HI) "." PVR_STR2(PVRVERSION_BUILD_LO)

#endif /* _PVRVERSION_H_ */
