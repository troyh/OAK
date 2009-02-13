.PHONY: ALL

ALL: post.fcgi login.fcgi createlogin.fcgi cookies.fcgi

post.fcgi: post.o loginutils.o
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -o $@

login.fcgi: login.o loginutils.o
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -o $@

logout.fcgi: logout.o loginutils.o
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -o $@

createlogin.fcgi: createlogin.o loginutils.o
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -o $@

cookies.fcgi: cookies.o
	g++ $^ -lfcgi -lcgic_fcgi -o $@
