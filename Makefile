CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99 -D_DEFAULT_SOURCE -D_GNU_SOURCE

# 소스 파일 목록
SRCS = main.c nanoterminal.c nanorow.c nanooutput.c nanofile.c nanodata.c

# 만들고 싶은 프로그램 이름
TARGET = mynano

$(TARGET): $(SRCS)
	$(CC) $(SRCS) -o $(TARGET) $(CFLAGS)

clean:
	rm -f $(TARGET)