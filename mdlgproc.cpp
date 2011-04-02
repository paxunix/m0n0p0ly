#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "monopoly.h"
#include "mondlg.h"
#include "monmenu.h"
#include "monstr.h"

extern char numplayers, computerplayer, gamefile[80], curplayer,
			playername[MAXPLAYERS][20], text[80], hotelsleft, housesleft;
extern HISCORE hiscores[5];
extern HANDLE hInst;
extern PROPERTY property[LASTLAND+1];
extern PLAYER player[MAXPLAYERS];
extern int dlgretval, boardvalue[LASTLAND+1], rent[LASTLAND+1][6], speedval;
extern WORD dlgparam;

char getpropertyindex(HWND hdlg, WORD listboxid)
{
// Returns the number of the property currently selected in listbox listboxid

	DWORD dwretval;
	char i, place[30], string[30];

	dwretval = SendDlgItemMessage(hdlg, listboxid, LB_GETCURSEL, 0, 0);
	SendDlgItemMessage(hdlg, listboxid, LB_GETTEXT, (int) dwretval, (LONG)(LPSTR) place);

	for (i=0; i<=LASTLAND; i++)
	{
		LoadString(hInst, i+SR_GO, string, 30);
		if (!lstrcmp(string, place))
			break;
	}

	return i;
}

void initmort(HWND hdlg, WORD listboxid, char type)
{
/* Fills listbox listboxid with curplayer's mortgaged or unmortgaged property
   depending on type.  If type==1, listbox has mortgaged property, else it
   has unmortgaged property.  Then it sets the current selection to the 1st
   string in the box
*/
	char i;

	for (i=0; i<=LASTLAND; i++)
		if (property[i].owner == curplayer)
			if ((property[i].mortgaged && type==1) || (!property[i].mortgaged && type!=1 && !property[i].numhouses))
			{
				LoadString(hInst, i+SR_GO, text, 30);
				SendDlgItemMessage(hdlg, listboxid, LB_ADDSTRING, 0, (LONG)(LPSTR) text);
			}

	SendDlgItemMessage(hdlg, listboxid, LB_SETCURSEL, 0, 0);
}

void initbuild(HWND hdlg, int listboxid, char type)
{
/*	Fill list box with blocks with/without houses depending on type.  type=1
	then listbox has properties with houses, else without houses.
*/
	char i;

	for (i=0; i<=LASTLAND; i++)
		if (property[i].owner == curplayer && rent[i][5])
			if (type)
			{
				if (property[i].numhouses)
				{
					LoadString(hInst, i+SR_GO, text, 30);
					SendDlgItemMessage(hdlg, listboxid, LB_ADDSTRING, 0, (LONG)(LPSTR) text);
				}
			}
			else
			if (colorblock(getblock(i), curplayer) && numhousesonblock(getblock(i)) != maxhousesonblock(getblock(i)))
			{
				LoadString(hInst, i+SR_GO, text, 30);
				SendDlgItemMessage(hdlg, listboxid , LB_ADDSTRING, 0, (LONG)(LPSTR) text);
			}

	SendDlgItemMessage(hdlg, listboxid, LB_SETCURSEL, 0, 0);
}

int gethousecost(char where)
{
//	Returns the cost of 1 house for property 'where'

	int housecost;

	if (where < 40)
		housecost = 200;

	if (where < 30)
		housecost = 150;

	if (where < 20)
		housecost = 100;

	if (where < 10)
		housecost = 50;

	return housecost;
}

char numhousesonblock(char block)
{
//	Returns the number of houses on block # 'block'

	char retval=0, i;

	for (i=block*5+1; i<block*5+5; i++)
		retval += property[i].numhouses;

	return retval;
}

char maxhousesonblock(char block)
{
	if (!block || block==7)
		return 10;
	else
	return 15;
}

