#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mem.h>
#include "monopoly.h"
#include "mondlg.h"
#include "monmenu.h"
#include "monstr.h"

extern PLAYER player[MAXPLAYERS];
extern char	playername[MAXPLAYERS][20], curplayer, doubles, text[80],
			chestcards[16], comchest, numplayers, chance, chancecards[16],
			globalflag;
extern HANDLE hInst;
extern HWND	phwnd;
extern HBITMAP hbm;
extern globalhdc;
extern int dlgretval, boardvalue[LASTLAND+1], rent[LASTLAND+1][6],
		   squareloc[LASTLAND+1][4], playeroffset[MAXPLAYERS][2], speedval;
extern DICE	dice;
extern PROPERTY	property[LASTLAND+1];
extern COLORREF	playercolor[MAXPLAYERS];

void updateplayerbox(char who)
{
	TEXTMETRIC tm;
	int y;

	y = -(55 + (who*281));
	GetTextMetrics(globalhdc, &tm);
	if (playername[who][0] == 0)
	{
		lstrcpy(text, "Player: ");
		text[8] = who + '1';
	}
	else
		lstrcpy(text, playername[who]);

	TextOut(globalhdc, 1576, y-28, text, lstrlen(text));
	sprintf(text, "Cash: $%-7u", (player[who].money==-1) ? 0 : player[who].money);
	TextOut(globalhdc, 1576, y-28-tm.tmHeight, text, lstrlen(text));
	sprintf(text, "Jail cards: %-2u ", player[who].jailcards);
	TextOut(globalhdc, 1576, y-28-tm.tmHeight*2, text, lstrlen(text));
}

COLORREF docolorbar(int where)
{
	switch(getblock(where))
	{
		case 0: return RGB(128,0,64);
		case 1: return RGB(0,255,255);
		case 2: return RGB(128,0,128);
		case 3: return RGB(224,159,0);
		case 4: return RGB(178,0,0);
		case 5: return RGB(255,255,0);
		case 6: return RGB(0,98,0);
		case 7: return RGB(0,0,255);
	}
}

void sendtojail()
{
	placemarkers(0, curplayer);
	player[curplayer].where = LASTLAND+1;
	placemarkers(1, curplayer);
	player[curplayer].rolls = doubles = 0;
}

void doincometax(void)
{
	WORD deduction=200, worth;

	ModalDialog tax(hInst, 18, phwnd, (FARPROC) taxDlgProc);

	worth = calctotalworth(curplayer) / 10;
	if (dlgretval == 2)
	{
		deduction = worth;
		sprintf(text, "Good choice.  The 10%% only costs you,\012"
					  "$%u, instead of $200", worth);
		MessageBox(phwnd, text, "Pay Up!", MB_OK);
	}
	else
	if (deduction > worth)
	{
		sprintf(text, "Bad choice.  If you had chosen 10%%,\012"
					  "it would have cost you $%u", worth);
		MessageBox(phwnd, text, "(Sucker!)", MB_OK);
	}

	deduct(deduction, curplayer, BANK, 0);
	updateplayerbox(curplayer);
}

