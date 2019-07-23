//******************************************************************************
//  Секция include:
//******************************************************************************
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "stm32f4xx_rtc.h"

#include "cgiRequest.h"
#ifndef HTTP_SERVER_DBG

#include "debug_serial.h"
#ifdef BOOTLOADER
#include "stm32f4xx.h"
#include "http_flash.h"
#include "crc.h"
#else
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"

#include "adcModule.h"
#include "io.h"
#endif
#endif

#include "macros.h"
#include "version.h"
#include "configurationModule.h"
#include "loopStateMachine.h"

#include "list_ds.h"
#include "eventLogging.h"

#include "sensorsTask.h"

#include "sensorsStateMachine.h"
#include "readersStateMachine.h"
//******************************************************************************
//  Секция определения локальных макросов
//******************************************************************************
#define STX 0x02
#define ETX 0x03

//******************************************************************************
//  Секция определения переменных, используемых в модуле
//******************************************************************************
static const char *cgiRequest[] = {
#define XX(name, request, function, allow) request,
  MAP_CGI_FUNCTION(XX)
#undef XX
  NULL
};

static const uint8_t cgiAllowRequest[] = {
#define XX(name, request, function, allow) allow,
  MAP_CGI_FUNCTION(XX)
#undef XX
  NULL
};
//------------------------------------------------------------------------------
// Глобальные
//------------------------------------------------------------------------------

#ifdef BOOTLOADER

#else
static const uint8_t B419_adr = 254;
#ifndef HTTP_SERVER_DBG
extern xSemaphoreHandle DeviceReboot;
extern xSemaphoreHandle DeviceInitCfg;

void resetWebAuthTimer (xTimerHandle xTimer);
static StaticTimer_t xTimerBuffer[5];
#endif
#endif

#ifdef BOOTLOADER
	uint8_t isAuthorization = 0;
#else
static uint8_t isAuthorization = 0;

struct authStack_t {
	in_addr_t addr;
	in_port_t port;
	xTimerHandle resetWebAuth;
	unsigned portBASE_TYPE id;
};

static const portBASE_TYPE resetWebAuthID[5] = {901, 902, 903, 904, 905};
static struct authStack_t authStack[5];
#endif

static authorization_t auth;
#ifndef BOOTLOADER
	static converterParam_t converter;
	static ethernet_t eth_param;
	static hw_addr_t hw_param;
	static alarmLoop_t alarmLoop[ALARM_LOOP_ELEMENT];
	static outputTactics_t outputTactics[6];
	static alarmLoopState_t alarmLoopState[ALARM_LOOP_ELEMENT];
	static boot_mode_t boot_mode;
	static session_t session;

	static alarmSens_t alarmSens[ALARM_SENS_ELEMENT];
	static alarmSensState_t alarmSensState[ALARM_SENS_ELEMENT];

	static reader_t readers[READERS_COUNT];
#endif


#define XX(name, request, function, allow) static void function (char *buf, char **param, uint8_t len_param);
 MAP_CGI_FUNCTION(XX)
#undef XX

static void (*cgiFunction[CGI_Q])(char *buf, char **param, uint8_t len_param) = {
#define XX(name, request, function, allow) function,
   MAP_CGI_FUNCTION(XX)
#undef XX
   };

static uint8_t parseParameter (char *param, char **buff);

#ifndef BOOTLOADER
uint8_t checkIpByAuthorization (struct sockaddr_in *from) {
	for (uint8_t ix_auth = 0; ix_auth != NELEMS(authStack); ++ix_auth) {
		if (authStack[ix_auth].addr == from->sin_addr.s_addr)
		{
			return 1;
		}
	}
	return 0;
}
#endif