BOOL FAR PASCAL _export gameattrDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch(wMsg)
	{
		case WM_INITDIALOG:
			SendDlgItemMessage(hdlg, IDD_INPUT1, EM_LIMITTEXT, 1, NULL);
			break;

		case WM_COMMAND:
			switch(wParam)
			{
				case 1:
				{
					GetDlgItemText(hdlg, IDD_INPUT1, text, 2);
					numplayers = text[0] - '0';

					if (numplayers==1)
					{
						computerplayer = 1;
						numplayers = 2;
						lstrcpy(playername[1], "Computer");
					}
					else
						computerplayer = 0;

					if (numplayers<1 || numplayers>MAXPLAYERS)
					{
						LoadString(hInst, SR_NUMPLAYERS, text, 80);
						MessageBox(hdlg, text, NULL, MB_OK | MB_ICONEXCLAMATION);
					}
					else
					{
						EndDialog(hdlg, TRUE);
						break;
					}
				}
				break;

				case IDD_INPUT1:
					if (HIWORD(lParam) == EN_CHANGE)
						EnableWindow(GetDlgItem(hdlg, 1),
									 (int) SendMessage(LOWORD(lParam), WM_GETTEXTLENGTH, 0, 0));
					break;
			}
			return TRUE;

		default :
			return FALSE;
	}
}

#pragma argsused
BOOL FAR PASCAL _export playernameDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch(wMsg)
	{
		case WM_INITDIALOG:
			dlgparam = LOWORD(lParam);
			SendDlgItemMessage(hdlg, IDD_INPUT1, EM_LIMITTEXT, 19, NULL);
			SetDlgItemInt(hdlg, IDD_TEXT3, dlgparam, 0);
			SetFocus(wParam);
			break;

		case WM_COMMAND:
			switch(wParam)
			{
				case 1:
					GetDlgItemText(hdlg, IDD_INPUT1, playername[dlgparam-1], 19);
					if (playername[dlgparam-1][0] == '\0')
					{
						lstrcpy(playername[dlgparam-1], "Player  ");
						playername[dlgparam-1][7] = dlgparam + '0';
					}

					EndDialog(hdlg, TRUE);
			}
			return TRUE;

		default :
			return FALSE;
	}
}

#pragma argsused
BOOL FAR PASCAL _export topfiveDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	/* Takes the hiscores[] array and shows it in a dialog box.  Make sure
	   the array is already sorted, as this routine only displays the box
	   and the data. */

	switch(wMsg)
	{
		case WM_INITDIALOG:
		{
			int c;

			for (c=IDD_TEXT1; c<=IDD_TEXT5; c++)
			{
				SetDlgItemText(hdlg, c, hiscores[c-IDD_TEXT1].name);
				SetDlgItemInt(hdlg, c+5, hiscores[c-IDD_TEXT1].score, 0);
			}
			return TRUE;
		}
		break;

		case WM_COMMAND:
			EndDialog(hdlg, TRUE);
			break;

		return TRUE;
		break;

	default : return FALSE;
	}
}

BOOL FAR PASCAL _export openDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch(wMsg)
	{
		case WM_INITDIALOG:
			if (lParam)
				SetWindowText(hdlg, "Save Game");
			else
			SetWindowText(hdlg, "Load Game");
			SendDlgItemMessage(hdlg, IDD_INPUT1, EM_LIMITTEXT, 8, 0);
			SetFocus(GetDlgItem(hdlg, IDD_INPUT1));
			break;

		case WM_COMMAND:
		{
			char *p;

			switch(wParam)
			{
				case 1:
					GetDlgItemText(hdlg, IDD_INPUT1, gamefile, 8);
					for (p=gamefile; *p!=0; p++)
						if (*p=='?' || *p=='*' || *p=='.')
						{
							gamefile[0] = 0;
							SetDlgItemText(hdlg, IDD_INPUT1, gamefile);
							goto L1;
						}

					lstrcat(gamefile, ".mon");
					dlgretval = TRUE;
					EndDialog(hdlg, TRUE);
					break;

				case 2:
					dlgretval = FALSE;
					EndDialog(hdlg, FALSE);
					break;

				case IDD_INPUT1:
					if (HIWORD(lParam) == EN_CHANGE)
						EnableWindow(GetDlgItem(hdlg, 1), (int) SendMessage(LOWORD(lParam), WM_GETTEXTLENGTH, 0, 0L));
			}
		}
L1:		return TRUE;

		default:
			return FALSE;
	}
}

