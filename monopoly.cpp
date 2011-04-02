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

HANDLE		hInst;
PLAYER		player[MAXPLAYERS]; //Room for MAXPLAYERS players
PROPERTY	property[LASTLAND+1]; //One for each space on board
HISCORE		hiscores[5]; //The top five.
int			playeroffset[MAXPLAYERS][2]={ //x,y offset for each player
				{0, 0,},				 //relative to upper-left corner of
				{TOKENSIZE, 0,},				 //a square
				{0, TOKENSIZE,},
				{TOKENSIZE, TOKENSIZE} };
COLORREF	playercolor[MAXPLAYERS] = {
			RGB(128,128,0), RGB(255,0,255), RGB(255,255,0), RGB(64,0,128) };

/*	squareloc holds the location of the coordinate of the square
	Any that are bitmaps have x2,y2 as the x and y sizes
	squareloc[property][0] = x1
					   [1] = y1
					   [2] = x2
					   [3] = y2
*/
int squareloc[LASTLAND+1][4] = {
	{ 1232, -1232,  195,  -195,},
	{ 1107, -1230, 1230, -1266,},
	{  996, -1232,  109,  -195,},
	{  877, -1230,  992, -1266,},
	{  764, -1232,  110,  -195,},
	{  650, -1232,  109,  -195,},
	{  532, -1230,  647, -1266,},
	{  419, -1232,  111,  -195,},
	{  302, -1230,  417, -1266,},
	{  187, -1230,  302, -1266,},
	{   57, -1232,  128,  -143,},
	{  151, -1107,  187, -1230,},
	{    5,  -996,  180,  -109,},
	{  151,  -877,  187,  -992,},
	{  151,  -764,  187,  -877,},
	{    5,  -650,  180,  -109,},
	{  151,  -532,  187,  -647,},
	{    5,  -419,  180,  -111,},
	{  151,  -302,  187,  -416,},
	{  151,  -187,  187,  -302,},
	{    5,    -5,  180,  -180,},
	{  187,  -151,  302,  -187,},
	{  304,    -5,  109,  -180,},
	{  417,  -151,  532,  -187,},
	{  532,  -151,  647,  -187,},
	{  650,    -5,  109,  -180,},
	{  762,  -151,  877,  -187,},
	{  877,  -151,  992,  -187,},
	{  996,    -5,  109,  -180,},
	{ 1107,  -151, 1230,  -187,},
	{ 1232,    -5,  195,  -180,},
	{ 1230,  -187, 1266,  -302,},
	{ 1230,  -302, 1266,  -417,},
	{ 1232,  -419,  195,  -111,},
	{ 1230,  -532, 1266,  -647,},
	{ 1232,  -650,  195,  -109,},
	{ 1232,  -764,  195,  -110,},
	{ 1230,  -877, 1266,  -992,},
	{ 1232,  -996,  195,  -109,},
	{ 1230, -1107, 1266, -1230} };

/*Since mortgage values are half of the property's board value, you only need
  the boardvalue array
  Since per building cost starts at $50 and goes to $200 by $50 increments
  on each successive side of the board, simply determine house cost like this:
	Property #1-9 = $50;  11-19=$100;  21-29=$150;  31-39=$200
*/

int	boardvalue[LASTLAND+1] =
		{ 0,  60,   0,  60,   0, 200, 100,   0, 100, 120,
		  0, 140, 150, 140, 160, 200, 180,   0, 180, 200,
		  0, 220,   0, 220, 240, 200, 260, 260, 150, 280,
		  0, 300, 300,   0, 320, 200,   0, 350,   0, 400 };
