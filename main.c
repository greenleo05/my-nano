#include <unistd.h>
#include <termios.h> //Linux terminal 관련 헤더
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>

#define CTRL_KEY(k) ((k) & 0x1f) //Ctrl 키 조합을 위해 같이 입력받은 문자의 맨 뒤 5비트를 제외하고 모두 날려버림

enum editorKey{
    ARROW_LEFT = 1000,
    ARROW_RIGHT,
    ARROW_UP,
    ARROW_DOWN
};

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

#define SCREEN_ROWS 24

void editorDrawRows()
{
    int y;
    for (int y = 0; y < SCREEN_ROWS; y++)
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

void editorMoveCursor(int key)
{
    switch (key)
    {
        case ARROW_LEFT:
            if(E.cx > 0) E.cx--;
            break;
        case ARROW_RIGHT:
            E.cx++;
            break;
        case ARROW_UP:
            if(E.cy > 0) E.cy--;
            break;
        case ARROW_DOWN:
            if (E.cy < SCREEN_ROWS - 1) E.cy++;
            break;
    }
}

int editorReadKey()
{
    char c;
    int nread = read(STDIN_FILENO, &c, 1);

    while(nread != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            die("read");

        nread = read(STDIN_FILENO, &c, 1);
    }

    if (c == '\x1b')
    {
        char seq[3];

        // 버퍼에 다음 글자가 있는지 확인 (없으면 ESC로 간주)
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        // 패턴 확인: \x1b[...
        if (seq[0] == '[')
        {
            switch (seq[1])
            {
                case 'A': return ARROW_UP;    //\x1b[A
                case 'B': return ARROW_DOWN;  //\x1b[B
                case 'C': return ARROW_RIGHT; //\x1b[C
                case 'D': return ARROW_LEFT;  //\x1b[D
            }
        }

        return '\x1b'; // 패턴이 안 맞으면 그냥 ESC 키로 간주
    }
    
    return c;
}

int main()
{
    setbuf(stdout, NULL);
    enableRawMode();
    initEditor();
    editorRefreshScreen();

    while(1)
    {
        editorRefreshScreen();
        int c = editorReadKey();
    
        if (c == CTRL_KEY('q'))
        {
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            break;
        }

        //모든 오류와 종료키 입력을 뚫고 온 결과!
        editorMoveCursor(c);
    }
    
    return 0;
}