#ifndef NANOROW_H
#define NANOROW_H

#include "nanodata.h"

/*Low-level*/
void editorUpdateRow(erow* row); //렌더링된 문자열을 만드는 함수
void editorAppendRow(char* s, size_t len); //한 줄 추가
void editorInsertRow(int at, char* s, size_t len); //특정 위치에 줄 삽입
void editorInsertNewLine(); //커서 위치에 새 줄 삽입

void editorRowDelChar(erow* row, int at); //한 줄에서 문자 삭제
void editorRowAppendString(erow* row, char* s, size_t len); //한 줄에 문자열 추가
void editorDelRow(int at); //특정 위치의 줄 삭제

void editorRowInsertChar(erow* row, int at, int c); //한 줄에 문자 삽입

/*Utiility*/
int editorRowCxToRx(erow* row, int cx); //커서 x 위치를 렌더링된 x 위치로 변환
void editorFind(); //문자열 찾기 기능

/*High-level*/
void editorDelChar(); //커서 위치에 따른 문자 삭제
void editorInsertChar(int c); //커서 위치에 따른 문자 삽입

#endif