/* use rent[] like this:
	RENT = [x][0]
	1 house = [x][1]
	2 houses= [x][2]
	3 houses= [x][3]
	4 houses= [x][4]
	hotel = [x][5]
  The utilities, chance, community chest, etc. squares are filled with 0.
  The railroads only use the first 4 elements.
*/
int	rent[LASTLAND+1][6] =
	{   {0,   0,   0,   0,   0,   0,},
		{2,  10,  30,  90, 160, 250,},
		{0,   0,   0,   0,   0,   0,},
		{4,  20,  60, 180, 320, 450,},
		{0,   0,   0,   0,   0,   0,},
	   {25,  50, 100, 200,   0,   0,},
		{6,  30,  90, 270, 400, 550,},
		{0,   0,   0,   0,   0,   0,},
		{6,  30,  90, 270, 400, 550,},
		{8,  40, 100, 300, 450, 600,},
		{0,   0,   0,   0,   0,   0,},
	   {10,  50, 150, 450, 625, 750,},
		{0,   0,   0,   0,   0,   0,},
	   {10,  50, 150, 450, 625, 750,},
	   {12,  60, 180, 500, 700, 900,},
	   {25,  50, 100, 200,   0,   0,},
	   {14,  70, 200, 550, 750, 950,},
		{0,   0,   0,   0,   0,   0,},
	   {14,  70, 200, 550, 750, 950,},
	   {16,  80, 220, 600, 800,1000,},
		{0,   0,   0,   0,   0,   0,},
	   {18,  90, 250, 700, 875,1050,},
		{0,   0,   0,   0,   0,   0,},
	   {18,  90, 250, 700, 875,1050,},
	   {20, 100, 300, 750, 925,1100,},
	   {25,  50, 100, 200,   0,   0,},
	   {22, 110, 330, 800, 975,1150,},
	   {22, 110, 330, 800, 975,1150,},
		{0,   0,   0,   0,   0,   0,},
	   {24, 120, 360, 850,1025,1200,},
		{0,   0,   0,   0,   0,   0,},
	   {26, 130, 390, 900,1100,1275,},
	   {26, 130, 390, 900,1100,1275,},
		{0,   0,   0,   0,   0,   0,},
	   {28, 150, 450,1000,1200,1400,},
	   {25,  50, 100, 200,   0,   0,},
		{0,   0,   0,   0,   0,   0,},
	   {35, 175, 500,1100,1300,1500,},
		{0,   0,   0,   0,   0,   0,},
	   {50, 200, 600,1400,1700,2000} };

char	numplayers, curplayer=0, started=0, computerplayer, gamefile[80],
		playername[MAXPLAYERS][20], doubles=0, comchest=0, chance=0,
		chancecards[16], chestcards[16], globalflag=0, hotelsleft,
		housesleft;
char	text[80];
int		dlgretval, speedval=100;
WORD	dlgparam;
HDC		globalhdc;
HBITMAP hbm;
HWND	phwnd;
DICE	dice;

#pragma argsused
int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpszCmdLine, int cmdShow)
{
	MSG		msg;
	WNDCLASS	wndclass;

	if ((GetWinFlags() & WF_CPU086))
	{
		MessageBox(GetDesktopWindow(), "Monopoly needs a 80286 or higher to run!", "Whoa, pal!", MB_OK);
		return 0;
	}

	if (!hPrevInstance)
	{
		wndclass.lpszClassName  = "MONOPOLY::MALACHAI";
		wndclass.hInstance	    = hInstance;
		wndclass.lpfnWndProc	= WndProc;
		wndclass.hCursor		= LoadCursor((HANDLE)NULL, IDC_ARROW);
		wndclass.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(1));
		wndclass.lpszMenuName	= MAKEINTRESOURCE(1);
		wndclass.hbrBackground	= COLOR_APPWORKSPACE+1;
		wndclass.style			= CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_BYTEALIGNCLIENT | CS_BYTEALIGNWINDOW;
		wndclass.cbClsExtra		= 0;
		wndclass.cbWndExtra		= 0;

		RegisterClass(&wndclass);
	}

	hInst = hInstance;

	phwnd = CreateWindow("MONOPOLY::MALACHAI",
						"Monopoly  v1.00",
						WS_OVERLAPPEDWINDOW | WS_MAXIMIZE,
						0,
						0,
						200,
						0,
						(HWND)NULL,
						(HMENU)NULL,
						hInstance,
						NULL);

	ShowWindow(phwnd, cmdShow);

	globalhdc = GetDC(phwnd);
	SetMapMode(globalhdc, MM_LOMETRIC);

	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