#pragma argsused
BOOL FAR PASCAL _export aboutDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch(wMsg)
	{
		case WM_COMMAND:
			EndDialog(hdlg, TRUE);
			return TRUE;

		default:
			return FALSE;
	}
}

#pragma argsused
BOOL FAR PASCAL _export seecardDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch(wMsg)
	{
		case WM_INITDIALOG:
		{
			WORD i;

			dlgparam = LOWORD(lParam);
			for (i=0; i<=LASTLAND; i++)
			{
				if (dlgparam == 65535L) //Just show curplayer's property in box
				{
					if (boardvalue[i] == 0)
						continue;
				}
				else
				if (property[i].owner != dlgparam)
					continue;

				LoadString(hInst, SR_GO+i, text, 20);
				SendDlgItemMessage(hdlg, IDD_INPUT1, LB_ADDSTRING, 0, (LONG)(LPSTR) text);
			}

			SendDlgItemMessage(hdlg, IDD_INPUT1, LB_SETCURSEL, 0, 0);
			if (!SendDlgItemMessage(hdlg, IDD_INPUT1, LB_GETCOUNT, 0, 0))
				EnableWindow(GetDlgItem(hdlg, 1), FALSE);

			SetFocus(wParam);
		}
		break;

		case WM_COMMAND:
			switch(wParam)
			{
				case 2:
					dlgretval = -1;
					EndDialog(hdlg, FALSE);
					break;

				case IDD_INPUT1:
					if (HIWORD(lParam) != LBN_DBLCLK)
						break;

				case 1:
					dlgretval = getpropertyindex(hdlg, IDD_INPUT1);
					whattypecard(hdlg);
			}
			return TRUE;

		default:
			return FALSE;
	}
}

BOOL FAR PASCAL _export utilDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch(wMsg)
	{
		case WM_INITDIALOG:
		{
			dlgparam = LOWORD(lParam);
			LoadString(hInst, dlgparam+SR_GO, text, 30);
			SetDlgItemText(hdlg, IDD_TEXT1, text);
			if (property[dlgparam].owner == -1)
				sprintf(text, "Unowned");
			else
				sprintf(text, "Owner: %s", playername[property[dlgparam].owner]);

			SetDlgItemText(hdlg, IDD_TEXT2, text);
			SetFocus(wParam);
		}
		return TRUE;

		case WM_COMMAND:
			EndDialog(hdlg, TRUE);
			return TRUE;

		default:
			return FALSE;
	}
}

BOOL FAR PASCAL _export railDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch(wMsg)
	{
		case WM_INITDIALOG:
		{
			dlgparam = LOWORD(lParam);
			LoadString(hInst, dlgparam+SR_GO, text, 20);
			SetDlgItemText(hdlg, IDD_TEXT1, text);
			if (property[dlgparam].owner == -1)
				sprintf(text, "Unowned");
			else
				sprintf(text, "Owner: %s", playername[property[dlgparam].owner]);

			SetDlgItemText(hdlg, IDD_TEXT2, text);
			SetFocus(wParam);
		}
		return TRUE;

		case WM_COMMAND:
			EndDialog(hdlg, TRUE);
			return TRUE;

		default:
			return FALSE;
	}
}

BOOL FAR PASCAL _export propDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch(wMsg)
	{
		case WM_INITDIALOG:
		{
			char i;
			int housecost;

			dlgparam = LOWORD(lParam);
			LoadString(hInst, dlgparam+SR_GO, text, 20);
			SetDlgItemText(hdlg, IDD_TEXT1, text);

			if (property[dlgparam].owner == -1)
				sprintf(text, "Unowned");
			else
				sprintf(text, "Owner: %s", playername[property[dlgparam].owner]);

			SetDlgItemText(hdlg, IDD_TEXT2, text);
			sprintf(text, "Rent: $%u", rent[dlgparam][0]);
			SetDlgItemText(hdlg, IDD_TEXT3, text);

			for (i=1; i<5; i++)
				SetDlgItemInt(hdlg, IDD_TEXT3+i, rent[dlgparam][i], 0);

			sprintf(text, "With HOTEL $%u", rent[dlgparam][5]);
			SetDlgItemText(hdlg, IDD_TEXT8, text);

			housecost = gethousecost((char) dlgparam);
			sprintf(text, "Houses cost $%u each", housecost);
			SetDlgItemText(hdlg, IDD_TEXT10, text);
			sprintf(text, "Hotels, $%u plus 4 houses", housecost);
			SetDlgItemText(hdlg, IDD_TEXT11, text);
			sprintf(text, "Mortgage value $%u", boardvalue[dlgparam]/2);
			SetDlgItemText(hdlg, IDD_TEXT9, text);

			SetFocus(wParam);
		}
		return TRUE;

		case WM_COMMAND:
			EndDialog(hdlg, TRUE);
			return TRUE;

		default:
			return FALSE;
	}
}

