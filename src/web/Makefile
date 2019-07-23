PERL_SC=perl
PERL_PRM=./makefsdata.pl
WEB_OBJ=httpd/index.html httpd/admin.html httpd/loops.html httpd/control.html httpd/style.css httpd/404.html httpd/favicon.ico
WEB_OBJ_IMG=httpd/img/*.gif
WEB_OBJ_JS=httpd/js/*.js
WEB_OBJ_TXT=httpd/txt/login.txt
WEB_OBJ_TXT+=httpd/txt/admin/*.txt
WEB_OBJ_TXT+=httpd/txt/loops/*.txt
WEB_OBJ_TXT+=httpd/txt/control/*.txt

all: fsdata.c

fsdata.c: $(WEB_OBJ) $(WEB_OBJ_IMG) $(WEB_OBJ_JS) $(WEB_OBJ_TXT)
	$(PERL_SC) $(PERL_PRM)