static char *buff[150];     //буфер параметров
#ifdef BOOTLOADER
void cgiHandler (char *uri, char *outBuf, char type) {
#else
void cgiHandler (char *uri, char *outBuf, struct sockaddr_in *from) {
#endif
  char *param;
#ifndef BOOTLOADER
	static bool isInit = true;

	if (isInit) {
		for (uint8_t ix = 0; ix != NELEMS(authStack); ++ix) {
			authStack[ix].addr = 0xffffffff;
			authStack[ix].port = 0xffff;
			authStack[ix].resetWebAuth = NULL;
			authStack[ix].id = resetWebAuthID[ix];

#ifndef HTTP_SERVER_DBG
	#ifndef BOOTLOADER
		if (authStack[ix].resetWebAuth == NULL)
			authStack[ix].resetWebAuth = xTimerCreateStatic("timAuth",
																	pdMS_TO_TICKS(300000),
																	pdFALSE,
																	(void *)&authStack[ix].id,
																	resetWebAuthTimer,
																	&xTimerBuffer[ix]);
	#endif
#endif
		}

		isInit = false;
	}
#endif
  if(strchr(uri, '?')) {
    uri = strtok(uri, "?");
    param = strtok(NULL, "?");
  }
  else
    param = NULL;

  uint8_t countParam = parseParameter(param, buff);
#ifndef BOOTLOADER
	isAuthorization = 0;
	uint8_t ix_auth;
	for (ix_auth = 0; ix_auth != NELEMS(authStack); ++ix_auth) {
		if ( authStack[ix_auth].addr == from->sin_addr.s_addr)
		{
			isAuthorization = 1;
			break;
		}
		else if (authStack[ix_auth].addr == 0xffffffff) {
			isAuthorization = 0;
			break;
		}
	}
	if (ix_auth >= NELEMS(authStack)) return;
#endif

  for (uint8_t ix = 0; ix != CGI_Q; ++ix) {
    const char *tmp = cgiRequest[ix];
    if ( strncmp(uri, tmp, strlen(tmp)) == 0) {
      if ( (isAuthorization == 0
#ifdef BOOTLOADER
        && (cgiAllowRequest[ix]))
#else
        && (cgiAllowRequest[ix]))
#endif
				#ifdef BOOTLOADER
					 || ( (isAuthorization == 1 || type == 'u')
				#else
					 || ( isAuthorization == 1
				#endif

        #ifndef HTTP_SERVER_DBG
					#ifndef BOOTLOADER
							 && xTimerReset (authStack[ix_auth].resetWebAuth, 1)
					#endif
         #endif
								)) {
        cgiFunction[ix](outBuf, buff, countParam);
			}
			else {
				cgiFunction[CGI_SEND_404](outBuf, buff, countParam);
			}
      break;
    }
		if (ix == CGI_Q - 1) cgiFunction[CGI_SEND_404](outBuf, buff, countParam);
  }

#ifndef BOOTLOADER
	if (   isAuthorization == 1
			&& (authStack[ix_auth].addr == 0xffffffff))
	{
		authStack[ix_auth].addr = from->sin_addr.s_addr;
		authStack[ix_auth].port = from->sin_port;

#ifndef HTTP_SERVER_DBG
	(void)xTimerReset (authStack[ix_auth].resetWebAuth, 1);
#endif
	}
#endif
}


//******************************************************************************
//  Секция описания локальных функций
//******************************************************************************
static uint8_t parseParameter (char *param, char **buff) {
  uint8_t num = 0;
  char *tmp = NULL;

  if (param) {
    if (strchr(param,'&')) {
      tmp = strtok(param,"&");

      while (tmp != NULL) {
        buff[num] = tmp;
        num++;
        tmp = strtok(NULL,"&");
      }
    }
    else {
			buff[num] = param;
			num++;
    }
  }

  return num;
}

static void send404(char *buf, char **param, uint8_t len_param) {
	(void)param;
	(void)len_param;
	sprintf(buf, "404 Not Found");
}

static void getVersion(char *buf, char **param, uint8_t len_param) {
  (void)param;
  (void)len_param;
#ifdef BOOTLOADER
	sprintf(buf, "%s\n", firmware_version);
#else
	sprintf(buf, "fw=%s\n", firmware_version);
#endif
}

#ifndef BOOTLOADER
static void getFirmware(char *buf, char **param, uint8_t len_param) {
	uint32_t seqn = 0;
	char *tempstr;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "seq=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			seqn = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	sprintf(buf, "%c{\"fw\":\"%s\",\"seq\":%lu}%c", STX, firmware_version, seqn, ETX);
}

static void getInfo(char *buf, char **param, uint8_t len_param) {
	uint32_t seqn = 0;
	char *tempstr;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "seq=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			seqn = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	memset(&session, 0, sizeof(session_t));
	getSaveConfiguration(CONF_ID_SESSION, &session);

	sprintf(buf, "%c{\"seq\":%lu,\"vsd\":%lu,\"si\":%lu}%c", STX, seqn, session.versionCfgDev, session.session, ETX);
}
#endif

#ifdef BOOTLOADER
/*
Загрузчик принимает прошивку, где первые 512 байт являются служебной информацией:
	- 4 байта : размер прошивки
	- 4 байта : хэш коммита, на основе которого собрана прошивка
	- 4 байта : код для загрузчика
	- 4 байта : crc32 прошивки
	- 496 байт : случайные числа
	- сама прошивка
*/

static const uint32_t bootloaderKey = 0x365b18a3;
static uint32_t firmInfoRecv[FI_SIZE] = {0};
static uint32_t firmInfoFromMem[FI_SIZE] = {0};
static uint32_t TotalReceived = 0;
static uint32_t lastWritePart = 0;
static volatile uint32_t flashWriteAddress;

static void convertBinToHex (char *from, char *to, uint32_t len) {
  char byte[3];

  for (uint32_t ix = 0; ix < len; ++ix) {
    byte[0] = from[ix*2];
    byte[1] = from[ix*2 + 1];
    byte[2] = '\0';
		to[ix] = (uint8_t)strtol(byte, NULL, 16);
  }
}

static void readSubtitle(char *data) {
  for(firmInfo_t ix = FI_LEN; ix != FI_SIZE; ++ix) {
    firmInfoRecv[ix] = (uint32_t)*data++ << 24;
    firmInfoRecv[ix] |= (uint32_t)*data++ << 16;
    firmInfoRecv[ix] |= (uint32_t)*data++ << 8;
    firmInfoRecv[ix] |= (uint32_t)*data++;
  }
}
/*
  firmInfo.cgi - получает в качестве данных пакет вида:
	"?numberPart=part&length=jx&crc=crc1&uploadPart=часть прошивки, размером 512 байт",
  где в данных:
  - 0 - 3 байты : размер прошивки
  - 4 - 7 байты : хэш коммита, на основе которого собрана прошивка
  - 8 - 11 байты : код для загрузчика
  - 12 - 15 байты : crc32 прошивки
	- остальные 496 байт : случайные числа
  после получения пакета, сравнивается полученный код загрузчика с записанным в прошивке,
  если совпадает, то посылает в ответ строку вида: "KEY IS MATCH\r\n",
  иначе - "NO MATCH\r\n"
*/
//static uint8_t cnt = 0;
static char dataArr[1024];
static char *ptr = NULL;
static void firmInfo(char *buf, char **param, uint8_t len_param) {
	char *tempstr;

	uint16_t tmpCrc = 0xFFFF;
	uint32_t tmpLen = 0;
	uint32_t tmpNumPart = 0;
	ptr = NULL;

	if (isAuthorization) {
    for (uint8_t i = 0; i != len_param; ++i) {
			tempstr = strpbrk(param[i], "=");
			uint32_t part = (uint32_t)strtol(tempstr + 1, NULL, 10);
			if(strstr(param[i], "crc=") != NULL) {
				tempstr = strpbrk(param[i], "=");
				tmpCrc = (uint16_t)part;
      }
			else if(strstr(param[i], "length=") != NULL) {
				tmpLen = part;
      }
			else if(strstr(param[i], "numberPart=") != NULL) {
				tmpNumPart = (uint32_t)part;
      }
			else if(strstr(param[i], "uploadPart=") != NULL) {
				ptr = tempstr + 1;
      }
    }

    if (tmpNumPart == 0) {
      memset(dataArr, 0, sizeof(dataArr));

      if (ptr != NULL)
        convertBinToHex(ptr, dataArr, tmpLen);

      uint16_t init = 0xffff;
			init = calcCrc16CCITT( (uint8_t*)dataArr, (uint16_t)tmpLen);

      if (init == tmpCrc) {
        readSubtitle(dataArr);

        if (firmInfoRecv[FI_KEY] == bootloaderKey) {
//          SYS_DEBUGF(SYS_ETH_DBG, ("count firm info %d\n", cnt++));
//          SYS_DEBUGF(SYS_ETH_DBG, ("    firmInfo: len:%d hash:%d key:%d crc:%d\n",
//                                   firmInfoRecv[FI_LEN], firmInfoRecv[FI_HASH], firmInfoRecv[FI_KEY], firmInfoRecv[FI_CRC]));
          sprintf(buf, "KEY IS MATCH\r\n");
//          SYS_DEBUGF(SYS_ETH_DBG, ("    firmInfo: len:%d hash:%d key:%d crc:%d\n",
//                                   firmInfoRecv[FI_LEN], firmInfoRecv[FI_HASH], firmInfoRecv[FI_KEY], firmInfoRecv[FI_CRC]));
        }
        else {
          sprintf(buf, "NO MATCH\r\n");
        }
      }
      else
        sprintf(buf, "error crc\r\n");
    }
  }
  else
    sprintf(buf, "error auth\r\n");
}

/*
  isFirmNew.cgi - происходит сравнение текущей прошивки с той, что хотят записать,
  если не совпадает, то в ответ посылается строка вида: "FIRM IS NEW"
  если подозрения, что это та же прошивка, то
	"FIRM IS MATCH::%d", где %d - это номер последней части прошивки (1 часть = 512 байт),
  Если прошивка до запроса была в контроллере, то номер последней части определяется, как
	lenght/512+lenght%512?1:0
  если уже происходила заливка прошивки, но произошел какой-то сбой не связанный с питанием, то присылается
  номер последней записанной части.
*/
static void isFirmNew(char *buf, char **param, uint8_t len_param) {
  (void)param;
  (void)len_param;
  if (isAuthorization) {
    readWriteConfiguration(CONFIG_READ, CONF_ID_FIRM_INFO, (uint8_t *)&firmInfoFromMem);

    if (  firmInfoRecv[FI_HASH] == firmInfoFromMem[FI_HASH]
      &&  firmInfoRecv[FI_CRC] == firmInfoFromMem[FI_CRC]
      &&  firmInfoRecv[FI_LEN] == firmInfoFromMem[FI_LEN] ) {
				uint32_t lost = firmInfoFromMem[FI_LEN] % 512 ? 1 : 0;
        if (lastWritePart == 0)
					lastWritePart = firmInfoFromMem[FI_LEN] / 512 + lost;

				sprintf(buf, "FIRM IS MATCH::%lu", lastWritePart);
    }
    else {
      sprintf(buf, "FIRM IS NEW");
    }
  }
  else
    sprintf(buf, "error auth");
}

/*
  uploadFile.cgi - получает пакет вида:
	"?numberPart=part&length=jx&crc=crc1&uploadPart=часть прошивки, размером 512 байт".
*/
static void uploadFile(char *buf, char **param, uint8_t len_param) {
  char *tempstr;

	static uint16_t tmpCrc = 0xFFFF;
	tmpCrc = 0xFFFF;
	uint32_t tmpLen = 0;
	uint32_t tmpNumPart = 0;
//	char *
	ptr = NULL;

  if (isAuthorization) {
		for (uint8_t i = 0; i != len_param; ++i) {
			tempstr = strpbrk(param[i], "=");
			uint32_t part = (uint32_t)strtol(tempstr + 1, NULL, 10);

			if(strstr(param[i], "crc=") != NULL) {
				tmpCrc = (uint16_t)part;
      }
			else if(strstr(param[i], "length=") != NULL) {
				tmpLen = part;
      }
			else if(strstr(param[i], "numberPart=") != NULL) {
				tmpNumPart = part;
      }
			else if(strstr(param[i], "uploadPart=") != NULL) {
				ptr = tempstr + 1;
      }
    }

    if (tmpNumPart <= lastWritePart) {
      sprintf(buf, "UPLOAD OK");
    }
    else {
      if (ptr != NULL) {
				memset(dataArr, 0, sizeof(dataArr));
        convertBinToHex(ptr, dataArr, tmpLen);

        uint16_t init = 0xffff;
				init = calcCrc16CCITT( (uint8_t*)dataArr, (uint16_t)tmpLen);

        if (init == tmpCrc) {
					flashWriteData(&flashWriteAddress, (uint32_t*)dataArr, tmpLen/4);

          TotalReceived += tmpLen;
          lastWritePart = tmpNumPart;

          if (TotalReceived >= firmInfoRecv[FI_LEN]) {
            unsigned int pageAddress = USER_FLASH_FIRST_PAGE_ADDRESS;
            uint32_t lenFirm = firmInfoRecv[FI_LEN];

            uint32_t crc = 0xffffffff;

            crc = calcCrc32BZIP2((uint8_t* )pageAddress, lenFirm);

            if (firmInfoRecv[FI_CRC] == crc) {
              boot_mode_t bm;
              bm.isBootFirm = BOOT_MODE_APP;
              bm.isNewFirm = BOOT_MODE_NEWF;

              readWriteConfiguration(CONFIG_WRITE, CONF_ID_BOOT_MODE, (uint8_t *)&bm);
              sprintf(buf, "upload done");
            }
          }
          else {
            sprintf(buf, "UPLOAD OK");
          }
        }
      }
    }
  }
  else
    sprintf(buf, "error auth");
}

/*
  eraseFirmBlock.cgi - очищает область памяти с прошивкой и записывает полученную информацию о прошивки
  в энергонезависимую память
*/
static void eraseFirmBlock(char *buf, char **param, uint8_t len_param) {
  (void)len_param;
  (void)param;

  if (isAuthorization) {
    TotalReceived = 0;
    lastWritePart = 0;

    flashEraseBlock();
    flashWriteAddress = USER_FLASH_FIRST_PAGE_ADDRESS;

    readWriteConfiguration(CONFIG_WRITE, CONF_ID_FIRM_INFO, (uint8_t *)&firmInfoRecv);
    sprintf(buf, "erase OK!");
  }
  else
    sprintf(buf, "error auth");
}

static void rebootMCU(char *buf, char **param, uint8_t len_param) {
	(void)buf;
	(void)param;
	(void)len_param;
	sprintf(buf, "Reboot OK!");

	__disable_irq();
	NVIC_SystemReset();
}

static void getCurrentFirm(char *buf, char **param, uint8_t len_param) {
	(void)buf;
	(void)param;
	(void)len_param;

	boot_mode_t bm;
	bm.isBootFirm = BOOT_MODE_APP;
	bm.isNewFirm = BOOT_MODE_NEWF;

	readWriteConfiguration(CONFIG_WRITE, CONF_ID_BOOT_MODE, (uint8_t *)&bm);
	sprintf(buf, "done");
}

#else
static uint8_t is_def;

static void getHwParam(char *buf, char **param, uint8_t len_param) {
  (void)param;
  (void)len_param;

  checkHw(buf, param, len_param);

  if (is_def) {
    sprintf(buf, "hw=%02x:%02x:%02x:%02x:%02x:%02x\r\n",
									default_hw_param.addr_default[0], default_hw_param.addr_default[1], default_hw_param.addr_default[2],
									default_hw_param.addr_default[3], default_hw_param.addr_default[4], default_hw_param.addr_default[5]);
  }
  else {
		memset(&hw_param, 0, sizeof(hw_param));
    getSaveConfiguration(CONF_ID_MAC_ADDR, &hw_param);

		if (   hw_param.addr_usr[0] == 0xFF
				&& hw_param.addr_usr[1] == 0xFF
				&& hw_param.addr_usr[2] == 0xFF)
		{
			sprintf(buf, "hw=%02x:%02x:%02x:%02x:%02x:%02x\r\n",
										hw_param.addr_default[0], hw_param.addr_default[1], hw_param.addr_default[2],
										hw_param.addr_default[3], hw_param.addr_default[4], hw_param.addr_default[5]);
		}
		else {
			sprintf(buf, "hw=%02x:%02x:%02x:%02x:%02x:%02x\r\n",
										hw_param.addr_usr[0], hw_param.addr_usr[1], hw_param.addr_usr[2],
										hw_param.addr_usr[3], hw_param.addr_usr[4], hw_param.addr_usr[5]);
		}

  }
}

static void checkHw(char *buf, char **param, uint8_t len_param) {
  (void)param;
  (void)len_param;

	memset(&hw_param, 0, sizeof(hw_param));

  getSaveConfiguration(CONF_ID_MAC_ADDR, &hw_param);

	for (uint8_t ix = 0; ix != NELEMS(default_hw_param.addr_default); ++ix)
		if (hw_param.addr_default[ix] == default_hw_param.addr_default[ix]
		 || hw_param.addr_default[ix] == 0xff)
      is_def = 1;
    else {
      is_def = 0;
      break;
    }

  sprintf(buf, "is_def=%d\r\n",
                is_def);
}

static void setHwParam(char *buf, char **param, uint8_t len_param){
  char *tempstr;

	checkHw(buf, param, len_param);

  for (uint8_t i = 0; i != len_param; ++i) {
    if (strstr(param[i], "default") != NULL) {
      setDefaultHWConfig(&hw_param);
    }

    if (strstr(param[i], "hw=") != NULL) {
			tempstr = strpbrk(param[i], "=");

			if (strchr(tempstr,'|') != NULL) {
				setDefaultHWConfig(&hw_param);
				for (uint8_t ix = 0; ix != NELEMS(hw_param.addr_default); ++ix) {
					uint8_t hw_part = (uint8_t)strtol(tempstr + 1, &tempstr, 16);
					if (is_def) hw_param.addr_default[ix] = hw_part;
					else hw_param.addr_usr[ix] = hw_part;
        }
      }
    }
  }

  if (setConfiguration(CONF_ID_MAC_ADDR, &hw_param) == 0)
    sprintf(buf, "OK!");
  else
    sprintf(buf, "error ip parameters!");
}



static void setIpParam(char *buf, char **param, uint8_t len_param) {
  char *tempstr;

  for (uint8_t i = 0; i != len_param; ++i) {
    if (strstr(param[i], "default") != NULL) {
      setDefaultEthernetConfig(&eth_param);
    }

		tempstr = strpbrk(param[i], "=");

    if (strstr(param[i], "ip=") != NULL) {
			if (strchr(tempstr,'|') != NULL) {
				for (uint8_t ix = 0; ix != NELEMS(eth_param.ip); ++ix) {
					eth_param.ip[ix] = (uint8_t)strtol(tempstr + 1, &tempstr, 10);
        }
      }
    }

    if (strstr(param[i], "mask=") != NULL) {
			if (strchr(tempstr,'|') != NULL) {
				for (uint8_t ix = 0; ix != NELEMS(eth_param.mask); ++ix) {
					eth_param.mask[ix] = (uint8_t)strtol(tempstr + 1, &tempstr, 10);
        }
      }
    }

    if (strstr(param[i], "gw=") != NULL) {
			if (strchr(tempstr,'|') != NULL) {
				for (uint8_t ix = 0; ix != NELEMS(eth_param.gw); ++ix) {
					eth_param.gw[ix] = (uint8_t)strtol(tempstr + 1, &tempstr, 10);
        }
      }
    }
  }

  if (setConfiguration(CONF_ID_ETHERNET, &eth_param) == 0)
    sprintf(buf, "OK!");
  else
    sprintf(buf, "error ip parameters!");
}

static void getIpAddr(char *buf, char **param, uint8_t len_param) {
  (void)param;
  (void)len_param;
	memset(&eth_param, 0, sizeof(eth_param));
  getSaveConfiguration(CONF_ID_ETHERNET, &eth_param);

  sprintf(buf, "new_ip=%d.%d.%d.%d\ndef_ip=%d.%d.%d.%d\n",
          eth_param.ip[0], eth_param.ip[1], eth_param.ip[2], eth_param.ip[3],
          default_ethernet_param.ip[0], default_ethernet_param.ip[1], default_ethernet_param.ip[2], default_ethernet_param.ip[3]);
}

static void getIpParam(char *buf, char **param, uint8_t len_param) {
  (void)param;
  (void)len_param;

  memset(&eth_param, 0, sizeof(ethernet_t));
  getSaveConfiguration(CONF_ID_ETHERNET, &eth_param);

  if (eth_param.ip[0] == 0xff && eth_param.ip[1] == 0xff && eth_param.ip[2] == 0xff && eth_param.ip[3] == 0xff) {
    setDefaultEthernetConfig(&eth_param);
  }

  sprintf(buf, "ip=%d.%d.%d.%d\nmask=%d.%d.%d.%d\ngw=%d.%d.%d.%d\r\n",
          eth_param.ip[0], eth_param.ip[1], eth_param.ip[2], eth_param.ip[3],
          eth_param.mask[0], eth_param.mask[1], eth_param.mask[2], eth_param.mask[3],
          eth_param.gw[0], eth_param.gw[1], eth_param.gw[2], eth_param.gw[3]);
}

static void rebootMCU(char *buf, char **param, uint8_t len_param) {
	uint32_t seqn = 0;
	char *tempstr;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "seq=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			seqn = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	if (seqn) {
		sprintf(buf, "%c{\"rbt\":\"OK\",\"seq\":%lu}%c", STX, seqn, ETX);
	}
	else {
		sprintf(buf, "OK!");
	}

#ifndef HTTP_SERVER_DBG
  xSemaphoreGive(DeviceReboot);
#endif
}

