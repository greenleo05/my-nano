#include "nanoterminal.h" 
#include "nanodata.h"


void disableRawMode() //Raw 모드 비활성화 함수
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios);
}

void enableRawMode() //Raw 모드 활성화 함수
{
    //tcgetattr(터미널 설정 가져오기)에서 오류가 났을 때
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1)
    {
        die("tcgetattr");
    }

    //원래 설정을 저장해두기
    atexit(disableRawMode);

    struct termios raw = E.orig_termios;

    //잡다한 기능 다 꺼버리기
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    //터미널에 설정 적용
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
    {
        die("tcsetattr");
    }
}

int editorReadKey() //키 입력 읽기
{
    char c;
    int nread = read(STDIN_FILENO, &c, 1); //1바이를 읽어 c에 저장하고 nread에 읽은 바이트 수 저장

    while(nread != 1) //nread가 1이 아니면 오류가 있다는 것
    {
        if (nread == -1 && errno != EAGAIN) //오류 처리
            die("read");

        nread = read(STDIN_FILENO, &c, 1); //다시 읽기 시도
    }

    if (c == '\x1b') //방향키로 의심되는 키가 입력된 것 같을 때
    {
        char seq[3]; //\x1b[...는 3바이트이므로 일단 끊어서 보기 위해 배열 생성

        // 버퍼에 다음 글자가 있는지 확인 (없으면 ESC로 간주)
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return '\x1b';
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return '\x1b';

        // 패턴 확인: \x1b[...
        if (seq[0] == '[') //일단 '['가 맞으면? 방향키일 가능성이 높음!
        {
            switch (seq[1]) //다음 글자에 따라 어떤 방향키일지 결정하고 반환
            {
                case 'A': return ARROW_UP;    //\x1b[A
                case 'B': return ARROW_DOWN;  //\x1b[B
                case 'C': return ARROW_RIGHT; //\x1b[C
                case 'D': return ARROW_LEFT;  //\x1b[D
            }
        }

        return '\x1b'; // 패턴이 안 맞으면 그냥 ESC 키로 간주
    }
    
    return c; //if문에 걸리지 않았으면 일반 문자이므로 그대로 반환
}

int getWindowSize(int* rows, int* cols) //시작했을 때 창 크기 가져오기
{
    struct winsize ws; //창 크기를 저장할 구조체

    //ioctl 함수를 사용해 창 크기 가져오기 + 오류 처리
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0)
    {
        return -1;
    }
    else
    {
        //받은 매개변수에 창 크기 저장
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}