void docomchest(void)
{
	char cardpicked, buf[160];

	do {
		cardpicked = chestcards[comchest];
		comchest++;
		if (comchest>15)
			comchest=0;
	} while (cardpicked==-1);

	cardpicked += SR_CHEST_1;

	LoadString(hInst, cardpicked, buf, 160);
	MessageBox(phwnd, buf, "Community Chest", MB_OK);

	switch(cardpicked)
	{
		case SR_CHEST_1:
			player[curplayer].money += 100;
			break;
		case SR_CHEST_2:
			player[curplayer].money += 100;
			break;
		case SR_CHEST_3:
			deduct(100, curplayer, BANK, 0);
			break;
		case SR_CHEST_4:
			player[curplayer].money += 20;
			break;
		case SR_CHEST_5:
			streetrepairs(40, 115);
			break;
		case SR_CHEST_6:
			player[curplayer].money += 25;
			break;
		case SR_CHEST_7:
			player[curplayer].money += 200;
			break;
		case SR_CHEST_8:
			dice.die1 = 0;
			dice.die2 = (LASTLAND+1)-player[curplayer].where;
			moveplayer();
			break;
		case SR_CHEST_9:
		{
			char i;

			for (i=0; i<numplayers; i++)
			{
				if (i == curplayer)
					continue;

				if (deduct(50, i, curplayer, 0) == TRUE)
					player[curplayer].money += 50;
			}
		}
		break;
		case SR_CHEST_10:
		{
			int which=comchest;

			if (which-1<0)
				which=15;

			chestcards[which]=-1;
			player[curplayer].jailcards++;
		}
		break;
		case SR_CHEST_11:
			player[curplayer].money += 10;
			break;
		case SR_CHEST_12:
			sendtojail();
			break;
		case SR_CHEST_13:
			player[curplayer].money += 100;
			break;
		case SR_CHEST_14:
			player[curplayer].money += 45;
			break;
		case SR_CHEST_15:
			deduct(50, curplayer, BANK, 0);
			break;
		case SR_CHEST_16:
			deduct(150, curplayer, BANK, 0);
	}

	updateplayerbox(curplayer);
}

void streetrepairs(char perhouse, char perhotel)
{
	char i;
	int total=0;

	for (i=0; i<=LASTLAND; i++)
		if (property[i].owner == curplayer)
			if (property[i].numhouses == 5)
				total += perhotel;
			else
				total += (perhouse * property[i].numhouses);

	if (total == 0)
	{
		MessageBox(phwnd, "Since you have no buildings,\012you don't owe anything!", "Street Repairs", MB_OK);
		return;
	}

	sprintf(text, "That'll cost you $%u", total);
	MessageBox(phwnd, text, "Street Repairs", MB_OK | MB_ICONEXCLAMATION);
	deduct(total, curplayer, BANK, 0);
}

int dochance(void)
{
	char cardpicked, buf[160];

	do {
		cardpicked = chancecards[chance];
		chance++;
		if (chance>15)
			chance=0;
	} while (cardpicked==-1);

	cardpicked += SR_CHANCE_1;

	LoadString(hInst, cardpicked, buf, 160);
	MessageBox(phwnd, buf, "Chance", MB_OK | MB_ICONQUESTION);

	switch(cardpicked)
	{
		case SR_CHANCE_1:
			dice.die1 = 0;
			dice.die2 = LASTLAND - player[curplayer].where;
			moveplayer();
			break;

		case SR_CHANCE_2:
			streetrepairs(25, 100);
			break;

		case SR_CHANCE_3:
		{
			char i;

			for (i=0; i<numplayers; i++)
				if (i != curplayer)
				{
					if (deduct(50, curplayer, i, 0) == TRUE)
						player[i].money += 50;

					updateplayerbox(i);
				}
		}
		break;

		case SR_CHANCE_4:
			player[curplayer].money += 50;
			break;

		case SR_CHANCE_5:
			dice.die1 = 0;
			dice.die2 = 24 - player[curplayer].where;
			if (dice.die2 == -12)
				dice.die2 = 28;

			moveplayer();
			break;

		case SR_CHANCE_6:
			dice.die1 = 0;
			dice.die2 = (LASTLAND+1) - (player[curplayer].where - 5);
			moveplayer();
			break;

		case SR_CHANCE_7:
			dice.die1 = 0;
			dice.die2 = 1;
			while (player[curplayer].where != 12 && player[curplayer].where !=28)
				moveplayer();
			return 1;

		case SR_CHANCE_8:
		{
			int which=chance;

			if (which-1<0)
				which=15;
			else
			which--;

			chancecards[which]=-1;
			player[curplayer].jailcards++;
		}
		break;

		case SR_CHANCE_9:
		case SR_CHANCE_10:
		{
			int where;

			where = player[curplayer].where;
			dice.die1 = 0;
			dice.die2 = 1;
			while (where!=5 && where !=15 && where!=25 && where!=35)
			{
				moveplayer();
				where = player[curplayer].where;
			}
			return 3;
		}

		case SR_CHANCE_11:
			player[curplayer].money += 150;
			break;

		case SR_CHANCE_12:
			dice.die1 = 0;
			dice.die2 = 11 - player[curplayer].where;
			if (dice.die2 == -11)
				dice.die2 = 29;
			if (dice.die2 == -25)
				dice.die2 = 15;

			moveplayer();
			break;

		case SR_CHANCE_13:
			deduct(15, curplayer, BANK, 0);
			break;

		case SR_CHANCE_14:
			dice.die1 = 0;
			dice.die2 = (LASTLAND+1) - player[curplayer].where;
			moveplayer();
			break;

		case SR_CHANCE_15:
			placemarkers(0, curplayer);
			player[curplayer].where -= 3;
			placemarkers(1, curplayer);
			break;

		case SR_CHANCE_16:
			sendtojail();
	}

	updateplayerbox(curplayer);
	return 0;
}