static void getAuthorization(char *buf, char **param, uint8_t len_param) {
  (void)param;
  (void)len_param;

  memset(&auth, 0, sizeof(authorization_t));
  getSaveConfiguration(CONF_ID_AUTHOR, &auth);

  if (auth.login[0] == 0xff && auth.login[1] == 0xff && auth.login[2] == 0xff && auth.login[3] == 0xff) {
    setDefaultAuthorization(&auth);
  }

  sprintf(buf, "login=%s\r\n", auth.login);
}
#endif

static uint8_t checkLoginPswd(char **param, uint8_t len_param) {
	char *tempstr;

	uint8_t flag = 0;

	memset(&auth, 0, sizeof(authorization_t));
#ifdef BOOTLOADER
	if (readWriteConfiguration(CONFIG_READ, CONF_ID_AUTHOR, (uint8_t *)&auth)) {
		setDefaultAuthorization(&auth);
	}
#else
	getSaveConfiguration(CONF_ID_AUTHOR, &auth);
#endif
	uint8_t login_len = (uint8_t)strlen(auth.login);
	uint8_t password_len = (uint8_t)strlen(auth.password);
	for (uint8_t i = 0; i != len_param; ++i) {
		tempstr = strpbrk(param[i], "=");

		if (strstr(param[i], "login=") != NULL) {
			tempstr++;
			if (login_len == strlen(tempstr)) {
				if (strncmp(auth.login, tempstr, login_len) == 0) {
					flag++;
				}
			}
		}
		else if (strstr(param[i], "password=") != NULL) {
			tempstr++;
			if (password_len == strlen(tempstr)) {
				if (strncmp(auth.password, tempstr, password_len) == 0) {
					flag++;
				}
			}
		}
	}
	return flag;
}

static void checkAuthorization(char *buf, char **param, uint8_t len_param) {
	uint8_t flag = checkLoginPswd(param, len_param);

	if (flag == 2) {
		isAuthorization = 1;
		sprintf(buf, "OK!");
	}
	else {
		isAuthorization = 0;
		sprintf(buf, "404 Not Found");
	}
}

#ifndef BOOTLOADER
static void checkAuthorization_udp(char *buf, char **param, uint8_t len_param) {
	uint8_t flag = checkLoginPswd(param, len_param);
	uint32_t seqn = 0;
	char *tempstr;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "seq=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			seqn = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	if (flag == 2) {
		isAuthorization = 1;
		sprintf(buf, "%c{\"auth\":\"OK\",\"seq\":%lu}%c", STX, seqn, ETX);
	}
	else {
		isAuthorization = 0;
		sprintf(buf, "%c{\"auth\":\"ERROR\",\"seq\":%lu}%c", STX, seqn, ETX);
	}
}

static void setAuthorization(char *buf, char **param, uint8_t len_param) {
  (void)buf;

  char *tempstr;

  memset(&auth, 0, sizeof(authorization_t));
  getSaveConfiguration(CONF_ID_AUTHOR, &auth);

  for (uint8_t i = 0; i != len_param; ++i) {
    if (strstr(param[i], "default") != NULL) {
      setDefaultAuthorization(&auth);
    }

		tempstr = strpbrk(param[i], "=");
		if (strstr(param[i], "login=") != NULL) {
			sprintf(auth.login, "%s", tempstr + 1);
    }

		if (strstr(param[i], "password=") != NULL) {
			if (tempstr != NULL && strstr(tempstr + 1, "null=") == NULL) {
				tempstr++;
				size_t len = strlen(tempstr);
				if (len >= 4 && len <= 20)
          sprintf(auth.password, "%s", tempstr);
			}
    }
  }

  if (setConfiguration(CONF_ID_AUTHOR, &auth) == 0)
    sprintf(buf, "OK!");
  else
    sprintf(buf, "error auth parameters!");
}

void getConverter(char *buf, char **param, uint8_t len_param) {
  (void)param;
  (void)len_param;
  getSaveConfiguration(CONF_ID_CONVERTER, &converter);

  sprintf(buf, "mode=%d\nhostport=%d\nhostip=%d.%d.%d.%d\nport=%d\nspeed=s%d\r\n",
          converter.mode,
          converter.host_port,
          converter.host_ip[0], converter.host_ip[1], converter.host_ip[2], converter.host_ip[3],
          converter.port, converter.serial_speed);
}

void setConverter(char *buf, char **param, uint8_t len_param) {
  (void)buf;

	char *tempstr;

  memset(&converter, 0, sizeof(converterParam_t));
  getSaveConfiguration(CONF_ID_CONVERTER, &converter);

  for (uint8_t i = 0; i != len_param; ++i) {
		tempstr = strpbrk(param[i], "=");

    if (strstr(param[i], "default") != NULL) {
      setDefaultConverter(&converter);
    }
		else if (strstr(param[i], "mode=") != NULL) {
			converter.mode = (uint16_t)strtol(tempstr + 1, NULL, 10);
    }
		else if (strstr(param[i], "speed=s") != NULL) {
			converter.serial_speed = (uint16_t)strtol(tempstr + 2, NULL, 10);
    }
		else if (strstr(param[i], "hostport=") != NULL) {
			converter.host_port = (uint16_t)strtol(tempstr + 1, NULL, 10);
    }
		else if (strstr(param[i], "hostip=") != NULL) {
			if (strchr(tempstr,'|') != NULL) {
				for (uint8_t ix = 0; ix != NELEMS(converter.host_ip); ++ix) {
					converter.host_ip[ix] = (uint8_t)strtol(tempstr + 1, &tempstr, 10);
        }
      }
    }
		else if (strstr(param[i], "port=") != NULL) {
			converter.port = (uint16_t)strtol(tempstr + 1, NULL, 10);
    }
  }

  if (setConfiguration(CONF_ID_CONVERTER, &converter) == 0)
    sprintf(buf, "OK!");
  else
    sprintf(buf, "error converter parameters!");
}


static char tempBuf[255];
void getAlarmLoop(char *buf, char **param, uint8_t len_param) {
  (void)param;
  (void)len_param;
  getSaveConfiguration(CONF_ID_ALARM_LOOP, alarmLoop);

	for (uint8_t ix = 0; ix != NELEMS(alarmLoop); ++ix) {
		if (alarmLoop[ix].mode == MODE_ALARM)
			alarmLoop[ix].check_param |= 0x04;
	}

	for (uint8_t ix = 0; ix != NELEMS(alarmLoop); ++ix) {
		memset(tempBuf, 0, sizeof(tempBuf));
		sprintf(tempBuf,  "mode%u=%lu\n"
											"check_param%u=%lu\n"\
											"delay_alarm%u=%lu\n"\
											"delay_recovery_alarm%u=%lu\n"\
											"time_delay_guard%u=%lu\n"\
											"range0%u=%lu\n"\
											"range1%u=%lu\n"\
											"range2%u=%lu\n"\
											"range3%u=%lu\n"\
											"range4%u=%lu\n",
											ix, alarmLoop[ix].mode,
											ix, alarmLoop[ix].check_param,
											ix, alarmLoop[ix].delay_alarm,
											ix, alarmLoop[ix].delay_recovery_alarm,
											ix, alarmLoop[ix].time_delay_guard,
											ix, alarmLoop[ix].range[0],
											ix, alarmLoop[ix].range[1],
											ix, alarmLoop[ix].range[2],
											ix, alarmLoop[ix].range[3],
											ix, alarmLoop[ix].range[4]);
    strcat(buf, tempBuf);
  }
}

static const char *alarmLoopName[] = {
    "mode",
    "check_param",
    "time_delay_guard",
    "delay_alarm",
    "delay_recovery_alarm",
    "range0",
    "range1",
    "range2",
    "range3",
    "range4",
    "range5"
};

void setAlarmLoop(char *buf, char **param, uint8_t len_param) {
	(void)buf;

	char *tempstr;

	memset(&converter, 0, sizeof(converterParam_t));
	getSaveConfiguration(CONF_ID_ALARM_LOOP, alarmLoop);

	uint8_t ix_loop = 0;
	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "default") != NULL) {
			for (uint8_t ix = 0; ix != NELEMS(alarmLoop); ++ix)
				setDefaultAlarmLoop(&alarmLoop[ix]);
		}
		for (uint8_t ix = 0; ix != NELEMS(alarmLoopName); ++ix) {
			if (strstr(param[i], alarmLoopName[ix]) != NULL) {
				tempstr = strpbrk(param[i], ";");

				ix_loop = (uint8_t)strtol(tempstr + 1, &tempstr, 10);
/*если получили некорректный номер шлейфа, то считаем ВСЕ параметры ошибочными*/
				if (ix_loop > NELEMS(alarmLoop) - 1) {
					sprintf(buf, "error converter parameters!");
					return;
				}

				uint32_t part = (uint32_t)strtol(tempstr + 1, NULL, 10);

				if ( strncmp(alarmLoopName[ix], "mode", sizeof("mode")) == 0 )
					alarmLoop[ix_loop].mode = part;
				else if ( strncmp(alarmLoopName[ix], "check_param", sizeof("check_param")) == 0 )
					alarmLoop[ix_loop].check_param = part;
				else if ( strncmp(alarmLoopName[ix], "time_delay_guard", sizeof("time_delay_guard")) == 0 )
					alarmLoop[ix_loop].time_delay_guard = part;
				else if ( strncmp(alarmLoopName[ix], "delay_alarm", sizeof("delay_alarm")) == 0 )
					alarmLoop[ix_loop].delay_alarm = part;
				else if ( strncmp(alarmLoopName[ix], "delay_recovery_alarm", sizeof("delay_recovery_alarm")) == 0 )
					alarmLoop[ix_loop].delay_recovery_alarm = part;
				else if ( strncmp(alarmLoopName[ix], "range0", sizeof("range0")) == 0 )
					alarmLoop[ix_loop].range[0] = part;
				else if ( strncmp(alarmLoopName[ix], "range1", sizeof("range1")) == 0 )
					alarmLoop[ix_loop].range[1] = part;
				else if ( strncmp(alarmLoopName[ix], "range2", sizeof("range2")) == 0 )
					alarmLoop[ix_loop].range[2] = part;
				else if ( strncmp(alarmLoopName[ix], "range3", sizeof("range3")) == 0 )
					alarmLoop[ix_loop].range[3] = part;
				else if ( strncmp(alarmLoopName[ix], "range4", sizeof("range4")) == 0 )
					alarmLoop[ix_loop].range[4] = part;
				else if ( strncmp(alarmLoopName[ix], "range5", sizeof("range5")) == 0 )
					alarmLoop[ix_loop].range[5] = part;
			}
		}
	}

	if (setConfiguration(CONF_ID_ALARM_LOOP, alarmLoop) == 0)
		sprintf(buf, "OK!");
	else
		sprintf(buf, "error converter parameters!");
}

