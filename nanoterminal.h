#ifndef NANOTERMINAL_H
#define NANOTERMINAL_H

void disableRawMode(); //Raw 모드 비활성화 함수
void enableRawMode(); //Raw 모드 활성화 함수
int editorReadKey(); //키 입력 읽기
int getWindowSize(int* rows, int* cols); //시작했을 때 창 크기 가져오기

#endif