long FAR PASCAL _export WndProc(HWND hwnd, WORD msg, WORD wParam, LONG lParam)
{
	switch(msg)
	{
		case WM_CREATE:
			initgame();
			break;

		case WM_PAINT:
			SetCursor(LoadCursor((HANDLE) NULL, IDC_WAIT));
			paintwindow();
			SetCursor(LoadCursor((HANDLE) NULL, IDC_ARROW));
			break;

		case WM_INITMENU:
			initbusmenu();
			break;

		case WM_RBUTTONDOWN:
		{
			char retval;
			POINT p;

			p = MAKEPOINT(lParam);
			retval = inplayerbox(p);
			if (retval != -1)
			{
				ModalDialog seecard(hInst, 17, hwnd, (FARPROC) seecardDlgProc, retval);
				whattypecard(hwnd);
			}
			else
			{
				dlgretval = inpropbox(p);
				if (dlgretval != -1)
					whattypecard(hwnd);
			}
		}
		break;

		case WM_LBUTTONDBLCLK:
		{
			POINT p;

			p.x = LOWORD(lParam);
			p.y = HIWORD(lParam);
			if (inplayerbox(p) == curplayer)
				doturn();
		}
		break;

		case WM_COMMAND:
			switch(wParam)
			{
				case IDM_SPEED:
					ModalDialog speed(hInst, 22, hwnd, (FARPROC) movespeedDlgProc);
					break;

				case IDM_HISCORE:
					ModalDialog topfive(hInst, 9, hwnd, (FARPROC) topfiveDlgProc);
					break;

				case IDM_NEW:
					initgame();
					DeleteObject(hbm);
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
					break;

				case IDM_EXIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
					break;

				case IDM_SAVE:
					savegame();
					break;

				case IDM_LOAD:
					loadgame();
					started = 1;
					InvalidateRect(hwnd, NULL, TRUE);
					UpdateWindow(hwnd);
					break;

				case IDM_ABOUT:
					ModalDialog about(hInst, 10, hwnd, (FARPROC) aboutDlgProc);
					break;

				case IDM_SEECARD:
					ModalDialog seecard(hInst, 17, hwnd, (FARPROC) seecardDlgProc, 65535L);
					whattypecard(hwnd);
					break;

				case IDM_SELLLAND:
				{
					char i;

					ModalDialog sellprop(hInst, 2, hwnd, (FARPROC) sellpropertyDlgProc);
					for (i=0; i<numplayers; i++)
						updateplayerbox(i);
				}
				break;

				case IDM_BUYLAND:
				{
					char i;

					ModalDialog buyprop(hInst, 1, hwnd, (FARPROC) buypropertyDlgProc);
					for (i=0; i<numplayers; i++)
						updateplayerbox(i);
				}
				break;

				case IDM_MORTGAGE:
					ModalDialog mortprop(hInst, 4, hwnd, (FARPROC) mortgagepropertyDlgProc);
					updateplayerbox(curplayer);
					break;

				case IDM_UNMORTGAGE:
					ModalDialog unmortprop(hInst, 6, hwnd, (FARPROC) unmortgagepropertyDlgProc);
					updateplayerbox(curplayer);
					break;

				case IDM_SELLHOUSE:
					ModalDialog sellhouse(hInst, 5, hwnd, (FARPROC) sellhousesDlgProc);
					removehouses(dlgretval);
					updateplayerbox(curplayer);
					break;

				case IDM_BUYHOUSE:
					ModalDialog buyhouse(hInst, 3, hwnd, (FARPROC) buyhousesDlgProc);
					drawhouses(dlgretval);
					updateplayerbox(curplayer);
			}
			break;

		case WM_DESTROY:
			DeleteObject(hbm);
			ReleaseDC(hwnd, globalhdc);
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
			break;
	}

	return FALSE;
}

