#ifdef __cplusplus
extern "C" {
#endif

/***************************************************
	include
***************************************************/
#include "base_mw_wifi.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>

/***************************************************
	macro / enum
***************************************************/
//#define BASE_MW_WIFI_DEBUG
#ifdef BASE_MW_WIFI_DEBUG
#define BMW_Debug(fmt, arg...) fprintf(stdout, "[ BMW ] : %s() <%d> "fmt, __func__, __LINE__, ##arg);
#else
#define BMW_Debug(fmt, arg...)
#endif

#define WIFICTL_DEV "/dev/WIFI_CTL"
#define WIFI_CONFIG_SCRIPT "/firmware/hostapd.conf"

#define BMW_FuncIn() BMW_Debug("in\n")
#define BMW_FuncOut() BMW_Debug("out\n")
#define BMW_iVal(iVal) BMW_Debug("%s <%d> @ %p\n", #iVal, iVal, &iVal)
#define BMW_lVal(lVal) BMW_Debug("%s <%ld> @ %p\n", #lVal, lVal, &lVal)
#define BMW_PtVal(ptVal) BMW_Debug("pt %s @ %p\n", #ptVal, ptVal)

typedef enum {
	BASE_MW_WIFI_SCRIPTTYPE_CONNECT,
	BASE_MW_WIFI_SCRIPTTYPE_DISCONNECT,
} eBASE_MW_WIFI_ScriptType;

/***************************************************
	variable
***************************************************/

/***************************************************
	prototype
***************************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_CreatConfig(FILE *fp,CHAR* pcUserName, CHAR* pcPassWord);
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_PowerOn(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType);
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_PowerOff(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType);
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_GeneralOpenScript(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType);
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_ExecuteOpenScript(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType);
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_GeneralCloseScript(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType);
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_ExecuteCloseScript(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType);

/***************************************************
	function
***************************************************/

/*********************************************
* func : eBASE_MW_WIFI_Init(sBASE_MW_WIFI_Info* psInfo)
* arg  : sBASE_MW_WIFI_Info* psInfo
* ret  : eBASE_MW_WIFI_Ret
* note : init the struct
*********************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_Init(sBASE_MW_WIFI_Info* psInfo) {
	eBASE_MW_WIFI_Ret eRet = BASE_MW_WIFI_SUCCESS;

	BMW_FuncIn();

	{
		char buf[2];
		/* clear the name */
		memset(psInfo->cUserName, 0, sizeof(psInfo->cUserName));
		memset(psInfo->cPassWord, 0, sizeof(psInfo->cPassWord));
		
		/* close wifi module */
		system("source /firmware/SSwificlose");
		
		/* close wifi power*/
		psInfo->fd = open(WIFICTL_DEV,O_RDWR); 
		if(psInfo->fd < 0){ 
			BMW_Debug("wifi module open failed\n"); 
			return BASE_MW_WIFI_FAIL; 
		}		
		write(psInfo->fd, buf,0);
		eRet = close(psInfo->fd); 
		if(eRet < 0){ 
			BMW_Debug("wifi module close failed\n"); 
			return BASE_MW_WIFI_FAIL; 
		}
		
		/* init wifi state*/
		psInfo->fd = -1;
		psInfo->lStatus = BASE_MW_WIFI_STATUS_DISCONNECT;

	}

	BMW_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_WIFI_Deinit(sBASE_MW_WIFI_Info* psInfo)
