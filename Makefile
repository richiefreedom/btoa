all: main.c
	gcc -O2 -Wall -Wextra -pedantic main.c -o btoa
	strip btoa
clean:
	rm -f btoa
	rm -f btoa.1.gz
install:
	sudo cp btoa /usr/bin
	gzip -c btoa.1 > btoa.1.gz
	sudo cp btoa.1.gz /usr/share/man/man1
uninstall:
	sudo rm -f /usr/bin/btoa
	sudo rm -f /usr/share/man/man1/btoa.1.gz