#pragma argsused
BOOL FAR PASCAL _export taxDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	if (wMsg == WM_INITDIALOG)
		SetFocus(wParam);

	if (wMsg == WM_COMMAND)
		switch(wParam)
		{
			case IDD_INPUT1:
				dlgretval = 1;
				EndDialog(hdlg, TRUE);
				return TRUE;

			case IDD_INPUT2:
				dlgretval = 2;
				EndDialog(hdlg, TRUE);
				return TRUE;
		}
	else
		return FALSE;
}

#pragma argsused
BOOL FAR PASCAL _export injailDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch(wMsg)
	{
		case WM_INITDIALOG:
			SetFocus(wParam);
			if (player[curplayer].jailcards == 0)
				EnableWindow(GetDlgItem(hdlg, IDD_INPUT3), 0);

			if (player[curplayer].money < 50)
				EnableWindow(GetDlgItem(hdlg, IDD_INPUT2), 0);

			break;

		case WM_COMMAND:
			switch(wParam)
			{
				case IDD_INPUT1:
					dlgretval = 1;
					break;
				case IDD_INPUT2:
					dlgretval = 2;
					break;
				case IDD_INPUT3:
					dlgretval = 3;
					break;
			}
			EndDialog(hdlg, TRUE);
			return TRUE;

		default : return FALSE;
	}
}

#pragma argsused
BOOL FAR PASCAL _export unownedDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch(wMsg)
	{
		case WM_INITDIALOG:
		{
			LoadString(hInst, player[curplayer].where+SR_GO, text, 30);
			SetDlgItemText(hdlg, IDD_TEXT1, text);
			sprintf(text, "Cost: $%u", boardvalue[player[curplayer].where]);
			SetDlgItemText(hdlg, IDD_TEXT2, text);
			SetFocus(wParam);
		}
		break;

		case WM_COMMAND:
			switch(wParam)
			{
				case IDD_INPUT1:
					dlgretval = 1;
					break;
				case IDD_INPUT3:
					dlgretval = 3;
			}
			EndDialog(hdlg, TRUE);
			return TRUE;

		default:
			return FALSE;
	}
}

#pragma argsused
BOOL FAR PASCAL _export sellpropertyDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch (wMsg)
	{
		case WM_INITDIALOG:
		{
			char i, flag=0;

			for (i=0; i<numplayers; i++)
			{
				SetDlgItemText(hdlg, IDD_INPUT1+i, playername[i]);

				if (i!=curplayer)
					flag = 1;
				else
				flag = 0;

				EnableWindow(GetDlgItem(hdlg, IDD_INPUT1+i), flag);
				if (flag)
					CheckRadioButton(hdlg, IDD_INPUT1, IDD_INPUT4, IDD_INPUT1+i);
			}

			for (i=0; i<=LASTLAND; i++)
				if (property[i].owner == curplayer && !property[i].numhouses)
				{
					LoadString(hInst, SR_GO+i, text, 80);
					SendDlgItemMessage(hdlg, IDD_INPUT5, LB_ADDSTRING, 0, (LONG)(LPSTR) text);
				}

			SendDlgItemMessage(hdlg, IDD_INPUT5, LB_SETCURSEL, 0, 0);
			SetFocus(wParam);
		}
		break;

		case WM_COMMAND:
		{
			char i;
			int amount;

			switch (wParam)
			{
				case 2:
					EndDialog(hdlg, FALSE);
					break;

				case IDD_INPUT1:
				case IDD_INPUT2:
				case IDD_INPUT3:
				case IDD_INPUT4:
					CheckRadioButton(hdlg, IDD_INPUT1, IDD_INPUT4, wParam);
					break;

				case 1:
					amount = GetDlgItemInt(hdlg, IDD_INPUT6, NULL, 0);
					for (i=IDD_INPUT1; i<=IDD_INPUT4; i++)
						if (SendDlgItemMessage(hdlg, i, BM_GETCHECK, 0, 0))
							break;

					i -= IDD_INPUT1;
					if (amount > player[i].money || amount < 1)
						break;

					if (deduct(amount, i, BANK, 1) == TRUE)
					{
						property[getpropertyindex(hdlg, IDD_INPUT5)].owner = i;
						player[curplayer].money += amount;
					}
					else
					break;

					EndDialog(hdlg, TRUE);
			}
		}
		return TRUE;

		default:
			return FALSE;
	}
}