void getSections (char *buf, char **param, uint8_t len_param) {
	(void)param;
	(void)len_param;

	getSaveConfiguration(CONF_ID_ALARM_LOOP, alarmLoop);

	for (uint8_t ix = 0; ix != NELEMS(alarmLoop); ++ix) {
		memset(tempBuf, 0, sizeof(tempBuf));
		sprintf(tempBuf,  "loop_254_%d=%d\n",
											ix, alarmLoop[ix].section);
		strcat(buf, tempBuf);
	}

	getSaveConfiguration(CONF_ID_ALARM_SENS, alarmSens);

	for (uint8_t ix = 0; ix != NELEMS(alarmSens); ++ix) {
		if (alarmSens[ix].addr == 0) continue;

		memset(tempBuf, 0, sizeof(tempBuf));
		sprintf(tempBuf,  "loop_%d=%d\n",
											alarmSens[ix].addr, alarmSens[ix].section);
		strcat(buf, tempBuf);
	}
}

void setSections (char *buf, char **param, uint8_t len_param) {
	(void)buf;

	char *tempstr;

	getSaveConfiguration(CONF_ID_ALARM_LOOP, alarmLoop);
	getSaveConfiguration(CONF_ID_ALARM_SENS, alarmSens);

	uint8_t ix_loop = 0;
	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "default") != NULL) {
			for (uint8_t ix = 0; ix != NELEMS(alarmLoop); ++ix)
				alarmLoop[ix].section = 0;
			for (uint8_t ix = 0; ix != NELEMS(alarmSens); ++ix)
				alarmSens[ix].section = 0;
		}
		else if (strstr(param[i], "loop_254") != NULL) {
			tempstr = strpbrk(param[i], ";");

			ix_loop = (uint8_t)strtol(tempstr + 1, &tempstr, 10);
			/*если получили некорректный номер шлейфа, то считаем ВСЕ параметры ошибочными*/
			if (ix_loop > NELEMS(alarmLoop) - 1) {
				sprintf(buf, "error converter parameters!");
				return;
			}

			alarmLoop[ix_loop].section = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "loop_") != NULL) {
			tempstr = strpbrk(param[i], "_");
			uint8_t tmp_addr = (uint8_t)strtol(tempstr + 1, &tempstr, 10);

			for (uint8_t ix = 0; ix != NELEMS(alarmSens); ++ix) {
				if (alarmSens[ix].addr != tmp_addr) continue;

				alarmSens[ix].section = (uint8_t)strtol( (tempstr+1), NULL, 10);
				break;
			}
		}
	}

	if (setConfiguration(CONF_ID_ALARM_LOOP, alarmLoop) == 0)
		sprintf(buf, "OK!");
	else
		sprintf(buf, "error section loop parameters!");

	if (setConfiguration(CONF_ID_ALARM_SENS, alarmSens) == 0)
		sprintf(buf, "OK!");
	else
		sprintf(buf, "error section sens parameters!");
}

void getOutput (char *buf, char **param, uint8_t len_param) {
	(void)param;
	(void)len_param;

	getSaveConfiguration(CONF_ID_OUTPUT_TACTICS, outputTactics);

	for (uint8_t ix = 0; ix != NELEMS(outputTactics); ++ix) {
		memset(tempBuf, 0, sizeof(tempBuf));
		sprintf(tempBuf,  "program%u=%lu\n"
											"time_ctrl%u=%lu\n"
											"period_ctrl%u=%lu\n"
											"source_sensors%u=%lu\n"
											"source_sensors_h%u=%lu\n"
											"source_states%u=%lu\n",
											ix, outputTactics[ix].id_program,
											ix, outputTactics[ix].time_ctrl,
											ix, outputTactics[ix].period_ctrl,
											ix, outputTactics[ix].source_sensors,
											ix, outputTactics[ix].source_sensors_h,
											ix, outputTactics[ix].source_states);

		strcat(buf, tempBuf);
	}
}

static const char *outputTacticsName[] = {
    "program",
    "time_ctrl",
    "period_ctrl",
		"source_sensors_h",
		"source_sensors",
    "source_states"
};

static char tmp_output_name[32];

void setOutput (char *buf, char **param, uint8_t len_param) {
	(void)buf;

	char *tempstr;

	memset(outputTactics, 0, sizeof(outputTactics_t) * 6);
	getSaveConfiguration(CONF_ID_OUTPUT_TACTICS, outputTactics);

	uint8_t ix_loop = 0;
	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "default") != NULL) {
			for (uint8_t ix = 0; ix != NELEMS(outputTactics); ++ix)
				setDefaultOutputTactics(&outputTactics[ix]);
		}

		for (uint8_t ix = 0; ix != NELEMS(outputTacticsName); ++ix) {
			if (strstr(param[i], outputTacticsName[ix]) != NULL) {
				memset(tmp_output_name, 0, sizeof(tmp_output_name));
				memmove(tmp_output_name, param[i], strspn(param[i], outputTacticsName[ix]));

				tempstr = strpbrk(param[i], ";");

				ix_loop = (uint8_t)strtol(tempstr + 1, &tempstr, 10);
/*если получили некорректный номер шлейфа, то считаем ВСЕ параметры ошибочными*/
				if (ix_loop > NELEMS(outputTactics) - 1) {
					sprintf(buf, "error ix output parameters!");
					return;
				}
				uint32_t part = (uint32_t)strtol(tempstr + 1, NULL, 10);
				if ( strncmp(tmp_output_name, "program", sizeof("program")) == 0 )
					outputTactics[ix_loop].id_program = part;
				else if ( strncmp(tmp_output_name, "period_ctrl", sizeof("period_ctrl")) == 0 )
					outputTactics[ix_loop].period_ctrl = part;
				else if ( strncmp(tmp_output_name, "source_sensors_h", sizeof("source_sensors_h")) == 0 )
					outputTactics[ix_loop].source_sensors_h = part;
				else if ( strncmp(tmp_output_name, "source_sensors", sizeof("source_sensors")) == 0 )
					outputTactics[ix_loop].source_sensors = part;
				else if ( strncmp(tmp_output_name, "time_ctrl", sizeof("time_ctrl")) == 0 )
					outputTactics[ix_loop].time_ctrl = part;
				else if ( strncmp(tmp_output_name, "source_states", sizeof("source_states")) == 0 )
					outputTactics[ix_loop].source_states = part;
			}
		}
	}

	for (uint8_t ix = 0; ix != NELEMS(outputTacticsName); ++ix) {
		if (outputTactics[ix].id_program == ID_P_SECUR_BUZ) {
			outputTactics[ix].source_states = (1 << (LSM_ALARM >> 4));
		}
		else if (outputTactics[ix].id_program == ID_P_SECUR_LIGHT) {
			outputTactics[ix].source_states = (1 << (LSM_ALARM >> 4))
																			| (1 << (LSM_UNAVAILABLE >> 4))
																			| (1 << (LSM_GUARD_ON >> 4))
																			| (1 << (LSM_FAULT >> 4));
		}
	}

	if (setConfiguration(CONF_ID_OUTPUT_TACTICS, outputTactics) == 0)
		sprintf(buf, "OK!");
	else
		sprintf(buf, "error output parameters!");
}

void getCtrlSensors (char *buf, char **param, uint8_t len_param) {
	char *tempstr;

	getSaveConfiguration(CONF_ID_ALARM_LOOP, alarmLoop);
	getConfiguration(CONF_ID_ALARM_LOOP_STATE, alarmLoopState);
	getSaveConfiguration(CONF_ID_ALARM_SENS, alarmSens);
	getConfiguration(CONF_ID_ALARM_SENS_STATE, alarmSensState);

	if (len_param == 0) {
		for (uint8_t ix = 0; ix != NELEMS(alarmLoopState); ++ix) {
			memset(tempBuf, 0, sizeof(tempBuf));
			if (alarmLoopState[ix].isGuard || (alarmLoop[ix].check_param & 0x04))
				sprintf(tempBuf,  "onguard_%d_%d\n", B419_adr, ix);
			else
				sprintf(tempBuf,  "offguard_%d_%d\n", B419_adr, ix);

			strcat(buf, tempBuf);
		}

		for (uint8_t ix = 0; ix != NELEMS(alarmSensState); ++ix) {
			if (alarmSensState[ix].addr == 0) continue;

			memset(tempBuf, 0, sizeof(tempBuf));
			if (alarmSensState[ix].isGuard || (alarmSens[ix].check_param & 0x04))
				sprintf(tempBuf,  "onguard_%d\n", alarmSensState[ix].addr);
			else
				sprintf(tempBuf,  "offguard_%d\n", alarmSensState[ix].addr);

			strcat(buf, tempBuf);
		}
		return;
	}

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "loop") != NULL) {
			tempstr = strpbrk(param[i], "01234567");

			if (tempstr) {
				uint32_t ix = (uint32_t)strtol(tempstr, NULL, 10);
				if (alarmLoopState[ix].isGuard || (alarmLoop[ix].check_param & 0x04))
					sprintf(tempBuf,  "onguard_%u_%lu\n", B419_adr, ix);
				else
					sprintf(tempBuf,  "offguard_%u_%lu\n", B419_adr, ix);
				strcat(buf, tempBuf);
			}
			else {
				sprintf(buf, "404 Not Found");
				break;
			}
		}
	}
}

