
PROG=raspberrytelescope
LUA_VERSION=5.2.1
LUA=lua-$(LUA_VERSION)/src
CC=gcc
CFLAGS=-O2 -Wall -g -rdynamic -std=c99
LDFLAGS=-ldl -pthread -lgphoto2 -lconfig -I$(LUA) -L$(LUA) -llua -lm
MONGOOSE_BUILD_OPTS=-DUSE_LUA

raspberrytelescope:
	$(CC) stringutils.c fileutils.c telescopecamera.c timelapse.c mongoose.c webserver.c $(CFLAGS) $(LDFLAGS) $(MONGOOSE_BUILD_OPTS) -o $(PROG)

.PHONY : clean
clean:
	rm -f *.o $(PROG)

setup: 
	wget http://www.lua.org/ftp/lua-$(LUA_VERSION).tar.gz -O lua-$(LUA_VERSION).tar.gz
	tar xzf lua-$(LUA_VERSION).tar.gz
	cd lua-$(LUA_VERSION); make linux
	cd -;
	
dist-clean: clean
	rm -rf lua-$(LUA_VERSION) lua-$(LUA_VERSION).tar.gz
