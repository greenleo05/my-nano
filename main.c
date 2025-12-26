#include <unistd.h>
#include <termios.h> //Linux terminal 관련 헤더
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#define CTRL_KEY(k) ((k) & 0x1f) //Ctrl 키 조합을 위해 같이 입력받은 문자의 맨 뒤 5비트를 제외하고 모두 날려버림

struct editorConfig{
    int cx, cy;
    struct termios orig_termios;
};

void die(const char* s)
{
    perror(s);
    exit(1);
}

struct editorConfig E;

void disableRawMode()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios);
}

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    {
        die("tcgetattr");
    }

    atexit(disableRawMode);

    struct termios raw = E.orig_termios;

    //잡다한 기능 다 꺼버리기
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;


    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        die("tcsetattr");
    }
}

#define SCREED_ROWS 24

void editorDrawRows()
{
    int y;
    for (int y = 0; y < SCREED_ROWS; y++)
    {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void initEditor()
{
    E.cx = 0;
    E.cy = 0;
}

void editorRefreshScreen()
{
    write(STDOUT_FILENO, "\x1b[2J", 4); //화면 지우기
    write(STDOUT_FILENO, "\x1b[H", 3);  //커서를 맨 위로 이동

    editorDrawRows();

    char buf[32];
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", E.cy + 1, E.cx + 1);
    write(STDOUT_FILENO, buf, strlen(buf));
}

void editorMoveCursor(char key)
{
    switch (key)
    {
        case 'a':
            if(E.cx > 0) E.cx--;
            break;
        case 'd':
            E.cx++;
            break;
        case 'w':
            if(E.cy > 0) E.cy--;
            break;
        case 's':
            if (E.cy < SCREED_ROWS - 1) E.cy++;
            break;
    }
}

int main()
{
    setbuf(stdout, NULL);
    enableRawMode();

    editorRefreshScreen();

    while(1)
    {
        char c = '\0'; //문자를 저장할 변수

        int nread = read(STDIN_FILENO, &c, 1);

        //오류처리
        if (nread == -1 && errno != EAGAIN)
            die("read");

        if (nread == 0) continue;
    
        if (c == CTRL_KEY('q'))
        {
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            break;
        }

        //모든 오류와 종료키를 뚫고 온 결과!
        if (c == 'w' || c == 'a' || c == 's' || c == 'd')
        {
            editorMoveCursor(c);
            editorRefreshScreen();
        }
    }
    

    return 0;
}