#include "nanooutput.h"
#include "nanodata.h"
#include "nanorow.h"
#include "nanoterminal.h"
#include <stdarg.h>

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
        if (y < E.screenrows)
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

    editorDrawStatusBar(); //상태 바 그리기
    editorDrawMessageBar(); //메시지 바 그리기

    char buf[32];

    //커서 위치 이동 계산
    snprintf(buf, sizeof(buf), "\x1b[%d;%dH", (E.cy - E.rowoff) + 1, (E.rx - E.coloff) + 1);
    //이동한 값을 버퍼에 저장
    write(STDOUT_FILENO, buf, strlen(buf));
    //커서 띄우기
    write(STDOUT_FILENO, "\x1b[?25h", 6);
}

void editorSetStatusMessage(const char* fmt, ...) //상태 메시지 설정
{
    va_list ap; //가변 인자 리스트
    va_start(ap, fmt); //가변 인자 처리 시작

    vsnprintf(E.statusmsg, sizeof(E.statusmsg), fmt, ap); //상태 메시지 포맷팅

    va_end(ap); //가변 인자 처리 종료
    E.statusmsg_time = time(NULL); //현재 시간 저장
}

void editorDrawStatusBar() //상태 바 그리기
{
    write(STDOUT_FILENO, "\x1b[7m", 4); //색상 반전

    char status[80], rstatus[80];

    int len = snprintf(status, sizeof(status), "%.20s - %d lines %s",
                        E.filename ? E.filename : "[No Name]", E.numrows,
                        E.dirty ? "(modified)" : ""); //상태 문자열 생성
    int rlen = snprintf(rstatus, sizeof(rstatus), "%d/%d",
                        E.cy + 1, E.numrows); //오른쪽 상태 문자열 생성

    if (len > E.screencols)
        len = E.screencols; //상태 문자열이 화면 너비보다 크면 잘라내기
    write(STDOUT_FILENO, status, len); //상태 문자열 출력

    while (len < E.screencols) //남은 공간 채우기
    {
        if (E.screencols - len == rlen) //오른쪽 상태 문자열이 들어갈 자리면
        {
            write(STDOUT_FILENO, rstatus, rlen); //오른쪽 상태 문자열 출력
            break;
        }
        else
        {
            write(STDOUT_FILENO, " ", 1); //공백 출력
            len++;
        }
    }

    write(STDOUT_FILENO, "\x1b[m", 3); //색상 반전 해제
    write(STDOUT_FILENO, "\r\n", 2); //줄 바꿈
}

void editorDrawMessageBar() //메시지 바 그리기
{
    write(STDOUT_FILENO, "\x1b[K", 3); //메시지 바 지우기

    int msglen = strlen(E.statusmsg); //현재 메시지 길이 계산
    if (msglen > E.screencols) //메시지 길이가 화면 너비보다 크면 잘라내기
        msglen = E.screencols;

    if (msglen > 0 && time(NULL) - E.statusmsg_time < 5) //메시지가 있고 5초 이내면
    {
        write(STDOUT_FILENO, E.statusmsg, msglen); //메시지 출력
    }
}

char* editorPrompt(char* prompt) //프롬프트 표시 및 입력 받기
{
    size_t bufsize = 128; //버퍼 크기
    char* buf = (char*)malloc(bufsize); //버퍼 메모리 할당
    size_t buflen = 0; //현재 버퍼 길이
    buf[0] = '\0'; //버퍼 초기화

    while(1)
    {
        editorSetStatusMessage(prompt, buf); //상태 메시지에 프롬프트와 현재 입력된 내용 표시
        editorRefreshScreen(); //화면 업데이트

        int c = editorReadKey(); //키 입력 읽기

        if (c == '\r') //엔터 키 : 입력 완료
        {
            if (buflen != 0) //버퍼에 내용이 있으면
            {
                editorSetStatusMessage(""); //상태 메시지 지우기
                return buf; //입력된 내용 반환
            }
        }
        else if (c == 127 || c == CTRL_KEY('h') || c == CTRL_KEY('?')) //백스페이스 키 : 마지막 문자 삭제
        {
            if (buflen != 0) //버퍼에 내용이 있으면
            {
                buf[--buflen] = '\0'; //버퍼에서 마지막 문자 제거
            }
        }
        else if (c == '\x1b') //Esc 키 : 입력 취소
        {
            editorSetStatusMessage(""); //상태 메시지 지우기
            free(buf); //버퍼 메모리 해제
            return NULL; //NULL 반환
        }
        else if (!iscntrl(c) && c < 128) //제어 문자가 아니고 ASCII 범위 내의 문자이면
        {
            if (buflen + 1 >= bufsize) //버퍼가 꽉 찼으면
            {
                bufsize *= 2; //버퍼 크기 두 배로 늘리기
                buf = (char*)realloc(buf, bufsize); //버퍼 재할당
            }
            buf[buflen++] = c; //버퍼에 문자 추가
            buf[buflen] = '\0'; //널 종료 문자 추가
        }
    }
}