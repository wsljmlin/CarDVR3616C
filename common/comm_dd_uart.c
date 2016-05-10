#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "comm_dd_uart.h"
#include <assert.h>
#include <fcntl.h>

/***************************************************
	macro / enum
***************************************************/
//#define COMM_DD_UART_DEBUG
#ifdef COMM_DD_UART_DEBUG
#define CDU_Debug(fmt, arg...) fprintf(stdout, "[ CDU ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define CDU_Debug(fmt, arg...)
#endif

#define CDU_FuncIn() CDU_Debug("in\n")
#define CDU_FuncOut() CDU_Debug("out\n")
#define CDU_iVal(iVal) CDU_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define CDU_lVal(lVal) CDU_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define CDU_PtVal(ptVal) CDU_Debug("pt %s @ %p\n", #ptVal, ptVal)

#define COMM_DD_UART_MAXREADCNT 1024

/***************************************************
	variable
***************************************************/
static LONG lUART_TIMEOUT_SLEEP_UNIT = 2;

/***************************************************
	prototype
***************************************************/

/***************************************************
	function
***************************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_SetSpeed(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt);
eCOMM_DD_UART_Ret eCOMM_DD_UART_SetFlowCtl(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt);
eCOMM_DD_UART_Ret eCOMM_DD_UART_SetDataBits(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt);
eCOMM_DD_UART_Ret eCOMM_DD_UART_SetParity(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt);
eCOMM_DD_UART_Ret eCOMM_DD_UART_SetStopBits(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt);
eCOMM_DD_UART_Ret eCOMM_DD_UART_Flush(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt);

/*********************************************
* func : eCOMM_DD_UART_Init(sCOMM_DD_UART_Info* psInfo, eCOMM_DD_UART_Speed eSpeed, eCOMM_DD_UART_FlowCtl eFlowCtl, eCOMM_DD_UART_Databits eDatabits, eCOMM_DD_UART_Stopbits eStopbits, CHAR cParity, CHAR* pcDev)
* arg : sCOMM_DD_UART_Info* psInfo, eCOMM_DD_UART_Speed eSpeed, eCOMM_DD_UART_FlowCtl eFlowCtl, eCOMM_DD_UART_Databits eDatabits, eCOMM_DD_UART_Stopbits eStopbits, CHAR cParity, CHAR* pcDev
* ret : eCOMM_DD_UART_Ret
* note : init the parameter info
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_Init(sCOMM_DD_UART_Info* psInfo,
					eCOMM_DD_UART_Speed eSpeed, eCOMM_DD_UART_FlowCtl eFlowCtl, 
					eCOMM_DD_UART_Databits eDatabits, eCOMM_DD_UART_Stopbits eStopbits,
					CHAR cParity, CHAR* pcDev) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_SUCCESS;

	CDU_FuncIn();

	{
		psInfo->cParity = cParity;
		psInfo->eDatabits = eDatabits;
		psInfo->eFlowCtl = eFlowCtl;
		psInfo->eSpeed = eSpeed;
		psInfo->eStopbits = eStopbits;
		psInfo->pcDev = strdup(pcDev);
		psInfo->lFd = -1;
	}

	CDU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_UART_Open(sCOMM_DD_UART_Info* psInfo)
* arg : sCOMM_DD_UART_Info* psInfo
* ret : eCOMM_DD_UART_Ret
* note : open the dev and set attribute through psInfo
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_Open(sCOMM_DD_UART_Info* psInfo) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_SUCCESS;
	LONG lRet = 0;
	struct termios sTermOpt;

	CDU_FuncIn();

	{
		/* open the dev */
		psInfo->lFd = open(psInfo->pcDev, O_RDWR | O_NOCTTY | O_NDELAY);
		if(psInfo->lFd == -1) {
			CDU_Debug("can not open dev : %s\n", psInfo->pcDev);
			return COMM_DD_UART_OPENDEV;
		}

		lRet = fcntl(psInfo->lFd, F_SETFL, 0);
		if(lRet != 0) {
			CDU_Debug("can not set fcntl with error <%ld>\n", lRet);
			return COMM_DD_UART_FCNTL;
		}

		/* check valid */
		assert(isatty(STDIN_FILENO));

		/* get the attribute */
		lRet = tcgetattr(psInfo->lFd, &sTermOpt);
		if(lRet != 0) {
			CDU_Debug("get attr error\n");
			return COMM_DD_UART_GETATTR;
		}

		eCOMM_DD_UART_SetSpeed(psInfo, &sTermOpt);
		eCOMM_DD_UART_SetFlowCtl(psInfo, &sTermOpt);
		eCOMM_DD_UART_SetDataBits(psInfo, &sTermOpt);
		eCOMM_DD_UART_SetParity(psInfo, &sTermOpt);
		eCOMM_DD_UART_SetStopBits(psInfo, &sTermOpt);

		
		eCOMM_DD_UART_Flush(psInfo, &sTermOpt);

		/* set the attribute */
		lRet = tcsetattr(psInfo->lFd, TCSANOW, &sTermOpt);
		if(lRet != 0) {
			CDU_Debug("set attr error\n");
			return COMM_DD_UART_GETATTR;
		}
	}

	CDU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_UART_Read(sCOMM_DD_UART_Info* psInfo, UCHAR* pucData, LONG lLen)
