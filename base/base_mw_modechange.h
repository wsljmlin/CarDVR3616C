#ifndef __BASE_MW_MODECHANGE_H__
#define __BASE_MW_MODECHANGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/****************************************************************
	include
****************************************************************/
#include "base_comm.h"

/****************************************************************
	macro / enum
****************************************************************/
/*get venc chn obey mode(high mode)*/
#define BASE_HIGH_MODE_GETVENCCH(Mode) ( ((Mode>>(8* (BASE_MODEFLAG_VENC)))&0xff)>>1)
#define BASE_MODE_GETMODE() (geModeFlag & 0xff)

typedef enum {
	BASE_MODE_VENC1080_VONULL_L = 0x01000100,
	BASE_MODE_VENC720_VONULL_L = 0x02000100,
} eBASE_MODEFLAG;

typedef enum {
	BASE_MODEFLAG_MODE,
	BASE_MODEFLAG_VI,
	BASE_MODEFLAG_VO,
	BASE_MODEFLAG_VENC,
} eBASE_MODEFLAG_TYPE;

typedef enum {
	BASE_MODEFLAG_1080P,
	BASE_MODEFLAG_720P,
	BASE_MODEFLAG_480P,
} eBASE_MODEFLAG_RESOLUTION;

typedef enum {
	BASE_LOWMODE,
	BASE_HIGHMODE,
} eBASE_MODESIGN;

/****************************************************************
	struct
****************************************************************/


/****************************************************************
	variable
****************************************************************/
eBASE_MODEFLAG geModeFlag;

/****************************************************************
	prototype
****************************************************************/
LONG lBASE_MODE_INIT(eBASE_MODEFLAG eMode);
LONG lBASE_MODE_START();
LONG lBASE_MODE_CHANGE(eBASE_MODEFLAG* peMode, eBASE_CH_CHANGE eChange);
 eBASE_RET_STATE eBASE_VI_GETVICH(eBASE_MODEFLAG* peMode, LONG lVencCh, LONG* plViCh);

#ifdef __cplusplus
}
#endif

#endif
