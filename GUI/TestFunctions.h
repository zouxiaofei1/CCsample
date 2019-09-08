#pragma once
#include "stdafx.h"


void charTowchar(const char *chr, wchar_t *wchar, int size);
void s(LPCWSTR cathy)
{
	MessageBox(NULL, cathy, L"", NULL);
}
void s(int cathy)
{
	wchar_t Cathytmp[34];
	_itow_s(cathy, Cathytmp, 10);
	MessageBox(NULL, Cathytmp, L"", NULL);
}

void charTowchar(const char *chr, wchar_t *wchar, int size)
{
	MultiByteToWideChar(CP_ACP, 
		0, chr, strlen(chr) + 1,
		wchar, 
		size / sizeof(wchar[0]));
}

#pragma warning(disable:4244)


unsigned int Hash(const wchar_t *str)
{
	unsigned int seed = 131;
	unsigned int hash = 0;

	while (*str)
	{
		hash = hash * seed + (*str++);
	}

	return (hash & 0x7FFFFFFF);
}