#pragma argsused
BOOL FAR PASCAL _export buypropertyDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch (wMsg)
	{
		case WM_INITDIALOG:
		{
			char i, i2;
			int  who;

			for (i=0; i<numplayers; i++)
			{
				SetDlgItemText(hdlg, IDD_INPUT1+i, playername[i]);
				EnableWindow(GetDlgItem(hdlg, IDD_INPUT1+i), 0);
				for (i2=0; i2<=LASTLAND; i2++)
					if (property[i2].owner==i && i!=curplayer)
					{
						EnableWindow(GetDlgItem(hdlg, IDD_INPUT1+i), 1);
						CheckRadioButton(hdlg, IDD_INPUT1, IDD_INPUT4, IDD_INPUT1+i);
						break;
					}
			}

			for (who=0; who<numplayers; who++)
				if (SendDlgItemMessage(hdlg, IDD_INPUT1+who, BM_GETCHECK, 0, 0))
					break;

			for (i=0; i<=LASTLAND; i++)
				if (property[i].owner == who && !numhousesonblock(getblock(i)))
				{
					LoadString(hInst, i+SR_GO, text, 80);
					SendDlgItemMessage(hdlg, IDD_INPUT5, LB_ADDSTRING, 0, (LONG)(LPSTR) text);
				}

			SendDlgItemMessage(hdlg, IDD_INPUT5, LB_SETCURSEL, 0, 0);
			SetFocus(wParam);
		}
		break;

		case WM_COMMAND:
		{
			char i, who;
			int amount;

			switch (wParam)
			{
				case 2:
					EndDialog(hdlg, FALSE);
					break;

				case IDD_INPUT1:
				case IDD_INPUT2:
				case IDD_INPUT3:
				case IDD_INPUT4:
					SendDlgItemMessage(hdlg, IDD_INPUT5, LB_RESETCONTENT, 0, 0);
					CheckRadioButton(hdlg, IDD_INPUT1, IDD_INPUT4, wParam);
					for (i=0; i<=LASTLAND; i++)
						if (property[i].owner == wParam-IDD_INPUT1)
						{
							LoadString(hInst, i+SR_GO, text, 80);
							SendDlgItemMessage(hdlg, IDD_INPUT5, LB_ADDSTRING, 0, (LONG)(LPSTR) text);
						}

					SendDlgItemMessage(hdlg, IDD_INPUT5, LB_SETCURSEL, 0, 0);
					break;

				case 1:
					for (who=0; who<numplayers; who++)
						if (SendDlgItemMessage(hdlg, IDD_INPUT1+who, BM_GETCHECK, 0, 0))
							break;

					amount = GetDlgItemInt(hdlg, IDD_INPUT6, NULL, 0);
					if (amount > player[curplayer].money || amount < 1)
						break;

					if (deduct(amount, curplayer, BANK, 1) == TRUE)
					{
						property[getpropertyindex(hdlg, IDD_INPUT5)].owner = curplayer;
						player[who].money += amount;
					}
					else
					break;

					EndDialog(hdlg, TRUE);
			}
		}
		return TRUE;

		default:
			return FALSE;
	}
}

