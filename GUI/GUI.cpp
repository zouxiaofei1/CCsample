#include "stdafx.h"
#include "GUI.h"
#include "WndShadow.h"
#include "TestFunctions.h"

#pragma comment(lib,"Imm32.lib")//自定义输入法位置用的Lib
#pragma warning(disable:4996)

//部分(重要)函数的前向声明
BOOL				InitInstance(HINSTANCE, int);//初始化
LRESULT	CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);//主窗口

//全局变量
HINSTANCE hInst;// 当前实例备份变量，CreateWindow&LoadIcon时需要
const wchar_t szWindowClass[] = L"GUI";

//和绘图有关的全局变量
HBRUSH DBlueBrush, LBlueBrush, WhiteBrush, BlueBrush, green, grey, yellow, Dgrey;//各色笔刷
HPEN YELLOW, RED, BLACK, White, GREEN, GREEN2, LGREY, BLUE, DBlue, LBlue;//笔
HDC hdc, rdc;//主窗口缓冲hdc + 贴图hdc
HBITMAP hBmp, lBmp;//主窗口hbmp
CWndShadow Cshadow;//主窗口阴影特效
BOOL Effect = TRUE;//特效开关
bool slient;

class CathyClass//控件主类
{
public:
	void InitClass(HINSTANCE HInstance)//新Class使用之前最好Init一下
	{								//  （不Init也行）
		hInstance = HInstance;//设置hinst
		CurButton = CurFrame = CurCheck = CurLine = CurText = 0;
		CurWnd = 1;//清"零"
		CurCover = -1;
		CoverCheck = 0;

		//默认宋体
		DefFont = CreateFontW((int)(16 * DPI), (int)(8 * DPI), 0, 0, FW_THIN, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("宋体"));
	}

	inline wchar_t* GetStr(LPCWSTR ID) { return str[Hash(ID)]; }//通过Hash + map 来快速索引字符串的数据结构
	//															ID(索引字符串) -> Hash -(map)> 字符串地址

	void SetStr(LPCWSTR Str, LPCWSTR ID)//通过ID设置字符串
	{
		delete[]str[Hash(ID)];//删除当前ID中原有的字符串
		str[Hash(ID)] = new wchar_t[wcslen(Str) + 1];
		wcscpy(str[Hash(ID)], Str);//复制新的
	}

	void CreateString(LPCWSTR Str, LPCWSTR ID)//创建新字符串
	{
		CurString++;
		if (Str != NULL)
		{
			string[CurString].str = new wchar_t[wcslen(Str) + 1];
			wcscpy(string[CurString].str, Str);
		}
		wcscpy_s(string[CurString].ID, ID);
		str[Hash(ID)] = string[CurString].str;
	}

	void CreateEditEx(int Left, int Top, int Wid, int Hei, int Page, LPCWSTR name, LPCWSTR ID, HFONT Font, BOOL Ostr)
	{//创建自绘输入框
		++CurEdit;
		Edit[CurEdit].Left = Left; Edit[CurEdit].Top = Top;
		Edit[CurEdit].Width = Wid; Edit[CurEdit].Height = Hei;
		Edit[CurEdit].Page = Page;

		if (!Ostr)
		{//设置灰色标识的提示字符串
			wcscpy_s(Edit[CurEdit].OStr, name);
			Edit[CurEdit].font = Font;
			Edit[CurEdit].str = new wchar_t[21];
		}
		else//没有提示字符串:
			SetEditStrOrFont(name, Font, CurEdit);
		wcscpy_s(Edit[CurEdit].ID, ID);
	}

	void CreateArea(int Left, int Top, int Wid, int Hei, int Page)//创建点击区域
	{
		++CurArea;
		Area[CurArea].Left = Left; Area[CurArea].Top = Top;
		Area[CurArea].Width = Wid; Area[CurArea].Height = Hei;
		Area[CurArea].Page = Page;
	}

	void CreateButtonEx(int Number, int Left, int Top, int Wid, int Hei, int Page, LPCWSTR name, HBRUSH Leave, \
		HBRUSH Hover, HBRUSH press, HPEN Leave2, HPEN Hover2, HPEN Press2, HFONT Font, BOOL Enabled, BOOL Visible, COLORREF FontRGB, LPCWSTR ID)
	{//创建按钮的复杂函数...
		Button[Number].Left = Left; Button[Number].Top = Top;
		Button[Number].Width = Wid; Button[Number].Height = Hei;
		Button[Number].Page = Page; Button[Number].Leave = Leave;
		Button[Number].Hover = Hover; Button[Number].Press = press;
		Button[Number].Leave2 = Leave2; Button[Number].Hover2 = Hover2;
		Button[Number].Press2 = Press2; Button[Number].Font = Font;
		Button[Number].Enabled = Enabled; Button[Number].Visible = Visible;
		Button[Number].FontRGB = FontRGB;
		wcscpy_s(Button[Number].Name, name);
		wcscpy_s(Button[Number].ID, ID);
		but[Hash(ID)] = Number;

		LOGBRUSH LogBrush;//从HBRUSH中提取出RGB颜色
		LOGPEN LogPen;//	(渐变色需要)
		GetObject(Leave, sizeof(LogBrush), &LogBrush);
		Button[Number].b1[0] = (byte)LogBrush.lbColor;
		Button[Number].b1[1] = (byte)(LogBrush.lbColor >> 8);
		Button[Number].b1[2] = (byte)(LogBrush.lbColor >> 16);
		GetObject(Hover, sizeof(LogBrush), &LogBrush);
		Button[Number].b2[0] = (byte)LogBrush.lbColor;
		Button[Number].b2[1] = (byte)(LogBrush.lbColor >> 8);
		Button[Number].b2[2] = (byte)(LogBrush.lbColor >> 16);

		GetObject(Leave2, sizeof(LogPen), &LogPen);
		Button[Number].p1[0] = (byte)LogPen.lopnColor;
		Button[Number].p1[1] = (byte)(LogPen.lopnColor >> 8);
		Button[Number].p1[2] = (byte)(LogPen.lopnColor >> 16);
		GetObject(Hover2, sizeof(LogPen), &LogPen);
		Button[Number].p2[0] = (byte)LogPen.lopnColor;
		Button[Number].p2[1] = (byte)(LogPen.lopnColor >> 8);
		Button[Number].p2[2] = (byte)(LogPen.lopnColor >> 16);
	}
	void CreateButton(int Left, int Top, int Wid, int Hei, int Page, LPCWSTR name, LPCWSTR ID)//创建按钮（简化版）
	{
		++CurButton;//这里的name Wid Hei 不用全名是因为警告"隐藏了全局声明"
		CreateButtonEx(CurButton, Left, Top, Wid, Hei, Page, name, WhiteBrush, DBlueBrush, LBlueBrush, BLACK, BLACK, BLACK, 0, TRUE, TRUE, RGB(0, 0, 0), ID);
	}

	void CreateFrame(int Left, int Top, int Wid, int Hei, int Page, LPCWSTR name)//创建内容框
	{
		++CurFrame;//															--- Example -----
		Frame[CurFrame].Left = Left; Frame[CurFrame].Page = Page;//			    |				|
		Frame[CurFrame].Height = Hei; Frame[CurFrame].Top = Top;//			    |				|
		Frame[CurFrame].Width = Wid;//										    |				|
		wcscpy_s(Frame[CurFrame].Name, name);//								    -----------------
	}

	void CreateCheck(int Left, int Top, int Page, int Wid, LPCWSTR name)//创建"√"
	{
		++CurCheck;
		Check[CurCheck].Left = Left; Check[CurCheck].Top = Top;
		Check[CurCheck].Page = Page; Check[CurCheck].Width = Wid;
		wcscpy_s(Check[CurCheck].Name, name);
	}

	void CreateText(int Left, int Top, int Page, LPCWSTR name, COLORREF rgb)//创建注释文字
	{
		++CurText;
		Text[CurText].Left = Left; Text[CurText].Top = Top;
		Text[CurText].Page = Page; Text[CurText].rgb = rgb;
		wcscpy_s(Text[CurText].Name, name);
	}

	void CreateLine(int StartX, int StartY, int EndX, int EndY, int Page, COLORREF rgb)//创建线段
	{
		++CurLine;
		Line[CurLine].StartX = StartX; Line[CurLine].StartY = StartY;
		Line[CurLine].EndX = EndX; Line[CurLine].EndY = EndY;
		Line[CurLine].Page = Page; Line[CurLine].Color = rgb;
	}

	BOOL InsideButton(int cur, POINT& point)//根据传入的point判断鼠标指针是否在按钮内
	{
		return (Button[cur].Left * DPI <= point.x && Button[cur].Top * DPI <= point.y && (long)((Button[cur].Left + Button[cur].Width) * DPI) >= point.x && (long)((Button[cur].Top + Button[cur].Height) * DPI) >= point.y);
	}

