#include <string.h>

#include "http_server.h"
#include "fs.h"
#include "httpParser.h"

#include "misc.h"

#include "FreeRTOS.h"
#include "task.h"

#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"

#include "lwip/inet.h"
#include "lwip/sockets.h"
#include "lwip/errno.h"

//******************************************************************************
//  Секция определения локальных макросов
//******************************************************************************
#define TCP_THREAD_PRIO  				( tskIDLE_PRIORITY + 2 )

struct http_state
{
  char *file;
  uint32_t left;
};


#define OUT_BUF_LEN     1500

#define MAP_HEADER(XX)  \
  XX(HTTP_HEADER_200,			"HTTP/1.1 200 OK\r\nServer: B-419\r\nConnection: close\r\n")                  \
	XX(HTTP_HEADER_400,			"HTTP/1.1 400 Bad Request\r\nServer: B-419\r\nConnection: close\r\n")           \
  XX(HTTP_HEADER_404,			"HTTP/1.1 404 Not Found\r\nServer: B-419\r\nConnection: close\r\n")           \
  XX(HTTP_HEADER_414,			"HTTP/1.1 414 Request-URI Too Long\r\nServer: B-419\r\nConnection: close\r\n")  \
  XX(HTTP_HEADER_415,			"HTTP/1.1 415 Unsupported Media Type\r\nServer: B-419\r\nConnection: close\r\n")   \
  XX(HTTP_HEADER_500,			"HTTP/1.1 500 Internal Server Error\r\nServer: B-419\r\nConnection: close\r\n")    \
	XX(HTTP_HEADER_501,			"HTTP/1.1 501 Not Implemented\r\nServer: B-419\r\nConnection: close\r\n")          \
  XX(CONTENT_UTF,					"Content-type: text/txt; charset=UTF-8\r\nContent-Length: " )                           \
  XX(CONTENT_JPG,					"Content-type: image/jpeg\r\nContent-Length: "            )                             \
  XX(CONTENT_PNG,					"Content-type: image/png\r\nContent-Length: ")                                          \
  XX(CONTENT_GIF	,				"Content-type: image/gif\r\nContent-Length: " )                                         \
  XX(CONTENT_ICO	,				"Content-type: image/icon\r\nContent-Length: " )                                        \
  XX(CONTENT_CSS	,				"Content-type: text/css\r\nContent-Length: "  )                                         \
  XX(CONTENT_HTML,				"Content-type: text/html\r\nContent-Length: "  )                                        \
/*  XX(CONTENT_JS,					"Content-type: application/javascript\r\nContent-Length: "  )                           \*/\
  XX(CONTENT_TXT,         "Content-type: text/txt; charset=utf-8\r\nContent-Length: " )                    \
	XX(CONTENT_JS,					"Content-type: application/x-javascript\r\nContent-Encoding: gzip\r\nContent-Length: " )


enum {
#define XX(name, string) name,
  MAP_HEADER(XX)
#undef XX
  CONTENT_Q,
};

static const char *headerArr[] = {
#define XX(name, string) string,
  MAP_HEADER(XX)
#undef XX
  NULL
};

#ifndef BOOTLOADER
static char buf1[1640];
#endif

static char  out_buf[OUT_BUF_LEN];
static char tmp_buf[1640];

static void httpServerTask (void *arg);
static void httpServer (int conn);
static int sendContent(int conn, const char *out_buf, int left);
static void methodGetHandler (int conn, ext_t ext, char *uri, struct sockaddr_in *from);

//******************************************************************************
//  Секция описания глобальных функций
//******************************************************************************

void http_server_init (void) {
  xTaskCreate (httpServerTask, "Srv", configMINIMAL_STACK_SIZE * 3, NULL, TCP_THREAD_PRIO+4, NULL);
}


//******************************************************************************
//  Секция описания локальных функций
//******************************************************************************
static void httpServerTask (void *arg) __attribute__((noreturn));



