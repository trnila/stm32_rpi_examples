PREFIX=/usr/local/

install:
	install -D -m 755 -t $(PREFIX)/bin/ tools/* 
