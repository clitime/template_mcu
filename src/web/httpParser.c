#include "httpParser.h"

#include <stdint.h>
#include <string.h>

#define CR                  '\r'
#define LF                  '\n'
#define LOWER(c)            (unsigned char)(c | 0x20)
#define IS_ALPHA(c)         (LOWER(c) >= 'a' && LOWER(c) <= 'z')
#define IS_NUM(c)           ((c) >= '0' && (c) <= '9')
#define IS_ALPHANUM(c)      (IS_ALPHA(c) || IS_NUM(c))
#define IS_HEX(c)           (IS_NUM(c) || (LOWER(c) >= 'a' && LOWER(c) <= 'f'))

#ifdef BOOTLOADER
extern uint8_t isAuthorization;
#endif

/**
 * @brief getMethod анализирует строку на наличие метода
 * @param[in] *m входящее сообщение
 * @return возвращает метод запроса, если запрос не поддерживается, то M_NOT_SUP
 */
method_t getMethod (const char *m) {
  method_t retVal = M_NOT_SUP;
  if(strncmp(m, "GET", sizeof("GET")) == 0) {
    retVal = M_GET;
  }
  else if (strncmp(m, "POST", sizeof("POST")) == 0) {
    retVal = M_POST;
  }
  return retVal;
}

/**
 * @brief getExtType определение типа расширения
 * @param[in] e входящее расширение
 * @return тип расширения
 */
static ext_t getExtType (const char *e) {
  ext_t retVal = E_NOT_SUP;

  if(strncmp(e, "cgi", 3) == 0) {
    retVal = E_CGI;
  }
  else if((strncmp(e,"html",4) == 0) || (strncmp(e,"htm",3) == 0) || (strncmp(e,"shtml",5) == 0)) {
    retVal = E_HTML;
  }
  else if( (strncmp(e,"jpg",3) == 0)) {
    retVal = E_JPG;
  }
  else if( (strncmp(e,"png",3) == 0)) {
    retVal = E_PNG;
  }
  else if( (strncmp(e,"gif",3) == 0)) {
    retVal = E_GIF;
  }
  else if( (strncmp(e,"css",3) == 0)) {
    retVal = E_CSS;
  }
  else if( (strncmp(e,"ico",3) == 0)) {
    retVal = E_ICO;
  }
  else if( (strncmp(e,"js", 2) == 0)) {
    retVal = E_JS;
  }
  else if ( (strncmp(e, "txt", 3) == 0)) {
    retVal = E_TXT;
  }
  return retVal;
}

/**
 * @brief getDataExtensionRequest функция ищет расширение запроса
 * @param[in] *uri  входящая строка сообщения
 * @return возвращает расширение
 *
 * При получении сообщения, функция ищет сообщение запроса, например .html или .js
 */
ext_t getDataExtensionRequest (char *uri) {
  uint32_t index = 0;
  char ext[5];
  memset(ext, 0, sizeof(ext));

	size_t uri_len = strlen(uri);
  if (strrchr(uri,'.')) {
		uint32_t i;

		for (i = 0; i < uri_len; i++) {
      if(uri[i] == '.') {
        index = i + 1;
        break;
      }
    }
    i = index;
		while( (i < uri_len) && ( (i - index) < 5)) {
      ext[i - index] = uri[i];
      i++;
    }
  }

	if ( (uri_len == 1) && (strncmp(uri,"/",1) == 0)) {
    strcat(uri, "index.html\0");
    strcat(ext,"html\0");
  }

#ifdef BOOTLOADER
  if (strncmp(uri, "/index.html\0", sizeof("/index.html\0")) == 0) {
    isAuthorization = 0;
  }
#endif
  return getExtType (ext);
}
