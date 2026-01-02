#include "nanooutput.h"
#include "nanodata.h"
#include "nanorow.h"

void editorScroll() //스크롤 조정
{
    //오프셋 (rowoff, coloff)를 계산하는 과정
    E.rx = 0; //rx 초기화
    if (E.cy < E.numrows) //현재 줄이 빈 줄이 아니면
    {
        E.rx = editorRowCxToRx(&E.row[E.cy], E.cx); //rx 계산
    }

    if (E.cy < E.rowoff) //커서가 화면 위쪽으로 나가면
    {
        E.rowoff = E.cy;
    }
    if (E.cy >= E.rowoff + E.screenrows) //커서가 화면 아래쪽으로 나가면
    {
        E.rowoff = E.cy - E.screenrows + 1;
    }

    if (E.rx < E.coloff) //커서가 화면 왼쪽으로 나가면
    {
        E.coloff = E.rx;
    }
    if (E.rx >= E.coloff + E.screencols) //커서가 화면 오른쪽으로 나가면
    {
        E.coloff = E.rx - E.screencols + 1;
    }
}

void editorDrawRows() //줄 그리기
{
    int y;
    for (y = 0; y < E.screenrows; y++) //y는 화면에 보이는 줄 번호
    {
        int filerow = y + E.rowoff; //실제 파일 줄 번호 (현재 보고 있는 줄(y)에 실제 오프셋(rowoff)을 더해야 하므로)

        if (filerow < E.numrows) //y가 최대 줄보다 작으면 (일반적인 경우)
        {
            int len = E.row[filerow].rsize; //y번째 줄 크기 가져오기

            if (len < E.coloff) //화면 왼쪽 오프셋보다 작으면
	            len = 0;
            else
	            len -= E.coloff; //오프셋만큼 뺀 것이 len(출력할 길이)

            if (len > E.screencols) //화면 너비보다 길면 잘라내기
                len = E.screencols;

            write(STDOUT_FILENO, &E.row[filerow].render[E.coloff], len); //y번째 줄 출력
        }
        else //빈 줄
        {
            write(STDOUT_FILENO, "~", 1); //빈 줄 출력
        }

        write(STDOUT_FILENO, "\x1b[K", 3); //줄 끝까지 지우기
        if (y < E.screenrows - 1)
        {
            write(STDOUT_FILENO, "\r\n", 2); //줄 바꿈
        }
    }
}

void editorRefreshScreen() //화면 업데이트
{
    editorScroll(); //스크롤 계산
    write(STDOUT_FILENO, "\x1b[?25l", 6); //커서 숨기기
    write(STDOUT_FILENO, "\x1b[H", 3);  //커서를 맨 위로 이동

    editorDrawRows(); //줄 그리기

    char buf[32];

    //커서 위치 이동 계산
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.rx - E.coloff) + 1);
    //이동한 값을 버퍼에 저장
    write(STDOUT_FILENO, buf, strlen(buf));
    //커서 띄우기
    write(STDOUT_FILENO, "\x1b[?25h", 6);
}
