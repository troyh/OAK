.PHONY: ALL

CXXFLAGS=-rdynamic 

ALL: post.fcgi login.fcgi logout.fcgi createlogin.fcgi cookies.fcgi

post.fcgi: post.o loginutils.o oak.o
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -ldl -o $@

login.fcgi: login.o loginutils.o oak.o
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -ldl -o $@

logout.fcgi: logout.o loginutils.o oak.o
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -ldl -o $@

createlogin.fcgi: createlogin.o loginutils.o oak.o
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -ldl -o $@

cookies.fcgi: cookies.o
	g++ $^ -lfcgi -lcgic_fcgi -o $@