BOOL deduct(int amount, char who, char towhom, char cancancel)
{ //Returns FALSE if who has been bankrupted, TRUE if deduction occurred,
  //42 if cancelled
	int retval;

	if (player[who].money < amount && cancancel)
		return 42;

L1:	if (player[who].money < amount)
	{
		HMENU hmenu = GetSubMenu(GetMenu(phwnd), 1);
		RECT r;

		GetClientRect(phwnd, &r);
		r.left = (r.right-r.left) / 2;
		r.top  = (r.bottom-r.top) / 2;

		globalflag = 1;
		initbusmenu();
		if (GetMenuState(hmenu, IDM_SELLLAND, MF_BYCOMMAND) == (MF_UNCHECKED | MF_GRAYED) &&
			GetMenuState(hmenu, IDM_SELLHOUSE,MF_BYCOMMAND) == (MF_UNCHECKED | MF_GRAYED) &&
			GetMenuState(hmenu, IDM_MORTGAGE, MF_BYCOMMAND) == (MF_UNCHECKED | MF_GRAYED))
		{
			retval = FALSE;
			bankrupt(towhom);
			player[who].money = -1; //Player bankrupt
		}
		else
		{
			retval = TrackPopupMenu(hmenu, 0, r.left, r.top, 0, phwnd, NULL);
			goto L1;
		}
	}
	else
	{
		player[who].money -= amount;
		retval = TRUE;
	}

	updateplayerbox(who);
	globalflag = 0;
	return retval;
}

