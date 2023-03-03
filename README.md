# ![df_logo](https://user-images.githubusercontent.com/72431617/222708775-4f31c252-9cb0-435d-8104-2271c4d8f711.png)</br>df-steam-hook-kr

[![XMake](https://github.com/dfint/df-steam-hook/actions/workflows/xmake.yml/badge.svg)](https://github.com/dfint/df-steam-hook/actions/workflows/xmake.yml)
[![clang-format Check](https://github.com/dfint/df-steam-hook/actions/workflows/clang-format-check.yml/badge.svg)](https://github.com/dfint/df-steam-hook/actions/workflows/clang-format-check.yml)

## 설치 방법:

- [RELEASE](https://github.com/dfint/df-steam-hook/releases) 에서 `dfint_release.zip` , `font.DLL.zip` 파일을 받아서 게임 최상위 폴더에 압축을 풉니다. 압축을 풀고 나서 `dfint_launcher.exe`를 실행합니다.

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

- `dfint_config.toml` 파일 내용을 수정해서 폰트, 사이즈 조절 가능</br></br> <img src="https://user-images.githubusercontent.com/72431617/222711176-b8b9ceee-d0ad-40e7-86f3-27c7a2f1ee19.jpg" height="200"/></br>

  - `log_level` - 위의 번호에 따라서 `dfint_data`폴더에 `dfint_log.log`파일 로그 내용이 달라집니다.
  - `crash_report` - 크래쉬 파일 생성 유무
  - `enable_search` - 한글버전에선 사용 안합니다 상관없음
  - `font_name` - 폰트설정
  - `font_size` - 폰트 크기를 설정합니다
  - `font_shiftup` - 폰트 위치를 숫자만큼 올립니다
  - `font_flagup` - 탭 글자 반 위쪽의 위치를 수정합니다
  - `font_flagdown` - 탭 글자 반 아래쪽의 위치를 수정합니다

## 문제점:
- 아직 많은 문제가 있습니다 몇가지 해결방법을 올립니다.
- 게임에서 기본적인 번역 절차가 게임 내 텍스트 읽기 -> 번역 파일 비교 -> 찾은 번역 글자 그림 생성 -> 원래 위치에 번역그림 넣기 순으로 돌아갑니다. 게임에서 지속적으로 화면 전체 텍스트를 뿌리기 떄문에 스크롤할때나 새로운 창, 내용이 나타낼때 게임 텍스트틀 읽을 때 순서가 꼬여서 번역을 제대로 못찾을 때가 있습니다. 그럴땐 <kbd>Ctrl</kbd>+<kbd>F5</kbd> 다시 읽기 하시면 됩니다.
- 화면갱신의 문제가 있어 번역 그림을 제대로 갱신이 안되서 글자가 깨질때가 있습니다. 그런 경우 윈도우 창 전환, 게임 내 층간 이동(마우스 휠), 마우스로 툴팁표시를 하시면 제대로 나옵니다.

## 번역 수정하기:
- 번역사이트는 게임 텍스트를 그대로 가져와서 글자가 정돈이 안되어 있습니다. 실제 게임 상에서 출력되는 텍스트를 단어, 짧은 문장, 긴 문장 구분을 해야되고 단어를 조합해서 문장을 만들기 때문에 `kr_regex.txt`파일을 이용해서 수정을 해야 됩니다.
 > 수정 예시
 >
 > <img src="https://user-images.githubusercontent.com/72431617/222723315-475781df-a234-43ee-b9f8-583f3020a553.jpg" height="100"/>
 > <img src="https://user-images.githubusercontent.com/72431617/222723488-21861a7a-8836-4c30-be72-fa974871b715.jpg" height="100"/>
 >
 > 여기 게임에서 번역 된것과 끈것이 있습니다. 확인을 한 후 `dfint_dictionary.csv` 파일과 `kr_regex.txt` 파일을 확인 합니다.
 >
 > <img src="https://user-images.githubusercontent.com/72431617/222725573-285e45c1-2722-412c-84f8-d14805afc82f.jpg" height="200"/>
 >
 > 살펴보면 kr_regex.txt 파일의 `{s,} her {s,}`가 적용 되어 이상하게 되었습니다. 번역 텍스트를 찾을 때 `,` `.` `줄` 기준으로 구분해서 찾습니다. 이 경우 `$2의 $1`->`culture, she holds의 다른사람들처럼`으로 되어서 한문장으로 인식하고 다음 줄은 다른 문장으로 되서 craftsdwarfship~ 은 찾지 못해서 영어로 출력 되었습니다.
 > 이것을 적절히 수정해 봅니다.
 >
 > <img src="https://user-images.githubusercontent.com/72431617/222729741-3dbf5380-5a9d-4a6c-9cf9-dadfec5f67bb.jpg" height="200"/>
 >
 > 문장의 구조와 csv 번역된 내용을 참고해서 `kr_regex.txt'에 추가합니다. 파일을 저장하고 <kbd>Ctrl</kbd>+<kbd>F5</kbd>로 확인을 해봅니다.
 >
 > <img src="https://user-images.githubusercontent.com/72431617/222730805-4cd71d6d-580e-46c6-a689-387f2a4bb7db.jpg" height="100"/>
 >
 > culture가 없었네요. `kr_regex.txt` , `dfint_dictionary.csv` 둘중 하나에 추가시켜 놓고 다시 확인을 해봅니다.
 >
 > <img src="https://user-images.githubusercontent.com/72431617/222731714-af984d39-32b3-4a8a-af00-6e5cf633621e.jpg" height="100"/>
 >
 > 이제 수정을 완료 했습니다. 
- `kr_regex.txt`파일의 정규식은 순서대로 읽습니다. 그래서 큰 범위가 적용 되는 것은 가급적 밑에 두시거나 잘게 쪼게시면 됩니다. 예를 들어 `She is dragon.` 의 문장이 있다면 위쪽에 `She {s,}` 가 있고 아래쪽에 `{s,} is dragon.` 순서로 되어 있으면 위쪽이 먼저 적용되어 `is dragon.` 이 남아서 `{s,} is dragon.`은 인식이 안됩니다. 쪼개서 다시 `{s,} dragon.` 하면 인식이 되나 이렇게 잘게 쪼개 버리면 `he are dragon, she was dragon...` 온갖 경우의 것들이 다 잡히기 때문에 번역을 정하기가 힙듭니다. 적절히 `dfint_dictionary.csv` 파일에 어떻게 되어 있는지 확인하고 수정하시면 됩니다.
- 한글 조사 자동 선택 기능이 추가 되었습니다. `(이)가|(와)과|(을)를|(은)는|(아)야|(이)여|(으)로|(이)라` 이 형태로 번역파일에 넣으면 알아서 선택해서 나옵니다. 예들들어 `드워프(은)는 키가 작다.` 를 `드워프는 키가 작다.` 로 바뀌어 출력됩니다. regex에도 적용 되어서 게임 내 조합형 문장이 있을 때 유용하게 사용 할 수 있습니다.

## 추가 사항:

- 아직 많이 부족합니다만 번역에 조금이라도 도움이 되고자 업로드 합니다.





