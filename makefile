
PROG=raspberrytelescope
CC=gcc
CFLAGS=-O2 -Wall -g -rdynamic -std=c99
LDFLAGS=-ldl -pthread -lgphoto2 -lconfig -lm
JQUERYVERSION=1.9.1
JQUERYUIVERSION=1.10.3

raspberrytelescope:
	$(CC) *.c $(CFLAGS) $(LDFLAGS) -o $(PROG)

.PHONY : clean
clean:
	rm -f *.o $(PROG)

dist-clean:
	rm -rf webRoot/jquery-ui

dependencies:
	mkdir -p webRoot/jquery-ui/images
	wget http://code.jquery.com/jquery-$(JQUERYVERSION).js -O webRoot/jquery-ui/jquery.js
	wget http://code.jquery.com/ui/$(JQUERYUIVERSION)/jquery-ui.js -O webRoot/jquery-ui/jquery-ui.js
	wget http://jqueryui.com/resources/download/jquery-ui-themes-$(JQUERYUIVERSION).zip -O jquery-ui-themes.zip
	unzip -j jquery-ui-themes.zip jquery-ui-themes-$(JQUERYUIVERSION)/themes/dark-hive/images/* -d webRoot/jquery-ui/images/
	unzip -j jquery-ui-themes.zip jquery-ui-themes-$(JQUERYUIVERSION)/themes/dark-hive/jquery-ui.css -d webRoot/jquery-ui/
	unzip -j jquery-ui-themes.zip jquery-ui-themes-$(JQUERYUIVERSION)/themes/dark-hive/jquery.ui.theme.css -d webRoot/jquery-ui/
	rm -f jquery-ui-themes.zip