void doproperty(int chanceretval)
{
	int where, bill;
	char place[20], multiple=4;

	where = player[curplayer].where;
	LoadString(hInst, where+SR_GO, place, 30);

	if (property[where].owner == curplayer)
	{
		sprintf(text, "You own %s.", place);
		MessageBox(phwnd, text, "Can't Buy It", MB_OK);
		return;
	}

	//Check if unowned
	if (property[where].owner == -1)
	{
		ModalDialog unowned(hInst, 15, phwnd, (FARPROC) unownedDlgProc);

		switch(dlgretval)
		{
			case 1:
				if (deduct(boardvalue[where], curplayer, BANK, 1) == TRUE)
					property[where].owner = curplayer;
				else
					MessageBox(phwnd, "You need more cash to buy that!", "Dork!", MB_OK);

				break;
		}
		return;
	}

	//Property must be owned
	if (where==12 || where==28)
	{
		if (chanceretval && property[28].owner == property[12].owner)
			multiple = 100;
		else
		if (chanceretval)
			multiple = 40;
		else
		if (!chanceretval && property[28].owner == property[12].owner)
			multiple = 10;

		bill = (dice.die1 + dice.die2) * multiple;
	}
	else
	if (where==5 || where==15 || where==25 || where==35)
	{
		char i, numowned=0;

		for (i=5; i<=LASTLAND; i+=10)
			if (property[i].owner == property[where].owner)
				numowned++;

		if (chanceretval)
			bill = rent[where][numowned-1] * 2;
		else
			bill = rent[where][numowned-1];
	}
	else
	{
		bill = rent[where][property[where].numhouses];
		if (colorblock(getblock(where), property[where].owner) && !numhousesonblock(getblock(where)))
			bill *= 2;
	}

	if (property[where].mortgaged)
	{
		sprintf(text, "%s is mortgaged,\12so you owe nothing.", place);
		bill = 0;
	}
	else
	sprintf(text, "%s owns %s\12You owe $%u for rent.", playername[property[where].owner], place, bill);

	MessageBox(phwnd, text, "Can't Buy It", MB_OK);

	if (deduct(bill, curplayer, property[where].owner, 0) == TRUE)
		player[property[where].owner].money += bill;

	updateplayerbox(property[where].owner);
}

void bankrupt(char towhom)
{
	int i, i2;
	char text[130], temp[20];

	LoadString(hInst, SR_PLAYERBANKRUPT, text, 130);
	LoadString(hInst, random(5) + SR_KNITTING, temp, 20);
	lstrcat(text, temp);

	//Dole out property
	for (i=0; i<=LASTLAND; i++)
		if (property[i].owner == curplayer)
			if (towhom == BANK)
				property[i].owner = -1;
			else
				property[i].owner = towhom;

	//Dole out who's jail cards (if any)
	if (towhom == BANK)
		for (i2=0; i2<player[curplayer].jailcards; i2++)
			for (i=0; i<16; i++)
			{
				if (chancecards[i] == -1)
				{
					chancecards[i] = 7;
					break;
				}
				if (chestcards[i] == -1)
				{
					chestcards[i] = 10;
					break;
				}
			}
	else
	player[towhom].jailcards += player[curplayer].jailcards;

	if (towhom != BANK)
		player[towhom].money += player[curplayer].money;

	player[curplayer].jailcards = 0;
	MessageBox(phwnd, text, "Bankrupt!", MB_OK);
}

WORD calctotalworth(int who)
{
	WORD total=0, i;

	total += player[who].money;

	for (i=0; i<=LASTLAND; i++)
		if (property[i].owner == who)
		{
			if (property[i].mortgaged)
				total += boardvalue[i] / 2;
			else
			total += boardvalue[i];

			if (property[i].numhouses)
				total += property[i].numhouses * gethousecost(i)/2;
		}
	return total;
}

void moveplayer(void)
{
	int dest;
	DWORD ti;

	SetCursor(LoadCursor((HANDLE) NULL, IDC_WAIT));
	dest = player[curplayer].where + dice.die1 + dice.die2;
	if (dest>LASTLAND)
		dest-=(LASTLAND+1);

L1:	ti = GetCurrentTime();
	while (abs(GetCurrentTime()-ti) < speedval);

	if (player[curplayer].where==dest)
	{
		SetCursor(LoadCursor((HANDLE) NULL, IDC_ARROW));
		return;
	}

	paintsquares();

	if (player[curplayer].where==0)
	{
		player[curplayer].money += 200;
		updateplayerbox(curplayer);
	}

	goto L1;
}

void paintsquares(void)
{
	int source;

	source = player[curplayer].where;
	placemarkers(0, curplayer); //Remove curplayer

	source++;
	if (source > LASTLAND)
		source = 0;

	player[curplayer].where = source;
	placemarkers(1, curplayer); //Redisplay curplayer having saved bitmap
}

