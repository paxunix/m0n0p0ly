#define LASTLAND 39
#define MAXPLAYERS 4
#define TOKENSIZE 44
#define BANK 6
#define getblock(where) ((where) / 5)
#ifndef max
#define max(a,b) (((a)>(b))?(a):(b))
#endif
#ifndef min
#define min(a,b) (((a)<(b))?(a):(b))
#endif

typedef struct {
	char 	where; //Position on board (0=GO, 39=Boardwalk)
	int		money; //Amount of money player currently has; =-1 if bankrupt
	char	jailcards; //Number of Get Out of Jail Free cards
	char	rolls; //Number of times doubles have been rolled in succession, and # of times dice rolled if in jail
} PLAYER;

typedef struct {
	char	numhouses; //If this equals 5, there is a hotel there.
	char	owner; //Value of 255=unowned
	char	mortgaged; //1 if mortgaged, 0 if unmortgaged
} PROPERTY;

typedef struct {
	char	name[20];
	WORD	score;
} HISCORE;

typedef struct {
	char die1;
	char die2;
} DICE;

long FAR PASCAL _export WndProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export gameattrDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export playernameDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export topfiveDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export openDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export aboutDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export utilDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export railDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export propDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export seecardDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export taxDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export injailDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export unownedDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export ownedDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export raisemoneyDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export sellpropertyDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export mortgagepropertyDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export sellhousesDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export unmortgagepropertyDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export buypropertyDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export buyhousesDlgProc(HWND,WORD,WORD,LONG);
BOOL FAR PASCAL _export movespeedDlgProc(HWND,WORD,WORD,LONG);
void initgame(void);
void paintwindow(void);
int savegame(void);
void loadgame(void);
void showbm(HDC,int,int,int,int,int);
void fill(HDC,int,int,COLORREF,COLORREF);
void doturn(void);
void rolldice(void);
void updateplayerbox(char);
void moveplayer(void);
void paintsquares(void);
void placemarkers(char,char);
COLORREF docolorbar(int);
void sendtojail(void);
void docomchest(void);
void doincometax(void);
int dochance(void);
void doproperty(int);
WORD calctotalworth(int);
void bankrupt(char);
void streetrepairs(char,char);
BOOL deduct(int,char,char,char);
void whattypecard(HWND);
char inplayerbox(POINT&);
char inpropbox(POINT&);
void hilitecurplayer(void);
BOOL colorblock(char,char);
char getpropertyindex(WORD);
void initmort(WORD,char);
void removehouses(int);
void drawhouses(int);
int gethousecost(char);
char numhousesonblock(char);
void initbusmenu(void);
char maxhousesonblock(char);
void endgame(void);

class ModalDialog {
private:
	FARPROC	lpfn;

public:
	ModalDialog(HANDLE,int,HWND,FARPROC); //Constructor for no params
	ModalDialog(HANDLE,int,HWND,FARPROC,DWORD); //Constructor for param
	~ModalDialog(void); //Destructor
};