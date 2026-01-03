#include "nanofile.h"
#include "nanodata.h"
#include "nanorow.h" //editorAppendRow
#include "nanooutput.h"
#include <stdio.h>
#include <fcntl.h>

void editorOpen(char* filename) //파일 열기
{
    free(E.filename);
    E.filename = strdup(filename); //파일 이름 저장

    FILE* fp = fopen(filename, "r");
    if (fp == NULL)
        die("fopen");

    char* line = NULL; //파일에서 입력받은 줄의 문자열 버퍼
    size_t linecap = 0; //입력받은 줄의 문자열 버퍼 크기
    ssize_t linelen; //입력받은 줄의 실제 길이
    
    while ((linelen = getline(&line, &linecap, fp)) != -1)
    {
        //줄 끝의 개행 문자 제거(파일에는 기본적으로 한 줄마다 개행 문자가 있기 때문)
        while (linelen > 0 && (line[linelen - 1] == '\n' || line[linelen - 1] == '\r'))
            linelen--;

        editorAppendRow(line, linelen); //그 줄 추가
    }

    //정리
    free(line);
    fclose(fp);
    E.dirty = 0; //파일이 열렸으므로 초기화
}

char* editorRowsToString(int* buflen) //에디터의 모든 줄을 하나의 문자열로 변환
{
    int totlen = 0; //총 길이
    int j;

    for (j = 0; j < E.numrows; j++)
        totlen += E.row[j].size + 1; //줄 길이 + 개행 문자 길이

    *buflen = totlen;

    char* buf = (char*)malloc(totlen); //총 길이만큼 메모리 할당
    char* p = buf; //버퍼 포인터

    for (j = 0; j < E.numrows; j++)
    {
        memcpy(p, E.row[j].chars, E.row[j].size); //해당 줄의 문자들을 버퍼에 복사
        p += E.row[j].size;
        *p = '\n'; //개행 문자 추가
        p++;
    }

    return buf;
}

void editorSave() //파일 저장
{
    if (E.filename == NULL) //파일 이름이 없으면 저장하지 않음
        return;

    int len;
    char* buf = editorRowsToString(&len); //에디터의 모든 줄을 하나의 문자열로 변환

    int fd = open(E.filename, O_RDWR | O_CREAT, 0644); //파일 열기(없으면 생성)

    if (fd != -1)
    {
        ftruncate(fd, len); //파일 크기 조절
        write(fd, buf, len); //버퍼 내용을 파일에 쓰기
        close(fd);
        free(buf); //버퍼 메모리 해제
        E.dirty = 0; //파일이 저장되었으므로 초기화
        editorSetStatusMessage("%d 바이트가 기록되었습니다.", len); //상태 메시지 설정
        return;
    }

    free(buf); //버퍼 메모리 해제
    editorSetStatusMessage("파일을 저장할 수 없습니다! I/O error : %s", strerror(errno)); //상태 메시지 설정
}