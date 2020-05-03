/*
  KeePass Password Safe - The Open-Source Password Manager
  Copyright (C) 2003-2005 Dominik Reichl <dominik.reichl@t-online.de>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "StdAfx.h"
#include <math.h>

#include "PwUtil.h"
#include "../Util/MemUtil.h"

#define CHARSPACE_ESCAPE      60
#define CHARSPACE_ALPHA       26
#define CHARSPACE_NUMBER      10
#define CHARSPACE_SIMPSPECIAL 16
#define CHARSPACE_EXTSPECIAL  17
#define CHARSPACE_HIGH       112

// Very simple password quality estimation function
C_FN_SHARE DWORD EstimatePasswordBits(const TCHAR *pszPassword)
{
	DWORD i, dwLen, dwCharSpace, dwBits;
	BOOL bChLower = FALSE, bChUpper = FALSE, bChNumber = FALSE;
	BOOL bChSimpleSpecial = FALSE, bChExtSpecial = FALSE, bChHigh = FALSE;
	BOOL bChEscape = FALSE;
	TCHAR tch;
	double dblBitsPerChar;

	ASSERT(pszPassword != NULL); if(pszPassword == NULL) return 0;

	dwLen = (DWORD)_tcslen(pszPassword);
	if(dwLen == 0) return 0; // Zero bits of information :)

	for(i = 0; i < dwLen; i++) // Get character types
	{
		tch = pszPassword[i];

		if(tch < _T(' ')) bChEscape = TRUE;
		if((tch >= _T('A')) && (tch <= _T('Z'))) bChUpper = TRUE;
		if((tch >= _T('a')) && (tch <= _T('z'))) bChLower = TRUE;
		if((tch >= _T('0')) && (tch <= _T('9'))) bChNumber = TRUE;
		if((tch >= _T(' ')) && (tch <= _T('/'))) bChSimpleSpecial = TRUE;
		if((tch >= _T(':')) && (tch <= _T('@'))) bChExtSpecial = TRUE;
		if((tch >= _T('[')) && (tch <= _T('`'))) bChExtSpecial = TRUE;
		if((tch >= _T('{')) && (tch <= _T('~'))) bChExtSpecial = TRUE;
		if(tch > _T('~')) bChHigh = TRUE;
	}

	dwCharSpace = 0;
	if(bChEscape == TRUE) dwCharSpace += CHARSPACE_ESCAPE;
	if(bChUpper == TRUE) dwCharSpace += CHARSPACE_ALPHA;
	if(bChLower == TRUE) dwCharSpace += CHARSPACE_ALPHA;
	if(bChNumber == TRUE) dwCharSpace += CHARSPACE_NUMBER;
	if(bChSimpleSpecial == TRUE) dwCharSpace += CHARSPACE_SIMPSPECIAL;
	if(bChExtSpecial == TRUE) dwCharSpace += CHARSPACE_EXTSPECIAL;
	if(bChHigh == TRUE) dwCharSpace += CHARSPACE_HIGH;

	ASSERT(dwCharSpace != 0); if(dwCharSpace == 0) return 0;

	dblBitsPerChar = log((double)dwCharSpace) / log(2.00);
	dwBits = (DWORD)(ceil(dblBitsPerChar * (double)dwLen));

	ASSERT(dwBits != 0);
	return dwBits;
}

C_FN_SHARE BOOL LoadHexKey32(FILE *fp, BYTE *pBuf)
{
	char buf[65], ch1, ch2;
	BYTE bt;
	int i;

	ASSERT(fp != NULL); if(fp == NULL) return FALSE;
	ASSERT(pBuf != NULL); if(pBuf == NULL) return FALSE;

	buf[64] = 0;
	if(fread(buf, 1, 64, fp) != 64) { ASSERT(FALSE); return FALSE; }

	for(i = 0; i < 32; i++)
	{
		ch1 = buf[i * 2];
		ch2 = buf[i * 2 + 1];

		if((ch1 >= '0') && (ch1 <= '9')) bt = (BYTE)(ch1 - '0');
		else if((ch1 >= 'a') && (ch1 <= 'f')) bt = (BYTE)(ch1 - 'a' + 10);
		else if((ch1 >= 'A') && (ch1 <= 'F')) bt = (BYTE)(ch1 - 'A' + 10);
		else return FALSE;

		bt <<= 4;

		if((ch2 >= '0') && (ch2 <= '9')) bt |= (BYTE)(ch2 - '0');
		else if((ch2 >= 'a') && (ch2 <= 'f')) bt |= (BYTE)(ch2 - 'a' + 10);
		else if((ch2 >= 'A') && (ch2 <= 'F')) bt |= (BYTE)(ch2 - 'A' + 10);
		else return FALSE;

		pBuf[i] = bt;
	}

	mem_erase((BYTE *)buf, 64);
	return TRUE;
}

C_FN_SHARE BOOL SaveHexKey32(FILE *fp, BYTE *pBuf)
{
	char buf[65], ch1, ch2, chq;
	int i;
	BYTE bt;

	ASSERT(fp != NULL); if(fp == NULL) return FALSE;
	ASSERT(pBuf != NULL); if(pBuf == NULL) return FALSE;

	buf[64] = 0;

	for(i = 0; i < 32; i++)
	{
		bt = pBuf[i];

		chq = (char)(bt >> 4);
		if(chq < 10) ch1 = (char)(chq + '0');
		else ch1 = (char)(chq - 10 + 'a');

		chq = (char)(bt & 0x0F);
		if(chq < 10) ch2 = (char)(chq + '0');
		else ch2 = (char)(chq - 10 + 'a');

		buf[i * 2] = ch1;
		buf[i * 2 + 1] = ch2;
	}

	fwrite(buf, 1, 64, fp);

	mem_erase((BYTE *)buf, 64);
	return TRUE;
}
