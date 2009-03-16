.PHONY: ALL INSTALL

CXXFLAGS=-rdynamic 

FCGI_PROGS=post.fcgi login.fcgi logout.fcgi createlogin.fcgi cookies.fcgi

ALL: $(FCGI_PROGS)

INSTALL: ALL
	sudo mkdir -p /usr/local/include/OAK/ /usr/local/lib/OAK/
	sudo cp -t /usr/local/include/OAK/ oak.h loginutils.h 
	sudo cp liboak.a /usr/local/lib/OAK/
	-sudo killall --quiet $(FCGI_PROGS)
	sudo cp *.fcgi /var/www/beer/api/

liboak.a: oak.o loginutils.o
	ar -rc $@ $^

post.fcgi: post.o loginutils.o liboak.a
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -ldl -lmemcached -L. -loak -o $@

login.fcgi: login.o loginutils.o liboak.a
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -ldl -lmemcached -L. -loak -o $@

logout.fcgi: logout.o loginutils.o liboak.a
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -ldl -lmemcached -L. -loak -o $@

createlogin.fcgi: createlogin.o loginutils.o liboak.a
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -ldl -lmemcached -L. -loak -o $@

cookies.fcgi: cookies.o
	g++ $^ -lfcgi -lcgic_fcgi -o $@

oak.o: oak.cc oak.h
	g++ -c $< -ldl -o $@
