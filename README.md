# ![df_logo](https://user-images.githubusercontent.com/72431617/222708775-4f31c252-9cb0-435d-8104-2271c4d8f711.png)</br>df-steam-hook-kr

[![XMake](https://github.com/dfint/df-steam-hook/actions/workflows/xmake.yml/badge.svg)](https://github.com/dfint/df-steam-hook/actions/workflows/xmake.yml)
[![clang-format Check](https://github.com/dfint/df-steam-hook/actions/workflows/clang-format-check.yml/badge.svg)](https://github.com/dfint/df-steam-hook/actions/workflows/clang-format-check.yml)

## 기본 기능:

- 드워프 포트리스 글자를 가로채서 dfint_dictionary.csv 와 kr_regex.txt를 참고해서 글자를 그림으로 생성 후에 화면에 뿌립니다.

## 추가 기능:

- 게임 충돌 할 때 `dfint_data\crash_reports`폴더에 (`cr_*.txt` files) 파일 생성됩니다
- DF의 버전 별로 동시에 지원할 수 있으며, `dfint_data/offset` 디렉토리에 구성 파일을 추가하여 새 버전을 추가할 수 있습니다.

- 단축키:
  - <kbd>Ctrl</kbd>+<kbd>F2</kbd> - csv 파일 다시 읽기
  - <kbd>Ctrl</kbd>+<kbd>F3</kbd> - 번역 끄기
  - <kbd>Ctrl</kbd>+<kbd>F4</kbd> - 번역 켜기
  - <kbd>Ctrl</kbd>+<kbd>F5</kbd> - 번역 파일 다시읽기

- `dfint_config.toml` 파일 내용을 수정해서 폰트, 사이즈 조절 가능</br><img src="https://user-images.githubusercontent.com/72431617/222711176-b8b9ceee-d0ad-40e7-86f3-27c7a2f1ee19.jpg" height="200"/>
-`log_level` - 위의 번호에 따라서 `dfint_data`폴더에 `dfint_log.log`파일 로그 내용이 달라집니다.
-`crash_report` - 크래쉬 파일 생성 유무
-`enable_search` - 한글버전에선 사용 안합니다 상관없음
-`font_name` - 폰트설정
-`font_size` - 폰트 크기를 설정합니다
-`font_shiftup` - 폰트 위치를 숫자만큼 올립니다
-`font_flagup` - 탭 글자 반 위쪽의 위치를 수정합니다
-`font_flagdown` - 탭 글자 반 아래쪽의 위치를 수정합니다

## 추가 사항:

- 아직 많이 부족합니다만 번역에 조금이라도 도움이 되고자 업로드 합니다.





