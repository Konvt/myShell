#ifndef __FNT_FONT_STYLE__
  #define __FNT_FONT_STYLE__

// 字体色
#define FNT_RESET   "\033[0m"  // 重置所有效果
#define FNT_BLACK   "\033[30m" // 黑
#define FNT_RED     "\033[31m" // 红
#define FNT_GREEN   "\033[32m" // 绿
#define FNT_YELLOW  "\033[33m" // 黄
#define FNT_BLUE    "\033[34m" // 蓝
#define FNT_MAGENTA "\033[35m" // 紫
#define FNT_CYAN    "\033[36m" // 青
#define FNT_WHITE   "\033[37m" // 白

// 背景色
#define FNT_BG_BLACK   "\033[40m" // 背景黑
#define FNT_BG_RED     "\033[41m" // 背景红
#define FNT_BG_GREEN   "\033[42m" // 背景绿
#define FNT_BG_YELLOW  "\033[43m" // 背景黄
#define FNT_BG_BLUE    "\033[44m" // 背景蓝
#define FNT_BG_MAGENTA "\033[45m" // 背景紫
#define FNT_BG_CYAN    "\033[46m" // 背景青
#define FNT_BG_WHITE   "\033[47m" // 背景白

// 字体效果
#define FNT_BOLD_TEXT      "\033[1m" // 粗体
#define FNT_ITALIC_TEXT    "\033[3m" // 斜体
#define FNT_UNDERLINE_TEXT "\033[4m" // 下划线
#define FNT_REVERSE_TEXT   "\033[7m" // 反色
#define FNT_HIDE_TEXT      "\033[8m" // 隐藏

// 移动光标
#define FNT_CLS          "\033[2J" // 清屏
#define FNT_RESET_CURSOR "\033[H"  // 重置光标位置

#define STYLIZE(STR, COL) COL STR FNT_RESET // 只接受字面量字符串

#endif
