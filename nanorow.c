#include "nanorow.h"
#include "nanodata.h"
#include "nanooutput.h"

void editorUpdateRow(erow* row) //렌더링된 문자열을 만드는 함수
{
    int tabs = 0;
    int j;

    //탭 문자의 개수 세기
    for (j = 0; j < row->size; j++)
    {
        if (row->chars[j] == '\t') tabs++;
    }

    //필요한 메모리 계산
    free(row->render); //이전에 할당했던 메모리 삭제
    row->render = (char*)malloc(row->size + tabs * (KILO_TAB_STOP - 1) + 1);

    //탭 문자를 만나면 공백으로 채우기
    int idx = 0; //render의 인덱스
    for (j = 0; j < row->size; j++)
    {
        if (row->chars[j] == '\t') //탭 문자를 발견하면
        {
            row->render[idx++] = ' '; //idx가 KILO_TAB_STOP의 배수일 경우 패싱을 막기 위함
            while (idx % KILO_TAB_STOP != 0) //탭 스톱까지 공백 채우기 (KILO_TAB_STOP의 배수 단위로)
                row->render[idx++] = ' ';
        }
        else
        {
            row->render[idx++] = row->chars[j]; //이외의 문자는 그대로 복사
        }
    }

    row->render[idx] = '\0'; //널문자 추가
    row->rsize = idx; //렌더링된 문자열의 길이 저장
}

void editorAppendRow(char* s, size_t len) //한 줄 추가
{
    editorInsertRow(E.numrows, s, len);
    E.dirty++; //파일이 수정되었음을 표시
}

void editorInsertRow(int at, char* s, size_t len) //특정 위치에 줄 삽입
{
    if (at < 0 || at > E.numrows) //at이 범위 밖에 있으면 아무것도 하지 않음
        return;
    
    //E.row 배열의 크기를 하나 늘림
    E.row = (erow*)realloc(E.row, sizeof(erow) * (E.numrows + 1));

    //at 위치부터 한 칸씩 뒤로 밀기
    memmove(&E.row[at + 1], &E.row[at], sizeof(erow) * (E.numrows - at));

    E.row[at].size = len; //삽입할 줄의 길이는 len(입력받은 길이)이다

    //at 위치의 길이를 (문자열 + 1)만큼으로 설정 (널문자 고려)
    E.row[at].chars = (char*)malloc(len + 1);

    memcpy(E.row[at].chars, s, len); //s를 at 위치의 문자열로 복사
    E.row[at].chars[len] = '\0'; //at 위치의 마지막 칸(len)에 널문자 넣기

    E.row[at].rsize = 0;
    E.row[at].render = NULL;
    editorUpdateRow(&E.row[at]); //렌더링된 문자열 생성

    E.numrows++; //전체 줄 수 1 증가
    E.dirty++; //파일이 수정되었음을 표시
}
void editorInsertNewLine() //커서 위치에 새 줄 삽입
{
    if (E.cx == 0) //커서가 줄 맨 앞에 있으면
    {
        editorInsertRow(E.cy, "", 0); //현재 커서가 있는 위치에 빈 줄을 삽입함
    }
    else //커서가 줄 맨 앞이 아니면
    {
        erow *row = &E.row[E.cy]; //현재 줄 정보 가져오기

        //현재 커서(cx)부터 문자열 맨 끝까지를 잘라내서 아랫줄(cy + 1)에 삽입하기
        editorInsertRow(E.cy + 1, &row->chars[E.cx], row->size - E.cx);

        row = &E.row[E.cy]; //현재 줄 정보 다시 가져오기

        row->size = E.cx; //현재 줄의 길이를 커서 위치로 자르기
        row->chars[row->size] = '\0'; //널문자 추가
        editorUpdateRow(row); //렌더링된 문자열 업데이트하기
    }

    E.cy++; //커서를 아래 줄로 이동
    E.cx = 0; //커서를 줄 맨 앞으로 이동
    E.dirty++; //파일이 수정되었음을 표시
}

void editorRowDelChar(erow* row, int at) //한 줄에서 문자 삭제
{
    //at이 row->size 범위 밖에 있다면 아무것도 하지 않음
    if (at < 0 || at >= row->size)
        return;

    //문자들 앞으로 당기기
    memmove(&row->chars[at], &row->chars[at + 1], row->size - at);

    //길이 감소시키기
    row->size--;

    //렌더링 업데이트하기
    editorUpdateRow(row);
    E.dirty++; //파일이 수정되었음을 표시
}

void editorRowAppendString(erow* row, char* s, size_t len) //한 줄에 문자열 추가
{
    row->chars = (char*)realloc(row->chars, row->size + len + 1); //메모리 확장

    memcpy(&row->chars[row->size], s, len); //문자열 추가

    //사이즈 업데이트
    row->size += len;
    row->chars[row->size] = '\0'; //널문자 추가

    //렌더링 업데이트하기
    editorUpdateRow(row);
    E.dirty++; //파일이 수정되었음을 표시
}

