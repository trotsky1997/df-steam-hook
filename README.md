# ![df_logo](https://user-images.githubusercontent.com/72431617/222708775-4f31c252-9cb0-435d-8104-2271c4d8f711.png)</br>df-steam-hook-kr

[![XMake](https://github.com/dfint/df-steam-hook/actions/workflows/xmake.yml/badge.svg)](https://github.com/dfint/df-steam-hook/actions/workflows/xmake.yml)
[![clang-format Check](https://github.com/dfint/df-steam-hook/actions/workflows/clang-format-check.yml/badge.svg)](https://github.com/dfint/df-steam-hook/actions/workflows/clang-format-check.yml)

## 使用方法:
[Korean](https://github.com/trotsky1997/df-steam-hook/blob/main/README.md)|[English](https://github.com/dfint/df-steam-hook/blob/main/README.md)|简体中文

- [RELEASE](https://github.com/Kheeman/df-steam-hook/releases) 从游戏中获取`dfint_release.zip`， `font.DLL.zip`文件，并将其压缩到游戏的顶级文件夹中。解压后启动`dfint_launcher.exe`。

![新建 BMP 图像](https://user-images.githubusercontent.com/30512160/226165993-e7628434-cb5e-4b0d-ac74-0365113ff1bb.png)

## 工作方式:

- 截取矮人要塞字符，参考dfint_dictionary.csv和kr_regex.txt将字符生成图片并铺在屏幕上。

### 附加功能:

### 模糊匹配
改进匹配规则，增加词条词库命中率
### 机器翻译和缓存
自适应处理游戏内随机生成文本，使用过程需要联网。
### 改进汉语语序正则

- 当游戏崩溃时，会在 dfint_data\crash_reports 文件夹中创建一个文件（ cr_*.txt 文件）。
- DF的各个版本可以同时支持，可以通过在 dfint_data/offset 目录下添加配置文件来添加新的版本。

- 단축키:
  - <kbd>Ctrl</kbd>+<kbd>F2</kbd> - 再次读取csv文件
  - <kbd>Ctrl</kbd>+<kbd>F3</kbd> - 关闭翻译
  - <kbd>Ctrl</kbd>+<kbd>F4</kbd> - 打开翻译
  - <kbd>Ctrl</kbd>+<kbd>F5</kbd> - 重读翻译文件

- `dfint_config.toml` 可以通过修改文件内容来调整字体和大小</br></br> <img src="https://user-images.githubusercontent.com/72431617/222711176-b8b9ceee-d0ad-40e7-86f3-27c7a2f1ee19.jpg" height="200"/></br>

  - `log_level` - 根据上面的数字， dfint_data 文件夹中的 dfint_log.log 文件日志的内容不同。
  - `crash_report` - 是否创建崩溃文件
  - `enable_search` - 缺省设置
  - `font_name` - 字体设置
  - `font_size` - 设置字体大小
  - `font_shiftup` - 将字体锚点增加一个数字
  - `font_flagup` - 固定制表符在上半部分的位置
  - `font_flagdown` - 固定制表符中间的位置





## 如何参与汉化项目
- 参与翻译

在[transifex](https://www.transifex.com/home/)官网注册账号并登陆后，打开[项目地址](https://www.transifex.com/dwarf-fortress-translation/dwarf-fortress-steam/translate/#zh-Hans/$/444386763?q=translated%3Ano)参与词条汉化、润色。

- 参与[汉化语序正则](https://github.com/trotsky1997/df-steam-hook/blob/main/config/kr_regex.txt)编写

- 参与项目代码改进
本地编译方法见[链接](https://github.com/trotsky1997/df-steam-hook/edit/main/README.md##构建)

- 报告项目错误或提出功能提议

- 赞助项目
 ![新建 BMP 图像](https://user-images.githubusercontent.com/30512160/226165993-e7628434-cb5e-4b0d-ac74-0365113ff1bb.png)

- 参与社区讨论


## 构建
首先，安装`Scoop`,安装`Visual Studio`和`MSVC`.使用`scoop`安装`Xmake`, `vcpkg`
```
scoop install xmake
scoop install vcpkg
```
设置`Xmake` .
`set_ targetdir("c:/Users/aka/Desktop/Game/")`中目录设 置为你的游戏根目录。

相应的检查你的Windows kit安装地址和Vcpkg安装地址，设置包含目录。
```
add_ linkdirs("C:\Users\\aka\\scoop\ \apps\\vcpkg\ \current\\installed\ \x64-windows\\lib")
add_ linkdirs("C:\\Program Files (x86)\\Windows Kits\10\\Lib\\10. 0.22000.0\ \um\\x64" )
add_ _includedirs("C:\\Program Files (x86)\Windows Kits \\10\\Include\\10.0.22000.0\\um")
add_ _includedirs("C:\\Users\\aka\\scoop\\apps\\vcpkg\ \current\ \installed\\x64-windows \\include")
```
在项目根目录执行`xmake` ,进行构建。

## 问题:
- 还有很多问题，这里有一些缓解方案。
- 游戏中的基本翻译过程可以追溯到阅读游戏内文本-> 比较翻译文件-> 为找到的翻译文本创建图片-> 将翻译图片放在原始位置。由于游戏不断地在屏幕上展开整个文本，当滚动时，出现新窗口或内容时，阅读游戏文本框时，顺序会被扭曲，有时无法正确找到翻译。在这种情况下，敲击<kbd>Ctrl</kbd>+<kbd>F5</kbd> 刷新使您可以再次阅读。
- 更新画面有问题，所以有时会因为翻译图片没有正常更新而导致文字断掉。在这种情况下，如果您切换窗口、在游戏中的楼层之间移动（鼠标滚轮）以及使用鼠标显示工具提示，它就会正常显示。

## 编辑翻译：
- 翻译网站采用游戏文字原样，所以字母没有排列。实际游戏中显示的文字需要分为单词、短句和长句，由于句子是单词组合而成的，所以需要使用 kr_regex.txt 文件进行修改。
 > 修改示例
 >
 > <img src="https://user-images.githubusercontent.com/72431617/222723315-475781df-a234-43ee-b9f8-583f3020a553.jpg" height="100"/>
 > <img src="https://user-images.githubusercontent.com/72431617/222723488-21861a7a-8836-4c30-be72-fa974871b715.jpg" height="100"/>
 >
 > 这是游戏中的翻译和弦乐。确认后，检查 dfint_dictionary.csv 文件和 kr_regex.txt 文件。
 >
 > <img src="https://user-images.githubusercontent.com/72431617/222725573-285e45c1-2722-412c-84f8-d14805afc82f.jpg" height="200"/>
 >
 >看了一下，应用了kr_regex.txt文件中的 {s,} her {s,} ，奇怪了。搜索译文时，除以 , . 줄 的条件即可找到。在这种情况下， $2의 $1 -> culture, she holds의 다른사람들처럼 被识别为一个句子，下一行是另一个句子，所以没有找到craftsdwarfship~，所以输出为英文。相应地修改它。
 >
 > <img src="https://user-images.githubusercontent.com/72431617/222729741-3dbf5380-5a9d-4a6c-9cf9-dadfec5f67bb.jpg" height="200"/>
 >
 > 参考句子结构和csv翻译后的内容添加到`kr_regex.txt'。保存文件并用<kbd>Ctrl</kbd>+<kbd>F5</kbd>确认。
 >
 > <img src="https://user-images.githubusercontent.com/72431617/222730805-4cd71d6d-580e-46c6-a689-387f2a4bb7db.jpg" height="100"/>
 >
 > 没有文化名称。将其添加到 kr_regex.txt 或 dfint_dictionary.csv 并再次检查。
 >
 > <img src="https://user-images.githubusercontent.com/72431617/222731714-af984d39-32b3-4a8a-af00-6e5cf633621e.jpg" height="100"/>
 >
 > 您现在已经完成了编辑过程。
- kr_regex.txt 文件中的正则表达式是按顺序读取的。因此，如果应用范围较大，可以将其放在下面或切成小块。比如有一个 She is dragon. 的句子，如果上面有 She {s,} ，下面有 {s,} is dragon. ，那么先应用上面的，剩下 is dragon. ，那么 {s,} is dragon. 就无法识别。如果你再拆分成 {s,} dragon. ，它会被识别，但是如果你把它拆分成这样的小块，很难决定翻译，因为 he are dragon, she was dragon... 中的各种东西都被捕获了。您可以在 dfint_dictionary.csv 文件中检查它是否正确并进行更正。
- 增加了韩国研究自动选择功能。 (이)가|(와)과|(을)를|(은)는|(아)야|(이)여|(으)로|(이)라 如果你把它以这种形式放入翻译文件中，它会自动出来。例如， 드워프(은)는 키가 작다. 被替换为 드워프는 키가 작다. 并打印出来。它也适用于正则表达式，所以当游戏中有组合语句时它会很有用。

