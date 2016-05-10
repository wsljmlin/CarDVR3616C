#ifndef __COMM_DD_UART_H__
#define __COMM_DD_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************************
	include
*********************************************************/
#include "comm_common.h"
#include <termios.h>

/*********************************************************
	macro / enum
*********************************************************/
typedef enum {
	COMM_DD_UART_SUCCESS,
	COMM_DD_UART_FAIL,
	COMM_DD_UART_OPENDEV,
	COMM_DD_UART_FCNTL,
	COMM_DD_UART_WRITE,
	COMM_DD_UART_GETATTR,
	COMM_DD_UART_SETATTR,
	COMM_DD_UART_TIMEOUT,
} eCOMM_DD_UART_Ret;

typedef enum {
	COMM_DD_UART_Speed38400 = B38400,
	COMM_DD_UART_Speed19200 = B19200,
	COMM_DD_UART_Speed115200 = B115200,
	COMM_DD_UART_Speed9600 = B9600,
	COMM_DD_UART_Speed4800 = B4800,
	COMM_DD_UART_Speed2400 = B2400,
	COMM_DD_UART_Speed1200 = B1200,
	COMM_DD_UART_Speed300 = B300,
} eCOMM_DD_UART_Speed;

typedef enum {
	COMM_DD_UART_DTR_DSR = 0,
	COMM_DD_UART_RTS_CTS = 1,
	COMM_DD_UART_XON_XOFF = 2,
} eCOMM_DD_UART_FlowCtl;

typedef enum {
	COMM_DD_UART_Databit5 = 5,
	COMM_DD_UART_Databit6 = 6,
	COMM_DD_UART_Databit7 = 7,
	COMM_DD_UART_Databit8 = 8,
} eCOMM_DD_UART_Databits;

typedef enum {
	COMM_DD_UART_Stopbit1 = 1,
	COMM_DD_UART_Stopbit2 = 2,
} eCOMM_DD_UART_Stopbits;

/*********************************************************
	struct
*********************************************************/
typedef struct {
	LONG lFd;
	eCOMM_DD_UART_Speed eSpeed;
	eCOMM_DD_UART_FlowCtl eFlowCtl;
	eCOMM_DD_UART_Databits eDatabits;
	eCOMM_DD_UART_Stopbits eStopbits;
	CHAR cParity;
	CHAR* pcDev;
} sCOMM_DD_UART_Info;

/*********************************************************
	variable
*********************************************************/

/*********************************************************
	prototype
*********************************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_Init(sCOMM_DD_UART_Info* psInfo, eCOMM_DD_UART_Speed eSpeed, eCOMM_DD_UART_FlowCtl eFlowCtl, eCOMM_DD_UART_Databits eDatabits, eCOMM_DD_UART_Stopbits eStopbits, CHAR cParity, CHAR* pcDev);
eCOMM_DD_UART_Ret eCOMM_DD_UART_Open(sCOMM_DD_UART_Info* psInfo);
eCOMM_DD_UART_Ret eCOMM_DD_UART_Read(sCOMM_DD_UART_Info* psInfo, UCHAR* pucData, LONG* plLen);
eCOMM_DD_UART_Ret eCOMM_DD_UART_Read_Timeout(sCOMM_DD_UART_Info* psInfo, UCHAR* pucData, LONG* plLen, LONG lTimeOutSec);
eCOMM_DD_UART_Ret eCOMM_DD_UART_Write(sCOMM_DD_UART_Info* psInfo, const UCHAR* pucData, LONG lLen);
eCOMM_DD_UART_Ret eCOMM_DD_UART_Close(sCOMM_DD_UART_Info* psInfo);
eCOMM_DD_UART_Ret eCOMM_DD_UART_Deinit(sCOMM_DD_UART_Info* psInfo);

#ifdef __cplusplus
}
#endif

#endif