#pragma argsused
BOOL FAR PASCAL _export mortgagepropertyDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch (wMsg)
	{
		case WM_INITDIALOG:
			initmort(hdlg, IDD_INPUT1, 0);
			SetDlgItemInt(hdlg, IDD_TEXT1, boardvalue[getpropertyindex(hdlg, IDD_INPUT1)] / 2, 0);
			SetFocus(wParam);
			break;

		case WM_COMMAND:
		{
			WORD wNotify = HIWORD(lParam);

			switch (wParam)
			{
				case 2:
					EndDialog(hdlg, FALSE);
					break;

				case IDD_INPUT1:
					if (wNotify == LBN_DBLCLK)
						goto DoOK;

					if (wNotify == LBN_SELCHANGE)
						SetDlgItemInt(hdlg, IDD_TEXT1, boardvalue[getpropertyindex(hdlg, IDD_INPUT1)] / 2, 0);

					break;

				case 1:
				DoOK:
				{
					char where = getpropertyindex(hdlg, IDD_INPUT1);
					property[where].mortgaged = 1;
					player[curplayer].money += (boardvalue[where] / 2);
					EndDialog(hdlg, TRUE);
				}
			}
		}
		return TRUE;

		default:
			return FALSE;
	}
}

#pragma argsused
BOOL FAR PASCAL _export unmortgagepropertyDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	char index;

	switch (wMsg)
	{
		case WM_INITDIALOG:
			initmort(hdlg, IDD_INPUT1, 1);
			index = getpropertyindex(hdlg, IDD_INPUT1);
			SetDlgItemInt(hdlg, IDD_TEXT1, boardvalue[index]/2+boardvalue[index]/20, 0);
			SetFocus(wParam);
			break;

		case WM_COMMAND:
		{
			WORD wNotify = HIWORD(lParam);

			switch (wParam)
			{
				case 2:
					EndDialog(hdlg, FALSE);
					break;

				case IDD_INPUT1:
					if (wNotify == LBN_DBLCLK)
						goto DoOK;

					if (wNotify == LBN_SELCHANGE)
					{
						index = getpropertyindex(hdlg, IDD_INPUT1);
						SetDlgItemInt(hdlg, IDD_TEXT1, boardvalue[index]/2+boardvalue[index]/20, 0);
					}
					break;

				case 1:
				DoOK:
				{
					index = getpropertyindex(hdlg, IDD_INPUT1);
					if (deduct(boardvalue[index]/2+boardvalue[index]/20, curplayer, BANK, 1) == TRUE)
						property[index].mortgaged = 0;
					else
					break;

					EndDialog(hdlg, TRUE);
				}
				break;
			}
		}
		return TRUE;

		default:
			return FALSE;
	}
}

#pragma argsused
BOOL FAR PASCAL _export sellhousesDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	char where;
	int amount, oldamount;
	WORD wNotify;

	switch (wMsg)
	{
		case WM_INITDIALOG:
			initbuild(hdlg, IDD_INPUT1, 1);
			SetDlgItemInt(hdlg, IDD_TEXT1, gethousecost(getpropertyindex(hdlg, IDD_INPUT1))/2, 0);
			SetFocus(wParam);
			break;

		case WM_COMMAND:
			switch (wParam)
			{
				case 2:
					dlgretval = -1;
					EndDialog(hdlg, FALSE);
					break;

				case IDD_INPUT1:
					wNotify = HIWORD(lParam);
					if (wNotify == LBN_DBLCLK)
						goto DoOK;

					if (wNotify == LBN_SELCHANGE)
					{
L3:						amount = GetDlgItemInt(hdlg, IDD_INPUT2, NULL, 0);
						SetDlgItemInt(hdlg, IDD_TEXT1, gethousecost(getpropertyindex(hdlg, IDD_INPUT1))/2*amount, 0);
					}

					break;

				case IDD_INPUT2:
					if (HIWORD(lParam) == EN_CHANGE);
						goto L3;

					break;

				case 1:
				DoOK:
				{
					oldamount = amount = GetDlgItemInt(hdlg, IDD_INPUT2, NULL, 0);
					where = getpropertyindex(hdlg, IDD_INPUT1);
					if (amount > numhousesonblock(getblock(where)) || !amount)
						break;

					hotelsleft += (amount/5);
					housesleft += (amount - amount/5*5);

					amount *= (gethousecost(where) / 2);
					player[curplayer].money += amount;
					EndDialog(hdlg, TRUE);
					dlgretval = getblock(where)+(oldamount << 8);
				}
			}
			return TRUE;

		default:
			return FALSE;
	}
}

