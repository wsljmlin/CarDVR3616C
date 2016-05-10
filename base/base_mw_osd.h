#ifndef __BASE_MW_OSD_H__ 
#define __BASE_MW_OSD_H__ 
 
#ifdef __cplusplus 
#if __cplusplus 
extern "C"{ 
#endif 
#endif /* __cplusplus */ 

/**********************************************************************************
	include
**********************************************************************************/
#include "base_comm.h"

/**********************************************************************************
	macro
**********************************************************************************/
#define BASE_MW_OSD_SETBIT(c, bit) (*c=*c|(0x01<<bit))
#define BASE_MW_OSD_CLRBIT(c, bit) (*c=*c&~(0x01<<bit))

#define BASE_MW_OSD_STRINGSIZE 20
#define BASE_MW_OSD_BITMAP_MAXSIZE (16*24*2*BASE_MW_OSD_STRINGSIZE)

#define BASE_MW_OSD_CHARHEIGHT 12
#define BASE_MW_OSD_CHARWIDTH 8


/**********************************************************************************
	enum
**********************************************************************************/
typedef enum {
	BASE_MW_OSD_SUCCESS,
	BASE_MW_OSD_FAILURE,
	BASE_MW_OSD_MALLOCFAILURE
}eBASE_MW_OSD_ERROR;

typedef enum {	
	BASE_MW_OSD_RED,	
	BASE_MW_OSD_GREEN,	
	BASE_MW_OSD_BLUE,	
	BASE_MW_OSD_BLACK,	
	BASE_MW_OSD_WHITE,	
	BASE_MW_OSD_YELLOW,	
	BASE_MW_OSD_PURPLE,	
	BASE_MW_OSD_INDIANRED,	
	BASE_MW_OSD_OLIVEDRAB,	
	BASE_MW_OSD_SKYBLUE,	
	BASE_MW_OSD_WHEATE,	
	BASE_MW_OSD_THISTLE,
	BASE_MW_OSD_GRAY,
	BASE_MW_OSD_COLORNUM
}eBASE_MW_OSD_COLOR;

typedef enum {
	BASE_MW_OSD_CHAR_16_24,
	BASE_MW_OSD_MODENUM
}eBASE_MW_OSD_CHARMODE;

/**********************************************************************************
	struct
**********************************************************************************/
typedef struct {
 UCHAR ucOsdString[BASE_MW_OSD_STRINGSIZE];
 LONG lOsdStringLen;
 UCHAR ucOsdBitmapBuf[BASE_MW_OSD_BITMAP_MAXSIZE];
 UCHAR *pucOsdBitmapBuf;
 LONG lOsdBitmapBufLen;
 LONG lOsdBitmapWidth;
 LONG lOsdBitmapHeight;
 LONG lOsdCharWidth;
 LONG lOsdCharHeight;
 eBASE_MW_OSD_CHARMODE eOsdMode;
 eBASE_MW_OSD_COLOR eBackColor;
 eBASE_MW_OSD_COLOR eForeColor;
 LONG lBackColorVal;
 LONG lForeColorVal;
}sBASE_MW_OSD_PARA;

/**********************************************************************************
	variable
**********************************************************************************/


/**********************************************************************************
	prototype
**********************************************************************************/

eBASE_MW_OSD_ERROR eBASE_MW_OSD_StrToBitmap(CHAR* pcOsdStr,LONG lLen,BITMAP_S* bitmap);
eBASE_MW_OSD_ERROR eBASE_MW_OSD_LoadBmp(const char *filename, BITMAP_S *pstBitmap);
#ifdef __cplusplus 
#if __cplusplus 
} 
#endif 
#endif /* __cplusplus */ 
 
#endif