void placemarkers(char putemon, char i)
// putemon=0 - restore saved bitmap at player[i] position
// putemon=1 - save bitmap at player[i] position and paint him on there
{
	int x1, y1;
	HPEN hpen, oldpen;
	HBRUSH hbrush, hbrold;
	LOGBRUSH logbrush = { BS_SOLID, playercolor[i], 0 };

	if (player[i].where == 10)
		switch(i)
		{
			case 0: x1 = 5; y1 = -1232; break;
			case 1: x1 = 5; y1 = -1276; break;
			case 2: x1 = 5; y1 = -1320; break;
			case 3: x1 = 5; y1 = -1364;
		}
	else
	{
		if (player[i].where == (LASTLAND+1))
		{
			x1 = squareloc[10][0];
			y1 = squareloc[10][1];
		}
		else
		{
			x1 = squareloc[player[i].where][0];
			y1 = squareloc[player[i].where][1];
		}

		x1 += playeroffset[i][0];
		y1 -= playeroffset[i][1];

		if (player[i].where>10 && player[i].where<20 && rent[player[i].where][5])
			x1 -= 146;

		if (player[i].where>20 && player[i].where<30 && rent[player[i].where][5])
			y1 += 146;
	}

	if (!putemon)
	{
		HDC hdcbitmap = CreateCompatibleDC(globalhdc);
		SetMapMode(hdcbitmap, MM_LOMETRIC);
		SelectObject(hdcbitmap, hbm);
		BitBlt(globalhdc, x1, y1, TOKENSIZE, -TOKENSIZE,
			   hdcbitmap, playeroffset[i][0], -playeroffset[i][1], SRCCOPY);
		DeleteDC(hdcbitmap);
		return;
	}

	if (putemon == 1)
	{
		HDC hdcbitmap = CreateCompatibleDC(globalhdc);
		SetMapMode(hdcbitmap, MM_LOMETRIC);
		SelectObject(hdcbitmap, hbm);
		BitBlt(hdcbitmap, playeroffset[i][0], -playeroffset[i][1], TOKENSIZE, -TOKENSIZE,
			   globalhdc, x1, y1, SRCCOPY);
		DeleteDC(hdcbitmap);
	}

	hpen = CreatePen(PS_INSIDEFRAME, 5, RGB(0,0,0));
	oldpen = SelectObject(globalhdc, hpen);
	hbrush = CreateBrushIndirect(&logbrush);
	hbrold = SelectObject(globalhdc, hbrush);
	Rectangle(globalhdc, x1, y1, x1+TOKENSIZE, y1-TOKENSIZE);
	SelectObject(globalhdc, oldpen);
	SelectObject(globalhdc, hbrold);
	DeleteObject(hpen);
	DeleteObject(hbrush);
}

char inplayerbox(POINT& p)
{
	POINT pr[2];
	RECT r;
	char i;

	for (i=0; i<numplayers; i++)
	{
		pr[0].x = 1550;
		pr[1].x = 1981;
		pr[0].y = -(55 + i * (226 + 55));
		pr[1].y = -(-pr[0].y + 226);
		LPtoDP(globalhdc, pr, 2);
		SetRect(&r, pr[0].x, pr[0].y, pr[1].x, pr[1].y);
		if (PtInRect(&r, p))
			return i;
	}
	return -1;
}