void setCtrlSensors (char *buf, char **param, uint8_t len_param) {
	(void)buf;

	char *tempstr;

	memset(alarmLoopState, 0, sizeof(alarmLoopState_t) * NELEMS (alarmLoopState));
	memset(alarmSensState, 0, sizeof(alarmSensState_t) * NELEMS (alarmSensState));
	getConfiguration(CONF_ID_ALARM_LOOP_STATE, alarmLoopState);
	getConfiguration(CONF_ID_ALARM_SENS_STATE, alarmSensState);

	uint8_t ix_loop = 0;
	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "all_offguard") != NULL) {
			for (uint8_t ix = 0; ix != NELEMS(alarmLoopState); ++ix) {
				alarmLoopState[ix].isGuard = 0;
				setEvent(LSM_EVENT_GUARD_OFF, ix);
			}

			for (uint8_t ix = 0; ix != NELEMS(alarmSensState); ++ix) {
				if (alarmSensState[ix].addr == 0) continue;

				alarmSensState[ix].isGuard = 0;
				setEventSensors(LSM_EVENT_SENS_GUARD_OFF, ix);
			}
		}
		else if (strstr(param[i], "all_onguard") != NULL) {
			for (uint8_t ix = 0; ix != NELEMS(alarmLoopState); ++ix) {
				alarmLoopState[ix].isGuard = 1;
				setEvent(LSM_EVENT_GUARD_ON, ix);
			}

			for (uint8_t ix = 0; ix != NELEMS(alarmSensState); ++ix) {
				if (alarmSensState[ix].addr == 0) continue;

				alarmSensState[ix].isGuard = 0xff;
				setEventSensors(LSM_EVENT_SENS_GUARD_ON, ix);
			}
		}
		else {
			if (strstr(param[i], "onguard_254_") != NULL) {
				tempstr = strpbrk(param[i], "01234567");
				tempstr = strpbrk(tempstr, "_");

				if (tempstr) {
					ix_loop = (uint8_t)strtol(tempstr + 1, NULL, 10);
		/*если получили некорректный номер шлейфа, то считаем ВСЕ параметры ошибочными*/
					if (ix_loop > NELEMS(alarmLoop) - 1) {
						sprintf(buf, "404 Not Found");
						return;
					}
					alarmLoopState[ix_loop].isGuard = 1;
					setEvent(LSM_EVENT_GUARD_ON, ix_loop);
				}
			}
			else if (strstr(param[i], "offguard_254_") != NULL) {
				tempstr = strpbrk(param[i], "01234567");
				tempstr = strpbrk(tempstr, "_");

				if  (tempstr) {
					ix_loop = (uint8_t)strtol(tempstr + 1, NULL, 10);
		/*если получили некорректный номер шлейфа, то считаем ВСЕ параметры ошибочными*/
					if (ix_loop > NELEMS(alarmLoop) - 1) {
						sprintf(buf, "404 Not Found");
						return;
					}
					alarmLoopState[ix_loop].isGuard = 0;
					setEvent(LSM_EVENT_GUARD_OFF, ix_loop);
				}
			}
			else if (strstr(param[i], "offguard_") != NULL) {
				tempstr = strpbrk(param[i], "_");
				uint8_t tmp_addr = (uint8_t)strtol(tempstr + 1, NULL, 10);

				for (uint8_t ix = 0; ix != NELEMS(alarmSensState); ++ix) {
					if (alarmSensState[ix].addr != tmp_addr) continue;

					alarmSensState[ix].isGuard = 0;
					setEventSensors(LSM_EVENT_SENS_GUARD_OFF, ix);
					break;
				}
			}
			else if (strstr(param[i], "onguard_") != NULL) {
				tempstr = strpbrk(param[i], "_");
				uint8_t tmp_addr = (uint8_t)strtol(tempstr + 1, NULL, 10);

				for (uint8_t ix = 0; ix != NELEMS(alarmSensState); ++ix) {
					if (alarmSensState[ix].addr != tmp_addr) continue;

					alarmSensState[ix].isGuard = 0xff;
					setEventSensors(LSM_EVENT_SENS_GUARD_ON, ix);
					break;
				}
			}
		}
	}

	if (setConfiguration(CONF_ID_ALARM_LOOP_STATE, alarmLoopState) == 0)
		sprintf(buf, "OK!");
	else
		sprintf(buf, "404 Not Found");

	if (setConfiguration(CONF_ID_ALARM_SENS_STATE, alarmSensState) == 0)
		sprintf(buf, "OK!");
	else
		sprintf(buf, "404 Not Found");
}

void applyParam (char *buf, char **param, uint8_t len_param) {
	(void)buf;
	(void)param;
	(void)len_param;

#ifndef HTTP_SERVER_DBG
	xSemaphoreGive(DeviceInitCfg);
#endif
}

static uint8_t currentStateSens[ALARM_SENS_ELEMENT];
void getAlarmLoopState (char *buf, char **param, uint8_t len_param) {
	(void)param;
	(void)len_param;

#ifndef HTTP_SERVER_DBG
	struct loopState_t tmp[NELEMS(alarmLoop)];
	adc_getAlarmLoopState(tmp);

	for (uint8_t ix = 0; ix != NELEMS(tmp); ++ix) {
		memset(tempBuf, 0, sizeof(tempBuf));
		sprintf(tempBuf,  "state_254_%d=%d\n",
											ix, tmp[ix].currentRangeLoop);

		strcat(buf, tempBuf);
	}

	getCurrentStateSens(currentStateSens);
	getConfiguration (CONF_ID_ALARM_SENS_STATE, alarmSensState);

	for (uint8_t ix = 0; ix != NELEMS(alarmSensState); ++ix) {
		if (alarmSensState[ix].addr == 0) continue;

		memset(tempBuf, 0, sizeof(tempBuf));
		uint8_t cur_state = 0;
		if (currentStateSens[ix] & 0x01 || currentStateSens[ix] & 0x08) {
			cur_state = 1;
		}
		else if (currentStateSens[ix] & 0x16) {
			cur_state = 2;
		}
		sprintf(tempBuf,  "state_%u=%u %u\n",
											alarmSensState[ix].addr, cur_state, currentStateSens[ix]);

		strcat(buf, tempBuf);
	}
#endif
}

void cgi_getOutputState (char *buf, char **param, uint8_t len_param) {
	(void)param;
	(void)len_param;

	for (uint8_t ix = 0, out = OUT_OUT1; out <= OUT_OUT6; ++out, ++ix) {
		memset(tempBuf, 0, sizeof(tempBuf));

		sprintf(tempBuf,  "out_254_%d=%d\n",
											ix, getOutputState(out));

		strcat(buf, tempBuf);
	}
}


void cgi_getInputState(char *buf, char **param, uint8_t len_param) {
	(void)param;
	(void)len_param;

	for (uint8_t ix = 0, in = IN_BOOT_JMP; in <= IN_TAMPER; ++in, ++ix) {
		memset(tempBuf, 0, sizeof(tempBuf));

		sprintf(tempBuf,  "in_254_%d=%d\n",
											ix, getInputState(in) ? 0 : 1);

		strcat(buf, tempBuf);
	}
}

void cgi_setOutputState (char *buf, char **param, uint8_t len_param) {
	(void)buf;

	char *tempstr;

	getSaveConfiguration(CONF_ID_OUTPUT_TACTICS, outputTactics);

	uint8_t ix_out = 0;
	uint8_t state_out = 0;
	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "out_254_") != NULL) {
			tempstr = strpbrk(param[i], "012345");
			tempstr = strpbrk(tempstr, "_");

			if  (tempstr) {
				ix_out = (uint8_t)strtol(tempstr + 1, &tempstr, 10);
	/*если получили некорректный номер шлейфа, то считаем ВСЕ параметры ошибочными*/
				if (ix_out >= NELEMS(outputTactics)) {
					sprintf(buf, "404 Not Found");
					return;
				}

				if (outputTactics[ix_out].id_program != ID_P_OFF) continue;

				state_out = (uint8_t)strtol(tempstr + 1, NULL, 10);

				setOutputState(ix_out, state_out ? IO_SET : IO_RESET);
			}
		}
	}
	sprintf(buf, "OK!");
}

void getLoopValue (char *buf, char **param, uint8_t len_param) {
	(void)param;
	(void)len_param;

	uint32_t tmp[NELEMS(alarmLoop)];
#ifndef HTTP_SERVER_DBG
	getSaveConfiguration(CONF_ID_ALARM_LOOP, alarmLoop);
	adc_getResistense(tmp);
#endif
	for (uint8_t ix = 0; ix != NELEMS(tmp); ++ix) {
		memset(tempBuf, 0, sizeof(tempBuf));
		if (alarmLoop[ix].mode == MODE_OFF) sprintf(tempBuf,  "loop%d=%s\n", ix, "off");
		else sprintf(tempBuf,  "loop%u=%lu\n", ix, tmp[ix]);

		strcat(buf, tempBuf);
	}

}
/*
отправляет состояние автомата ШС на Б-419
*/
void getLoopState (char *buf, char **param, uint8_t len_param) {
	(void)param;
	(void)len_param;

#ifndef HTTP_SERVER_DBG
	getConfiguration(CONF_ID_ALARM_LOOP_STATE, alarmLoopState);
	getConfiguration(CONF_ID_ALARM_SENS_STATE, alarmSensState);
#endif

	for (uint8_t ix = 0; ix != NELEMS(alarmLoopState); ++ix) {
		memset(tempBuf, 0, sizeof(tempBuf));
		sprintf(tempBuf,  "stateguard_254_%d=%d\n",
											ix, ((alarmLoopState[ix].stateLoop & 0xf0) >> 4));

		strcat(buf, tempBuf);
	}

	for (uint8_t ix = 0; ix != NELEMS(alarmSensState); ++ix) {
		if (alarmSensState[ix].addr == 0) continue;

		memset(tempBuf, 0, sizeof(tempBuf));
		/*TODO: для каждого датчика сделать разбор состояний по элементам*/
		sprintf(tempBuf,  "stateguard_%u=%d\n",
											alarmSensState[ix].addr, ((alarmSensState[ix].stateLoop & 0xf0) >> 4));

		strcat(buf, tempBuf);
	}
}

void goToBoot (char *buf, char **param, uint8_t len_param) {
	(void)param;
	(void)len_param;

	boot_mode.isBootFirm = BOOT_MODE_BOOT;
	setConfiguration(CONF_ID_BOOT_MODE, &boot_mode);

	sprintf(buf, "OK!");
}

#ifndef HTTP_SERVER_DBG
void resetWebAuthTimer (xTimerHandle xTimer) {
	unsigned portBASE_TYPE *pxTimerID;
	pxTimerID = pvTimerGetTimerID(xTimer);

	uint8_t ix_auth;
	for (ix_auth = 0; ix_auth != NELEMS(authStack); ++ix_auth) {
		if (authStack[ix_auth].id == *pxTimerID) break;
	}

	if (ix_auth >= NELEMS(authStack)) return;

	authStack[ix_auth].addr = 0xffffffff;
	authStack[ix_auth].port = 0xffff;

	struct authStack_t tmp;
	for (uint8_t ix = 0; ix != NELEMS(authStack); ++ix) {
		tmp = authStack[ix];
		uint8_t j = ix;
		while (j > 0 && authStack[j-1].addr > tmp.addr) {
				authStack[j] = authStack[j-1];
				--j;
		}
		authStack[j] = tmp;
	}

	isAuthorization = 0;
}


/*
 в общем виде отправляемое сообщение выглядит так:

0x02{
	"seq":0xffff,
	"di":[
		{"ad":0xff,"as":0xff,"st":0xff,"md":0xff},
		...
		{"ad":0xff,"as":0xff,"st":0xff,"md":0xff}
	],
	"do":[
		{"ad":0xff,"as":0xff,"st":0xff},
		...
		{"ad":0xff,"as":0xff,"st":0xff}
	]
}0x03

*/
static char dio_buf[1500];
static char di_buf[1500];
static char do_buf[1500];