void initgame(void)
{
	FILE	*scores;
	int		i, num, num2;
	char	temp, temp2;

	randomize();
	hotelsleft = 12;
	housesleft = 32;
	for (i=0; i<MAXPLAYERS; i++)
	{
		player[i].where = 0;
		player[i].money = 1500;
		player[i].jailcards = 0;
		player[i].rolls = 0;
	}

	for (i=0; i<=LASTLAND; i++)
	{
		property[i].numhouses = 0;
		property[i].owner = -1;
		property[i].mortgaged = 0;
	}

	for (i=0; i<16; i++)
		chancecards[i] = i;

	for (i=0; i<8; i++)
	{
		num = random(16);
		num2 = random(16);
		temp = chancecards[num];
		temp2 = chancecards[num2];
		chancecards[num2] = temp;
		chancecards[num] = temp2;
	}

	for (i=0; i<16; i++)
		chestcards[i] = chancecards[i];

	for (i=0; i<5; i++)
	{
		hiscores[i].name[0] = 'X';
		hiscores[i].name[1] = 0;
		hiscores[i].score = 0;
	}

	scores = fopen("monopoly.scr", "rb");
	if (scores!=NULL)
		for (i=0; i<5; i++)
			fread(&hiscores[i], sizeof(HISCORE), 1, scores);

	fclose(scores);

	numplayers=curplayer=started=computerplayer=0;
}

int savegame(void)
{
	FILE *out;
	char i;

	ModalDialog open(hInst, 7, phwnd, (FARPROC) openDlgProc, 1);

	if (!dlgretval)
		return IDCANCEL;

	out = fopen(gamefile, "wb");
	lstrcpy(text, "Monopoly saved game");
	fwrite(&text[0], 20, 1, out); //write ID string
	fwrite(&numplayers, sizeof(char), 1, out); //Save number of players
	fwrite(&computerplayer, sizeof(char), 1, out); //Save computerplayer state
	fwrite(&curplayer, sizeof(char), 1, out);  //Save current player
	for (i=0; i<numplayers; i++)  //Save players' names and states
	{
		fputs(playername[i], out);
		fputc('\0', out);
		fputc('\n', out);
		fwrite(&player[i], sizeof(PLAYER), 1, out);
	}

	fwrite(&doubles, sizeof(char), 1, out); //Save doubles state
	fwrite(&comchest, sizeof(char), 1, out); //Save comchest pointer
	fwrite(&chance, sizeof(char), 1, out); //Save chance pointer
	for (i=0; i<16; i++) //Save chance and chest cards arrays
	{
		fwrite(&chancecards[i], sizeof(char), 1, out);
		fwrite(&chestcards[i], sizeof(char), 1, out);
	}

	for (i=0; i<=LASTLAND; i++)	//Save the state of all properties
		fwrite(&property[i], sizeof(PROPERTY), 1, out);

	MessageBox(phwnd, "Game has been saved", "Save Game", MB_OK);
	fclose(out);
	return IDYES;
}

void loadgame(void)
{
	FILE *in;
	char i;

	ModalDialog open(hInst, 7, phwnd, (FARPROC) openDlgProc, 0);
	if (!dlgretval)
		return;

	in = fopen(gamefile, "rb");
	fread(&text[0], 20, 1, in);
	if (lstrcmp(text, "Monopoly saved game"))
	{
		MessageBox(phwnd, "Bad file format", NULL, MB_OK);
		fclose(in);
	}

	fread(&numplayers, sizeof(char), 1, in);
	fread(&computerplayer, sizeof(char), 1, in);
	fread(&curplayer, sizeof(char), 1, in);
	for (i=0; i<numplayers; i++)
	{
		fgets(playername[i], 22, in);
		fread(&player[i], sizeof(PLAYER), 1, in);
	}

	fread(&doubles, sizeof(char), 1, in);
	fread(&comchest, sizeof(char), 1, in);
	fread(&chance, sizeof(char), 1, in);
	for (i=0; i<16; i++)
	{
		fread(&chancecards[i], sizeof(char), 1, in);
		fread(&chestcards[i], sizeof(char), 1, in);
	}

	for (i=0; i<=LASTLAND; i++)
		fread(&property[i], sizeof(PROPERTY), 1, in);

	MessageBox(phwnd, "Game has been loaded in", "Load Game", MB_OK);
	fclose(in);
}