char inpropbox(POINT& p)
{
	POINT pt;
	RECT r;
	char i, side=4, prop;

	pt.x = p.x;
	pt.y = -p.y;

	DPtoLP(globalhdc, &pt, 1);
	SetRect(&r, 0, 187, 187, 1230);
	if (PtInRect(&r, pt))
		side = 0; //left
	else
	{
		SetRect(&r, 187, 0, 1230, 187);
		if (PtInRect(&r, pt))
			side = 1; //top
	}

	SetRect(&r, 1230, 187, 1432, 1230);
	if (PtInRect(&r, pt))
		side = 2; //right

	SetRect(&r, 187, 1230, 1230, 1432);
	if (PtInRect(&r, pt))
		side = 3; //bottom

	if (side == 4)
		return -1;

	pt.y = -pt.y;
	if (!side || side==2)
		for (i=0; i<9; i++)
			if (pt.y <= -(187+(i*115)) && pt.y > -(187+(i*115)+115))
				break;

	if (side==1 || side==3)
		for (i=0; i<9; i++)
			if (pt.x >= 187+(i*115) && pt.x < 187+(i*115)+115)
				break;

	if (!side)
		prop = 19-i;

	if (side==1)
		prop = 21+i;

	if (side==2)
		prop = 31+i;

	if (side==3)
		prop = 9-i;

	if (boardvalue[prop] && prop<40 && prop>0)
		return prop;
	else
		return -1;
}

void hilitecurplayer(void)
{
	RECT r;
	POINT p[2];

	p[0].x = 1540;
	p[1].x = 1991;
	p[0].y = -(45 + curplayer * (226 + 55));
	p[1].y = -(-p[0].y + 246);
	LPtoDP(globalhdc, p, 2);
	SetRect(&r, p[0].x, p[0].y, p[1].x, p[1].y);
	SetMapMode(globalhdc, MM_TEXT);
	DrawFocusRect(globalhdc, &r);
	SetMapMode(globalhdc, MM_LOMETRIC);
}

BOOL colorblock(char whichblock, char owner) //Does owner own whichblock
{
	char i;

	for(i=(whichblock*5)+1; i<(whichblock*5)+5; i++)
		if (rent[i][5])
			if (property[i].owner == -1 || property[i].owner != owner)
				return FALSE;

	return TRUE;
}

void whattypecard(HWND hwnd)
{
	if (dlgretval==-1)
		return;

	if (dlgretval==12 || dlgretval==28) //Utilities
	{
		ModalDialog card(hInst, 14, hwnd, (FARPROC) utilDlgProc, dlgretval);
	}
	else
	if (dlgretval==5 || dlgretval==15 || dlgretval==25 || dlgretval==35) //Railroads
	{
		ModalDialog card1(hInst, 13, hwnd, (FARPROC) railDlgProc, dlgretval);
	}
	else //Must be a property
	{
		ModalDialog card2(hInst, 12, hwnd, (FARPROC) propDlgProc, dlgretval);
	}
}

void drawhouses(int param)
{
// Draws all the buildings on block param

	HPEN hpen, oldpen;
	char i, i2, flag;
	int x, y, x2, y2, oldy, oldx;

	if (param == -1)
		return;

	hpen = CreatePen(PS_INSIDEFRAME, 2, RGB(130,130,130));
	oldpen = SelectObject(globalhdc, hpen);

	for (i=param*5+1; i<param*5+5; i++)
		if (property[i].numhouses)
		{
			for (i2=0; i2<numplayers; i2++)
				if (player[i2].where == i)
					placemarkers(0, i2);

			if (i>10 && i<20 || i<40 && i>30)
				flag = 0;
			else
			flag = 1;

			oldx = x = squareloc[i][0]+5;
			oldy = y = squareloc[i][1]-5;
			x2 = squareloc[i][2]-5;
			y2 = squareloc[i][3]+5;

			if (property[i].numhouses == 5)
			{
				fill(globalhdc, oldx, oldy, docolorbar(i), RGB(0,0,0));
				if (!flag)
					y2 = oldy+(y2-oldy)/2;
				else
					x2 = oldx+(x2-oldx)/2;

				Rectangle(globalhdc, x, y, x2, y2);
				fill(globalhdc, oldx+5, oldy-5, RGB(128,0,0), RGB(130,130,130));
			}
			else
			for (i2=0; i2<property[i].numhouses; i2++)
				if (!flag)
				{
					Rectangle(globalhdc, x, y, x2, y+(y2-oldy)/4);
					fill(globalhdc, x+5, y-5, RGB(0,128,0), RGB(130,130,130));
					y += ((y2-oldy)/4);
				}
				else
				{
					Rectangle(globalhdc, x, y, x+(x2-oldx)/4, y2);
					fill(globalhdc, x+5, y-5, RGB(0,128,0), RGB(130,130,130));
					x += ((x2-oldx)/4);
				}

			for (i2=0; i2<numplayers; i2++)
				if (player[i2].where == i)
					placemarkers(1, i2);
		}

	SelectObject(globalhdc, oldpen);
	DeleteObject(hpen);
}

