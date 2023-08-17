<div align="center">
	<img src="https://github.com/ShitVM/ShitVM/blob/master/docs/logo/Logo.png" />
</div>

---

가벼운 가상머신

## 컴파일
```
$ git clone https://github.com/ShitVM/ShitVM.git --recurse-submodules
$ cd ShitVM
$ cmake --config --build .
```

## 사용법
```
$ cd bin
$ ./ShitVM <입력: ShitVM 바이트 파일 경로> [명령줄 옵션...]
```

### 명령줄 옵션
#### 일반
- `--version`<br>ShitVM의 버전을 확인합니다.
- `--dump-bytefile`<br>불러온 ShitVM 바이트 파일의 내용을 출력합니다.

#### 의존성
- `-L<디렉터리 경로>`<br>라이브러리 디렉터리를 추가합니다.

#### 메모리
- `-stack=<크기>`<br>스택의 크기를 바이트 단위로 설정합니다. 기본값은 1048576입니다. 0일 수 없으며, 1024 이상으로 설정하는 것을 권장합니다.
- `-fno-gc`<br>관리되는 메모리 영역을 사용하지 않도록 설정합니다. 이 옵션을 사용할 경우, 관리되는 메모리 영역에 메모리를 할당할 수 없게 됩니다. 대신 ShitVM 초기화 성능 및 메모리 사용량이 개선될 수 있습니다.
- `-young=<크기>`<br>관리되는 메모리 영역 중 Young Generation의 블록 크기를 바이트 단위로 설정합니다. 기본값은 8388608입니다. 0일 수 없으며, 512의 배수여야 합니다.
- `-old=<크기>`<br>관리되는 메모리 영역 중 Old Generation의 블록 크기를 바이트 단위로 설정합니다. 기본값은 33554432입니다. 0일 수 없으며, 512의 배수여야 합니다.

## [문서](docs/README.md)

## 관련된 레포지토리
- **[ShitAsm](https://github.com/ShitVM/ShitAsm):** ShitBC 어셈블러
- **[ShitGen](https://github.com/ShitVM/ShitGen):** ShitVM 바이트 파일 생성기

## 사용된 외부 라이브러리
- [UTF8-CPP](https://github.com/nemtrif/utfcpp)