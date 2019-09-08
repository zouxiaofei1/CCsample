//为了减短GUI.cpp长度
//多数和Class不相关的函数都被放到这里

#pragma once

#include "resource.h"

#define BUTTON_IN(x,y) if(x == Hash(y))
#define Delta 10 //按钮渐变色速度

constexpr auto MAX_BUTTON = 50;
constexpr auto MAX_CHECK = 20;
constexpr auto MAX_FRAME = 15;
constexpr auto MAX_TEXT = 25;
constexpr auto MAX_EDIT = 10;
constexpr auto MAX_LINE = 20;
constexpr auto MAX_STRING = 90;
constexpr auto MAX_AREA = 5;
constexpr auto MAX_EXPLENGTH = 200;
constexpr auto MAX_EXPLINES = 15;