# myShell

**Contents**  
- [myShell](#myshell)
- [myShell](#myshell-1)
  - [关于 Makefile](#关于-makefile)
  - [潜在问题](#潜在问题)

# myShell
一个简易的 shell 程序，复现了 bash 中的几条简单指令. 由 C 编写，且没有使用第三方库，linux 下的系统 C 库除外.

编译好程序并运行后，输入 `help` 指令会显示这个 shell 中的所有功能指令.

> 这个 shell 的特色是用了比较 OOP 的方式编写 C 程序. 不过说实话比较没用，只是应付课程实验倒是够了.

## 关于 Makefile
Makefile 里被塞进了几个功能，主要是：
1. 将中间文件生成在 build 目录中
2. 目标文件的生成位置与 Makefile 在同一个目录中
3. 根据源文件的依赖关系，在代码有变动时自动重编译部分文件而不是重编译整个项目
4. 添加了一个名称为 `debug` 的伪目标，使用 `make debug` 即可使用 `gdb` 开启项目的 tui 调试界面

详细可以去查看 Makefile 源码.

> 什么年代了还在用传统 Makefile？

## 潜在问题
不支持递归指令操作.

调用一些外部指令时（如 gcc 编译文件）会提示找不到输入文件.

比较少见的情况下会吃掉软件的输出提示（如在 shell 中直接输入 apt）.