	int InsideCheck(int cur, POINT& point)//同理 判断鼠标指针是否在check内
	{
		if (Check[cur].Left * DPI <= point.x && Check[cur].Top * DPI <= point.y && Check[cur].Left * DPI + 15 * DPI + 1 >= point.x
			&& Check[cur].Top * DPI + 15 * DPI + 1 >= point.y)return 1;//在check的方框内

		if (Check[cur].Left * DPI <= point.x && Check[cur].Top * DPI <= point.y && Check[cur].Left * DPI + Check[cur].Width * DPI >= point.x
			&& Check[cur].Top * DPI + 15 * DPI + 1 >= point.y)return 2;//在check方框右侧一定距离内
		return 0;//哪边都不在
	}

	void DrawFrames(int cur)//绘制Frames
	{
		int i;
		if (cur != 0) { i = cur; goto begin; }//如果使用ObjectRedraw则跳过其他Frame
		for (i = 1; i <= CurFrame; ++i)
		{
		begin:
			if (Frame[i].Page == CurWnd || Frame[i].Page == 0)
			{
				SelectObject(hdc, BLACK);//绘制方框
				SelectObject(hdc, DefFont);
				MoveToEx(hdc, (int)(Frame[i].Left * DPI), (int)(Frame[i].Top * DPI), NULL);//多加点(int)xx*DPI 减少警告
				LineTo(hdc, (int)(Frame[i].Left * DPI), (int)(Frame[i].Top * DPI + Frame[i].Height * DPI));
				LineTo(hdc, (int)(Frame[i].Left * DPI + Frame[i].Width * DPI), (int)(Frame[i].Top * DPI + Frame[i].Height * DPI));
				LineTo(hdc, (int)(Frame[i].Left * DPI + Frame[i].Width * DPI), (int)(Frame[i].Top * DPI));
				LineTo(hdc, (int)(Frame[i].Left * DPI), (int)(Frame[i].Top * DPI));
				SetTextColor(hdc, Frame[i].rgb);//文字颜色
				RECT rc = GetRECTf(i);
				SetBkMode(hdc, OPAQUE);//打印文字
				DrawTextW(hdc, Frame[i].Name, (int)wcslen(Frame[i].Name), &rc, NULL);
				SetTextColor(hdc, NULL);
			}
			if (cur != 0)return;
		}
	}

