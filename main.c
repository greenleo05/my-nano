#include "nanodata.h"
#include "nanofile.h"
#include "nanooutput.h"
#include "nanorow.h"
#include "nanoterminal.h"

/*전역 변수*/
struct editorConfig E; //에디터 설정 전역 변수

void initEditor(); //에디터 초기화 함수 선언
void editorProcessKeypress(); //키 입력 처리 함수 선언
void editorMoveCursor(int key); //커서 이동 함수 선언

int main(int argc, char* argv[])
{
    //초기 세팅
    setbuf(stdout, NULL);
    enableRawMode();
    initEditor();

    //파일 이름 받기
    if (argc >= 2)
    {
        editorOpen(argv[1]);
    }
    
    editorRefreshScreen(); //화면 초기 새로고침

    editorSetStatusMessage("환영합니다! Ctrl-S = 저장 | Ctrl-Q = 종료");

    while(1)
    {
        editorRefreshScreen(); //화면 업데이트
        editorProcessKeypress(); //키 입력 처리
    }
    
    return 0;
}

void initEditor() //에디터 초기화
{
    //변수 초기화
    E.cx = 0;
    E.cy = 0;
    E.rowoff = 0;
    E.coloff = 0;
    E.numrows = 0;
    E.row = NULL;
    E.filename = NULL;

    //윈도우 값 받아오기 + 오류 처리
    if (getWindowSize(&E.screenrows, &E.screencols) == -1)
    {
        die("getWindowSize");
    }

    E.screenrows -= 2; //상태 바와 메시지 바를 위해 2줄 빼기
}

void editorProcessKeypress() //키 입력 처리
{
    int c = editorReadKey(); //키 입력 읽기

    switch (c)
    {
        case CTRL_KEY('q'): //Ctrl-Q 입력 시 종료
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);
            exit(0);
            break;

        case CTRL_KEY('s'): //Ctrl-S 입력 시 파일 저장
            editorSave();
            break;

        case CTRL_KEY('f'): //Ctrl-F 입력 시 찾기 기능
            editorFind();
            break;

        case ARROW_UP:
        case ARROW_DOWN:
        case ARROW_LEFT:
        case ARROW_RIGHT:
            editorMoveCursor(c); //커서 이동
            break;

        case 127: //백스페이스 키
        case CTRL_KEY('h'): //Ctrl-H 키 (백스페이스와 동일)
        case CTRL_KEY('?'): //Ctrl-? 키 (백스페이스와 동일)
            editorDelChar(); //문자 삭제
            break;

        case '\r': //엔터 키
            editorInsertNewLine(); //새 줄 삽입
            break;
        
        default:
            if (c == '\x1b') //ESC 키
            {
                //일단 비우기
            }
            else
            {
                editorInsertChar(c); //문자 삽입
            }
            break;
    }
}

void editorMoveCursor(int key) //커서 이동
{
    switch (key)
    {
        case ARROW_LEFT:
            if(E.cx > 0) E.cx--;
            break;
        case ARROW_RIGHT:
            if (E.cy < E.numrows)
            { 
                erow *row = &E.row[E.cy]; //E.cy번째 줄(커서가 현재 있는 줄)의 정보를 row 포인터에 저장
                if (E.cx < row->size) E.cx++; //그 줄의 길이가 커서의 x 위치보다 커야 커서를 오른쪽으로 이동할 수 있음
            }
            break;
        case ARROW_UP:
            if(E.cy > 0) E.cy--;
            break;
        case ARROW_DOWN:
            if (E.cy < E.numrows) E.cy++;
            break;
    }

    //커서가 현재 줄의 길이를 넘어가지 않도록 조정

    int rowlen = 0;
    if (E.cy < E.numrows) //현재 줄이 빈 줄이 아니면
    {
        rowlen = E.row[E.cy].size; //현재 줄의 길이 저장
    }

    if (E.cx > rowlen) //현재 커서가 줄 바깥에 있다면
    {
        E.cx = rowlen; //커서를 당겨서 붙이기
    }
}