static void getDIOstate (char *buf, char **param, uint8_t len_param) {
	uint32_t seqn = 0;
	char *tempstr;

	uint8_t ad_device = 0xff;
	uint8_t as_device = 0xff;
	char type_device[5] = "in";

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "seq=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			seqn = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "ad=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			ad_device = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "as=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			as_device = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "tp=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			strncpy(type_device, tempstr + 1, sizeof(type_device));
		}
	}

	memset(di_buf, 0, sizeof(di_buf));
	memset(do_buf, 0, sizeof(do_buf));

	if (as_device == 0xff && ad_device == 0xff) {
		for (device_t *cur_dev = dev; cur_dev != NULL; cur_dev = cur_dev->next) {
			for (sensors_t *cur_sens = cur_dev->ps; cur_sens != NULL; cur_sens = cur_sens->next) {
				memset(dio_buf, 0, sizeof(dio_buf));

				if (cur_sens->tp == SENS_TP_IN) {
					snprintf(dio_buf, sizeof(dio_buf), "{\"ad\":%u,\"as\":%u,\"st\":%u,\"md\":%u},",
													 cur_dev->ad, cur_sens->as, cur_sens->state.state & 0x0f, cur_sens->state.mode);
					strncat(di_buf, dio_buf, (sizeof(di_buf) - strlen(di_buf)) - 1);
				}
				else if (cur_sens->tp == SENS_TP_OUT) {
					snprintf(dio_buf, sizeof(dio_buf), "{\"ad\":%u,\"as\":%u,\"st\":%u},",
													 cur_dev->ad, cur_sens->as, cur_sens->state.state & 0x0f);
					strncat(do_buf, dio_buf, (sizeof(do_buf) - strlen(do_buf)) - 1);
				}
			}
		}
	}
	else if (as_device == 0xff && ad_device != 0x00 && ad_device != 0xff) {
		device_t *cur_dev = getDeviceByAddr(ad_device);

		if (cur_dev == NULL) {
			snprintf(buf, 1500, "%c{\"seq\":%lu, \"res\":\"FAIL\"}%c", STX, seqn, ETX);
			return;
		}

		for (sensors_t *cur_sens = cur_dev->ps; cur_sens != NULL; cur_sens = cur_sens->next) {
			memset(dio_buf, 0, sizeof(dio_buf));

			if (cur_sens->tp == SENS_TP_IN) {
				snprintf(dio_buf, sizeof(dio_buf), "{\"ad\":%u,\"as\":%u,\"st\":%u,\"md\":%u},",
												 cur_dev->ad, cur_sens->as, cur_sens->state.state & 0x0f, cur_sens->state.mode);
				strncat(di_buf, dio_buf, (sizeof(di_buf) - strlen(di_buf)) - 1);
			}
			else if (cur_sens->tp == SENS_TP_OUT) {
				snprintf(dio_buf, sizeof(dio_buf), "{\"ad\":%u,\"as\":%u,\"st\":%u},",
												 cur_dev->ad, cur_sens->as, cur_sens->state.state & 0x0f);
				strncat(do_buf, dio_buf, (sizeof(do_buf) - strlen(do_buf)) - 1);
			}
		}
	}
	else if (as_device != 0xff && ad_device != 0x00 && ad_device != 0xff) {
		device_t *cur_dev = getDeviceByAddr(ad_device);

		if (cur_dev == NULL) {
			snprintf(buf, 1500, "%c{\"seq\":%lu, \"res\":\"FAIL\"}%c", STX, seqn, ETX);
			return;
		}

		uint8_t tp_dev = strncmp(sens_io_type[SENS_TP_IN], type_device, strlen(sens_io_type[SENS_TP_IN])) == 0 ? SENS_TP_IN : SENS_TP_OUT;

		sensors_t *cur_sens = getSensorByAddr(as_device, tp_dev, cur_dev);

		memset(dio_buf, 0, sizeof(dio_buf));

		if (cur_sens->tp == SENS_TP_IN) {
			snprintf(dio_buf, sizeof(dio_buf), "{\"ad\":%u,\"as\":%u,\"st\":%u,\"md\":%u},",
											 cur_dev->ad, cur_sens->as, cur_sens->state.state & 0x0f, cur_sens->state.mode);
			strncat(di_buf, dio_buf, (sizeof(di_buf) - strlen(di_buf)) - 1);
		}
		else if (cur_sens->tp == SENS_TP_OUT) {
			snprintf(dio_buf, sizeof(dio_buf), "{\"ad\":%u,\"as\":%u,\"st\":%u},",
											 cur_dev->ad, cur_sens->as, cur_sens->state.state & 0x0f);
			strncat(do_buf, dio_buf, (sizeof(do_buf) - strlen(do_buf)) - 1);
		}
	}

	if (strlen(di_buf) && strlen(do_buf)) {
		di_buf[strlen(di_buf) - 1] = '\0';
		do_buf[strlen(do_buf) - 1] = '\0';
		snprintf(buf, 1500, "%c{\"seq\":%lu,\"di\":[%s],\"do\":[%s]}%c", STX, seqn, di_buf, do_buf, ETX);
	}
	else if (strlen(di_buf)) {
		di_buf[strlen(di_buf) - 1] = '\0';
		snprintf(buf, 1500, "%c{\"seq\":%lu,\"di\":[%s]}%c", STX, seqn, di_buf, ETX);
	}
	else if (strlen(do_buf)) {
		do_buf[strlen(do_buf) - 1] = '\0';
		snprintf(buf, 1500, "%c{\"seq\":%lu,\"do\":[%s]}%c", STX, seqn, do_buf, ETX);
	}
	else {
		snprintf(buf, 1500, "%c{\"seq\":%lu, \"res\":\"FAIL\"}%c", STX, seqn, ETX);
	}
}

static void setDiState (char *buf, char **param, uint8_t len_param) {
	uint32_t seqn = 0;
	char *tempstr;

	uint8_t ad_device = 0xff;
	uint8_t as_device = 0xff;
	uint8_t md_device = 0xff;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "seq=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			seqn = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "ad=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			ad_device = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "as=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			as_device = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "md=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			md_device = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	memset(alarmLoopState, 0, sizeof(alarmLoopState));
	getConfiguration(CONF_ID_ALARM_LOOP_STATE, alarmLoopState);

	memset(alarmSensState, 0, sizeof(alarmSensState));
	getConfiguration(CONF_ID_ALARM_SENS_STATE, alarmSensState);

	char result[] = "FAIL";
	if (ad_device != 0xff && as_device != 0xff && md_device != 0xff) {
		device_t *cur_dev = getDeviceByAddr(ad_device);
		sensors_t *cur_sens = NULL;

		if (cur_dev == NULL) {
			snprintf(buf, 1500, "%c{\"seq\":%lu, \"res\":\"FAIL\"}%c", STX, seqn, ETX);
			return;
		}

		cur_sens = getSensorByAddr(as_device, SENS_TP_IN, cur_dev);

		if (cur_sens != NULL && cur_dev->name == SENS_NM_B419) {
			alarmLoopState[cur_sens->as].isGuard = md_device == 1 ? 1 : 0;

			if (alarmLoopState[cur_sens->as].isGuard)
				setEvent(LSM_EVENT_GUARD_ON, cur_sens->as);
			else
				setEvent(LSM_EVENT_GUARD_OFF, cur_sens->as);

			if (setConfiguration(CONF_ID_ALARM_LOOP_STATE, alarmLoopState) == 0)
				strncpy(result, "OK", sizeof(result));
		}
		else if (cur_sens != NULL) {
			for (uint8_t ix = 0; ix != NELEMS(alarmSensState); ++ix) {
				if (alarmSensState[ix].addr != ad_device) continue;

				if (md_device == 1) {
					alarmSensState[ix].isGuard |= 1 << as_device;
				}
				else if (md_device == 2) {
					alarmSensState[ix].isGuard &= ~(1 << as_device);
				}

				if (alarmSensState[ix].isGuard & (1 << as_device))
					setEventSensors(LSM_EVENT_SENS_GUARD_ON, ix);
				else {
					if (cur_dev->name == SENS_NM_STS107) {
						if (!(alarmSensState[ix].isGuard & 0x03))
							setEventSensors(LSM_EVENT_SENS_GUARD_OFF, ix);
					}
				}
			}

			if (setConfiguration(CONF_ID_ALARM_SENS_STATE, alarmSensState) == 0)
				strncpy(result, "OK", sizeof(result));
		}
	}

	snprintf(buf, 1500, "%c{\"seq\":%lu, \"res\":\"%s\"}%c", STX, seqn, result, ETX);
}

static void setDoState (char *buf, char **param, uint8_t len_param) {
	uint32_t seqn = 0;
	char *tempstr;

	uint8_t ad_device = 0xff;
	uint8_t as_device = 0xff;
	uint8_t st_device = 0xff;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "seq=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			seqn = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "ad=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			ad_device = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "as=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			as_device = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "st=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			st_device = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	char result[] = "FAIL";
	if (ad_device != 0xff && as_device != 0xff && st_device != 0xff) {
		device_t *cur_dev = getDeviceByAddr(ad_device);
		sensors_t *cur_sens = NULL;
		if (cur_dev == NULL) {
			snprintf(buf, 1500, "%c{\"seq\":%lu, \"res\":\"%s\"}%c", STX, seqn, result, ETX);
			return;
		}

		cur_sens = getSensorByAddr(as_device, SENS_TP_OUT, cur_dev);
		if (cur_dev->name == SENS_NM_B419) {
			getSaveConfiguration(CONF_ID_OUTPUT_TACTICS, outputTactics);
			if (outputTactics[cur_sens->as].id_program == ID_P_OFF) {

				memset(alarmLoopState, 0, sizeof(alarmLoopState));
				getConfiguration(CONF_ID_ALARM_LOOP_STATE, alarmLoopState);

				setOutputState(cur_sens->as, st_device == 2 ? IO_SET : IO_RESET);

				strncpy(result, "OK", sizeof(result));
			}
		}
	}
	snprintf(buf, 1500, "%c{\"seq\":%lu, \"res\":\"%s\"}%c", STX, seqn, result, ETX);
}

static void getIoList (char *buf, char **param, uint8_t len_param) {
	uint32_t seqn = 0;
	char *tempstr;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "seq=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			seqn = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	memset(di_buf, 0, sizeof(di_buf));
	memset(do_buf, 0, sizeof(do_buf));

	for (device_t *cur_dev = dev; cur_dev != NULL; cur_dev = cur_dev->next) {
		memset(dio_buf, 0, sizeof(dio_buf));
		char name_dev[10] = "";

		if (cur_dev->name == SENS_NM_B419)
			memmove(name_dev, sens_name_type[0], sizeof(name_dev));
		else
			memmove(name_dev, sens_name_type[cur_dev->name], sizeof(name_dev));

		for (sensors_t *cur_sens = cur_dev->ps; cur_sens != NULL; cur_sens = cur_sens->next) {
			snprintf(dio_buf, sizeof(dio_buf), "{\"ad\":%u,\"as\":%u,\"n\":\"%s.%u.%u\"},",
							 cur_dev->ad, cur_sens->as, name_dev, cur_dev->ad, cur_sens->as);
			if (cur_sens->tp == SENS_TP_IN) {
				strncat(di_buf, dio_buf, (sizeof(di_buf) - strlen(di_buf)) - 1);
			}
			else if (cur_sens->tp == SENS_TP_OUT) {
				strncat(do_buf, dio_buf, (sizeof(do_buf) - strlen(do_buf)) - 1);
			}
		}
	}

	if (strlen(di_buf) && strlen(do_buf)) {
		di_buf[strlen(di_buf) - 1] = '\0';
		do_buf[strlen(do_buf) - 1] = '\0';
		snprintf(buf, 1500, "%c{\"seq\":%lu,\"di\":[%s],\"do\":[%s]}%c", STX, seqn, di_buf, do_buf, ETX);
	}
	else if (strlen(di_buf)) {
		di_buf[strlen(di_buf) - 1] = '\0';
		snprintf(buf, 1500, "%c{\"seq\":%lu,\"di\":[%s]}%c", STX, seqn, di_buf, ETX);
	}
	else if (strlen(do_buf)) {
		do_buf[strlen(do_buf) - 1] = '\0';
		snprintf(buf, 1500, "%c{\"seq\":%lu,\"do\":[%s]}%c", STX, seqn, do_buf, ETX);
	}
	else {
		snprintf(buf, 1500, "%c{\"seq\":%lu, \"res\":\"FAIL\"}%c", STX, seqn, ETX);
	}
}

static void getLastEvNumber (char *buf, char **param, uint8_t len_param) {
	uint32_t seqn = 0;
	char *tempstr;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "seq=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			seqn = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	snprintf(buf, 1500, "%c{\"seq\":%lu, \"ln\":%lu}%c", STX, seqn, getLastNE(), ETX);
}

