# myShell

**Contents**  
- [myShell](#myshell)
  - [About Makefile](#about-makefile)
  - [Potential Problem](#potential-problem)
- [myShell\_zh-cn](#myshell_zh-cn)
  - [关于 Makefile](#关于-makefile)
  - [潜在问题](#潜在问题)

This is a simple shell program that mimics the bash, and written by C. No third-party libraries other than those provided by linux are used.

After compiling and running the target file, entering `help` commands will show all the commands that the shell supports.

_The source code have been packaged in .tar file._

## About Makefile
There are some functions in the Makefile, the main is:

1. The intermediate files will be built into "build/" directory (it'll cause an error if there isn't the dir, but you can automatically create the missing directory with `make new`)

2. The target file will be built in the same directory as Makefile

3. Use `make pack` to easily package the entire project directory into a .tar file in the current directory. The default name of the file is the name of the current directory. You can respecify the file name with `make PATH_NAME=the_name_what_you_want pack`.

See the source code for more details.

## Potential Problem
Recursive instruction operations are not supported.

- - -

# myShell_zh-cn
一个简易的 shell 程序，复现了 bash 中的几条简单指令. 由 C 编写，且没有使用第三方库，linux 下的系统 C 库除外.

编译好程序并运行后，输入 `help` 指令会显示这个 shell 中的所有功能指令.

_源码已经被打包进了 .tar 文件中._

> 这个 shell 的特色是用了比较 OOP 的方式编写 C 程序. 不过说实话比较没用，只是应付课程实验倒是够了.

## 关于 Makefile
Makefile 里被塞进了几个功能，主要是：
1. 将中间文件生成在 build 目录中（所以没有这个目录就会报错，不过可以用 `make new` 自动补充缺失的目录）
2. 目标文件的生成位置与 Makefile 在同一个目录中
3. 使用 `make pack` 可以方便的把整个工程目录打包到当前目录的 .tar 文件中，文件的默认命名是当前目录名. 可以用 `make PATH_NAME=myShell pack` 重新指定生成的 .tar 文件名称.

详细可以去查看 Makefile 源码.

## 潜在问题
不支持递归指令操作.
