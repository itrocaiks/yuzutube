# Проверка наличия компилятора Clang или GCC
ifeq ($(shell command -v clang 2>/dev/null),)
	CC = gcc
else
	CC = clang
endif

CFLAGS = -Wall -Werror -g -O2 -DNCURSES_WIDECHAR=1
LDFLAGS = -lcjson -lncursesw
INCLUDES = -Ilib
SRC = src/main.c
OBJ = $(SRC:.c=.o)
OUT = yuzutube

HOME := $(shell echo $$HOME)

# Основная цель
all: $(OUT)

# Линковка
$(OUT): $(OBJ)
	$(CC) $(OBJ) -o $(OUT) $(LDFLAGS)

# Компиляция исходников в объектные файлы
$(OBJ): $(SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $(SRC) -o $(OBJ)

# Очистка объектных файлов и исполнимого файла
clean:
	rm -f $(OBJ) $(OUT)

install:
	mkdir -p $(HOME)/.config/yuzutube/bin
	mkdir -p $(HOME)/.config/yuzutube/conf
	mkdir -p $(HOME)/.config/yuzutube/tmp
	cp src/downloader.py $(HOME)/.config/yuzutube/bin
	cp src/query_sender.py $(HOME)/.config/yuzutube/bin
	sudo install -m 755 $(OUT) /usr/bin/$(OUT)

uninstall:
	rm -rf $(HOME)/.config/yuzutube/
	sudo rm /usr/bin/$(OUT)
# Псевдоним для clean
.PHONY: clean
