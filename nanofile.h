#ifndef NANOFILE_H
#define NANOFILE_H

#include "nanodata.h"
void editorOpen(char* filename); //파일 열기
char* editorRowsToString(int* buflen); //에디터의 모든 줄을 하나의 문자열로 변환
void editorSave(); //파일 저장

#endif