#include "nanodata.h"

void die(const char* s) //오류 처리 함수
{
    //오류 메시지(stderr) 출력 후 종료
    perror(s);
    exit(1);
}
