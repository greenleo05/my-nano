#ifndef NANOOUTPUT_H
#define NANOOUTPUT_H

#include "nanodata.h"

void editorScroll(); //스크롤 조정
void editorDrawRows(); //줄 그리기
void editorRefreshScreen(); //화면 업데이트
void editorSetStatusMessage(const char* fmt, ...); //상태 메시지 설정
void editorDrawStatusBar(); //상태 바 그리기
void editorDrawMessageBar(); //메시지 바 그리기
char* editorPrompt(char* prompt); //프롬프트 표시 및 입력 받기

#endif