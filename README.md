# ShitVM
<img width=256 height=66 align="right" src="https://github.com/ShitVM/ShitVM/blob/master/docs/logo/ShitVM.svg">

가벼운 가상머신

## 컴파일
```
$ git clone https://github.com/ShitVM/ShitVM.git
$ cd ShitVM
$ cmake CMakeLists.txt
$ make
```

## 사용법
```
$ cd bin
$ ./ShitVM <입력: ShitVM 바이트 파일> [명령줄 옵션...]
```

### 명령줄 옵션
- `--version`<br>ShitVM 버전을 확인합니다.
- `-f<플래그>`<br>플래그를 활성화합니다.
- `-fno-<플래그>`<br>플래그를 비활성화합니다.
- `-<변수>=<값>`<br>변수의 값을 설정합니다.

### 플래그 목록
|이름|기본값|설명|
|:-:|:-:|:-|
|`gc`|활성화|관리되는 메모리 영역을 사용할지 설정합니다. 비활성화 할 경우 관리되는 메모리 영역에 메모리를 할당할 수 없습니다. 대신 ShitVM 초기화 성능 및 메모리 사용량이 개선될 수 있습니다.|

### 변수 목록
|이름|기본값|설명|
|:-:|:-:|:-|
|`stack`|1048576|스택의 크기를 바이트 단위로 설정합니다. 0일 수 없으며, 1024 이상으로 설정하는 것을 권장합니다.|
|`young`|8388608|Young Generation의 블록 크기를 바이트 단위로 설정합니다. 0일 수 없으며, 512의 배수여야 합니다.|
|`old`|33554432|Old Generation의 최소 블록 크기를 바이트 단위로 설정합니다. 0일 수 없으며, 512의 배수여야 합니다.|

## [문서](https://github.com/ShitVM/ShitVM/tree/master/docs)

## 관련된 레포지토리
- **[ShitAsm](https://github.com/ShitVM/ShitAsm):** ShitBC 어셈블러
- **[ShitGen](https://github.com/ShitVM/ShitGen):** ShitVM 바이트 파일 생성기