#pragma once
#ifndef _HTTP_PARSER_
#define _HTTP_PARSER_

typedef enum {
  M_GET,
  M_POST,
  M_NOT_SUP,
} method_t;

typedef enum {
  E_CGI,
  E_HTML,
  E_JPG,
  E_PNG,
  E_GIF,
  E_CSS,
  E_ICO,
  E_JS,
  E_TXT,
  E_NOT_FOUND,
  E_NOT_SUP,
} ext_t;

method_t getMethod (const char *m);
ext_t getDataExtensionRequest (char *url);

#endif