void paintwindow(void)
{
	PAINTSTRUCT ps;
	HDC hdc;
	HPEN hpen, oldpen;
	POINT p[1];
	int y=187, i;
	MSG msg;

	hdc = BeginPaint(phwnd, &ps);
	SetMapMode(hdc, MM_LOMETRIC);
	hpen = CreatePen(PS_INSIDEFRAME, 5, RGB(0,0,0));
	oldpen = SelectObject(hdc, hpen);

	p[0].x = TOKENSIZE;
	p[0].y = -TOKENSIZE;
	LPtoDP(globalhdc, p, 1);
	hbm = CreateCompatibleBitmap(globalhdc, p[0].x*2, p[0].y*2);

	//Draw the board outline
	Rectangle(hdc, 0, 0, 1432, -1432);

	//Draw horiz inner lines
	MoveTo(hdc, 0, -187); //Top
	LineTo(hdc, 1427, -187);
	MoveTo(hdc, 0, -1230); //Bottom
	LineTo(hdc, 1427, -1230);

	//Draw vert inner lines
	MoveTo(hdc, 187, 0); //Left
	LineTo(hdc, 187, -1427);
	MoveTo(hdc, 1230, 0); //Right
	LineTo(hdc, 1230, -1427);

	//Draw vert bar lines
	MoveTo(hdc, 151, -187); //Left
	LineTo(hdc, 151, -1230);
	MoveTo(hdc, 1266, -187); //Right
	LineTo(hdc, 1266, -1230);

	//Draw horiz bar lines
	MoveTo(hdc, 187, -151); //Top
	LineTo(hdc, 1230, -151);
	MoveTo(hdc, 187, -1266); //Bottom
	LineTo(hdc, 1230, -1266);

	do {
		MoveTo(hdc, 0, -y); //Left property lines
		LineTo(hdc, 182, -y);

		MoveTo(hdc, 1230, -y); //Right property lines
		LineTo(hdc, 1427, -y);

		MoveTo(hdc, y, 0); //Top property lines
		LineTo(hdc, y, -182);

		MoveTo(hdc, y, -1230); //Bottom property lines
		LineTo(hdc, y, -1427);

		y+=115;
		} while (y<=1123);

	//Draw lines in Jail
	MoveTo(hdc, 55, -1230); //Vert
	LineTo(hdc, 55, -1377);
	LineTo(hdc, 187, -1377); //Horiz

	//Put bitmaps on board and paint property color bars
	for (i=0; i<=LASTLAND; i++)
	{
		PeekMessage(&msg, (HWND) NULL, 0, 0, PM_NOREMOVE); //Let other apps run
		if (rent[i][5])
			fill(hdc, squareloc[i][0]+10, squareloc[i][1]-10, docolorbar(i), RGB(0,0,0));
		else
			showbm(hdc, squareloc[i][0], squareloc[i][1], squareloc[i][2], squareloc[i][3], i);
	}

	fill(hdc, 83, -1253, RGB(255,128,0), RGB(0,0,0)); //Paint jail
	showbm(hdc, 399, -326, 632, -432, 1);
	showbm(hdc, 483, -894, 464, -194, 3);

	if (!started)
	{
		ModalDialog game(hInst, 11, phwnd, (FARPROC) gameattrDlgProc);

		if (computerplayer)
			numplayers--;

		for (i=0; i<numplayers; i++)
		{
			ModalDialog playername(hInst, 21, phwnd, (FARPROC) playernameDlgProc, i+1);
		}

		if (computerplayer)
			numplayers++;
	}

	//Draw player boxes
	y = -55;
	for (i=0; i<numplayers; i++, y-=55)
	{
		Rectangle(hdc, 1550, y, 1981, y-226);
		Rectangle(hdc, 1460, y, 1517, y-226);
		fill(hdc, 1485, y-55, playercolor[i], RGB(0,0,0));
		y-=226;
	}

	for (i=0; i<numplayers; i++)
	{
		updateplayerbox(i);
		placemarkers(1, i);
	}

	for (i=0; i<8; i++)
		drawhouses(i);

	hilitecurplayer();
	started = 1;
	SelectObject(hdc, oldpen);
	DeleteObject(hpen);
	EndPaint(phwnd, &ps);
}

