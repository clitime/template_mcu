#pragma once
#ifndef CGI_REQUEST_
#define CGI_REQUEST_
//******************************************************************************
//  Секция include: здесь подключаются заголовочные файлы используемых модулей
//******************************************************************************
#include <string.h>
#include "sockets.h"

//******************************************************************************
//  Секция определения констант
//******************************************************************************
#define MAP_CGI_FUNCTION(XX)                  \
	XX(GET_VERSION,                 "/getversion.cgi",                getVersion,              1)   \
	XX(GET_FIRMWARE,                "/getfirmware.cgi",               getFirmware,             1)   \
	XX(GET_IP_PARAM,                "/getipparam.cgi",                getIpParam,              0)   \
	XX(SET_IP_PARAM,                "/setipparam.cgi",                setIpParam,              0)   \
	XX(GET_HW_PARAM,                "/gethwparam.cgi",                getHwParam,              0)   \
	XX(SET_HW_PARAM,                "/sethwparam.cgi",                setHwParam,              0)   \
	XX(CHECK_AUTHORIZATION,         "/checkauthorizations.cgi",       checkAuthorization,      1)   \
	XX(CHECK_AUTHORIZATION_UDP,     "/checkauthorization.cgi",				checkAuthorization_udp,  1)   \
	XX(SET_AUTH,                    "/setlogin.cgi",                  setAuthorization,        0)   \
	XX(GET_AUTH,                    "/getlogin.cgi",                  getAuthorization,        0)   \
	XX(GET_IP_ADDR_FOR_REBOOT,      "/rstip.cgi",                     getIpAddr,               0)   \
	XX(REBOOT,                      "/reboot.cgi",                    rebootMCU,               0)   \
	XX(CHECK_HW,                    "/checkhw.cgi",                   checkHw,                 1)   \
	XX(GET_CONVERTER,               "/getconverter.cgi",              getConverter,            0)   \
	XX(SET_CONVERTER,               "/setconverter.cgi",              setConverter,            0)   \
	XX(GET_ALARM_LOOP,              "/getloopparam.cgi",              getAlarmLoop,            0)   \
	XX(SET_ALARM_LOOP,              "/setloopparam.cgi",              setAlarmLoop,            0)   \
	XX(GET_SECTIONS,                "/getsectionparam.cgi",           getSections,             0)   \
	XX(SET_SECTION,                 "/setsectionparam.cgi",           setSections,             0)		\
	XX(GET_OUTPUT,                  "/getoutputparam.cgi",            getOutput,               0)   \
	XX(SET_OUTPUT,                  "/setoutputparam.cgi",            setOutput,               0)   \
	XX(GET_CTRL_SENSORS,            "/getcontrolsensors.cgi",         getCtrlSensors,          0)   \
	XX(SET_CTRL_SENSORS,            "/setcontrolsensors.cgi",         setCtrlSensors,          0)   \
	XX(APPLY_PARAM,                 "/applyparam.cgi",                applyParam,              0)   \
	XX(GET_ALARM_LOOP_STATE,        "/getalarmloopstate.cgi",         getAlarmLoopState,       1)   \
	XX(GET_OUTPUT_STATE,            "/getoutputstate.cgi",            cgi_getOutputState,      1)   \
	XX(GET_INPUT_STATE,             "/getinputstate.cgi",             cgi_getInputState,       1)   \
	XX(SET_OUTPUT_STATE,            "/setoutputstate.cgi",            cgi_setOutputState,      0)   \
	XX(GET_LOOP_VALUE,              "/getloopvalue.cgi",              getLoopValue,            1)   \
	XX(GET_LOOP_STATE,              "/getloopstate.cgi",              getLoopState,            1)   \
	XX(GO_TO_BOOT,                  "/gotoboot.cgi",                  goToBoot,                0)   \
	XX(GET_VERPROTO,                "/getinfo.cgi",                   getInfo,                 1)   \
	XX(SEND_404,                    "/send404.cgi",                   send404,                 1)   \
	XX(GET_IO_STATE,                "/getdiostate.cgi",               getDIOstate,             0)   \
	XX(SET_DI_STATE,                "/setdistate.cgi",                setDiState,              0)   \
	XX(SET_DO_STATE,                "/setdostate.cgi",                setDoState,              0)   \
	XX(GET_IO_LIST,                 "/getiolist.cgi",                 getIoList,               0)   \
	XX(GET_LAST_NUMB,               "/getlastevnumber.cgi",           getLastEvNumber,         0)   \
	XX(GET_LAST_CC_NUMB,            "/getlastccnumber.cgi",           getLastCcNumber,         0)   \
	XX(GET_EVENT,                   "/getevent.cgi",                  cgi_getEvent,            0)   \
	XX(GET_CC,                      "/getreaderscards.cgi",           cgi_getReadersCC,        0)   \
	XX(SEARCH_DEV,                  "/searchdev.cgi",                 cgi_searchDev,           0)   \
	XX(SAVE_SEARCH_SENS,            "/savesearchsens.cgi",            cgi_saveSearchSens,      0)   \
	XX(GET_CUR_DEV,                 "/getcurdev.cgi",                 cgi_getCurrentDevice,    1)   \
	XX(SAVE_SEARCH_SENSORS,         "/savesearchsensors.cgi",         cgi_saveSearchSensors,   0)   \
	XX(GET_CUR_READER,              "/getcurreader.cgi",              getCurReader,            0)   \
	XX(SAVE_READERS,                "/savereaders.cgi",               saveReaders,             0)   \
	XX(SEARCH_READER,               "/searchreader.cgi",              searchReaders,           0)   \
	XX(RESTART_READER,              "/resetreader.cgi",               restartReader,           0)   \
	XX(RESET_ADR_READER,            "/resetadrreader.cgi",            resetAdrReader,          0)   \
	XX(GET_LAST_CODE_CARD,          "/getlastcodecard.cgi",           getLastCC,               0)   \
	XX(SET_PARAM_READER,            "/setparamreaders.cgi",           setParamReader,          0)   \

//******************************************************************************
//  Секция определения типов
//******************************************************************************
enum {
#define XX(name, request, function, allow)  CGI_##name,
  MAP_CGI_FUNCTION(XX)
#undef XX
  CGI_Q
};

//******************************************************************************
//  Секция определения глобальных переменных
//******************************************************************************



//******************************************************************************
//  Секция прототипов глобальных функций
//******************************************************************************

#ifdef BOOTLOADER
void cgiHandler (char *uri, char *outBuf, char type);
#else
uint8_t checkIpByAuthorization (struct sockaddr_in *from);
void cgiHandler (char *uri, char *outBuf, struct sockaddr_in *from);
#endif


//******************************************************************************
//  Секция определения макросов
//******************************************************************************




//******************************************************************************
//  ENF OF FILE
//******************************************************************************
#endif