* arg : sCOMM_DD_UART_Info* psInfo, UCHAR* pucData, LONG lLen
* ret : eCOMM_DD_UART_Ret
* note :
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_Read(sCOMM_DD_UART_Info* psInfo, UCHAR* pucData, LONG* plLen) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_SUCCESS;
	LONG lReadLen = 0;

	CDU_FuncIn();

	{

		/* now, we have data here */
		lReadLen = read(psInfo->lFd, pucData, COMM_DD_UART_MAXREADCNT);

		/* for debug */
		#if 0
		LONG lReadCnt = 0;
		while (lReadLen != lReadCnt) {
			CDU_Debug("[ %02ld ] : %-4d (0x%02x)\n", lReadCnt, pucData[lReadCnt], pucData[lReadCnt]);
			lReadCnt ++;
		}
		#endif

		*plLen = lReadLen;
	}

	CDU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_UART_Read_Timeout(sCOMM_DD_UART_Info* psInfo, UCHAR* pucData, LONG* plLen, LONG lTimeOutSec)
* arg : sCOMM_DD_UART_Info* psInfo, UCHAR* pucData, LONG* plLen, LONG lTimeOutSec
* ret : eCOMM_DD_UART_Ret
* note : read from the uart, but with time out constraint
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_Read_Timeout(sCOMM_DD_UART_Info* psInfo, UCHAR* pucData, LONG* plLen, LONG lTimeOutSec) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_TIMEOUT;
	LONG lReadLen = 0;
	struct timeval sBeginTime;
	struct timeval sEndTime;
	LONG lUseTime;
	LONG lUartFdFlag;

	CDU_FuncIn();

	{
		/* get begin time */
		lUseTime = 0;
		
		/* set the fd to be noblock */
		lUartFdFlag = fcntl(psInfo->lFd, F_GETFL);

		/* the flag must not be set to nonblock */
		assert(!(lUartFdFlag & O_NONBLOCK));
		
		fcntl(psInfo->lFd, F_SETFL, lUartFdFlag | O_NONBLOCK);

		/* read the uart data until timeout */
		do {
			
			/* try to read */
			gettimeofday(&sBeginTime, NULL);
			lReadLen = read(psInfo->lFd, pucData, COMM_DD_UART_MAXREADCNT);
			if (lReadLen > 0) {
				eRet = COMM_DD_UART_SUCCESS;
				break;
			}
			
			/* not read any thing, update lUseTime */
			sleep(lUART_TIMEOUT_SLEEP_UNIT);
			gettimeofday(&sEndTime, NULL);
			lUseTime += sEndTime.tv_sec - sBeginTime.tv_sec;
			CDU_Debug("not read anything for %ld sec\n", lUseTime);
			
		} while (lUseTime < lTimeOutSec);

		/* reset the fd to be block */
		fcntl(psInfo->lFd, F_SETFL, lUartFdFlag);
		
		/* update the return length */
		*plLen = lReadLen;
	}

	CDU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_UART_Write(sCOMM_DD_UART_Info* psInfo, const UCHAR* pucData, LONG lLen)
* arg : sCOMM_DD_UART_Info* psInfo, const UCHAR* pucData, LONG lLen
* ret : eCOMM_DD_UART_Ret
* note :
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_Write(sCOMM_DD_UART_Info* psInfo, const UCHAR* pucData, LONG lLen) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_SUCCESS;
	LONG lWtLen = 0;

	CDU_FuncIn();

	{
		lWtLen = write(psInfo->lFd, pucData, lLen);
		if(lWtLen != lLen) {
			CDU_Debug("write error : <%ld, %ld>\n", lLen, lWtLen);
			tcflush(psInfo->lFd, TCOFLUSH);
			return COMM_DD_UART_WRITE;
		}

	}

	CDU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_UART_Close(sCOMM_DD_UART_Info* psInfo)
* arg : sCOMM_DD_UART_Info* psInfo
* ret : eCOMM_DD_UART_Ret
* note :
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_Close(sCOMM_DD_UART_Info* psInfo) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_SUCCESS;

	CDU_FuncIn();

	{
		if(psInfo->lFd != -1) {
			close(psInfo->lFd);
			psInfo->lFd = -1;
		}
	}

	CDU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_UART_Deinit(sCOMM_DD_UART_Info* psInfo)
* arg : sCOMM_DD_UART_Info* psInfo
* ret : eCOMM_DD_UART_Ret
* note : deinit the parameter info
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_Deinit(sCOMM_DD_UART_Info* psInfo) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_SUCCESS;

	CDU_FuncIn();

	{
		free(psInfo->pcDev);
		psInfo->pcDev = NULL;
	}

	CDU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_UART_SetSpeed(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt)
