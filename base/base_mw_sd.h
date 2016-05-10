#ifndef __BASE_MW_SD_H__
#define __BASE_MW_SD_H__

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************************
	include
**********************************************************************************/
#include "base_comm.h"

/**********************************************************************************
	macro
**********************************************************************************/
#define BASE_MW_SDDEV_FILE "/etc/hotplug/sddev"
/*
#define BASE_MW_SD_PATH "/opt"
#define BASE_MW_SD_MAINCARDPATH "/opt/sd/mainCard/"
#define BASE_MW_SD_SUBCARDPATH "/opt/sd/subCard/"
#define BASE_MW_SD_SNAPPATH "/opt/sd/snapshot/"
*/
#define NANDFLASH	"nandflash"
#define TFCARD		"TF_Card"

#define NAND_TESTFILE		"/opt/mainCard/test.264"
#define TFCARD_TESTFILE		"/opt/subCard/test.264"

#define BASE_MW_SD_PATH "/opt/mainCard"
#define BASE_MW_NAND_PATH "/opt/subCard/"

#define BASE_MW_SD_MAINCARDPATH "/opt/mainCard/rec/"
#define BASE_MW_SD_SNAPPATH "/opt/mainCard/snapshot/"
#define BASE_MW_SD_SUBCARDPATH "/opt/subCard/"

#define BASE_MW_SD_MOUNT_LENGTH 64
#define BASE_MW_SD_DEVNAME_LENGTH 32

#define BASE_MW_SD_B2KB(size) ((size)>>10)
#define BASE_MW_SD_KB2MB(size) ((size)>>10)
#define BASE_MW_SD_MB2GB(size) ((size)>>10)

#define BASE_MW_SD_KB_SIZE (1<<10)
#define BASE_MW_SD_MB_SIZE (1<<20)
#define BASE_MW_SD_GB_SIZE (1<<30)

#define BASE_MW_RECCARD_RATIO	(0.9)
#define BASE_MW_SNAPCARD_RATIO	(0.1)
/*
#define BASE_MW_SD_FILE_SIZE (1<<30)
#define BASE_MW_SD_FILE_SIZEKB (BASE_MW_SD_B2KB(BASE_MW_SD_FILE_SIZE))
*/

#define BASE_MW_SD_FILE_SIZE (2 * BASE_MW_SD_GB_SIZE)
// #define BASE_MW_SD_FILE_SIZEKB (BASE_MW_SD_B2KB(BASE_MW_SD_FILE_SIZE))
#define BASE_MW_SD_FILE_SIZEKB (1 << 19)

#define BASE_MW_SNAP_FILE_SIZE (10 * BASE_MW_SD_MB_SIZE)
#define BASE_MW_SNAP_FILE_SIZEKB (BASE_MW_SD_B2KB(BASE_MW_SNAP_FILE_SIZE))

#define BASE_MW_NAND_FILE_SIZE (100 * BASE_MW_SD_MB_SIZE)
#define BASE_MW_NAND_FILE_SIZEKB (BASE_MW_SD_B2KB(BASE_MW_NAND_FILE_SIZE))

#define BASE_MW_SD_FILE_TEST (0 * 1024 * 1024)

#define TFCARD_MAX_FILE_NUM	4096
#define NAND_MAX_FILE_NUM		4096
#define SD_CHECK_DEBUG
/**********************************************************************************
	enum
**********************************************************************************/
typedef enum {
	BASE_MW_SD_SUCCESS,
	BASE_MW_SD_FAILURE,
	BASE_MW_SD_MOUNTFAIL,
	BASE_MW_SD_OPENFILEFAIL,
	BASE_MW_SD_GETSTATERR,
	BASE_MW_SD_HAVENOFILE,
	BASE_MW_SD_REMOVEFILE,
	BASE_MW_SD_DEVNOTFOUND,
	BASE_MW_SD_FILESORTERR,
	BASE_MW_SD_INVALID,
	BASE_MW_SD_HAVENOPATH,
	BASE_MW_SD_SPACENOTENOUGH,
} eBASE_MW_SD_ERROR;

typedef enum {
	BASE_MW_SD_NOTMOUNT,
	BASE_MW_SD_MOUNT,
} eBASE_MW_SD_MOUNTSTAT;

typedef enum {
	BASE_MW_VIDEO_NOREC,
	BASE_MW_VIDEO_REC,
} eBASE_MW_SD_VIDEOSTAT;

typedef enum {
	BASE_MW_AUDIO_NOREC,
	BASE_MW_AUDIO_REC,
} eBASE_MW_SD_AUDIOSTAT;

typedef enum {
	BASE_MW_SNAP_NOREC,
	BASE_MW_SNAP_REC,
} eBASE_MW_SD_SNAPSTAT;

/**********************************************************************************
	struct
**********************************************************************************/
typedef struct {
	eBASE_MW_SD_MOUNTSTAT eSDMount;
	eBASE_MW_SD_VIDEOSTAT eVideoStat;
	eBASE_MW_SD_AUDIOSTAT eAudioStat;
	eBASE_MW_SD_SNAPSTAT   eSnapStat;
	CHAR cDevName[BASE_MW_SD_DEVNAME_LENGTH];
	CHAR cMountPoint[BASE_MW_SD_MOUNT_LENGTH];
	LONG lTotalKB;
	LONG lUsedKB;
	LONG lFreeKB;
	LONG lSdState;
	pthread_mutex_t sMutStatus;
}sBASE_MW_SD_STAT;

typedef struct {
	CHAR cOriginalDir[BASE_MW_SD_MOUNT_LENGTH*2];
	CHAR cNewDir[BASE_MW_SD_MOUNT_LENGTH*2];
	pthread_mutex_t sMutStatus;
}sBASE_MW_DIR_STAT;

/**********************************************************************************
	variable
**********************************************************************************/
sBASE_MW_SD_STAT gsSDStat;
#ifdef SUBCARD_CHECK_DEBUG
sBASE_MW_SD_STAT gsNANDStat;
#endif /* SUBCARD_CHECK_DEBUG */
sBASE_MW_DIR_STAT gsDIRStat;
/**********************************************************************************
	prototype
**********************************************************************************/
eBASE_MW_SD_ERROR eBASE_MW_SD_CHECK(sBASE_MW_SD_STAT *psSDStat);
eBASE_MW_SD_ERROR eBASE_SD_MW_REMOVEFILE(CHAR* pcPath,CHAR* pcPostfix,LONG lFileNum);
eBASE_MW_SD_ERROR eBASE_SD_MW_CHECKSTATUS(sBASE_MW_SD_STAT *psSDStat);
eBASE_MW_SD_ERROR eBASE_SD_MW_GETSPACE(sBASE_MW_SD_STAT *psSDStat, CHAR* pcPath, CHAR* pcPostfix, LONG lFileNum);
eBASE_MW_SD_ERROR eBASE_SD_MW_CheckSpace(CHAR* pcPath, LONG lSpaceThreshold);

#ifdef __cplusplus
}
#endif

#endif