static char example[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nConnection: close\r\n\r\n\
                        <!DOCTYPE html>\n\
                        <html>\n<head>\n<title>B-419-stat</title>\n\
                        <meta http-equiv='Content-Type' content='text/html; charset=windows-1251'>\n\
                        </head>\n<body bgcolor='#8dbdef' text='black'>\n\
<body>\n\
	<div>\n\
		<div>\n\
			<div>\n\
				<div>Главная</a></div>\n\
				<div>Контроллер Б-419</a></div>\n\
			</div>\n\
		</div>\n\
			<div>\n\
				<a href='/loops.html' id='menu_1'>Параметры охранной сигнализации</a>\n\
				<a href='/admin.html' id='menu_0'>Служебные параметры</a>\n\
			</div>\n\
			<div>\n\
				<div>\n\
					<h1>Информация</h1>\n\
					<h3>Назначение</h3>\n\
					<p text-align='justify'>Контроллер предназначен для подключения к нему охранных извещателей, проксимити-считывателей (с и без функций кодонаборной панели). Контроллер разрабатывается для применения в составе Контроллеров серии STS-504..</p>\n\
				</div>\n\
			</div>\n\
		</div>\n\
		<div>Версия ПО:</div>\n\
		<div>&copy; 2018 ГК 'Стилсофт'.</a> Все права защищены.</div>\n\
	</div>\n\
</body>\n\
</html>\n";


static void httpServerTask (void *arg) {
	(void)arg;
	struct sockaddr_in local;
	struct sockaddr_in from;
	int fromlen;
	int opt;

  int listen_fd;
  if ( (listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    // SYS_DEBUGF(SYS_SERVER_DBG, ("SocketRecv: create filed!\r\n"));
    vTaskDelete(NULL);
  }

  memset(&local, 0, sizeof(local));
  local.sin_family = AF_INET;
  local.sin_port = htons(80);
  local.sin_addr.s_addr = INADDR_ANY;

	if (bind(listen_fd, (struct sockaddr *)&local, sizeof(local)) < 0) {
		closesocket(listen_fd);
		// SYS_DEBUGF(SYS_SERVER_DBG, ("SocketRecv: bind filed!\r\n"));
		vTaskDelete(NULL);
	}

	opt = 1;
	if (setsockopt(listen_fd, IPPROTO_TCP, TCP_NODELAY, (char *)&opt, sizeof(opt)) < 0) {
		// SYS_DEBUGF(SYS_SERVER_DBG, ("Error setsockopt!\r\n"));
	}

	if (listen (listen_fd, 5) < 0) {
		// SYS_DEBUGF(SYS_SERVER_DBG, ("Error listen!\r\n"));
	}

	fromlen = sizeof(from);

	int conn_fd;
	for ( ; ; ) {
		if ( (conn_fd = accept (listen_fd, (struct sockaddr *)&from, (socklen_t *)&fromlen)) < 0) {
			// if (errno == ECONNABORTED) {
			// 	SYS_DEBUGF(SYS_SERVER_DBG, ("socket: accept ECONNABORTED!\r\n"));
			// }
			// else {
			// 	SYS_DEBUGF(SYS_SERVER_DBG, ("socket: accept filed!\r\n"));
			// }
		}
		else {
			// httpServer (conn_fd);
      send(conn_fd, example, sizeof(example), NETCONN_NOCOPY);
			close(conn_fd);
		}
	}
}


static void httpServer (int conn) {
	char *uri = NULL;

	char ext[5];
	memset(ext, 0, sizeof(ext));

	memset(buf1, 0, sizeof(buf1));

	struct sockaddr_in from;
	int fromlen = sizeof(from);
	int ret = recvfrom(conn, &buf1, sizeof(buf1), 0, (struct sockaddr *)&from, (socklen_t*)&fromlen);

	if ( (ret <= 0)) {
		return;
	}
	else if (ret > (int32_t)(sizeof(buf1) - 40)) {
		sprintf(out_buf, "%s\r\n\r\n", headerArr[HTTP_HEADER_400]);
		sendContent (conn, out_buf, (int32_t)strlen(out_buf));
		return;
	}
	else {
		method_t method = getMethod ( strtok(buf1, " "));

		uri = strtok(NULL, " ");

		ext_t ext = E_NOT_SUP;
		if (uri)
			ext = getDataExtensionRequest(uri);
		else {
			sprintf(out_buf, "%s\r\n\r\n", headerArr[HTTP_HEADER_400]);
			sendContent (conn, out_buf, (int32_t)strlen(out_buf));
			return;
		}

		memset(out_buf, 0, sizeof(out_buf));
		memset(tmp_buf, 0, sizeof(tmp_buf));

		switch ( (uint8_t)method) {
			case M_GET: {
				methodGetHandler(conn, ext, uri, &from);
			}
				break;
			default:
				sprintf(out_buf, "%s\r\n\r\n", headerArr[HTTP_HEADER_501]);
				sendContent (conn, out_buf, (int32_t)strlen(out_buf));
				break;
		}
  }
}

//------------------------------------------------------------------------------------------------------------
static void methodGetHandler (int conn, ext_t ext, char *uri, struct sockaddr_in* from) {
  struct fs_file file = {NULL, 0, 0, NULL};

  if ((ext != E_CGI) && (fs_open(&file, uri) != ERR_OK)) {
    fs_close(&file);

    if (fs_open(&file,"/404.html\0") != ERR_OK)
      fs_close(&file);
    else
      ext = E_NOT_FOUND;
  }

  const char *content;
  const char *header;

  switch ((uint8_t)ext) {
    case E_HTML:
      header = headerArr[HTTP_HEADER_200];
      content = headerArr[CONTENT_HTML];
      break;
    // case E_CGI:
    //   header = headerArr[HTTP_HEADER_200];
    //   content = headerArr[CONTENT_UTF];
	// 		cgiHandler (uri, tmp_buf, from);
    //   break;
    case E_TXT:
      header = headerArr[HTTP_HEADER_200];
      content = headerArr[CONTENT_TXT];
      break;
    case E_JS:
      header = headerArr[HTTP_HEADER_200];
      content = headerArr[CONTENT_JS];
      break;
    case E_CSS:
      header = headerArr[HTTP_HEADER_200];
      content = headerArr[CONTENT_CSS];
      break;
    case E_ICO:
      header = headerArr[HTTP_HEADER_200];
      content = headerArr[CONTENT_ICO];
      break;
    case E_GIF:
      header = headerArr[HTTP_HEADER_200];
      content = headerArr[CONTENT_GIF];
      break;
    case E_JPG:
      header = headerArr[HTTP_HEADER_200];
      content = headerArr[CONTENT_JPG];
      break;
    case E_PNG:
      header = headerArr[HTTP_HEADER_200];
      content = headerArr[CONTENT_PNG];
      break;
    case E_NOT_FOUND:
      header = headerArr[HTTP_HEADER_404];
      content = headerArr[CONTENT_HTML];
      break;
    case E_NOT_SUP:
    default:
			header = headerArr[HTTP_HEADER_501];
      content = headerArr[CONTENT_HTML];
      break;
  }

  if (strlen(tmp_buf)) {
		sprintf(out_buf, "%s%s%u\r\n\r\n", header, content, strlen(tmp_buf));
    strcat (out_buf, tmp_buf);
  }
  else {
    sprintf(out_buf, "%s%s%d\r\n\r\n", header, content, file.len - 1);
  }

	sendContent (conn, out_buf, (int32_t)strlen(out_buf));
  sendContent (conn, file.data, file.len - 1);
}


static int sendContent (int conn, const char *out_buf, int left) {
  int ret = 0, total = 0, len = left;

  while (total < len) {
		ret = lwip_send(conn, out_buf + total, (size_t)left, NETCONN_NOCOPY);

    if (ret == -1) {
    //   SYS_DEBUGF(SYS_SERVER_DBG, ("Error send file content!\n"));
      vTaskDelay(100);

			ret = lwip_send(conn, out_buf + total, (size_t)left, NETCONN_NOCOPY);

      if (ret == -1) {
        // SYS_DEBUGF(SYS_SERVER_DBG, ("Error second send file content!\r\n"));
        break;
      }
    }

    if (ret == 0) {
    //   SYS_DEBUGF(SYS_SERVER_DBG, ("Error, connection close while file sending!\r\n"));
      break;
    }

    total += ret;
    left -= ret;
  }

  return left;
}