void showbm(HDC hdc, int xorg, int yorg, int xsize, int ysize, int resource)
{
	HDC hbmdc;
	HBITMAP hbitmap, hbmold;
	BITMAP bm;

	hbmdc = CreateCompatibleDC(hdc);
	if (resource==0)
		hbitmap = LoadBitmap(hInst, MAKEINTRESOURCE(50));
	else
		hbitmap = LoadBitmap(hInst, MAKEINTRESOURCE(resource));

	hbmold = SelectObject(hbmdc, hbitmap);
	GetObject(hbitmap, sizeof(BITMAP), (LPSTR) &bm);
	StretchBlt(hdc, xorg, yorg, xsize, ysize, hbmdc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	SelectObject(hbmdc, hbmold);
	DeleteObject(hbitmap);
	DeleteObject(hbmold);
	DeleteDC(hbmdc);
}

void fill(HDC hdc, int xorg, int yorg, COLORREF crcolorfill, COLORREF crcolorbound)
{
	HBRUSH hbrold, hbr;

	hbr = CreateSolidBrush(crcolorfill);
	hbrold = SelectObject(hdc, hbr);
	FloodFill(hdc, xorg, yorg, crcolorbound);
	SelectObject(hdc, hbrold);
	DeleteObject(hbr);
}

void doturn(void)
{
	int chanceretval=0, where;
	char i, count=0;

L3:	if (player[curplayer].where == LASTLAND+1) //Player is in Jail
	{
		int i;

		ModalDialog injail(hInst, 19, phwnd, (FARPROC) injailDlgProc);

		switch(dlgretval)
		{
			case 1:
				rolldice();
				player[curplayer].rolls++;
				if (dice.die1 == dice.die2)
				{
					player[curplayer].rolls = 0;
					placemarkers(0, curplayer);
					player[curplayer].where = 10;
					placemarkers(1, curplayer);
				}
				else
				if (player[curplayer].rolls == 3)
				{
					MessageBox(phwnd, "You have to pay the $50 to get out.", "Too Bad!  (Ha, ha, ha)", MB_OK);
					if (!deduct(50, curplayer, BANK, 0))
						goto L1;

					player[curplayer].rolls = 0;
					placemarkers(0, curplayer);
					player[curplayer].where = 10;
					placemarkers(1, curplayer);
				}
				goto L1;

			case 2:
				deduct(50, curplayer, BANK, 1);
				player[curplayer].rolls = 0;
				placemarkers(0, curplayer);
				player[curplayer].where = 10;
				placemarkers(1, curplayer);
				goto L1;

			case 3:
				player[curplayer].rolls = 0;
				placemarkers(0, curplayer);
				player[curplayer].where = 10;
				placemarkers(1, curplayer);
				player[curplayer].jailcards--;
				for (i=0; i<16; i++)
				{
					if (chancecards[i] == -1)
					{
						chancecards[i] = 7;
						break;
					}

					if (chestcards[i] == -1)
						chestcards[i] = 10;
				}
				updateplayerbox(curplayer);
				goto L1;
		}
	}

	rolldice();

	if (dice.die1 == dice.die2)
	{
		player[curplayer].rolls++;
		doubles=1;
	}
	else
		player[curplayer].rolls = doubles = 0;

	if (player[curplayer].rolls == 3)
	{
		MessageBox(phwnd, "You rolled doubles three times\12"
						 "in a row, so you go to jail!", "Tough break!", MB_OK);

		sendtojail();
		goto L1;
	}

	moveplayer();

	if (player[curplayer].where == 30)
	{
		MessageBox(phwnd, "Go Directly to JAIL!", "HA, HA, HA!!", MB_OK);
		sendtojail();
		goto L1;
	}

	where = player[curplayer].where;
	if (where==7 || where==22 || where==36)
	{
		chanceretval = dochance(); //returns 1=utility card, 2=railroad card
		where = player[curplayer].where;
	}

	if (where==LASTLAND+1 || where==7 || where==22 || where==36)
		goto L1;

	if (where==2 || where==17 || where==33)
	{
		docomchest();
		goto L1;
	}

	if (where==4)
	{
		doincometax();
		goto L1;
	}

	if (where==0)
	{
		MessageBox(phwnd, "You're on GO", "Where am I?", MB_OK);
		goto L1;
	}

	if (where==20)
	{
		MessageBox(phwnd, "You're on Free Parking", "Where am I?", MB_OK);
		goto L1;
	}

	if (where==10)
	{
		MessageBox(phwnd, "You're on Just Visiting", "Where am I?", MB_OK);
		goto L1;
	}

	if (where==38)
	{
		MessageBox(phwnd, "You're on Luxury Tax", "Where am I?", MB_OK);
		deduct(75, curplayer, BANK, 0);
		goto L1;
	}

	doproperty(chanceretval);

L1: hilitecurplayer();
	if (!doubles)
		curplayer++;

	for (i=0; i<numplayers; i++)
		if (player[i].money != -1)
			count++;

	if (count == 1)
	{
		endgame();
		PostMessage(phwnd, WM_CLOSE, 0, 0);
		return;
	}


L4:	if (curplayer > numplayers-1)
		curplayer = 0;

	if (player[curplayer].money == -1) //curplayer is bankrupt so he's toast
	{
		curplayer++;
		goto L4;
	}

	hilitecurplayer();

	if (player[curplayer].where == LASTLAND+1) //curplayer is in jail
		goto L3;
}

void rolldice(void)
{
	dice.die1 = random(6)+1;
	dice.die2 = random(6)+1;
	showbm(globalhdc, 993, -1110, 104, -104, dice.die1+39);
	showbm(globalhdc, 1110, -1110, 104,-104, dice.die2+39);
}

void endgame(void)
{
	FILE *scores;
	HISCORE temp;
	int i;
	WORD amount;

	for (i=0; i<numplayers; i++)
		if (player[i].money != -1)
			break;

	amount = calctotalworth(i);
	if (amount >= hiscores[4].score)
	{
		lstrcpy(hiscores[4].name, playername[i]);
		hiscores[4].score = amount;
	}

	i=0;
	do {
		if (hiscores[i].score < hiscores[i+1].score)
		{
			lstrcpy(temp.name, hiscores[i].name);
			temp.score = hiscores[i].score;
			lstrcpy(hiscores[i].name, hiscores[i+1].name);
			hiscores[i].score = hiscores[i+1].score;
			lstrcpy(hiscores[i+1].name, temp.name);
			hiscores[i+1].score = temp.score;
			i = 0;
		}
		else
		i++;

		} while (i < 4);

	ModalDialog topfive(hInst, 9, phwnd, (FARPROC) topfiveDlgProc);

	scores = fopen("monopoly.scr", "wb");
	if (scores!=NULL)
		for (i=0; i<5; i++)
			fwrite(&hiscores[i], sizeof(HISCORE), 1, scores);

	fclose(scores);
}
