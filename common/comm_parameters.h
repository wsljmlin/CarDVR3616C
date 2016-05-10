#ifndef __PARA_COM_H__
#define __PARA_COM_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "comm_common.h"

/* define */
#define COMM_MAXPARACHAR 10
#define COMM_MAXPARAFEEDBACKCHAR (10 * 10)
#define COMM_MAXPARACNT ( COMM_CMDMAXSUBCNT * COMM_CMDMAXCNT )


#define COMM_PARAGETID(cmd,opt) ((cmd << COMM_CMDMAXSUBSHIFT )+opt)

/* invalid parameter value */
#define COMM_PARAINVALIDVAL -1			// invalid value

/* struct */
typedef struct  {
	LONG lParaId;
	LONG lParaVal;
}  sCOMM_PARA;

/* mutex */
pthread_mutex_t muxPara;

/*------------------------------------------------------------------------------*/
/*!
	@brief			init the parameter list

	@param		sCOMM_PARA** ppParas			[in]			the pointer of pointer point to the parameter list
	@param		sCOMM_PARA** ppParas			[out]		the pointer of pointer point to the parameter list
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_InitParas( sCOMM_PARA** ppParas);

/*------------------------------------------------------------------------------*/
/*!
	@brief			init the parameter list according to the config file

	@param		sCOMM_PARA** ppParas			[in]			the pointer of pointer point to the parameter list
	@param		sCOMM_PARA** ppParas			[out]		the pointer of pointer point to the parameter list

	@retval		COMM_RET_SUCCESS						success
					COMM_RET_FAIL							fail
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_PARA_InitCfgParas( sCOMM_PARA** ppParas);

/*------------------------------------------------------------------------------*/
/*!
	@brief			init the parameter list according to the default

	@param		sCOMM_PARA** ppParas			[in]			the pointer of pointer point to the parameter list
	@param		sCOMM_PARA** ppParas			[out]		the pointer of pointer point to the parameter list
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_InitDefaultParas( sCOMM_PARA** ppParas);

/*------------------------------------------------------------------------------*/
/*!
	@brief			update the parameter list

	@param		sCOMM_PARA** ppParas			[in]			the pointer of pointer point to the parameter list
					sCOMM_PARA* sPara				[in]			the pointer to the parameter to be update
	@param		sCOMM_PARA** ppParas			[out]		the pointer of pointer point to the parameter list

	@retval		-1												update fail
					other											the old value of the parameter
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_PARA_UpdateParas( sCOMM_PARA** ppParas,  sCOMM_PARA* sPara);

/*------------------------------------------------------------------------------*/
/*!
	@brief			construct a parameter

	@param		LONG lId							[in]			the id of the new parameter
					LONG lVal							[in]			the value of the new parameter
	@param		sCOMM_PARA* pParas			[out]		the pointer point to a parameter
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_SetPara( sCOMM_PARA* pPara, LONG lId, LONG lVal);

/*------------------------------------------------------------------------------*/
/*!
	@brief			get the paraId of the parameter

	@param		sCOMM_PARA* pParas			[in]		the pointer point to a parameter

	@retval		lParaId										the paraId of the parameter
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_PARA_GetParaId( sCOMM_PARA* pPara);

/*------------------------------------------------------------------------------*/
/*!
	@brief			get the paraVal of the parameter

	@param		sCOMM_PARA* pParas			[in]		the pointer point to a parameter

	@retval		lParaVal									the paraVal of the parameter
*/
/* -----------------------------------------------------------------------------*/
LONG lCOMM_PARA_GetParaVal( sCOMM_PARA* pPara);

/*------------------------------------------------------------------------------*/
/*!
	@brief			write the parameter list back to the config file para.cfg

	@param		sCOMM_PARA* pParas			[in]		the pointer point to the parameter list
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_WriteParasBack( sCOMM_PARA* pPara);

/*------------------------------------------------------------------------------*/
/*!
	@brief			get the comment according to the paraId of a parameter

	@param		LONG lParaId				[in]			the paraId of a parameter
	@param		CHAR* pcCmt				[out]		the output comment
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_ParaIdToComment( LONG lParaId, CHAR* pcCmt);

/*------------------------------------------------------------------------------*/
/*!
	@brief			get the system sub comment according to the paraId of a parameter

	@param		LONG lParaId				[in]			the paraId of a parameter
	@param		CHAR* pcSubCmt			[out]		the output sub comment
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_ParaIdToSubCommentSystem( LONG lParaId, CHAR* pcSubCmt);

/*------------------------------------------------------------------------------*/
/*!
	@brief			get the eth sub comment according to the paraId of a parameter

	@param		LONG lParaId				[in]			the paraId of a parameter
	@param		CHAR* pcSubCmt			[out]		the output sub comment
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_ParaIdToSubCommentVenc( LONG lParaId, CHAR* pcSubCmt);

/*------------------------------------------------------------------------------*/
/*!
	@brief			get the rec sub comment according to the paraId of a parameter

	@param		LONG lParaId				[in]			the paraId of a parameter
	@param		CHAR* pcSubCmt			[out]		the output sub comment
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_ParaIdToSubCommentRec( LONG lParaId, CHAR* pcSubCmt);

/*------------------------------------------------------------------------------*/
/*!
	@brief			get the eth hdmi comment according to the paraId of a parameter

	@param		LONG lParaId				[in]			the paraId of a parameter
	@param		CHAR* pcSubCmt			[out]		the output sub comment
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_ParaIdToSubCommentHdmi( LONG lParaId, CHAR* pcSubCmt);

/*------------------------------------------------------------------------------*/
/*!
	@brief			convert the parameter into parameter char

	@param		sCOMM_PARA* pPara			[in]			the pointer point to the parameter
	@param		CHAR* cpPara						[out]		the output char of the parameter
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_ParaToChar( sCOMM_PARA* pPara, char* cpPara);

/*------------------------------------------------------------------------------*/
/*!
	@brief			convert the parameter list into parameter char

	@param		sCOMM_PARA** ppParas			[in]			the pointer point to the parameter list
	@param		CHAR* cpPara						[out]		the output char of the parameter
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_ParasToChar( sCOMM_PARA** ppParas, char* cpPara);

/*------------------------------------------------------------------------------*/
/*!
	@brief			convert the parameter char into parameter

	@param		CHAR* cpPara						[in]			the input char of the parameter
	@param		sCOMM_PARA* pPara			[out]		the pointer point to the parameter
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_CharToPara( sCOMM_PARA* pPara, char* cpPara);
#ifdef SHOW_INFO
/*------------------------------------------------------------------------------*/
/*!
	@brief			show the information of the parameter

	@param		sCOMM_PARA* pPara			[in]		the pointer point to the parameter
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_ShowPara( sCOMM_PARA* pPara);
#endif /* SHOW_INFO */

#ifdef SHOW_INFO
/*------------------------------------------------------------------------------*/
/*!
	@brief			show the information of the parameter list

	@param		sCOMM_PARA** ppPara			[in]		the pointer point to the parameter list
*/
/* -----------------------------------------------------------------------------*/
void vCOMM_PARA_ShowParas( sCOMM_PARA** ppPara);
#endif /* SHOW_INFO */


#ifdef __cplusplus
}
#endif


#endif /* __PARA_COM_H__ */