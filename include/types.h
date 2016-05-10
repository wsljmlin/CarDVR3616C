#ifndef	__TYPES_H__
#define	__TYPES_H__

#if 0
typedef unsigned char	UCHAR;
typedef unsigned short	USHORT;
typedef unsigned long	ULONG;
typedef signed char		CHAR;
typedef signed short	SHORT;
typedef signed long		LONG;
#endif

typedef unsigned char	UINT8;
typedef unsigned short	UINT16;
typedef unsigned long	UINT32;

typedef signed char		INT8;
typedef signed short	INT16;
typedef signed long		INT32;

typedef signed char		SINT8;
typedef signed short	SINT16;
typedef signed long		SINT32;

typedef unsigned char 	UB;
typedef unsigned short 	UH;
typedef unsigned long 	UW;
typedef signed char  	B;
typedef signed short 	H;
typedef signed long   	W;

typedef	UB				VB;
typedef	UH				VH;
typedef	UW				VW;

typedef void			*VP;

typedef UB				BOOL;
typedef H				ID;

#ifndef	NULL
#define	NULL			0
#endif

#ifndef	TRUE
#define	TRUE			1
#endif

#ifndef	FALSE
#define	FALSE			0
#endif


#if !defined(NULL)
	#define		NULL	(void *)0
#endif/* NULL */


#endif	/* __TYPES_H__ */