void editorDelRow(int at) //특정 위치의 줄 삭제
{
    if (at < 0 || at >= E.numrows) //at이 범위 밖에 있으면 아무것도 하지 않음
        return;

    free(E.row[at].chars); //문자열 메모리 해제
    free(E.row[at].render); //렌더링된 문자열 메모리 해제

    //at 위치부터 한 칸씩 앞으로 당기기
    memmove(&E.row[at], &E.row[at + 1], sizeof(erow) * (E.numrows - at - 1));

    E.numrows--; //전체 줄 수 1 감소
    E.dirty++; //파일이 수정되었음을 표시
}

void editorRowInsertChar(erow* row, int at, int c) //한 줄에 문자 삽입
{
    //at이 row->size 범위 밖에 있다면 맨 끝으로 고정
    if (at < 0 || at > row->size)
        at = row->size;

    //메모리 확장
    row->chars = (char*)realloc(row->chars, row->size + 2); //문자 하나 추가 + 널문자

    //문자들 뒤로 밀기
    memmove(&row->chars[at + 1], &row->chars[at], row->size - at + 1);

    //문자 삽입 및 길이 증가시키기
    row->size++;
    row->chars[at] = c;

    //렌더링 업데이트하기
    editorUpdateRow(row);
    E.dirty++; //파일이 수정되었음을 표시
}

int editorRowCxToRx(erow* row, int cx) //커서 x 위치를 렌더링된 x 위치로 변환
{
    int rx = 0;
    int j;
    for (j = 0; j < cx; j++) //cx만큼 쭉 돌기
    {
        if (row->chars[j] == '\t')
            rx += (KILO_TAB_STOP - 1) - (rx % KILO_TAB_STOP); //탭 문자를 만나면 탭 스톱까지 공백을 채운 것을 감안하여 커서 위치 조정
        rx++;
    }
    return rx;
}

void editorDelChar() //커서 위치에 따른 문자 삭제
{
    if (E.cy == E.numrows) //커서가 마지막 줄 다음에 있으면 아무것도 하지 않음
        return;

    if (E.cx == 0 && E.cy == 0) //커서가 맨 앞에 있으면 아무것도 하지 않음
        return;

    erow* row = &E.row[E.cy]; //현재 줄 정보 가져오기

    if (E.cx > 0) //커서가 줄 맨 앞이 아니면
    {
        editorRowDelChar(row, E.cx - 1); //현재 줄에서 커서 왼쪽 문자 삭제
        E.cx--; //커서 왼쪽으로 이동
    }
    else //커서가 줄 맨 앞에 있으면
    {
        E.cx = E.row[E.cy - 1].size; //윗줄의 맨 끝으로 커서 이동

        editorRowAppendString(&E.row[E.cy - 1], row->chars, row->size); //윗줄에 현재 줄 문자열 추가
        editorDelRow(E.cy); //현재 줄 삭제
        
        E.cy--; //커서를 윗줄로 이동
    }
    E.dirty++; //파일이 수정되었음을 표시
}

void editorInsertChar(int c) //커서 위치에 따른 문자 삽입
{
    if (E.cy == E.numrows) //커서가 마지막 줄 다음에 있으면 새 줄 추가
    {
        editorAppendRow("", 0);
    }

    editorRowInsertChar(&E.row[E.cy], E.cx, c); //현재 줄에 문자 삽입
    E.cx++; //커서 오른쪽으로 이동
    E.dirty++; //파일이 수정되었음을 표시
}

void editorFind() //문자열 찾기 기능
{
    int saved_cx = E.cx;
    int saved_cy = E.cy;
    int saved_coloff = E.coloff;
    int saved_rowoff = E.rowoff;

    //검색어 입력
    char *query = editorPrompt("찾기: %s (ESC로 취소)");

    if (query == NULL) //취소되었으면 커서를 돌려놓고 종료
    {
        E.cx = saved_cx;
        E.cy = saved_cy;
        E.coloff = saved_coloff;
        E.rowoff = saved_rowoff;
        return;
    }

    int i;
    for (i = 0; i < E.numrows; i++) 
    {
        erow *row = &E.row[i];
        char *match = strstr(row->chars, query);
        if (match != NULL) //성공적으로 찾았다면
        {
            E.cy = i;
            E.cx = match - row->chars;
            E.rowoff = E.numrows; //화면 갱신을 위한 트릭

            free(query);
            return;
        }
    }

    //못 찾았다면 메시지 띄우기
    editorSetStatusMessage("'%s'을/를 찾을 수 없습니다", query);
    free(query);
}