	void DrawButtons(int cur)//绘制按钮
	{
		int i;
		if (cur != 0) { i = cur; goto begin; }//如果使用ObjectRedraw则跳过其他Button
		for (i = 1; i <= CurButton; ++i)
		{
		begin:
			if (Button[i].Page == CurWnd || Button[i].Page == 0)
			{
				HPEN tmp = 0; HBRUSH tmb = 0;
				if (Button[i].Enabled == false)//禁用则显示灰色
				{
					SelectObject(hdc, Dgrey);
					SelectObject(hdc, Button[i].Leave2);
					SetTextColor(hdc, RGB(100, 100, 100));
					goto ok;//直接跳过渐变色
				}
				SetTextColor(hdc, Button[i].FontRGB);
				if (Button[i].Percent != 0 && Button[i].Percent != 100 && Button[i].DownTot == 0)//渐变色绘制
				{
					tmp = CreatePen(PS_SOLID, 1, RGB((Button[i].p2[0] - Button[i].p1[0]) * Button[i].Percent / 100 + Button[i].p1[0], \
						(Button[i].p2[1] - Button[i].p1[1]) * Button[i].Percent / 100 + Button[i].p1[1], (Button[i].p2[2] - Button[i].p1[2]) * Button[i].Percent / 100 + Button[i].p1[2]));

					SelectObject(hdc, tmp);
					tmb = CreateSolidBrush(RGB((Button[i].b2[0] - Button[i].b1[0]) * Button[i].Percent / 100 + Button[i].b1[0], \
						(Button[i].b2[1] - Button[i].b1[1]) * Button[i].Percent / 100 + Button[i].b1[1], (Button[i].b2[2] - Button[i].b1[2]) * Button[i].Percent / 100 + Button[i].b1[2]));
					SelectObject(hdc, tmb);
					goto ok;
				}
				if (CurCover == i && Button[i].DownTot == 0)//没有禁用&渐变色 -> 默认颜色
					if (Press == 1) {
						SelectObject(hdc, Button[i].Press);//按下按钮
						SelectObject(hdc, Button[i].Press2);
					}
					else {
						SelectObject(hdc, Button[i].Hover);//悬浮
						SelectObject(hdc, Button[i].Hover2);
					}
				else
				{
					SelectObject(hdc, Button[i].Leave);//离开
					SelectObject(hdc, Button[i].Leave2);
				}
			ok:
				if (Button[i].Font == NULL)SelectObject(hdc, DefFont); else SelectObject(hdc, Button[i].Font);//字体

				Rectangle(hdc, (int)(Button[i].Left * DPI), (int)(Button[i].Top * DPI),
					(int)(Button[i].Left * DPI + Button[i].Width * DPI), (int)(Button[i].Top * DPI + Button[i].Height * DPI));//绘制方框

				if (Button[i].DownTot != 0)//下载进度条
				{
					SelectObject(hdc, Button[i].Hover);
					Rectangle(hdc, (int)(Button[i].Left * DPI), (int)(Button[i].Top * DPI),
						(int)(Button[i].Left * DPI + Button[i].Width * DPI * (Button[i].Download - 1) / 100), (int)(Button[i].Top * DPI + Button[i].Height * DPI));
				}

				RECT rc = GetRECT(i);

				SetBkMode(hdc, TRANSPARENT);//去掉文字背景
				if (Button[i].DownTot == 0)//打印文字
					DrawTextW(hdc, Button[i].Name, (int)wcslen(Button[i].Name), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
				else
				{
					if (Button[i].Download == 101 && (Button[i].DownTot < 2 || Button[i].DownTot == Button[i].DownCur))//下载文字
					{//已全部下载完成
						DrawTextW(hdc, GetStr(L"Loaded"), 4, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
						Button[i].Download = Button[i].DownTot = 0;
					}
					else
					{
						if (Button[i].DownTot < 2)
							DrawTextW(hdc, GetStr(L"Loading"), 4, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
						else
						{
							wchar_t tmp1[101], tmp2[11];//正在下载 （已下载个数）/（总数）
							wcscpy_s(tmp1, GetStr(L"Loading"));
							wcscat_s(tmp1, L" ");
							_itow_s(Button[i].DownCur, tmp2, 10);
							wcscat_s(tmp1, tmp2);
							wcscat_s(tmp1, L"/");
							_itow_s(Button[i].DownTot, tmp2, 10);
							wcscat_s(tmp1, tmp2);
							DrawTextW(hdc, tmp1, (int)wcslen(tmp1), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
						}
					}
				}
				if (tmp != NULL)DeleteObject(tmp);//回收句柄
				if (tmb != NULL)DeleteObject(tmb);
			}
			if (cur != 0)return;
		}
		SetTextColor(hdc, RGB(0, 0, 0));
	}

	void DrawChecks(int cur)//绘制Checks
	{
		int i;
		if (cur != 0) { i = cur; goto begin; }//如果使用ObjectRedraw则跳过其他Check
		for (i = 1; i <= CurCheck; ++i)
		{
		begin:
			if (Check[i].Page == 0 || Check[i].Page == CurWnd)
			{
				if (i == CoverCheck)SelectObject(hdc, BLUE); else SelectObject(hdc, LGREY);

				SelectObject(hdc, grey);
				SelectObject(hdc, DefFont);//check默认边长为15，并不能调整
				Rectangle(hdc, (int)(Check[i].Left * DPI), (int)(Check[i].Top * DPI), (int)(Check[i].Left * DPI + 15 * DPI), (int)(Check[i].Top * DPI + 15 * DPI));
				TextOut(hdc, (int)(Check[i].Left * DPI + 20 * DPI), (int)(Check[i].Top * DPI), Check[i].Name, (int)wcslen(Check[i].Name));
				if (Check[i].Value == 1)//打勾
				{						//比较难看
					SelectObject(hdc, GREEN2);//试过了直接贴图，更难看
					MoveToEx(hdc, (int)(Check[i].Left * DPI + 2 * DPI), (int)(Check[i].Top * DPI + 7 * DPI), NULL);//然后就只能这样了
					LineTo(hdc, (int)(Check[i].Left * DPI + 7 * DPI), (int)(Check[i].Top * DPI + 12 * DPI));
					LineTo(hdc, (int)(Check[i].Left * DPI + 12 * DPI), (int)(Check[i].Top * DPI + 2 * DPI));
				}
			}
			if (cur != 0)return;
		}
	}
	void DrawLines()//绘制线段
	{//线段一般不需要重绘
		for (int i = 1; i <= CurLine; ++i)//因此没有加ObjectRedraw
			if (Line[i].Page == 0 || Line[i].Page == CurWnd)
			{
				SelectObject(hdc, CreatePen(0, 1, Line[i].Color));//直接用lineto
				MoveToEx(hdc, (int)(Line[i].StartX * DPI), (int)(Line[i].StartY * DPI), NULL);
				LineTo(hdc, (int)(Line[i].EndX * DPI), (int)(Line[i].EndY * DPI));
			}
	}
	void DrawTexts(int cur)//绘制文字
	{
		int i;
		if (cur != 0) { i = cur; goto begin; }//如果使用ObjectRedraw则跳过其他Texts
		for (i = 1; i <= CurText; ++i)
		{
		begin:
			if (Text[i].Page == 0 || Text[i].Page == CurWnd)
			{
				SetTextColor(hdc, Text[i].rgb);
				SelectObject(hdc, DefFont);//文字的字体缩放效果不太理想
				wchar_t* tmp = str[Hash(Text[i].Name)];
				TextOutW(hdc, (int)(Text[i].Left * DPI), (int)(Text[i].Top * DPI), tmp, (int)wcslen(tmp));
			}
			if (cur != 0)return;
		}
	}
	void DrawExp()//绘制注释
	{//注释只有一个，也不用ObjectRedraw
		if (ExpExist == false)return;//注释不存在？
		SelectObject(hdc, DefFont);
		SelectObject(hdc, YELLOW);
		SelectObject(hdc, yellow);
		if (ExpPoint.x > Width / 2)ExpPoint.x -= (ExpWidth + 6); else ExpPoint.x += 12;//自动选择注释的位置
		if (ExpPoint.y > Height / 2)ExpPoint.y -= (ExpHeight + 6); else ExpPoint.y += 14;//防止打印到窗口外面
		Rectangle(hdc, ExpPoint.x, ExpPoint.y, ExpWidth + ExpPoint.x, ExpHeight + ExpPoint.y);
		SetTextColor(hdc, RGB(0, 0, 0));
		for (int i = 1; i <= ExpLine; ++i)//逐行打印
			TextOutW(hdc, ExpPoint.x + 4, (int)(ExpPoint.y - 12 * DPI + 16 * i * DPI), Exp[i], (int)wcslen(Exp[i]));//注意这里的ExpPoint是真实坐标
	}

	void DrawEdits(int cur)//绘制输入框
	{
		int i;
		HDC mdc;//创建缓存dc
		mdc = CreateCompatibleDC(tdc);

		SelectObject(mdc, bitmap);
		SetBkMode(mdc, TRANSPARENT);
		if (cur != 0) { i = cur; goto begin; }//如果使用ObjectRedraw则跳过其他Edits

		for (i = 1; i <= CurEdit; ++i)
		{
		begin:
			if (Edit[i].Page == CurWnd || Edit[i].Page == 0)
			{
				SelectObject(mdc, White);//清空缓存dc
				SelectObject(mdc, WhiteBrush);
				Rectangle(mdc, 0, 0, 8000, 80);

				SelectObject(hdc, WhiteBrush);
				if (i == CoverEdit)SelectObject(hdc, BLUE); else SelectObject(hdc, BLACK);//如果当前Edit被选中则用蓝色绘制边框
				Rectangle(hdc, (int)((Edit[i].Left - 5) * DPI), (int)(Edit[i].Top * DPI),
					(int)((Edit[i].Left + Edit[i].Width + 5) * DPI), (int)((Edit[i].Top + Edit[i].Height) * DPI));

				if (*Edit[i].OStr != 0)//如果当前Edit显示的是Ostr(仅用于提示的灰色文字)
				{//在Hdc上直接打印完走人
					SetTextColor(hdc, RGB(150, 150, 150));
					RECT rc = { (long)((Edit[i].Left - 5) * DPI) , (long)(Edit[i].Top * DPI),(long)((Edit[i].Left + Edit[i].Width + 5) * DPI),(long)((Edit[i].Top + Edit[i].Height) * DPI) };
					DrawTextW(hdc, Edit[i].OStr, (int)wcslen(Edit[i].OStr), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
					continue;
				}

				SIZE sel, ser;//pos1 / pos2:确定打印时“选中部分”真正的左右两端点
				int pos1, pos2;//因为有时候从左到右移动鼠标选中文字，有时从右向左
				if (Edit[i].Pos1 > Edit[i].Pos2 && Edit[i].Pos2 != -1)pos1 = Edit[i].Pos2, pos2 = Edit[i].Pos1; else pos1 = Edit[i].Pos1, pos2 = Edit[i].Pos2;
				//pos1&pos2里记录的数值只表示选中的先后顺序，不代表左右，因此这里要特殊处理

				if (Edit[i].font != NULL)SelectObject(mdc, Edit[i].font); else SelectObject(mdc, DefFont);//字体
				if (pos2 == -1)
				{//如果没有选中，直接打印+贴图 -> 走人
					TextOutW(mdc, 0, 4, Edit[i].str, (int)wcslen(Edit[i].str));
					goto next;
				}
				//如果选中:较为复杂的情况
				GetTextExtentPoint32(mdc, Edit[i].str, pos1, &sel);//选中条左边字符总长度
				GetTextExtentPoint32(mdc, Edit[i].str, pos2, &ser);//选中条长度+左边字符总长度
				SelectObject(mdc, BLUE);
				SelectObject(mdc, BlueBrush);//用蓝色绘制选中条背景
				Rectangle(mdc, sel.cx, 0, ser.cx, (int)(ser.cy + 5 * DPI));
				SetTextColor(mdc, RGB(0, 0, 0));
				TextOutW(mdc, 0, 4, Edit[i].str, pos1);//黑色打印选中条左边文字
				SetTextColor(mdc, RGB(255, 255, 255));
				TextOutW(mdc, sel.cx, 4, &Edit[i].str[pos1], pos2 - pos1);//白色打印选中条中间文字
				SetTextColor(mdc, RGB(0, 0, 0));
				TextOutW(mdc, ser.cx, 4, &Edit[i].str[pos2], (int)wcslen(&Edit[i].str[pos2]));//黑色打印选中条右边文字

			next:
				int yMax = (int)((Edit[i].Top + Edit[i].Height / 2) * DPI - 4 - Edit[i].strHeight / 2);
				if (yMax < Edit[i].Top * DPI + 1)yMax = (int)(Edit[i].Top * DPI + 1);//贴图
				if (Edit[i].XOffset == 0)
					BitBlt(hdc, (int)((Edit[i].Left + Edit[i].Width / 2) * DPI) - Edit[i].strWidth / 2, yMax//Xoffset为0时居中
						, Edit[i].strWidth
						, Edit[i].strHeight + 4, mdc, 0, 0, SRCCOPY);
				else
					BitBlt(hdc, (int)(Edit[i].Left * DPI), yMax//有Xoffset时直接根据Xoffset贴
						, (int)(Edit[i].Width * DPI)//注意Xoffset不用再乘上DPI
						, Edit[i].strHeight + 4, mdc, Edit[i].XOffset, 0, SRCCOPY);
			}
			if (cur != 0)goto end;
		}
	end:
		DeleteDC(mdc);
	}

	void RedrawObject(int type, int cur)//1=Frame,2=Button,3=Check,4=Text,5=Edit
	{//ObjectRedraw技术的分派函数
		if (type == 1)DrawFrames(cur);//type和控件类型的关系不好记啊~
		if (type == 2)DrawButtons(cur);
		if (type == 3)DrawChecks(cur);
		if (type == 4)DrawTexts(cur);
		if (type == 5)DrawEdits(cur);
		DrawExp();//如果type不是1~5就自动DrawExp , 毕竟Exp只能有一个
	}
	//自动绘制所有控件的函数，效率低，不应经常使用
	void DrawEVERYTHING() { DrawFrames(0); DrawButtons(0); DrawChecks(0); DrawLines(); DrawTexts(0); DrawEdits(0); DrawExp(); }
	RECT GetRECT(int cur)//更新Buttons的rc
	{
		RECT rc = { (long)(Button[cur].Left * DPI), (long)(Button[cur].Top * DPI),
			(long)(Button[cur].Left * DPI + Button[cur].Width * DPI),(long)(Button[cur].Top * DPI + Button[cur].Height * DPI) };
		return rc;
	}
	RECT GetRECTf(int cur)//更新Frames的rc
	{
		RECT rc = { (long)(Frame[cur].Left * DPI + 10 * DPI) ,(long)(Frame[cur].Top * DPI - 7 * DPI) ,
		(long)(Frame[cur].Left * DPI + Frame[cur].Width * DPI),(long)(Frame[cur].Top * DPI + 30 * DPI) };
		return rc;
	}
	void SetEditStrOrFont(LPCWSTR Newstr, HFONT font, int cur)//改变制定Edit的字体或文字
	{
		HDC mdc;
		HBITMAP bmp;//创建一个空bmp用来GetTextExtentPoint32(获取字符宽度)
		mdc = CreateCompatibleDC(hdc);
		bmp = CreateCompatibleBitmap(hdc, 1, 1);
		SelectObject(mdc, bmp);
		if (font != NULL) Edit[cur].font = font;//先设置font，因为文字宽度和字体有关
		if (Edit[cur].font != NULL)SelectObject(mdc, Edit[cur].font); else SelectObject(mdc, DefFont);
		if (Newstr != NULL)//改变文字
		{
			if (Edit[cur].str != NULL)if (*Edit[cur].str != NULL)delete[] Edit[cur].str;
			Edit[cur].str = new wchar_t[wcslen(Newstr) + 1];
			wcscpy(Edit[cur].str, Newstr);
		}
		SIZE se;
		GetTextExtentPoint32(mdc, Edit[cur].str, (int)wcslen(Edit[cur].str), &se);
		Edit[cur].strWidth = se.cx; if (se.cy != 0) Edit[cur].strHeight = se.cy;
		if ((int)(Edit[cur].Width * DPI) < se.cx)Edit[cur].XOffset = (int)(se.cx - Edit[cur].Width * DPI) / 2; else Edit[cur].XOffset = 0;
		DeleteDC(mdc);
		DeleteObject(bmp);
	}
	int GetNearestChar(int cur, POINT Point)//试着获取输入框中离光标最近的字符
	{
		POINT point;
		if (Edit[cur].strWidth <= Edit[cur].Width * DPI)//如果字符较少没有填满
		{//按居中计算真实的偏移量
			if (Point.x < (long)((Edit[cur].Left + Edit[cur].Width / 2) * DPI - Edit[cur].strWidth / 2))return 0;
			if (Point.x > (long)((Edit[cur].Left + Edit[cur].Width / 2) * DPI + Edit[cur].strWidth / 2))return (int)wcslen(Edit[cur].str);
			point.x = Point.x - (long)((Edit[cur].Left + Edit[cur].Width / 2) * DPI) + Edit[cur].strWidth / 2;
		}
		else//反之直接用XOffset计算
			point.x = (long)(Point.x - Edit[cur].Left * DPI + Edit[cur].XOffset);
		HDC mdc;
		HBITMAP bmp;//创建一个临时dc
		mdc = CreateCompatibleDC(hdc);
		bmp = CreateCompatibleBitmap(hdc, 1, 1);
		SelectObject(mdc, bmp);
		ReleaseDC(hWnd, hdc);
		if (Edit[cur].font != NULL)SelectObject(mdc, Edit[cur].font); else SelectObject(mdc, DefFont);
		int l = (int)wcslen(Edit[cur].str), sum = 0, pos = -1;
		for (int i = 0; i <= l - 1; ++i)
		{
			++pos;//循环逐个累加字符宽度，超过光标位置时停止
			SIZE se;//(感觉效率好低.)
			GetTextExtentPoint32(mdc, &Edit[cur].str[i], 1, &se);
			sum += se.cx;
			if (sum >= point.x)  break;
		}
		SIZE sel, ser; //int t;
		GetTextExtentPoint32(mdc, Edit[cur].str, pos, &sel);//计算前一个字符宽度
		GetTextExtentPoint32(mdc, Edit[cur].str, pos + 1, &ser);//计算后一个

		DeleteDC(mdc);
		DeleteObject(bmp);
		if (point.x < (sel.cx + ser.cx) / 2)return pos; else return pos + 1;//比较，判断是选左边还是右边的
	}
	void EditPaste(int cur)//将剪贴板中文字粘贴到Edit中的函数
	{
		__try {
			if (cur == 0)return;//如果没有选中Edit则退出
			if (CoverEdit == 0)return;//（一般不会出现这种情况的......吧）
			char* buffer = NULL;
			if (OpenClipboard(hWnd))
			{//打开剪切板
				HANDLE hData = GetClipboardData(CF_TEXT);
				buffer = (char*)GlobalLock(hData);
				GlobalUnlock(hData);
				CloseClipboard();
			}
			int len = (int)strlen(buffer), len2 = (int)wcslen(Edit[cur].str) + 1;//这段代码有个缺陷
			wchar_t* Buffer = new wchar_t[len + 1], * zxf = new wchar_t[len + len2];//只能粘贴文字
			ZeroMemory(Buffer, sizeof(wchar_t) * len);//粘贴文件上去时会直接崩溃
			ZeroMemory(zxf, sizeof(wchar_t) * (len + len2));//所以全部用__try包住了
			if (buffer != NULL)//读取
				MultiByteToWideChar(CP_ACP, 0, buffer, -1, Buffer, sizeof(wchar_t) * len);
			int pos1 = min(Edit[cur].Pos1, Edit[cur].Pos2), pos2 = max(Edit[cur].Pos1, Edit[cur].Pos2);
			if (pos1 == -1)
			{//如果只有单光标选中
				wchar_t t = Edit[cur].str[pos2];
				Edit[cur].str[pos2] = '\0';
				wcscpy(zxf, Edit[cur].str);
				wcscat(zxf, Buffer);
				Edit[cur].str[pos2] = t;
				wcscat(zxf, &Edit[cur].str[pos2]);
				Edit[cur].Pos1 += (int)wcslen(Buffer);
				SetEditStrOrFont(zxf, 0, cur);
				RefreshXOffset(cur);
				RefreshCaretByPos(cur);
			}
			else//选中区段
			{
				Edit[cur].str[pos1] = '\0';
				wcscpy(zxf, Edit[cur].str);
				wcscat(zxf, Buffer);
				wcscat(zxf, &Edit[cur].str[pos2]);
				Edit[cur].Pos1 += (int)wcslen(Buffer);
				Edit[cur].Pos2 = -1;
				SetEditStrOrFont(zxf, 0, cur);
				RefreshXOffset(cur);
				RefreshCaretByPos(cur);
			}
			delete[] zxf;//清理内存
			delete[] Buffer;
			EditRedraw(cur);//重绘控件
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{//语言文件不规范时会说“启动失败”
			InfoBox(L"StartFail");
		}
	}
	void EditHotKey(int wParam)//Edit框按下热键时的分派函数
	{
		if (CoverEdit == 0)return;
		switch (wParam)
		{
		case 34:EditMove(CoverEdit, -1); break;//<-键
		case 35:EditMove(CoverEdit, 1); break;//->键
		case 36:EditPaste(CoverEdit); break;//粘贴
		case 37:EditCopy(CoverEdit); break;//复制
		case 38://剪切
			if (Edit[CoverEdit].Pos2 != -1)
				EditCopy(CoverEdit),
				EditDelete(CoverEdit, min(Edit[CoverEdit].Pos1, Edit[CoverEdit].Pos2),
					max(Edit[CoverEdit].Pos1, Edit[CoverEdit].Pos2));
			break;
		case 39:EditAll(CoverEdit); break;//全选
		case 40://Delete键（不是Backspace!）
			if (wcslen(Edit[CoverEdit].str) == (UINT)Edit[CoverEdit].Pos1)break;
			int pos1 = Edit[CoverEdit].Pos1, pos2 = Edit[CoverEdit].Pos2;
			if (pos2 != -1)//Backspace键直接算在WM_CHAR里面了
				EditDelete(CoverEdit, min(pos1, pos2), max(pos1, pos2));
			else EditDelete(CoverEdit, pos1, pos1 + 1);
			break;
		}
		return;
	}
	void EditUnHotKey()//取消注册Edit的热键
	{//在点击一个Edit外部时自动执行
		for (int i = 34; i < 41; ++i)UnregisterHotKey(hWnd, i);
		HideCaret(hWnd);///隐藏闪烁的光标
	}
	void EditRegHotKey()//注册Edit的热键
	{//在点击一个Edit时自动执行
		RegisterHotKey(hWnd, 34, NULL, VK_LEFT);
		RegisterHotKey(hWnd, 35, NULL, VK_RIGHT);
		RegisterHotKey(hWnd, 36, MOD_CONTROL, 'V');
		RegisterHotKey(hWnd, 37, MOD_CONTROL, 'C');
		RegisterHotKey(hWnd, 38, MOD_CONTROL, 'X');
		RegisterHotKey(hWnd, 39, MOD_CONTROL, 'A');
		RegisterHotKey(hWnd, 40, NULL, VK_DELETE);
		DestroyCaret();
		CreateCaret(hWnd, NULL, 1, (int)(20 * DPI));
	}
	void EditCHAR(wchar_t wParam)//输入
	{
		if (Edit[CoverEdit].Press == true || CoverEdit == 0)return;
		if (wParam >= 0x20 && wParam != 0x7F)//当按下的是正常按键时:
		{
			int pos = min(Edit[CoverEdit].Pos1, Edit[CoverEdit].Pos2);
			int pos2 = max(Edit[CoverEdit].Pos1, Edit[CoverEdit].Pos2);
			if (pos == -1)//没有选中文字
				EditAdd(CoverEdit, pos2, pos2, wParam);
			else//选中了一段文字(替换)
				EditAdd(CoverEdit, pos, pos2, wParam);
		}
		else if (wParam == VK_BACK)//Backspace键
		{
			int pos1 = Edit[CoverEdit].Pos1, pos2 = Edit[CoverEdit].Pos2;
			if (pos2 != -1)//删除光标的前一个文字
				EditDelete(CoverEdit, min(pos1, pos2), max(pos1, pos2));
			else//删除选中的一段文字
				EditDelete(CoverEdit, pos1 - 1, pos1);
		}
		return;
	}

	void EditAdd(int cur, int Left, int Right, wchar_t Char)//向某个Edit中添加一个字符 或 把一段字符替换成一个字符
	{
		int len = (int)wcslen(Edit[cur].str) + 5;//计算原Edit中文字的长度
		wchar_t* zxf = new wchar_t[len], t = 0;//申请对应长度的缓存空间
		if (Left == Right)t = Edit[cur].str[Left];//如果只是添加字符，则先备份光标右侧的字符
		Edit[cur].str[Left] = '\0';
		wcscpy(zxf, Edit[cur].str);//将光标左侧的所有字符复制到缓存中
		zxf[Left] = Char;
		zxf[Left + 1] = 0;
		Edit[cur].str[Left] = t;//添加新的字符
		wcscat(zxf, Edit[cur].str + Right);
		Edit[cur].Pos1 = Left + 1;//将右侧的字符再接上去
		Edit[cur].Pos2 = -1;
		ZeroMemory(Edit[cur].str, sizeof(Edit[cur].str));
		SetEditStrOrFont(zxf, 0, cur);//刷新
		RefreshXOffset(cur);
		RefreshCaretByPos(cur);
		EditRedraw(cur);
	}

	void EditDelete(int cur, int Left, int Right)//删除一个或一段字符
	{
		if (Left == -1)return;
		wchar_t* zxf = new wchar_t[wcslen(Edit[cur].str)];
		Edit[cur].str[Left] = '\0';
		wcscpy(zxf, Edit[cur].str);
		wcscat(zxf, Edit[cur].str + Right);
		Edit[cur].Pos1 = Left;
		Edit[cur].Pos2 = -1;
		SetEditStrOrFont(zxf, 0, cur);
		RefreshXOffset(cur);
		RefreshCaretByPos(cur);
		EditRedraw(cur);
	}

	void EditAll(int cur)//选中一个Edit中所有字符
	{
		if (cur == 0)return;
		Edit[cur].Pos1 = 0;
		Edit[cur].Pos2 = (int)wcslen(Edit[cur].str);
		RefreshXOffset(CoverEdit);
		RefreshCaretByPos(CoverEdit);
		EditRedraw(cur);
	}
	void EditMove(int cur, int off)//移动选中的Edit中光标的位置
	{
		int xback;
		xback = Edit[cur].XOffset;
		if (Edit[cur].Pos2 != -1)return;
		Edit[cur].Pos1 += off;
		if (Edit[cur].Pos1 < 0)Edit[cur].Pos1 = 0;
		if ((unsigned int)Edit[cur].Pos1 > (unsigned int)wcslen(Edit[cur].str))Edit[cur].Pos1 = (int)wcslen(Edit[cur].str);
		RefreshXOffset(cur);
		RefreshCaretByPos(cur);
	}
	void EditCopy(int cur)//复制一个Edit中已选中的内容
	{
		if (cur == 0)return;
		wchar_t* source, t;
		char* Source;
		if (Edit[cur].Pos2 == -1)return;
		int pos1 = min(Edit[cur].Pos1, Edit[cur].Pos2);
		int pos2 = max(Edit[cur].Pos1, Edit[cur].Pos2);
		source = new wchar_t[pos2 - pos1 + 1];
		Source = new char[pos2 - pos1 + 1];
		t = Edit[cur].str[pos2];
		Edit[cur].str[pos2] = '\0';
		wcscpy(source, Edit[cur].str + pos1);
		Edit[cur].str[pos2] = t;
		WideCharToMultiByte(CP_ACP, 0, source, -1, Source, 0xffff, NULL, NULL);

		if (OpenClipboard(hWnd))
		{
			HGLOBAL clipbuffer;
			char* buffer;
			EmptyClipboard();
			clipbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(Source) + 1);
			buffer = (char*)GlobalLock(clipbuffer);
			if (buffer != NULL)strcpy(buffer, LPCSTR(Source));
			GlobalUnlock(clipbuffer);
			SetClipboardData(CF_TEXT, clipbuffer);
			CloseClipboard();
		}
	}
	void RefreshCaretByPos(int cur)//刷新选中的Edit中光标的位置
	{
		if (Edit[cur].Pos1 == -1)return;//指定Edit未被选中->退出
		HDC mdc;
		HBITMAP bmp;
		mdc = CreateCompatibleDC(hdc);
		bmp = CreateCompatibleBitmap(hdc, 1, 1);
		SelectObject(mdc, bmp);
		ReleaseDC(hWnd, hdc);
		if (Edit[cur].font != NULL)SelectObject(mdc, Edit[cur].font); else SelectObject(mdc, DefFont);
		SIZE se;//通过这个Edit的Pos1的字符来计算字符长度
		if (Edit[cur].Pos2 != -1)
			GetTextExtentPoint32(mdc, Edit[cur].str, Edit[cur].Pos2, &se);
		else
			GetTextExtentPoint32(mdc, Edit[cur].str, Edit[cur].Pos1, &se);
		if (Edit[cur].XOffset == 0)//减去Xoffset，再加上Edit的坐标就是光标位置了
			SetCaretPos(se.cx + (long)((Edit[cur].Left + Edit[cur].Width / 2) * DPI) - Edit[cur].strWidth / 2, (int)((Edit[cur].Top + Edit[cur].Height / 2 - 4) * DPI - Edit[cur].strHeight / 2));
		else
			SetCaretPos((int)(se.cx + Edit[cur].Left * DPI - Edit[cur].XOffset), (int)((Edit[cur].Top + Edit[cur].Height / 2 - 4) * DPI - Edit[cur].strHeight / 2));
		ShowCaret(hWnd);
		DeleteDC(mdc);//做好清理
		DeleteObject(bmp);
	}
	void EditDown(int cur)//鼠标左键在某个Edit上按下
	{
		EditRegHotKey();//先注册下热键再说
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hWnd, &point);
		if (CoverEdit != cur)Edit[CoverEdit].Pos1 = Edit[CoverEdit].Pos2 = -1, EditRedraw(CoverEdit);//如果直接从一个Edit点到
		CoverEdit = cur;//												另一个Edit，那么先把之前的Pos和蓝框问题解决好
		if (*Edit[cur].OStr != 0)//去掉灰色的提示文字
		{
			*Edit[cur].OStr = 0;
			ZeroMemory(Edit[cur].str, sizeof(Edit[cur].str));
		}
		Edit[cur].Pos1 = Edit[cur].Pos2 = -1;
		Edit[cur].Press = true;
		Edit[cur].Pos1 = GetNearestChar(cur, point);//计算Pos1

		RefreshCaretByPos(cur);
		EditRedraw(cur);//重绘这个Edit
	}

	RECT GetRECTe(int cur)//更新Edit的rc
	{
		RECT rc{ (long)((Edit[cur].Left - 5) * DPI) ,(long)(Edit[cur].Top * DPI),
		(long)((Edit[cur].Left + Edit[cur].Width + 5) * DPI),(long)((Edit[cur].Top + Edit[cur].Height) * DPI) };
		return rc;
	}

	RECT GetRECTc(int cur)//更新Check的rc
	{
		RECT rc{ (long)(Check[cur].Left * DPI) ,(long)(Check[cur].Top * DPI) ,
			(long)(Check[cur].Left * DPI + 15 * DPI) ,(long)(Check[cur].Top * DPI + 15 * DPI) };
		return rc;
	}

	BOOL InsideArea(int cur, POINT point)//通过POINT判断是否在指定Area内
	{
		return (Area[cur].Left * DPI <= point.x) && (Area[cur].Top * DPI <= point.y) &&
			((Area[cur].Left + Area[cur].Width) * DPI >= point.x) && ((Area[cur].Top + Area[cur].Height) * DPI >= point.y);
	}

	BOOL InsideEdit(int cur, POINT point)//通过POINT判断是否在指定Edit内
	{
		return ((Edit[cur].Left - 5) * DPI <= point.x && Edit[cur].Top * DPI <= point.y &&
			(long)((Edit[cur].Left + Edit[cur].Width + 5) * DPI) >= point.x && (Edit[cur].Top + Edit[cur].Height) * DPI >= point.y);
	}

	void LeftButtonDown()//鼠标左键按下
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hWnd, &point);//获取坐标
		if (CurCover != -1)//当按钮按下 & 停留在按钮上时
		{
			Press = 1;//重绘这个按钮
			RECT rc = GetRECT(CurCover);
			if (Obredraw)Readd(2, CurCover);
			Redraw(&rc);
		}
		if (!InsideEdit(CoverEdit, point) && Edit[CoverEdit].Pos2 == -1 && CoverEdit != 0)
		{
			int tmp0 = CoverEdit;//原来一个Edit被激活
			CoverEdit = 0;//现在鼠标点在那个Edit外面时
			if (Obredraw)Readd(5, tmp0);//重绘原来的Edit
			RECT rc = GetRECTe(tmp0);
			Redraw(&rc);
			EditUnHotKey();//取消热键
		}
		if (EditPrv != 0)
		{//鼠标点在另一个Edit上时同样重绘Edit(好吧有点啰嗦= =)
			if (Obredraw)Readd(5, EditPrv);
			RECT rc = GetRECTe(EditPrv);
			Redraw(&rc);
			EditPrv = 0;
		}
		if (CoverEdit != 0)//停留在Edit上时
			EditDown(CoverEdit);
		else
			EditUnHotKey();
		Timer = GetTickCount();
		DestroyExp();//任何操作都会导致exp的关闭
	}

	void CheckGetNewInside(POINT& point)//检查point是否在check内
	{
		for (int i = 1; i <= CurCheck; ++i)//遍历所有check
		{
			if (Check[i].Page == CurWnd || Check[i].Page == 0)//不再同一页面 -> 直接跳过
				if (InsideCheck(i, point) != 0)//在check的文字中或方框内
				{
					CoverCheck = i;//设置covercheck
					if (Obredraw)Readd(3, i);
					RECT rc = GetRECTc(i);//重绘
					Redraw(&rc);
					break;
				}
		}
	}
	void ButtonGetNewInside(POINT& point)//检查point是否在check内
	{
		for (int i = 0; i <= CurButton; ++i)//历史原因，Button编号是从0开始的
			if ((Button[i].Page == CurWnd || Button[i].Page == 0) && Button[i].Enabled)
				if (InsideButton(i, point))//在按钮中
				{
					CurCover = i;//设置curcover
					if (ButtonEffect)//特效开启
					{
						Button[i].Percent += 40;//先给40%的颜色 （太淡了看不出来）
						if (Button[i].Percent > 100)Button[i].Percent = 100;
					}
					if (Obredraw)Readd(2, i);
					RECT rc = GetRECT(i);//重绘
					Redraw(&rc);
					return;
				}
	}
	void AreaGetNewInside(POINT& point)//Area 同理
	{
		for (int i = 1; i <= CurArea; ++i)
			if (Area[i].Page == CurWnd || Area[i].Page == 0)
				if (InsideArea(i, point))CoverArea = i;
	}
	void EditGetNewInside(POINT& point) //Edit 同理
	{
		for (int i = 1; i <= CurEdit; ++i)
			if (Edit[i].Page == CurWnd || Edit[i].Page == 0)
				if (InsideEdit(i, point))
				{
					EditPrv = CoverEdit;//记录好之前的Edit编号
					CoverEdit = i;
					EditRegHotKey();
					return;
				}
	}
	void MouseMove()//鼠标移动
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hWnd, &point);
		if (CurCover == -1)ButtonGetNewInside(point);//原来不在按钮内 -> 看看现在是否移进按钮
		else//原来在
		{
			if (!Button[CurCover].Enabled) { CurCover = -1; goto disabled; }//这个按钮被禁用了  直接跳到下面
			if ((Button[CurCover].Page != CurWnd && Button[CurCover].Page != 0) || !InsideButton(CurCover, point))
			{//现在不在
				if (Obredraw)Readd(2, CurCover);
				if (ButtonEffect)
				{//curcover设为-1 , 重绘
					Button[CurCover].Percent -= Delta;
					if (Button[CurCover].Percent < 0)Button[CurCover].Percent = 0;
				}
				RECT rc = GetRECT(CurCover);
				CurCover = -1;
				Redraw(&rc);
				ButtonGetNewInside(point);//有可能从一个按钮直接移进另一个按钮内
			}
		}
	disabled:
		if (CoverCheck == 0)CheckGetNewInside(point);//在外面 -> 寻找新check
		else
		{//同理(真的)
			if ((Check[CoverCheck].Page != CurWnd && Check[CoverCheck].Page != 0) || InsideCheck(CoverCheck, point) == 0)
			{
				if (Obredraw)Readd(3, CoverCheck);
				RECT rc = GetRECTc(CoverCheck);
				CoverCheck = 0;
				Redraw(&rc);
				CheckGetNewInside(point);
			}
		}
		if (CoverEdit == 0)EditGetNewInside(point);
		else//在一个Edit内
		{
			if (Edit[CoverEdit].Press == true)
			{//如果Edit被按下 (拖动选择条)
				int t = Edit[CoverEdit].Pos2;
				Edit[CoverEdit].Pos2 = GetNearestChar(CoverEdit, point);//寻找和鼠标指针最近的字符
				RefreshCaretByPos(CoverEdit);//移动Caret(闪烁的光标)
				if (Edit[CoverEdit].Pos2 == Edit[CoverEdit].Pos1) { Edit[CoverEdit].Pos2 = -1; goto end; }//只选择了一个字符
				if (Edit[CoverEdit].Pos2 != t && Edit[CoverEdit].Width < Edit[CoverEdit].strWidth && !InsideEdit(CoverEdit, point))RefreshXOffset(CoverEdit);//Edit中文本过长，移动到了框外面
				if (Edit[CoverEdit].Pos2 != t)EditRedraw(CoverEdit);//只要和原来有任何不同就重绘
			}

		}
	end:
		if (CoverArea == 0)
			AreaGetNewInside(point);
		else
			if (!InsideArea(CoverArea, point))CoverArea = 0;

		if (Msv == 0)
		{
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);//检测鼠标移进移出的函数
			tme.hwndTrack = hWnd;//在鼠标移出窗体时会触发一个WM_LEAVE消息，根据这个可以改变按钮颜色
			tme.dwFlags = TME_LEAVE;//缺点是当焦点直接被另一个窗口夺取时(比如按下windows键)
			TrackMouseEvent(&tme);//什么反应都没有
			Msv = 1;//移出
		}
		else Msv = 0;//移进
		if (point.x != ExpPoint2.x || point.y != ExpPoint2.y)
		{//鼠标移动时销毁Exp
			ExpPoint2 = point;
			Timer = GetTickCount();
			DestroyExp();
		}
	}
	void EditRedraw(int cur)//重绘Edit的外壳函数
	{
		if (Obredraw)Readd(5, cur);
		RECT rc = GetRECTe(cur);
		Redraw(&rc);//标准ObjectRedraw写法
	}
	void InfoBox(LPCWSTR Str)//全自动的MessageBox
	{
		const bool f = (bool)GetStr(Str);
		if (!slient)//如果Str中是GUIstr的ID则打印str的内容，否则直接打印Str
			if (f)MessageBox(hWnd, GetStr(Str), GetStr(L"Info"), 0x40L); else MessageBox(hWnd, Str, GetStr(L"Info"), 0x40L);
		else if (f)printf("%ls\n", GetStr(Str)); else printf("%ls\n", Str);//打印到命令行中
	}
	void RefreshXOffset(int cur)//重新计算Edit的Xoffset
	{
		if (Edit[cur].strWidth < Edit[cur].Width * DPI) {
			Edit[cur].XOffset = 0; return;//如果Edit中内容太少，XOffset直接为0，退出
		}
		HDC mdc;
		HBITMAP bmp;
		mdc = CreateCompatibleDC(hdc);//创建临时dc
		bmp = CreateCompatibleBitmap(hdc, 1, 1);
		SelectObject(mdc, bmp);
		ReleaseDC(hWnd, hdc);
		if (Edit[cur].font != NULL)SelectObject(mdc, Edit[cur].font); else SelectObject(mdc, DefFont);
		SIZE se;

		if (Edit[cur].Pos2 != -1)
			GetTextExtentPoint32(mdc, Edit[cur].str, Edit[cur].Pos2, &se);//计算长度
		else
			GetTextExtentPoint32(mdc, Edit[cur].str, Edit[cur].Pos1, &se);//好吧 过程比较复杂，但原理简单
		if (se.cx < Edit[cur].XOffset)Edit[cur].XOffset = se.cx - 1;//偷个懒不写了
		if (se.cx > (long)(Edit[cur].XOffset + Edit[cur].Width * DPI))Edit[cur].XOffset += (int)(se.cx - (Edit[cur].XOffset + Edit[cur].Width * DPI) - 1);
		if (Edit[cur].strWidth - Edit[cur].XOffset < Edit[cur].Width * DPI)Edit[cur].XOffset = (int)(Edit[cur].strWidth - Edit[cur].Width * DPI);
		RefreshCaretByPos(cur);
		EditRedraw(cur);
		DeleteDC(mdc);//清理
		DeleteObject(bmp);
	}

	int GetNumByIDe(LPCWSTR ID)//通过Edit的ID获取其编号
	{
		for (int i = 1; i <= CurEdit; ++i)if (wcscmp(ID, Edit[i].ID) == 0)return i;
		return 0;//找不到返回0
	}

	void SetPage(int Page)//设置窗口的页数
	{
		if (Page == CurWnd)return;//点了当前页的按钮，直接退出
		HideCaret(hWnd);//换页时自动隐藏闪烁的光标
		Edit[CoverEdit].Press = false;
		Edit[CoverEdit].Pos1 = Edit[CoverEdit].Pos2 = -1;
		CoverEdit = 0;//取消Edit的选中
		CurWnd = Page;
		while (!rs.empty())rs.pop();
		while (!es.empty())es.pop();
		Redraw(NULL);//切换页面时当然需要全部重绘啦
	}
	void SetDPI(DOUBLE NewDPI)//改变窗口的缩放大小
	{//							(由于某未知的历史原因，缩放大小的变量被我命名成了DPI)
		DPI = NewDPI;//创建新大小的字体
		DefFont = CreateFontW((int)(16 * DPI), (int)(8 * DPI), 0, 0, FW_THIN, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("宋体"));
		for (int i = 1; i <= CurEdit; ++i)SetEditStrOrFont(0, DefFont, i), RefreshXOffset(i);//设置字体
		RefreshCaretByPos(CoverEdit);
		if (Width != 0 && Height != 0)SetWindowPos(hWnd, NULL, 0, 0, (int)(Width * DPI), (int)(Height * DPI), SWP_NOMOVE | SWP_NOREDRAW);
		DestroyCaret();//设置闪烁的光标
		CreateCaret(hWnd, NULL, 1, (int)(20 * DPI));
		while (!es.empty())es.pop();
		while (!rs.empty())rs.pop();
		Redraw(NULL);//全部重绘
	}

	LPWSTR GetCurInsideID()//获取当前鼠标处于的按钮的ID
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hWnd, &point);
		for (int i = 0; i <= CurButton; ++i)
			if ((Button[i].Page == CurWnd || Button[i].Page == 0) && Button[i].Enabled)
				if (InsideButton(i, point) && Button[i].DownTot == 0)
					return Button[i].ID;
		return Button[0].ID;
	}
	inline int GetNumbyID(LPCWSTR ID) { return but[Hash(ID)]; }//通过按钮的ID获取其编号

	void SetHDC(HDC HDc)//给要绘制的窗口设置一个新的hdc
	{
		hdc = HDc;
		if (bitmap != NULL)DeleteObject(bitmap);
		bitmap = CreateCompatibleBitmap(hdc, 8000, 80);
	}
	void Try2CreateExp()//尝试去绘制一个Exp
	{
		if (ExpExist == true || CurCover == -1)return;//如果Exp已经存在，，或者Exp内容为空，那么就没他什么事了
		if (wcslen(Button[CurCover].Exp) == 0)return;
		CurButtonBack = CurButton;
		ExpExist = true;
		ExpLine = 0;//清零
		ZeroMemory(Exp, sizeof(Exp));

		wchar_t* x = Button[CurCover].Exp, * y = Button[CurCover].Exp;
		ExpHeight = 8;
		ExpWidth = 0;
		while (1)
		{
			ExpLine++;
			x = wcsstr(x + 1, L"\\n");//在Exp的字符串中寻找\n字符
			if (x != 0)x[0] = '\0';//然后存储在一个二维数组内
			if (ExpLine == 1)wcscpy(Exp[ExpLine], y); else wcscpy(Exp[ExpLine], y + 2);
			SIZE* se = new SIZE;
			if (ExpLine == 1)GetTextExtentPoint32(hdc, y, (int)wcslen(y), se); else GetTextExtentPoint32(hdc, y + 2, (int)wcslen(y + 2), se);//获取字符串的宽度
			if (x != 0)x[0] = '\\';
			ExpHeight += se->cy;//计算这个Exp的宽和高
			ExpWidth = max(ExpWidth - 8, se->cx) + 8;
			if (x == 0)break;
			y = x;
		}
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hWnd, &point);
		ExpPoint = point;//绘制这个Exp
		Readd(6, 1);
		Redraw(NULL);
	}
	void DestroyExp()//清除这个Exp
	{
		if (ExpExist == false)return;
		ExpExist = false;
		ExpLine = ExpHeight = ExpWidth = 0;
		Redraw(NULL);//删除Exp时要绘制这个Exp下面的按钮什么的，所以分类绘制很麻烦，干脆就全部刷新一下吧
	}
	FORCEINLINE void Erase(RECT& rc) { es.push(rc); }//设置要擦除的区域
	void Redraw(const RECT* rc) { InvalidateRect(hWnd, rc, FALSE); UpdateWindow(hWnd); }//自动重绘 & 刷新指定区域
	void Readd(int type, int cur) { rs.push(std::make_pair(type, cur)); }//添加要刷新的控件

	//下面是Class的变量

	struct ButtonEx//按钮
	{
		long Left, Top, Width, Height, Page, Download, Percent, DownCur, DownTot;
		bool Visible, Enabled, Border = true;//border:是否有边框
		HBRUSH Leave, Hover, Press;
		HPEN Leave2, Hover2, Press2;
		HFONT Font;
		wchar_t Name[31], ID[11], Exp[MAX_EXPLENGTH];
		COLORREF FontRGB;
		BYTE b1[3], b2[3], p1[3], p2[3];
	}Button[MAX_BUTTON];
	struct FrameEx//控件框结构体
	{
		int Left, Top, Width, Height, Page;
		COLORREF rgb;//自定义颜色
		wchar_t Name[51];
	}Frame[MAX_FRAME];//现在为了节约内存空间都用MAX_XXX了 ， 具体可以到GUI.h里改
	struct CheckEx//选择框结构体
	{
		int Left, Top, Page, Width;//width跟绘制无关，用来检测是否按下
		bool Value;
		wchar_t Name[51];
	}Check[MAX_CHECK];
	struct LineEx//线段
	{
		int StartX, StartY, EndX, EndY, Page;//线段的起始坐标和终点坐标
		COLORREF Color;
	}Line[MAX_LINE];
	struct TextEx//文字
	{
		int Left, Top, Page;
		COLORREF rgb;
		wchar_t Name[11];//这里的"Name"其实是GUIString的ID
	}Text[MAX_TEXT];
	struct EditEx//输入框
	{
		int Left, Top, Width, Height, Page;
		int strWidth, strHeight, Pos1, Pos2, XOffset;
		bool Press;
		wchar_t* str, ID[11], OStr[21];
		HFONT font;
	}Edit[MAX_EDIT];
	struct AreaEx//点击区域
	{
		int Left, Top, Width, Height, Page;
	}Area[MAX_AREA];
	struct GUIString//GUI工程专用带ID标签的字符串
	{
		wchar_t* str, ID[11];
	}string[MAX_STRING];

	int ExpLine, ExpHeight, ExpWidth;//关于Explaination的几个变量
	wchar_t Exp[MAX_EXPLINES][81];
	POINT ExpPoint, ExpPoint2;
	BOOL ShowExp = FALSE;//Exp是否被显示
	DWORD Timer;//exp开启的时间
	bool ExpExist = false;//exp是否存在

	std::map<unsigned int, wchar_t*> str;//GUIstr的ID ->编号
	std::map<unsigned int, int>but;//button的ID -> 编号
	HFONT DefFont;//默认的字体
	int Msv;//鼠标移出检测变量
	int CurString, CurButton, CurFrame, CurCheck, CurLine, CurText, CurEdit, CurArea;//各种控件的数量
	double DPI = 1;
	int CurCover, CoverCheck, CoverEdit, CoverArea;//当前被鼠标覆盖的东西
	bool Obredraw = false;//是否启用ObjectRedraw技术
	bool ButtonEffect = false;//是否开启渐变色
	int CurWnd;//当前的页面
	int Press;//鼠标左键是否按下
	std::stack<std::pair<int, int>>rs;//重绘列表
	std::stack<RECT>es;//清理列表
	HDC hdc;//缓存dc
	HDC tdc;//真实dc
	HBITMAP bitmap;//Edit专用缓存bitmap
	HBITMAP Bitmap;//缓存窗口bitmap
	int Width, Height;//窗口的宽和高
	HWND hWnd;//Class绘制的窗体的hwnd
	HINSTANCE hInstance;//程序的hInst
	int CurButtonBack;
	int EditPrv = 0;//之前被激活的edit序号