static void getLastCcNumber (char *buf, char **param, uint8_t len_param) {
	uint32_t seqn = 0;
	char *tempstr;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "seq=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			seqn = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	snprintf(buf, 1500, "%c{\"seq\":%lu, \"ln\":%lu}%c", STX, seqn, getLastNCC (), ETX);
}

static void cgi_getEvent(char *buf, char **param, uint8_t len_param) {
	uint32_t seqn = 0;
	uint32_t nev = 0;
	char *tempstr;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "seq=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			seqn = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "nev=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			nev = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	if (nev == 0) {
		snprintf(buf, 1500, "%c{\"seq\":%lu, \"res\":\"FAIL\"}%c", STX, seqn, ETX);
	}
	else {
		memset(dio_buf, 0, sizeof(dio_buf));
		memset(do_buf, 0, sizeof(do_buf));
		struct event_t ev;
		uint32_t lastEv = getLastNE();
		uint8_t max_ev = 10;
		while (nev <= lastEv && max_ev && ( (nev = getEvent(&ev, nev)) > 0) ) {
			snprintf(dio_buf, sizeof(dio_buf), "{\"nb\":%lu,"
																				 "\"tm\":\"20%u.%u.%u-%u:%u:%u\","
																				 "\"ad\":%u,"
																				 "\"as\":%u, "
																				 "\"tp\":\"%s\","
																				 "\"st\":%u,"
																				 "\"md\":%u},",
							 ev.eventNumber,
							 ev.yy, ev.MM, ev.dd, ev.hh, ev.mm, ev.ss,
							 ev.data.addr_device,
							 ev.data.addr_sens,
							 sens_io_type[ev.data.type],
							 ev.data.state,
							 ev.data.mode);

			strncat(do_buf, dio_buf, (sizeof(do_buf) - strlen(do_buf)) - 1);
			--max_ev;
		}

		if (strlen(do_buf)) {
			getSaveConfiguration(CONF_ID_SESSION, &session);

			RTC_DateTypeDef   RTC_DateStructure;
			RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
			RTC_TimeTypeDef   RTC_TimeStructure;
			RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

			do_buf[strlen(do_buf) - 1] = '\0';
			snprintf(buf, 1500, "%c{\"seq\":%lu,"
													"\"ct\":\"20%u.%u.%u-%u:%u:%u\","
													"\"si\":%lu,"
													"\"ev\":[%s]}%c", STX, seqn,
													RTC_DateStructure.RTC_Year, RTC_DateStructure.RTC_Month, RTC_DateStructure.RTC_Date,
													RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds,
													session.session,
													do_buf, ETX);
		}
		else {
			snprintf(buf, 1500, "%c{\"seq\":%lu, \"res\":\"FAIL\"}%c", STX, seqn, ETX);
		}
	}
}

static void cgi_getReadersCC(char *buf, char **param, uint8_t len_param) {
	uint32_t seqn = 0;
	uint32_t nev = 0;
	char *tempstr;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "seq=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			seqn = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "nev=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			nev = (uint32_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	if (nev == 0) {
		snprintf(buf, 1500, "%c{\"seq\":%lu, \"res\":\"FAIL\"}%c", STX, seqn, ETX);
	}
	else {
		memset(dio_buf, 0, sizeof(dio_buf));
		memset(do_buf, 0, sizeof(do_buf));
		struct codeCard_t cc;
		uint32_t lastEv = getLastNCC ();
		uint8_t max_ev = 10;
		while (nev <= lastEv && max_ev && ( (nev = getCodeCard(&cc, nev)) > 0) ) {
			uint64_t tmp_cc = ((uint64_t)cc.d[0] << (8*7)) |
												(uint64_t)cc.d[1] << (8*6) |
												(uint64_t)cc.d[2] << (8*5) |
												(uint64_t)cc.d[3] << (8*4) |
												(uint64_t)cc.d[4] << (8*3) |
												(uint64_t)cc.d[5] << (8*2) |
												(uint64_t)cc.d[6] << (8*1) |
												(uint64_t)cc.d[7] << (8*0);
			snprintf(dio_buf, sizeof(dio_buf), "{\"nb\":%lu,"
																				 "\"tm\":\"20%u.%u.%u-%u:%u:%u\","
																				 "\"ad\":%u,"
																				 "\"cc\":%"PRIu64"},",
							 cc.eventNumber,
							 cc.yy, cc.MM, cc.dd, cc.hh, cc.mm, cc.ss,
							 cc.addr,
							 tmp_cc);

			strncat(do_buf, dio_buf, (sizeof(do_buf) - strlen(do_buf)) - 1);
			--max_ev;
		}

		if (strlen(do_buf)) {
			getSaveConfiguration(CONF_ID_SESSION, &session);

			RTC_DateTypeDef   RTC_DateStructure;
			RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
			RTC_TimeTypeDef   RTC_TimeStructure;
			RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);

			do_buf[strlen(do_buf) - 1] = '\0';
			snprintf(buf, 1500, "%c{\"seq\":%lu,"
													"\"ct\":\"20%u.%u.%u-%u:%u:%u\","
													"\"si\":%lu,"
													"\"ev\":[%s]}%c", STX, seqn,
													RTC_DateStructure.RTC_Year, RTC_DateStructure.RTC_Month, RTC_DateStructure.RTC_Date,
													RTC_TimeStructure.RTC_Hours, RTC_TimeStructure.RTC_Minutes, RTC_TimeStructure.RTC_Seconds,
													session.session,
													do_buf, ETX);
		}
		else {
			snprintf(buf, 1500, "%c{\"seq\":%lu, \"res\":\"FAIL\"}%c", STX, seqn, ETX);
		}
	}
}

static void cgi_searchDev (char *buf, char **param, uint8_t len_param) {
	uint8_t addr = 0;
	char *tempstr;

	getConfiguration(CONF_ID_CONVERTER, &converter);

	if (converter.mode != 2) {
		snprintf(buf, 1500, "NOPSUP");
		return;
	}

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "addr=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			addr = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	sensQuery_t sensQuery;
	sensQuery.cmd = 's';
	sensQuery.addr = addr;
	xQueueSend(sensSendHandle, &sensQuery, 0);



	if (xQueueReceive(searchHandle, &sensQuery, portMAX_DELAY) == pdPASS) {
		if (sensQuery.addr == 0) snprintf(buf, 1500, "FAIL");
		else snprintf(buf, 1500, "addr=%u&type=%u\n", sensQuery.addr, sensQuery.name);
	}
	else {
		snprintf(buf, 1500, "FAIL");
	}
}

static void cgi_saveSearchSens (char *buf, char **param, uint8_t len_param) {
	alarmSens_t tmp;
	char *tempstr;
	tmp.addr = 0;

	for (uint8_t i = 0; i != len_param; ++i) {
		tempstr = strpbrk(param[i], "=");
		uint32_t part = (uint32_t)strtol(tempstr + 1, NULL, 10);

		if (strstr(param[i], "addr=") != NULL) {
			tmp.addr = (uint8_t)part;
		}
		else if (strstr(param[i], "type=") != NULL) {
			tmp.type = (uint8_t)part;
		}
		else if (strstr(param[i], "name=") != NULL) {
			tmp.name = (uint8_t)part;
		}
		else if (strstr(param[i], "mode=") != NULL) {
			tmp.mode = part;
		}
		else if (strstr(param[i], "check_param=") != NULL) {
			tmp.check_param = part;
		}
		else if (strstr(param[i], "time_delay_guard=") != NULL) {
			tmp.time_delay_guard = part;
		}
		else if (strstr(param[i], "delay_alarm=") != NULL) {
			tmp.delay_alarm = part;
		}
		else if (strstr(param[i], "delay_recovery_alarm=") != NULL) {
			tmp.delay_recovery_alarm = part;
		}
	}

	uint8_t result = 1;
	if (tmp.addr > 0) {
		getSaveConfiguration(CONF_ID_ALARM_SENS, alarmSens);
		getSaveConfiguration(CONF_ID_ALARM_SENS_STATE, alarmSensState);

		uint8_t ix;
		for (ix = 0; ix != NELEMS(alarmSens); ++ix) {
			if (alarmSens[ix].addr == tmp.addr) {
				snprintf(buf, 1500, "BE");
				return;
			}

			if (alarmSens[ix].addr != 0) continue;

			alarmSens[ix] = tmp;
			alarmSensState[ix].addr = tmp.addr;
			alarmSensState[ix].type = tmp.type;
			break;
		}

		result = setConfiguration(CONF_ID_ALARM_SENS, alarmSens);
		result = setConfiguration(CONF_ID_ALARM_SENS_STATE, alarmSensState);

		if (ix >= NELEMS(alarmSens)) result = 0xf1;
	}

	memset(&session, 0, sizeof(session));
	getSaveConfiguration(CONF_ID_SESSION, &session);
	session.versionCfgDev++;
	setConfiguration(CONF_ID_SESSION, &session);

	if (result == 0)
		snprintf(buf, 1500, "OK");
	else if (result == 0xf1)
		snprintf(buf, 1500, "FULL");
	else
		snprintf(buf, 1500, "FAIL");
}

static void cgi_getCurrentDevice (char *buf, char **param, uint8_t len_param) {
	(void)param;
	(void)len_param;

	getSaveConfiguration(CONF_ID_ALARM_SENS, alarmSens);

	memset(dio_buf, 0, sizeof(dio_buf));
	snprintf(buf, 1500, "");
	uint8_t ix;
	for (ix = 0; ix != NELEMS(alarmSens); ++ix) {
		if (alarmSens[ix].addr == 0) continue;

		snprintf(do_buf, sizeof(do_buf), "addr=%u&name=%u&mode=%lu&check_param=%lu&time_delay_guard=%lu&delay_alarm=%lu&delay_recovery_alarm=%lu\n",
						 alarmSens[ix].addr, alarmSens[ix].name, alarmSens[ix].mode, alarmSens[ix].check_param,
						 alarmSens[ix].time_delay_guard, alarmSens[ix].delay_alarm, alarmSens[ix].delay_recovery_alarm);
		strncat(buf, do_buf, 1500);
	}

	if (!strlen(buf)) {
		snprintf(buf, 1500, "EMPTY");
	}
}

static void cgi_saveSearchSensors (char *buf, char **param, uint8_t len_param) {
	alarmSens_t tmp[28];
	char *tempstr;

	uint8_t i_a = 0;

	memset(tmp, 0, NELEMS(tmp) * sizeof(alarmSens_t));
	for (uint8_t i = 0; i != len_param && i_a < NELEMS(tmp); ++i) {
		tempstr = strpbrk(param[i], "=");
		uint32_t part = (uint32_t)strtol(tempstr + 1, NULL, 10);

		if (strstr(param[i], "addr=") != NULL) {
			tmp[i_a].addr = (uint8_t)part;
		}
		else if (strstr(param[i], "name=") != NULL) {
			tmp[i_a].name = (uint8_t)part;
		}
		else if (strstr(param[i], "type=") != NULL) {
			tmp[i_a].type = (uint8_t)part;
		}
		else if (strstr(param[i], "mode=") != NULL) {
			tmp[i_a].mode = part;
		}
		else if (strstr(param[i], "check_param=") != NULL) {
			tmp[i_a].check_param = part;
		}
		else if (strstr(param[i], "time_delay_guard=") != NULL) {
			tmp[i_a].time_delay_guard = part;
		}
		else if (strstr(param[i], "delay_alarm=") != NULL) {
			tmp[i_a].delay_alarm = part;
		}
		else if (strstr(param[i], "delay_recovery_alarm=") != NULL) {
			tmp[i_a++].delay_recovery_alarm = part;
		}
	}

	if (i_a == 28) {
		snprintf(buf, 1500, "SENSORS MORE");
		return;
	}

	memset(alarmSens, 0, sizeof(alarmSens));
	memset(alarmSensState, 0, sizeof(alarmSensState));

	for (uint8_t ix = 0; ix != i_a; ++ix) {
		alarmSens[ix] = tmp[ix];
		alarmSensState[ix].addr = tmp[ix].addr;
		alarmSensState[ix].type = tmp[ix].type;
	}

	memset(&session, 0, sizeof(session));
	getSaveConfiguration(CONF_ID_SESSION, &session);
	session.versionCfgDev++;
	setConfiguration(CONF_ID_SESSION, &session);

	if (setConfiguration(CONF_ID_ALARM_SENS, alarmSens) == 0)
		snprintf(buf, 1500, "OK");
	else
		snprintf(buf, 1500, "FAIL");

	if (setConfiguration(CONF_ID_ALARM_SENS_STATE, alarmSensState) == 0)
		snprintf(buf, 1500, "OK");
	else
		snprintf(buf, 1500, "FAIL");
}

