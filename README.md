# MY-NANO TEXT EDITOR
C언어로 직접 구현해 본 Linux 기반 Text Editor입니다.

## 실행 영상
(추후 첨부)

## 주요 기능
* 파일 입출력 : 파일을 열고 수정 내용 저장 가능
* 검색 기능 : 텍스트 내 단어 검색 및 커서 이동
* 편집 기능 : Enter, Backspace 등 텍스트 편집 기능 지원
* 상태 표시/메시지 : 커서 위치, 수정 여부를 상태 표시줄을 통해 표시

## 실행 방법 (Linux / WSL)

이미 리눅스 환경이거나 Windows 환경에서 **WSL** 환경을 구축한 경우, 아래 명령어로 즉시 실행이 가능합니다.

### 소스 코드 다운로드
```bash
git clone (주소)
cd my-nano
```

### 컴파일
```bash
make
```

### 실행
```bash
./mynano (파일 경로)
```

## Windows 사용자를 위한 실행 가이드
본 시스템은 Linux System call을 사용하므로 Windows에서 직접 실행이 불가능합니다.<br>
**WSL(Windows Subsystem for Linux)** 설치 방법을 따라하시면 Windows에서도 실행이 가능합니다.

### 1. WSL 설치 (한 번만)
1. PowerShell을 관리자 권한으로 실행합니다.
2. 다음 명령어를 입력합니다.
```bash
wsl --install
```
3. 설치가 완료되면 컴퓨터를 재부팅합니다.
4. 재부팅 후 WSL을 부팅하면 뜨는 창에서 `Username`과 `Password`를 설정하면 설치가 완료됩니다.

### 2. 필수 도구 설치
1. 새로 설치된 WSL 창에 아래 명령어를 입력해 `gcc`과 `make`를 설치합니다.
```bash
sudo apt update
sudo apt install build-essential
```

### 3. 소스 코드 다운로드 및 실행
이후 위에 설명한 "**실행 방법**"에 따라 명령어를 입력하여 에디터를 실행할 수 있습니다.