void removehouses(int param)
{
// Removes HIBYTE(param) houses from block # LOBYTE(param)

	char i, i2, block, numhouses, amount;

	if (param == -1)
		return;

	numhouses = HIBYTE(param);
	block = LOBYTE(param);

	amount = numhousesonblock(block)-numhouses;
	for (i=block*5+1; i<block*5+5; i++)
		if (rent[i][5])
			property[i].numhouses = 0;

	i = block*5+1;
	i2 = i+3;
	do {
			if (rent[i][5])
			{
				property[i].numhouses++;
				amount--;
			}

			i++;
			if (i>i2)
				i = block*5+1;
		} while (amount);

	for (i=block*5+1; i<block*5+5; i++)
		if (rent[i][5])
		{
			for (i2=0; i2<numplayers; i2++)
				if (player[i2].where == i)
					placemarkers(0, i2);

			fill(globalhdc, squareloc[i][0]+10, squareloc[i][1]-10, docolorbar(i), RGB(0,0,0));

			for (i2=0; i2<numplayers; i2++)
				if (player[i2].where == i)
					placemarkers(1, i2);
		}

	if (numhousesonblock(block))
		drawhouses(block); //Redraw houses on block 'param'
}

void initbusmenu(void)
{
	char i;
	HMENU hmenu = GetMenu(phwnd);

	EnableMenuItem(hmenu, IDM_SELLLAND, MF_GRAYED);
	EnableMenuItem(hmenu, IDM_BUYLAND, MF_GRAYED);
	EnableMenuItem(hmenu, IDM_MORTGAGE, MF_GRAYED);
	EnableMenuItem(hmenu, IDM_UNMORTGAGE, MF_GRAYED);
	EnableMenuItem(hmenu, IDM_SELLHOUSE, MF_GRAYED);
	EnableMenuItem(hmenu, IDM_BUYHOUSE, MF_GRAYED);

	for (i=0; i<=LASTLAND; i++)
	{
		if (property[i].owner == -1)
			continue;

		if (property[i].owner != curplayer && !globalflag && !property[i].numhouses)
		{
			EnableMenuItem(hmenu, IDM_BUYLAND, MF_ENABLED);
			continue;
		}

		if (property[i].owner != curplayer)
			continue;

		if (!property[i].numhouses)
			for (i=0; i<numplayers; i++)
				if (i != curplayer && player[i].money)
				{
					EnableMenuItem(hmenu, IDM_SELLLAND, MF_ENABLED);
					break;
				}

		if (property[i].mortgaged && !globalflag)
			EnableMenuItem(hmenu, IDM_UNMORTGAGE, MF_ENABLED);

		if (!property[i].mortgaged && !property[i].numhouses)
			EnableMenuItem(hmenu, IDM_MORTGAGE, MF_ENABLED);

		if (numhousesonblock(getblock(i)))
			EnableMenuItem(hmenu, IDM_SELLHOUSE, MF_ENABLED);
	}

	for (i=0; i<8; i++)
		if (colorblock(i, curplayer) && !globalflag)
			if (((!i || i==7) && numhousesonblock(i) != 10) ||
				((i>0 && i<7) && numhousesonblock(i) != 15))
				EnableMenuItem(hmenu, IDM_BUYHOUSE, MF_ENABLED);
}