static void getCurReader (char *buf, char **param, uint8_t len_param) {
	(void)param;
	(void)len_param;

	struct reader_param_t const *const rp = getReaderParam();

	memset(dio_buf, 0, sizeof(dio_buf));
	snprintf(buf, 1500, "");

	for (uint8_t ix = 0; ix != NELEMS(readers); ++ix) {
		if (rp[ix].reader->addr == 0) continue;

		snprintf(do_buf, sizeof(do_buf) - 1, "addr=%u&cnt_bytes=%u&speed=%u&time_storage=%u\n",
																		rp[ix].reader->addr, rp[ix].reader->cnt_bytes, rp[ix].speed, rp[ix].time_storage);
		strncat(buf, do_buf, 1500);
	}

	if (!strlen(buf)) {
		snprintf(buf, 1500, "EMPTY");
	}
}

static void saveReaders (char *buf, char **param, uint8_t len_param) {
	reader_t tmp[NELEMS (readers)];
	uint8_t readers_cnt = 0;
	memset(&tmp, 0, sizeof(tmp));
	char *tempstr;

	bool is_clear_readers = false;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "clear")) {
			is_clear_readers = true;
		}
		if (strstr(param[i], "addr=")) {
			tempstr = strpbrk(param[i], "=");
			if (tempstr) tmp[readers_cnt].addr = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "cnt_bytes=")) {
			tempstr = strpbrk(param[i], "=");
			if (tempstr) tmp[readers_cnt].cnt_bytes = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "end")) {
			readers_cnt++;
		}
	}

	uint8_t result = 1;

	/*если считывателей нет или больше, чем можем записать, то посылаем FAIL и выходим*/
	if ((!readers_cnt && !is_clear_readers) || readers_cnt >= sizeof(readers)) {
		snprintf(buf, 1500, "FAIL");
		return;
	}

	if (is_clear_readers) {
		memset (readers, 0, sizeof(readers));
	}
	else {
		getSaveConfiguration(CONF_ID_READERS, &readers);
		readers_cnt = NELEMS (readers);
	}

	uint8_t ix;
	for (ix = 0; ix != readers_cnt; ++ix) {
		if (is_clear_readers) {
			readers[ix] = tmp[ix];
			continue;
		}

		if (readers[ix].addr == tmp[0].addr) {
			snprintf(buf, 1500, "BE");
			return;
		}

		if (readers[ix].addr != 0) continue;

		readers[ix] = tmp[0];
		break;
	}

	result = setConfiguration(CONF_ID_READERS, readers);

	if (!is_clear_readers && ix >= NELEMS(readers)) result = 0xf1;

	memset(&session, 0, sizeof(session));
	getSaveConfiguration(CONF_ID_SESSION, &session);
	session.versionCfgDev++;
	setConfiguration(CONF_ID_SESSION, &session);

	if (result == 0)
		snprintf(buf, 1500, "OK");
	else if (result == 0xf1)
		snprintf(buf, 1500, "FULL");
}

static void searchReaders (char *buf, char **param, uint8_t len_param) {
/******************************************************************************/
	getConfiguration(CONF_ID_CONVERTER, &converter);

	if (converter.mode != 2) {
		snprintf(buf, 1500, "NOPSUP");
		return;
	}
/******************************************************************************/
	uint8_t addr = 0;
	char *tempstr;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "addr=") != NULL) {
			tempstr = strpbrk(param[i], "=");
			addr = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	sensQuery_t sensQuery;
	sensQuery.cmd = 's';
	sensQuery.addr = addr;
	xQueueSend(readerSendHandle, &sensQuery, 0);

	if (xQueueReceive(searchHandleReader, &sensQuery, portMAX_DELAY) == pdFAIL) {
		snprintf(buf, 1500, "FAIL");
		return;
	}

	if (sensQuery.addr == 0) snprintf(buf, 1500, "FAIL");
	else snprintf(buf, 1500, "addr=%u\n", sensQuery.addr);
}

static void restartReader (char *buf, char **param, uint8_t len_param) {
	char *tempstr;
	uint8_t addr = 0;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "addr=")) {
			tempstr = strpbrk(param[i], "=");
			if (tempstr) addr = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	if (addr) {
		sensQuery_t sensQuery;
		sensQuery.cmd = 'r';
		sensQuery.addr = addr;
		sensQuery.data[0] = 2;
		xQueueSend(readerSendHandle, &sensQuery, 0);

		if (xQueueReceive(restartReaderHndl, &sensQuery, portMAX_DELAY) == pdFAIL) {
			snprintf(buf, 1500, "FAIL");
			return;
		}

		if (sensQuery.addr == 0) snprintf(buf, 1500, "FAIL");
		else snprintf(buf, 1500, "addr=%u\n", sensQuery.addr);
		return;
	}

	snprintf(buf, 1500, "OK");
}

static void setParamReader (char *buf, char **param, uint8_t len_param) {
	char *tempstr;
	uint8_t addr = 0;
	uint8_t new_addr = 0;
	uint8_t speed = 0;
	uint8_t time_storage = 0;
	uint8_t is_def_addr = 0;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "def_addr")) {
			/*если сбрасываем адрес, то остальные параметры опускаем*/
			is_def_addr = 1;
			break;
		}
		else if (strstr(param[i], "new_addr=")) {
			tempstr = strpbrk(param[i], "=");
			if (tempstr) new_addr = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "addr=")) {
			tempstr = strpbrk(param[i], "=");
			if (tempstr) addr = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "speed=")) {
			tempstr = strpbrk(param[i], "=");
			if (tempstr) speed = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
		else if (strstr(param[i], "time_storage=")) {
			tempstr = strpbrk(param[i], "=");
			if (tempstr) time_storage = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	if (!addr) {
		snprintf(buf, 1500, "FAIL");
		return;
	}

	struct reader_param_t *const rp = getReaderParam();
	if (is_def_addr) {
		sensQuery_t sensQuery;
		sensQuery.cmd = 'r';
		sensQuery.addr = addr;
		sensQuery.data[0] = 1;
		xQueueSend(readerSendHandle, &sensQuery, 0);

		if (xQueueReceive(restartReaderHndl, &sensQuery, portMAX_DELAY) == pdFAIL) {
			snprintf(buf, 1500, "FAIL");
			return;
		}

		if (sensQuery.addr == 0) snprintf(buf, 1500, "FAIL");
		else {
			snprintf(buf, 1500, "addr=%u\n", sensQuery.addr);
			getSaveConfiguration(CONF_ID_READERS, &readers);

			for (uint8_t ix = 0; ix != NELEMS (readers); ++ix) {
				if (readers[ix].addr != addr) continue;

				readers[ix].addr = 1;
				rp->reader->addr = 1;
				break;
			}
			setConfiguration(CONF_ID_READERS, readers);
		}

		snprintf(buf, 1500, "DEF_OK");
		return;
	}

	if (new_addr) {
		getSaveConfiguration(CONF_ID_READERS, &readers);

		for (uint8_t ix = 0; ix != NELEMS (readers); ++ix) {
			if (readers[ix].addr != addr) continue;

			readers[ix].addr = new_addr;
			rp->reader->addr = new_addr;
			break;
		}
		setConfiguration(CONF_ID_READERS, readers);
	}

	if (speed) rp->speed = speed;
	if (time_storage) rp->time_storage = time_storage;

	sensQuery_t readerQuery;

	memset(&readerQuery, 0, sizeof(readerQuery));
	readerQuery.cmd = 'W';
	readerQuery.data[0] = 0x01; /*чтение*/
	readerQuery.data[1] = 0x01; /*параметры считывателя*/
	readerQuery.data[2] = time_storage;
	readerQuery.addr = addr;
	xQueueSend(readerSendHandle, &readerQuery, 0);

	xQueueReceive(writeReaderHndl, &readerQuery, portMAX_DELAY);

	uint16_t d1[16];
	memcpy (d1, readerQuery.data, sizeof(readerQuery.data));

	memset(&readerQuery, 0, sizeof(readerQuery));
	readerQuery.cmd = 'A';
	readerQuery.data[0] = 0x01; /*чтение*/
	readerQuery.data[1] = 0x00; /*параметры считывателя*/
	readerQuery.data[2] = rp->reader->addr & 0xff;
	readerQuery.data[3] = rp->reader->addr >> 8;
	readerQuery.data[4] = speed;
	readerQuery.addr = addr;
	xQueueSend(readerSendHandle, &readerQuery, 0);

	xQueueReceive(writeReaderHndl, &readerQuery, portMAX_DELAY);

	uint16_t d2[16];
	memcpy (d2, readerQuery.data, sizeof(readerQuery.data));

	char tmp_buf[32];

	for (uint8_t ix = 0; ix != sizeof(readerQuery.data); ++ix) {
		snprintf(tmp_buf, sizeof(tmp_buf), "%#x ", d1[ix]);
		strncat(buf, tmp_buf, strlen(tmp_buf));
	}

	for (uint8_t ix = 0; ix != sizeof(readerQuery.data); ++ix) {
		snprintf(tmp_buf, sizeof(tmp_buf), "%#x ", d2[ix]);
		strncat(buf, tmp_buf, strlen(tmp_buf));
	}
}

static void resetAdrReader (char *buf, char **param, uint8_t len_param) {
	snprintf(buf, 1500, "OK");
	if (!strlen(buf)) {
		snprintf(buf, 1500, "EMPTY");
	}
}

static void getLastCC (char *buf, char **param, uint8_t len_param) {
	char *tempstr;
	uint8_t addr = 0;

	for (uint8_t i = 0; i != len_param; ++i) {
		if (strstr(param[i], "addr=")) {
			tempstr = strpbrk(param[i], "=");
			if (tempstr) addr = (uint8_t)strtol(tempstr + 1, NULL, 10);
		}
	}

	if (addr) {
		uint8_t code[8] = {0,0,0,0,0,0,0,0};
		getLastCodeCard (code, addr);
		snprintf(buf, 1500, "%02X%02X%02X%02X%02X%02X%02X%02X", code[0], code[1], code[2], code[3], code[4], code[5], code[6], code[7]);
		return;
	}
	snprintf(buf, 1500, "FAIL");
}

#if configSUPPORT_STATIC_ALLOCATION && configUSE_TIMERS
static StaticTask_t xTimerTaskTCBBuffer;
static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

/* If static allocation is supported then the application must provide the
   following callback function - which enables the application to optionally
   provide the memory that will be used by the timer task as the task's stack
   and TCB. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer, StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
  *ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
  *ppxTimerTaskStackBuffer = &xTimerStack[0];
  *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
#endif
#endif

#endif
//******************************************************************************
//  ENF OF FILE
//******************************************************************************
