#include "nanofile.h"
#include "nanodata.h"
#include "nanorow.h" //editorAppendRow
#include <stdio.h>

void editorOpen(char* filename) //파일 열기
{
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
}
