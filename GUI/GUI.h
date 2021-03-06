//为了减短GUI.cpp长度
//多数和Class不相关的函数都被放到这里

#pragma once

#include "resource.h"

//杂项
#define MAX_STR					256											//默认最大字符串长度
#define error()					Main.InfoBox(L"unk")						//一键报错

//绘图相关
#define Delta					10		//按钮渐变色速度
#define DEFAULT_WIDTH			621		//默认窗口宽度
#define DEFAULT_HEIGHT			549		//默认窗口高度
constexpr float	  MAX_DPI = 1.5;


//定义颜色
#define COLOR_WHITE			RGB(255,255,255)
#define COLOR_BLACK			RGB(0,0,0)
#define COLOR_RED			RGB(255,0,0)
#define COLOR_NORMAL_BLUE	RGB(40, 130, 240)
#define COLOR_LIGHTER_BLUE	RGB(230, 255, 255)
#define COLOR_LIGHT_BLUE	RGB(210, 255, 255)
#define COLOR_BSOD_BLUE		RGB(0,   117, 213)
#define COLOR_OBSOD_BLUE	RGB(1,   0, 0x80)
#define COLOR_LIGHTER_GREY	RGB(243, 243, 243)
#define COLOR_LIGHT_GREY	RGB(220, 220, 220)
#define COLOR_DARKER_GREY	RGB(150, 150, 150)
#define COLOR_NORMAL_GREY	RGB(135, 135, 135)
#define COLOR_DARKEST_GREY	RGB(90,90,90)
#define COLOR_DARK_YELLOW	RGB(193, 205, 205)
#define COLOR_LIGHT_YELLOW	RGB(244, 238, 175)
#define COLOR_TITLE_1		RGB(0x66, 0xCC, 0xFF)
#define COLOR_TITLE_2		RGB(0x70, 0xCF, 0xFF)
#define COLOR_GREEN			RGB(20, 200, 80)
#define COLOR_GREENEST		RGB(0, 255, 0)
#define COLOR_ORANGE		RGB(255, 100, 0)
#define COLOR_LIGHT_ORANGE	RGB(255, 180, 10)
#define COLOR_DARKER_BLUE	0xDA8B01
#define COLOR_CLOSE_LEAVE	RGB(255, 109, 109)
#define COLOR_CLOSE_HOVER	RGB(250, 100, 100)
#define COLOR_CLOSE_PRESS	RGB(232, 95, 95)
#define COLOR_OK			RGB(5, 200, 135)
#define COLOR_RECOMMENDED	RGB(10, 255, 10)
#define COLOR_NOTREC		RGB(0x63, 0xB8, 0xFF)

//定义笔刷
#define InitBrushs \
	WhiteBrush = CreateSolidBrush(COLOR_WHITE);\
	DBlueBrush = CreateSolidBrush(COLOR_LIGHT_BLUE);\
	LBlueBrush = CreateSolidBrush(COLOR_LIGHTER_BLUE);\
	NormalBlueBrush = CreateSolidBrush(COLOR_NORMAL_BLUE);\
	DBluePen = CreatePen(PS_SOLID, 1, COLOR_LIGHT_BLUE);\
	LBluePen = CreatePen(PS_SOLID, 1, COLOR_LIGHTER_BLUE);\
	LGreyBrush = CreateSolidBrush(COLOR_LIGHTER_GREY);\
	DGreyBrush = CreateSolidBrush(COLOR_LIGHT_GREY);\
	DarkGreyPen =CreatePen(PS_SOLID, 1,COLOR_DARKEST_GREY);\
	YellowBrush = CreateSolidBrush(COLOR_LIGHT_YELLOW);\
	TitleBrush = CreateSolidBrush(COLOR_TITLE_1); \
	YellowPen = CreatePen(PS_SOLID, 1, COLOR_DARK_YELLOW);\
	BlackPen = CreatePen(PS_SOLID, 1, COLOR_BLACK);\
	WhitePen = CreatePen(PS_SOLID, 1, COLOR_WHITE);\
	TitlePen = CreatePen(PS_SOLID, 2, COLOR_TITLE_2);\
	CheckGreenPen = CreatePen(PS_SOLID, 2,COLOR_GREEN);\
	NormalGreyPen = CreatePen(PS_SOLID, 1, COLOR_NORMAL_GREY );\
	NormalBluePen = CreatePen(PS_SOLID, 1,COLOR_NORMAL_BLUE );\
	BSODBrush=CreateSolidBrush(COLOR_BSOD_BLUE);\
	BSODPen=CreatePen(PS_SOLID, 1,COLOR_BSOD_BLUE);\
	BlackBrush=CreateSolidBrush(COLOR_BLACK);

struct GETLAN
{//自定义的一个获取语言文件信息的结构体
	int Left, Top, Width, Height;
	wchar_t* begin, * str1, * str2;
};

//定义最大项目数
#define MAX_BUTTON			45
#define MAX_CHECK			20
#define MAX_FRAME			12
#define MAX_TEXT			30
#define MAX_EDIT			10
#define MAX_LINE			20
#define MAX_STRING			110
#define MAX_AREA			5
#define MAX_EXPLINES		8
#define MAX_EXPLENGTH		61		//每行的Exp最多字符数
#define MAX_EDIT_WIDTH		500		//每个Edit控件的最大宽度(缩放后)
#define MAX_EDIT_HEIGHT		80		//每个Edit控件的最大高度(缩放后)
#define MAX_LANGUAGE_LENGTH	8000	//默认语言文件最大长度

#define REDRAW_FRAME	1
#define REDRAW_BUTTON	2
#define REDRAW_CHECK	3
#define REDRAW_TEXT		4
#define REDRAW_EDIT		5
#define REDRAW_EXP		6

#define BUTTON_IN(x,y) if(x==Hash(y))










void charTowchar(const char* chr, wchar_t* wchar, int size);
void s(LPCWSTR a)//调试用MessageBox
{
	MessageBox(NULL, a, L"", NULL);
}
void s(int a)
{
	wchar_t tmp[34] = { 0 };
	_itow_s(a, tmp, 10);
	MessageBox(NULL, tmp, L"", NULL);
}
//void s2(LPCWSTR a)//调试用OutputDebugString
//{
//	OutputDebugString(a);
//}
//void s2(int a)
//{
//	wchar_t tmp[34] = { 0 };
//	_itow_s(a, tmp, 10); wcscat(tmp, L"\n");
//	OutputDebugString(tmp);
//}
void charTowchar(const char* chr, wchar_t* wchar, int size)
{
	MultiByteToWideChar(CP_ACP,
		0, chr, strlen(chr) + 1,
		wchar,
		size / sizeof(wchar[0]));
}

#pragma warning(disable:4244)


unsigned int Hash(const wchar_t* str)
{
	unsigned int seed = 131;
	unsigned int hash = 0;

	while (*str)
	{
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}
