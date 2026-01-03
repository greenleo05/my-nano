#ifndef NANODATA_H
#define NANODATA_H

#include <unistd.h>
#include <termios.h> //Linux terminal 관련 헤더
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h> //윈도우 크기 조절 관련 헤더
#include <time.h>

#define KILO_TAB_STOP 8
#define CTRL_KEY(k) ((k) & 0x1f) //Ctrl 키 조합을 위해 같이 입력받은 문자의 맨 뒤 5비트를 제외하고 모두 날려버림

enum editorKey
{
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN
};

typedef struct erow
{
	int size;
    int rsize;
    char *chars;
    char *render;
} erow;

struct editorConfig
{
    int cx, cy; //커서 위치
    int rx; //렌더링된 커서 위치
    int rowoff; //줄 오프셋
    int coloff; //열 오프셋
    int screenrows; //화면에 보이는 줄의 수
    int screencols; //화면에 보이는 열의 수

    int numrows; //총 줄 수
    erow *row; //해당 줄의 정보들 (길이, 문자열)

    struct termios orig_termios; //원래 터미널 설정을 저장할 구조체

    char *filename; //열려있는 파일 이름

    int dirty; //파일이 수정되었는지 여부
    char statusmsg[80]; //상태 메시지
    time_t statusmsg_time; //상태 메시지가 뜬 시각
};

extern struct editorConfig E;

void die(const char* s); //오류 처리 함수

#endif