* arg  : sBASE_MW_WIFI_Info* psInfo
* ret  : eBASE_MW_WIFI_Ret
* note : init the struct
*********************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_Deinit(sBASE_MW_WIFI_Info* psInfo) {
	eBASE_MW_WIFI_Ret eRet = BASE_MW_WIFI_SUCCESS;

	BMW_FuncIn();

	{

		/* judge current status */
		if( psInfo->lStatus & BASE_MW_WIFI_STATUS_DISCONNECT) {
			/* no need to connect again */
			BMW_Debug("wifi not connected, no need to deinit !\n");
			return BASE_MW_WIFI_SUCCESS;
		}
		
		/* toggle the status */
		eBASE_MW_WIFI_DisConnect(psInfo);
		psInfo->fd = -1;

	}

	BMW_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_WIFI_Update(sBASE_MW_WIFI_Info* psInfo, CHAR* pcUserName, CHAR* pcPassWord)
* arg  : sBASE_MW_WIFI_Info* psInfo, CHAR* pcUserName, CHAR* pcPassWord
* ret  : eBASE_MW_WIFI_Ret
* note : update the struct with name and passwd
*********************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_Update(sBASE_MW_WIFI_Info* psInfo, CHAR* pcUserName, CHAR* pcPassWord){
	eBASE_MW_WIFI_Ret eRet = BASE_MW_WIFI_SUCCESS;

	BMW_FuncIn();

	{
		BMW_Debug("pcUserName:[%s]\n",pcUserName);
		BMW_Debug("pcPassWord:[%s]\n",pcPassWord);
		if (psInfo->lStatus & BASE_MW_WIFI_STATUS_DISCONNECT) {
			/* assign the name */
			if ((pcUserName != NULL)  && (pcPassWord != NULL)) {
				strcpy(psInfo->cUserName, pcUserName);
				strcpy(psInfo->cPassWord, pcPassWord);
				BMW_Debug("pcUserName:[%s]\n",psInfo->cUserName);
				BMW_Debug("pcPassWord:[%s]\n",psInfo->cPassWord);
			} else {
				eRet = BASE_MW_WIFI_CHECK_FAIL;
				BMW_Debug("error username or password <%d>\n", eRet);
				return eRet;			
			}			
		} else if (psInfo->lStatus & BASE_MW_WIFI_STATUS_CONNECT) {
			/* assign the name */
			if ( (0 != strcmp(psInfo->cUserName,pcUserName)) || 
				(0 != strcmp(psInfo->cPassWord,pcPassWord)) ) 
			{			
				eRet = BASE_MW_WIFI_CHECK_FAIL;
				BMW_Debug("error username or password <%d>\n", eRet);
				return eRet;
			}
		}

	}

	BMW_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_WIFI_Connect(sBASE_MW_WIFI_Info* psInfo)
* arg  : sBASE_MW_WIFI_Info* psInfo
* ret  : eBASE_MW_WIFI_Ret
* note :
*********************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_Connect(sBASE_MW_WIFI_Info* psInfo) {
	eBASE_MW_WIFI_Ret eRet = BASE_MW_WIFI_SUCCESS;

	BMW_FuncIn();

	{
		/* judge current status */
		if( psInfo->lStatus & BASE_MW_WIFI_STATUS_CONNECT) {
			/* no need to connect again */
			BMW_Debug("wifi already connected, no need to connect !\n");
			return BASE_MW_WIFI_SUCCESS;
		}
		
		/* open wifi power*/
		eRet = eBASE_MW_WIFI_PowerOn(psInfo, BASE_MW_WIFI_SCRIPTTYPE_CONNECT);
		if (eRet != BASE_MW_WIFI_SUCCESS) {
			BMW_Debug("error to open wifi power <%d>\n", eRet);
			return BASE_MW_WIFI_POWERON;
		}		
		
		
		/* connect the wifi */
		eRet = eBASE_MW_WIFI_GeneralOpenScript(psInfo, BASE_MW_WIFI_SCRIPTTYPE_CONNECT);
		if (eRet != BASE_MW_WIFI_SUCCESS) {
			BMW_Debug("error to general connect script <%d>\n", eRet);
			return BASE_MW_WIFI_GENERAL_OPENSCRIPT;
		}
				
		eRet = eBASE_MW_WIFI_ExecuteOpenScript(psInfo, BASE_MW_WIFI_SCRIPTTYPE_CONNECT);
		if (eRet != BASE_MW_WIFI_SUCCESS) {
			BMW_Debug("error to execute connect script <%d>\n", eRet);
			return BASE_MW_WIFI_EXECUTE_OPENSCRIPT;
		}

		/* toggle the status */
		psInfo->lStatus &= ~ BASE_MW_WIFI_STATUS_DISCONNECT;
		psInfo->lStatus |= BASE_MW_WIFI_STATUS_CONNECT;
	}

	BMW_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_WIFI_DisConnect(sBASE_MW_WIFI_Info* psInfo)
* arg  : sBASE_MW_WIFI_Info* psInfo
* ret  : eBASE_MW_WIFI_Ret
* note :
*********************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_DisConnect(sBASE_MW_WIFI_Info* psInfo) {
	eBASE_MW_WIFI_Ret eRet = BASE_MW_WIFI_SUCCESS;

	BMW_FuncIn();

	{
		/* judge current status */
		if( psInfo->lStatus & BASE_MW_WIFI_STATUS_DISCONNECT) {
			/* no need to connect again */
			BMW_Debug("wifi not connected, no need to disconnect !\n");
			return BASE_MW_WIFI_SUCCESS;
		}

		/* connect the wifi */
		eRet = eBASE_MW_WIFI_GeneralCloseScript(psInfo, BASE_MW_WIFI_SCRIPTTYPE_DISCONNECT);
		if (eRet != BASE_MW_WIFI_SUCCESS) {
			BMW_Debug("error to general disconnect script <%d>\n", eRet);
			return BASE_MW_WIFI_GENERAL_CLOSESCRIPT;
		}
		
		eRet = eBASE_MW_WIFI_ExecuteCloseScript(psInfo, BASE_MW_WIFI_SCRIPTTYPE_DISCONNECT);
		if (eRet != BASE_MW_WIFI_SUCCESS) {
			BMW_Debug("error to execute disconnect script <%d>\n", eRet);
			return BASE_MW_WIFI_EXECUTE_CLOSESCRIPT;
		}

		/* close wifi power*/
		eRet = eBASE_MW_WIFI_PowerOff(psInfo, BASE_MW_WIFI_SCRIPTTYPE_CONNECT);
		if (eRet != BASE_MW_WIFI_SUCCESS) {
			BMW_Debug("error to close wifi power <%d>\n", eRet);
			return BASE_MW_WIFI_POWEROFF;
		}		

		/* toggle the status */
		psInfo->lStatus &= ~ BASE_MW_WIFI_STATUS_CONNECT;
		psInfo->lStatus |= BASE_MW_WIFI_STATUS_DISCONNECT;
	}

	BMW_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_WIFI_GeneralOpenScript(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType)
* arg  : sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType
* ret  : eBASE_MW_WIFI_Ret
* note :
*********************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_GeneralOpenScript(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType) {
	eBASE_MW_WIFI_Ret eRet = BASE_MW_WIFI_SUCCESS;
	
	BMW_FuncIn();

	{	
		FILE *fp;
		
		if (psInfo->lStatus & BASE_MW_WIFI_GENERAL_OPENSCRIPT) {
			BMW_Debug("script already general\n");
			return BASE_MW_WIFI_SUCCESS;
		}

		/* general the script */
		fp = fopen(WIFI_CONFIG_SCRIPT, "r+");
		if (NULL == fp) {
			BMW_Debug("open hostapd config file failed\n");
			return BASE_MW_WIFI_FAIL;
		}
		
		eBASE_MW_WIFI_CreatConfig(fp,psInfo->cUserName,psInfo->cPassWord);
		
		fclose(fp);
		psInfo->lStatus &= ~ BASE_MW_WIFI_GENERAL_CLOSESCRIPT;
		psInfo->lStatus |= BASE_MW_WIFI_GENERAL_OPENSCRIPT;
	}

	BMW_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_WIFI_GeneralCloseScript(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType)
* arg  : sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType
* ret  : eBASE_MW_WIFI_Ret
* note :
*********************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_GeneralCloseScript(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType) {
	eBASE_MW_WIFI_Ret eRet = BASE_MW_WIFI_SUCCESS;

	BMW_FuncIn();

	{		
		if (psInfo->lStatus & BASE_MW_WIFI_GENERAL_CLOSESCRIPT) {
			BMW_Debug("script already general\n");
			return BASE_MW_WIFI_SUCCESS;
		}

		/* general the script */
		
		psInfo->lStatus &= ~ BASE_MW_WIFI_GENERAL_OPENSCRIPT;
		psInfo->lStatus |= BASE_MW_WIFI_GENERAL_CLOSESCRIPT;
	}

	BMW_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_WIFI_ExecuteScript(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType)
* arg  : sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType
* ret  : eBASE_MW_WIFI_Ret
* note :
*********************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_ExecuteOpenScript(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType) {
	eBASE_MW_WIFI_Ret eRet = BASE_MW_WIFI_SUCCESS;

	BMW_FuncIn();

	{
		eRet = system("source /firmware/SSwifiopen");

	}

	BMW_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_WIFI_ExecuteCoseScript(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType)
* arg  : sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType
* ret  : eBASE_MW_WIFI_Ret
* note :
*********************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_ExecuteCloseScript(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType) {
	eBASE_MW_WIFI_Ret eRet = BASE_MW_WIFI_SUCCESS;

	BMW_FuncIn();

	{
		eRet = system("source /firmware/SSwificlose");

	}

	BMW_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_WIFI_PowerOn(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType)
* arg  : sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType
* ret  : eBASE_MW_WIFI_Ret
* note :
*********************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_PowerOn(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType){
	eBASE_MW_WIFI_Ret eRet = BASE_MW_WIFI_SUCCESS;

	BMW_FuncIn();

	{
		char buf[2]; 
		if (psInfo->lStatus & BASE_MW_WIFI_POWERON) {
			BMW_Debug("wifi power already open\n");
			return BASE_MW_WIFI_SUCCESS;
		}		
		
		psInfo->fd = open(WIFICTL_DEV,O_RDWR); 
		if(psInfo->fd < 0){ 
			BMW_Debug("wifi module open failed\n"); 
			return BASE_MW_WIFI_FAIL; 
		}		
		write(psInfo->fd, buf,1);
		eRet = close(psInfo->fd); 
		if(eRet < 0){ 
			BMW_Debug("wifi module close failed\n"); 
			return BASE_MW_WIFI_FAIL; 
		}		
		psInfo->lStatus &= ~ BASE_MW_WIFI_POWEROFF;
		psInfo->lStatus |= BASE_MW_WIFI_POWERON;

	}

	BMW_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_WIFI_PowerOff(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType)
* arg  : sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType
* ret  : eBASE_MW_WIFI_Ret
* note :
*********************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_PowerOff(sBASE_MW_WIFI_Info* psInfo, eBASE_MW_WIFI_ScriptType eType){
	eBASE_MW_WIFI_Ret eRet = BASE_MW_WIFI_SUCCESS;

	BMW_FuncIn();

	{
		char buf[2];
		if (psInfo->lStatus & BASE_MW_WIFI_POWEROFF) {
			BMW_Debug("wifi power already close\n");
			return BASE_MW_WIFI_SUCCESS;
		}		

		psInfo->fd = open(WIFICTL_DEV,O_RDWR); 
		if(psInfo->fd < 0){ 
			BMW_Debug("wifi module open failed\n"); 
			return BASE_MW_WIFI_FAIL; 
		}		
		write(psInfo->fd, buf,0);
		eRet = close(psInfo->fd); 
		if(eRet < 0){ 
			BMW_Debug("wifi module close failed\n"); 
			return BASE_MW_WIFI_FAIL; 
		}		
		psInfo->lStatus &= ~ BASE_MW_WIFI_POWERON;
		psInfo->lStatus |= BASE_MW_WIFI_POWEROFF;

	}

	BMW_FuncOut();

	return eRet;
}

/*********************************************
* func : eBASE_MW_WIFI_CreatConfig(CHAR* pcUserName, CHAR* pcPassWord)
* arg  : CHAR* pcUserName, CHAR* pcPassWord
* ret  : eBASE_MW_WIFI_Ret
* note :
*********************************************/
eBASE_MW_WIFI_Ret eBASE_MW_WIFI_CreatConfig(FILE *fp,CHAR* pcUserName, CHAR* pcPassWord) {
	eBASE_MW_WIFI_Ret eRet = BASE_MW_WIFI_SUCCESS;
	
	BMW_FuncIn();
	{
		CHAR cContent[1024];
		memset(cContent,0,sizeof(cContent));
		CHAR* pcContent = cContent;

		pcContent += sprintf(pcContent,"interface=wlan0\n");
		pcContent += sprintf(pcContent,"channel=1\n");
		pcContent += sprintf(pcContent,"logger_syslog=-1\n");
		pcContent += sprintf(pcContent,"logger_syslog_level=2\n");
		pcContent += sprintf(pcContent,"logger_stdout=-1\n");
		pcContent += sprintf(pcContent,"logger_stdout_level=2\n");
		pcContent += sprintf(pcContent,"ssid=%s\n",pcUserName);
		pcContent += sprintf(pcContent,"dtim_period=2\n");
		pcContent += sprintf(pcContent,"max_num_sta=255\n");
		pcContent += sprintf(pcContent,"macaddr_acl=0\n");
		pcContent += sprintf(pcContent,"auth_algs=1\n");
		pcContent += sprintf(pcContent,"ignore_broadcast_ssid=0\n");
		pcContent += sprintf(pcContent,"wme_enabled=0\n");
		pcContent += sprintf(pcContent,"ieee8021x=0\n");
		pcContent += sprintf(pcContent,"eapol_version=2\n");
		pcContent += sprintf(pcContent,"eapol_key_index_workaround=0\n");
		pcContent += sprintf(pcContent,"eap_server=1\n");
		pcContent += sprintf(pcContent,"wpa=3\n");
		pcContent += sprintf(pcContent,"wpa_passphrase=%s\n",pcPassWord);
		pcContent += sprintf(pcContent,"wpa_key_mgmt=WPA-PSK\n");
		pcContent += sprintf(pcContent,"wpa_pairwise=TKIP CCMP\n");
		pcContent += sprintf(pcContent,"wpa_strict_rekey=1\n");
		pcContent += sprintf(pcContent,"hw_mode=g\n");
		pcContent += sprintf(pcContent,"ieee80211n=1\n");
		pcContent += sprintf(pcContent,"wmm_enabled=1\n");
		fwrite(cContent,strlen(cContent)+1,1,fp);
	}

	BMW_FuncOut();

	return eRet;

}

#ifdef __cplusplus
}
#endif
