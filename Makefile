CC      = gcc
CFLAGS  = -g -Wall -O2 --pedantic `pkg-config glib-2.0 --cflags`
LIBS    = `pkg-config glib-2.0 --libs`

BIN     = hcat
OBJS    = hcat.o

all: $(BIN)

$(BIN): $(OBJS) Makefile
	$(CC) $(OBJS) -o $@ $(LIBS) $(CFLAGS)

clean:
	rm -rf $(BIN) *.o

install:
ifeq ($(USER), root)
	@if (cp hcat.3.gz /usr/share/man/man3/); then echo [OK]; else echo "Fail on install man page"; fi
	@if (cp hcat /usr/sbin/); then echo [OK]; else echo "Fail to copy the hcat to /usr/sbin/"; fi
else
	@echo "You must be root to install hcat!"
endif

unistall:
ifeq ($(USER), root)
	@if (rm -rf /usr/share/man/man3/hcat.3.gz); then echo [OK]; else echo "Falha ao remover a pagina do manual"; fi
	@if (rm -rf /usr/sbin/hcat); then echo [OK]; else echo "Falha ao remover binario do PATH"; fi
else
	@echo "You must be root to remove hcat!"
endif