* arg : sCOMM_DD_UART_Info* psInfo, struct termios* psOpt
* ret : eCOMM_DD_UART_Ret
* note : set the speed
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_SetSpeed(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_SUCCESS;

	CDU_FuncIn();

	{
		/* set input speed */
		cfsetispeed(psOpt, psInfo->eSpeed);

		/* set output speed */
		cfsetospeed(psOpt, psInfo->eSpeed);
	}

	CDU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_UART_SetSpeed(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt)
* arg : sCOMM_DD_UART_Info* psInfo, struct termios* psOpt
* ret : eCOMM_DD_UART_Ret
* note : set the speed
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_SetFlowCtl(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_SUCCESS;

	CDU_FuncIn();

	{
		psOpt->c_cflag |= CLOCAL;
		psOpt->c_cflag |= CREAD;

		switch(psInfo->eFlowCtl) {
			case COMM_DD_UART_DTR_DSR:
				psOpt->c_cflag &= ~CRTSCTS;
				break;
			case COMM_DD_UART_RTS_CTS:
				psOpt->c_cflag |= CRTSCTS;
				break;
			case COMM_DD_UART_XON_XOFF:
				psOpt->c_cflag |= IXON | IXOFF | IXANY;
				break;
			default:
				assert(0);
				break;
		};

	}

	CDU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_UART_SetDataBits(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt)
* arg : sCOMM_DD_UART_Info* psInfo, struct termios* psOpt
* ret : eCOMM_DD_UART_Ret
* note : set data bits
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_SetDataBits(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_SUCCESS;

	CDU_FuncIn();

	{
		psOpt->c_cflag &= ~ CSIZE;
		switch(psInfo->eDatabits) {
			case COMM_DD_UART_Databit5:
				psOpt->c_cflag |= CS5;
				break;
			case COMM_DD_UART_Databit6:
				psOpt->c_cflag |= CS6;
				break;
			case COMM_DD_UART_Databit7:
				psOpt->c_cflag |= CS7;
				break;
			case COMM_DD_UART_Databit8:
				psOpt->c_cflag |= CS8;
				break;
			default:
				assert(0);
				break;
		};
	}

	CDU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_UART_SetParity(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt)
* arg : sCOMM_DD_UART_Info* psInfo, struct termios* psOpt
* ret : eCOMM_DD_UART_Ret
* note : set parity
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_SetParity(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_SUCCESS;

	CDU_FuncIn();

	{
		switch(psInfo->cParity) {
			case 'n':
			case 'N':
				psOpt->c_cflag &= ~PARENB;
				psOpt->c_iflag &= ~INPCK;
				break;
			case 'o':
			case 'O':
				psOpt->c_cflag |= (PARODD | PARENB);
				psOpt->c_iflag |= INPCK;
				break;
			case 'e':
			case 'E':
				psOpt->c_cflag |= PARENB;
				psOpt->c_cflag &= ~PARODD;
				psOpt->c_iflag |= INPCK;
				break;
			case 's':
			case 'S':
				psOpt->c_cflag &= ~PARENB;
				psOpt->c_cflag &= ~CSTOPB;
				break;
			default:
				assert(0);
				break;
		};

	}

	CDU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_UART_SetStopBits(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt)
* arg : sCOMM_DD_UART_Info* psInfo, struct termios* psOpt
* ret : eCOMM_DD_UART_Ret
* note : set stop bits
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_SetStopBits(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_SUCCESS;

	CDU_FuncIn();

	{
		switch(psInfo->eStopbits) {
			case COMM_DD_UART_Stopbit1:
				psOpt->c_cflag &= ~CSTOPB;
				break;
			case COMM_DD_UART_Stopbit2:
				psOpt->c_cflag |= CSTOPB;
				break;
			default:
				assert(0);
				break;
		};

	}

	CDU_FuncOut();

	return eRet;
}

/*********************************************
* func : eCOMM_DD_UART_Flush(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt)
* arg : sCOMM_DD_UART_Info* psInfo, struct termios* psOpt
* ret : eCOMM_DD_UART_Ret
* note : flush
*********************************************/
eCOMM_DD_UART_Ret eCOMM_DD_UART_Flush(sCOMM_DD_UART_Info* psInfo, struct termios* psOpt) {
	eCOMM_DD_UART_Ret eRet = COMM_DD_UART_SUCCESS;

	CDU_FuncIn();

	{
		/* set raw mode */
		psOpt->c_lflag &= ~(ICANON|ECHO|ECHOE|ISIG);
		psOpt->c_oflag &= ~OPOST;
		
		psOpt->c_cc[VTIME] = 1;
		psOpt->c_cc[VMIN] = 255;

		tcflush(psInfo->lFd, TCIFLUSH);

	}

	CDU_FuncOut();

	return eRet;
}


#ifdef __cplusplus
}
#endif
