# aptitude install libgcrypt-dev

.PHONY: ALL INSTALL

CXXFLAGS=-rdynamic -I/usr/include/libxml2 

#FCGI_PROGS=login.fcgi logout.fcgi createlogin.fcgi cookies.fcgi

liboak.a: oak.o loginutils.o Config.o
	ar -rc $@ $^

INSTALL: liboak.a
	sudo mkdir -p /usr/local/include/OAK/ /usr/local/lib/OAK/
	sudo cp -t /usr/local/include/OAK/ oak.h loginutils.h 
	sudo cp liboak.a /usr/local/lib/OAK/
	-sudo killall --quiet $(FCGI_PROGS)
	sudo cp *.fcgi /var/www/beer/api/


#post.fcgi: post.o loginutils.o liboak.a
#	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -ldl -lmemcached -lxml2 -lxslt -lboost_filesystem -L. -loak -o $@

login.fcgi: login.o loginutils.o liboak.a
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -ldl -lmemcached -lxml2 -lxslt -lboost_filesystem -L. -loak -o $@

logout.fcgi: logout.o loginutils.o liboak.a
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -ldl -lmemcached -lxml2 -lxslt -lboost_filesystem -L. -loak -o $@

createlogin.fcgi: createlogin.o loginutils.o liboak.a
	g++ $^ -lfcgi -lcgic_fcgi -lgcrypt -ldl -lmemcached -lxml2 -lxslt -lboost_filesystem -L. -loak -o $@

cookies.fcgi: cookies.o
	g++ $^ -lfcgi -lcgic_fcgi -lxml2 -o $@

oak.o: oak.cc oak.h
	g++ -c $< -ldl -I/usr/include/libxml2 -I/usr/local/include/thrift -I/usr/local/include/thrudoc -o $@