#pragma argsused
BOOL FAR PASCAL _export buyhousesDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	char where, i, i2;
	int amount, oldamount;
	WORD wNotify;

	switch (wMsg)
	{
		case WM_INITDIALOG:
			initbuild(hdlg, IDD_INPUT1, 0);
			SetDlgItemInt(hdlg, IDD_TEXT1, gethousecost(getpropertyindex(hdlg, IDD_INPUT1)), 0);
			SetFocus(wParam);
			break;

		case WM_COMMAND:
			switch (wParam)
			{
				case 2:
					dlgretval = -1;
					EndDialog(hdlg, FALSE);
					break;

				case IDD_INPUT1:
					wNotify = HIWORD(lParam);
					if (wNotify == LBN_DBLCLK)
						goto DoOK;

					if (wNotify == LBN_SELCHANGE)
					{
L3:						amount = GetDlgItemInt(hdlg, IDD_INPUT2, NULL, 0);
						SetDlgItemInt(hdlg, IDD_TEXT1, gethousecost(getpropertyindex(hdlg, IDD_INPUT1))*amount, 0);
					}

					break;

				case IDD_INPUT2:
					if (HIWORD(lParam) == EN_CHANGE);
						goto L3;

					break;

				case 1:
				DoOK:
				{
					char oldhot, oldhou;

					oldamount = amount = GetDlgItemInt(hdlg, IDD_INPUT2, NULL, 0);
					where = getpropertyindex(hdlg, IDD_INPUT1);
					if (amount*gethousecost(where) > player[curplayer].money || !amount || amount > 15)
						break;

					amount += numhousesonblock(getblock(where));
					if ((getblock(where)==7 || !getblock(where)) && amount>10)
						break;

					oldhot = hotelsleft;
					oldhou = housesleft;

					hotelsleft -= (amount/5);
					housesleft -= (amount - amount/5*5);
					if (hotelsleft < 0 || housesleft < 0)
					{
						MessageBox(hdlg, "Not enough buildings left!", "Try Again", MB_OK);
						hotelsleft = oldhot;
						housesleft = oldhou;
						break;
					}

					for (i=getblock(where)*5; i<=getblock(where)*5+5; i++)
						if (rent[i][5])
							property[i].numhouses = 0;

					i = getblock(where)*5+1;
					i2 = i+3;
					do {
							if (rent[i][5])
							{
								property[i].numhouses++;
								amount--;
							}

							i++;
							if (i>i2)
								i = getblock(where)*5+1;
						} while (amount);

					deduct(oldamount*gethousecost(where), curplayer, BANK, 1);
					EndDialog(hdlg, TRUE);
					dlgretval = getblock(where);
				}
			}
			return TRUE;

		default:
			return FALSE;
	}
}

BOOL FAR PASCAL _export movespeedDlgProc(HWND hdlg, WORD wMsg, WORD wParam, LONG lParam)
{
	switch (wMsg)
	{
		case WM_INITDIALOG:
			speedval = 500 - speedval;
			SetScrollRange(GetDlgItem(hdlg, IDD_INPUT1), SB_CTL, 0, 500, FALSE);
			SetScrollPos(GetDlgItem(hdlg, IDD_INPUT1), SB_CTL, speedval, TRUE);
			SetFocus(wParam);
			break;

		case WM_HSCROLL:
			switch (wParam)
			{
				case SB_PAGEUP:
				case SB_LINEUP:
					speedval = max(0, speedval-50);
					break;

				case SB_PAGEDOWN:
				case SB_LINEDOWN:
					speedval = min(500, speedval+50);
					break;

				case SB_THUMBTRACK:
				case SB_THUMBPOSITION:
					speedval = LOWORD(lParam);
			}

			SetScrollPos(GetDlgItem(hdlg, IDD_INPUT1), SB_CTL, speedval, TRUE);
			break;

		case WM_COMMAND:
			if (wParam == 1)
			{
				speedval = 500 - speedval;
				EndDialog(hdlg, TRUE);
			}
			return TRUE;

		default:
			return FALSE;
	}
}