private://没有任何private变量或函数= =
}Main, CatchWnd, UpWnd;

//Class的声明结束
//下面是各种函数

ATOM MyRegisterClass(HINSTANCE h, WNDPROC proc, LPCWSTR ClassName)
{//封装过的注册Class函数
	WNDCLASSEXW wcex = { 0 };
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = proc;
	wcex.hInstance = h;
	wcex.hIcon = LoadIcon(h, MAKEINTRESOURCE(IDI_GUI));//不能自定义窗体图标
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_GUI);
	wcex.lpszClassName = ClassName;//自定义ClassName和WndProc
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_GUI));//小图标
	return RegisterClassExW(&wcex);
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,//程序入口点
	_In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);

	if (!InitInstance(hInstance, nCmdShow))return FALSE;
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GUI));

	MSG msg;
	// 主消息循环: 
	while (GetMessageW(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)//初始化
{
	DBlueBrush = CreateSolidBrush(RGB(210, 255, 255)); 
		LBlueBrush = CreateSolidBrush(RGB(230, 255, 255)); 
		WhiteBrush = CreateSolidBrush(RGB(255, 255, 255)); 
		green = CreateSolidBrush(RGB(0x66, 0xCC, 0xFF)); 
		grey = CreateSolidBrush(RGB(248, 248, 248)); 
		Dgrey = CreateSolidBrush(RGB(230, 230, 230)); 
		DBlue = CreatePen(PS_SOLID, 1, RGB(210, 255, 255)); 
		LBlue = CreatePen(PS_SOLID, 1, RGB(230, 255, 255)); 
		BLACK = CreatePen(PS_SOLID, 1, RGB(0, 0, 0)); 
		White = CreatePen(PS_SOLID, 1, RGB(255, 255, 255)); 
		GREEN = CreatePen(PS_SOLID, 2, RGB(0x70, 0xCF, 0xFF)); 
		GREEN2 = CreatePen(PS_SOLID, 2, RGB(5, 195, 195)); 
		LGREY = CreatePen(PS_SOLID, 1, RGB(115, 115, 115)); 
		BLUE = CreatePen(PS_SOLID, 1, RGB(40, 130, 240)); 

	hInst = hInstance; // 将实例句柄存储在全局变量中
	Main.InitClass(hInst);//初始化主类
	if (!MyRegisterClass(hInst, WndProc, szWindowClass))return FALSE;//初始化Class

	Main.Obredraw = true;//默认使用ObjectRedraw
	Main.hWnd = CreateWindowW(szWindowClass, Main.GetStr(L"Title"), NULL, \
		CW_USEDEFAULT, CW_USEDEFAULT, 1, 1, NULL, nullptr, hInstance, nullptr);//创建主窗口
	Main.Timer = GetTickCount();
	CreateCaret(Main.hWnd, NULL, 1, 20);
	SetCaretBlinkTime(500);//初始化闪烁光标
	if (!Main.hWnd)return FALSE;//创建主窗口失败就直接退出
	if (Effect)
	{
		SetWindowLong(Main.hWnd, GWL_EXSTYLE, GetWindowLong(Main.hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(Main.hWnd, NULL, 234, LWA_ALPHA);//半透明特效
	}
	SetWindowLong(Main.hWnd, GWL_STYLE, GetWindowLong(Main.hWnd, GWL_STYLE) & ~WS_CAPTION & ~WS_THICKFRAME & ~WS_SYSMENU & ~WS_GROUP & ~WS_TABSTOP);//无边框窗口


	Main.CreateEditEx(20 + 5, 120, 110 - 10, 50, 1, L"样例文字", L"E_runinVD", 0, false);//创建输入框
	Main.CreateText(10, 20, 0, L"test", 0);
	Main.CreateString(L"样例程序",L"test");
	Main.CreateLine(10, 10, 100, 10, 0, 0);

	Main.CreateFrame(10, 65, 180, 125, 0, L" 样例文字 ");
	Main.CreateButton(10, 100, 110, 50, 0, L"q", L"DelR3");
	Main.CreateCheck(20, 90, 1, 150, L" 样例程序");

	Main.CreateButtonEx(Main.CurButton, 130, 10, 60, 30, 0, L"×", \
		CreateSolidBrush(RGB(255, 109, 109)), CreateSolidBrush(RGB(250, 100, 100)), CreateSolidBrush(RGB(232, 95, 95)), \
		CreatePen(PS_SOLID, 1, RGB(255, 109, 109)), CreatePen(PS_SOLID, 1, RGB(250, 100, 100)), CreatePen(PS_SOLID, 1, RGB(232, 95, 95)), \
		Main.DefFont, 1, 1, RGB(255, 255, 255), L"Close");

	SetWindowPos(Main.hWnd, 0, 0, 0,200, 200, SWP_NOMOVE);
	Main.Width = 201; Main.Height = 200;

	typedef DWORD(CALLBACK * SEtProcessDPIAware)(void);
	SEtProcessDPIAware SetProcessDPIAware;
	HMODULE huser;//让系统不对这个程序进行缩放
	huser = LoadLibrary(L"user32.dll");//在一些笔记本上有用
	SetProcessDPIAware = (SEtProcessDPIAware)GetProcAddress(huser, "SetProcessDPIAware");
	if (SetProcessDPIAware != NULL)SetProcessDPIAware();

	Main.Redraw(NULL);//第一次创建窗口时全部重绘
	ShowWindow(Main.hWnd, nCmdShow);

	return TRUE;
}

//响应函数
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)//主窗口响应函数
{
	switch (message)
	{
	case WM_CLOSE://关闭
		PostQuitMessage(0);
		break;
	case	WM_CREATE://创建窗口
		if (Effect)
		{//启动阴影特效
			Cshadow.Initialize(hInst);
			Cshadow.Create(hWnd);
		}
		rdc = GetDC(Main.hWnd);//创建bitmap
		hdc = CreateCompatibleDC(rdc);
		hBmp = CreateCompatibleBitmap(rdc, 1330, 1100);
		SelectObject(hdc, hBmp);
		ReleaseDC(Main.hWnd, rdc);
		DragAcceptFiles(hWnd, true);
		break;

	case WM_HOTKEY://热键
	{
		Main.EditHotKey((int)wParam);//分派Edit操作的热键信息
		break;
	}
	case WM_PAINT://绘图
	{
		HBRUSH BitmapBrush = NULL;
		RECT rc; bool f = false;
		GetUpdateRect(hWnd, &rc, false);
		if (rc.top != 0)f = true;
		if (Main.hdc == NULL)Main.SetHDC(hdc);
		PAINTSTRUCT ps;
		rdc = BeginPaint(hWnd, &ps);
		if (!Main.es.empty())//根据es来擦除区域
		{
			SelectObject(Main.hdc, White);
			SelectObject(Main.hdc, WhiteBrush);
			while (!Main.es.empty())
			{
				Rectangle(Main.hdc, Main.es.top().left, Main.es.top().top, Main.es.top().right, Main.es.top().bottom);
				Main.es.pop();
			}
		}
		if (!Main.rs.empty())
		{
			while (!Main.rs.empty())
			{
				Main.RedrawObject(Main.rs.top().first, Main.rs.top().second);
				Main.rs.pop();//根据rs用redrawobject绘制
			}
			goto finish;
		}
		SetBkMode(rdc, TRANSPARENT);
		SetBkMode(hdc, TRANSPARENT);

		SelectObject(hdc, WhiteBrush);//白色背景
		Rectangle(hdc, 0, 0, (int)(900 * Main.DPI), (int)(Main.Height * Main.DPI + 1));

		SelectObject(hdc, GREEN);//绿色顶部
		SelectObject(hdc, green);
		Rectangle(hdc, 0, 0, (int)(900 * Main.DPI), (int)(50 * Main.DPI));

		SetTextColor(hdc, RGB(0, 0, 0));
		SelectObject(hdc, BLACK);
		SelectObject(hdc, WhiteBrush);

		Main.DrawEVERYTHING();//重绘全部
	finish://贴图
		BitBlt(rdc, rc.left, rc.top, max((long)(Main.Width * Main.DPI), rc.right - rc.left), max((long)(Main.Height * Main.DPI), rc.bottom - rc.top), hdc, rc.left, rc.top, SRCCOPY);
		EndPaint(hWnd, &ps);
	}
	break;

	case WM_LBUTTONDOWN://点下鼠标左键时
	{
		POINT point; GetCursorPos(&point); ScreenToClient(Main.hWnd, &point);
		Main.EditGetNewInside(point);//试图预先确定一下是否点在某个控件内
		Main.ButtonGetNewInside(point);
		Main.CheckGetNewInside(point);
		Main.AreaGetNewInside(point);//点在控件内 -> 触发控件特效
		if (Main.CurCover != -1 || Main.CoverCheck != 0 || Main.CoverEdit != 0 || Main.CoverArea != 0)Main.LeftButtonDown();
		else PostMessage(Main.hWnd, WM_SYSCOMMAND, SC_MOVE | HTCAPTION, 0);//点在外面 -> 拖动窗口
		break;
	}
	case WM_LBUTTONUP://抬起鼠标左键时
		if (Main.CurCover != -1)//这时候就要做出相应的动作了
		{
			Main.Press = 0;
			RECT rc;
			rc = Main.GetRECT(Main.CurCover);
			InvalidateRect(Main.hWnd, &rc, FALSE);
		}
		if (Main.CoverEdit == 0)Main.EditUnHotKey();
		Main.Edit[Main.CoverEdit].Press = false;
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(Main.hWnd, &point);

		unsigned int x;//通过hash来确定按钮编号
		x = Hash(Main.GetCurInsideID());//这样就可以不受编号干扰，随便在前后添加按钮了
		BUTTON_IN(x, L"QuickSetup")
		{//一键安装

			break;
		}
		BUTTON_IN(x, L"Close") { PostQuitMessage(0); }

		for (int i = 1; i <= Main.CurCheck; ++i)//分派check事件
		{
			if (Main.Check[i].Page == 0 || Main.Check[i].Page == Main.CurWnd)
			{
				int result;
				result = Main.InsideCheck(i, point);
				if (result != 0)
				{
					if (!Main.Check[i].Value)
					{//未选中->选中
						switch (i)
						{
						case 1: {
							break; }
						}
					}
					else
					{//选中 -> 未选中
						switch (i)
						{
						case 1: {break; }
						}
						
					}
					RECT rc = Main.GetRECTc(i);
						Main.Check[i].Value = 1 - Main.Check[i].Value;
						Main.Readd(3, i);
						Main.Redraw(&rc);
				}
			}
		}
		break;

	case WM_MOUSEMOVE: {Main.MouseMove(); break; }
	case WM_IME_STARTCOMPOSITION:
	{
		LOGFONT lf;
		COMPOSITIONFORM cf;
		HIMC himc = ImmGetContext(hWnd);
		if (himc)
		{
			//为Edit设置输入法显示位置。
			GetCaretPos(&point);
			cf.dwStyle = CFS_POINT;
			cf.ptCurrentPos.y = point.y + 10;
			cf.ptCurrentPos.x = point.x + 10;
			ImmSetCompositionWindow(himc, &cf);

			//设置输入法字体样式(很多时候没用)
			GetObject(Main.DefFont, sizeof(LOGFONT), &lf);
			ImmSetCompositionFont(himc, &lf);

			ImmReleaseContext(hWnd, himc);
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
		//break;
	}
	case WM_MOUSELEAVE://TrackMouseEvent带来的消息
		PostMessage(Main.hWnd, WM_MOUSEMOVE, NULL, 0);
		break;
	case WM_CHAR://给Edit转发消息
		Main.EditCHAR((wchar_t)wParam);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
