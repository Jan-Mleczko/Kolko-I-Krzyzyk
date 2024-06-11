#include <windows.h>

#define MAINWND_W 340
#define MAINWND_HE 280
#define LSIDE_X 10
#define BAR_W 310
#define BAR_HE 20
#define STATUSBAR_Y 10
#define AUTHORBAR_Y 220
#define FIELDS_X LSIDE_X
#define FIELDS_Y 40
#define FIELDSIZE 50
#define FIELDSPC 60
#define RSIDE_X	200
#define RSIDE_W 120
#define OCOUNTLBL_Y 40
#define OCOUNT_Y 60
#define XCOUNTLBL_Y 90
#define XCOUNT_Y 110
#define STAT_HE BAR_HE
#define NEWBTN_Y 140
#define ZEROBTN_Y 180
#define RSBTN_HE 30

#define WINCNTSIZE 21

struct Player {
	char *symbol;
	char *turnstatus;
	char *winstatus;
	unsigned int wincount;
	char wincntstr[WINCNTSIZE];
	HWND visualcount;
	};

char initstatus[] = "Tura O";

struct Player player_o = {"O", initstatus, "Wygralo O"};
struct Player player_x = {"X", "Tura X", "Wygral X"};
char neutralsymb[] = "-";

struct Player *currplayer;
struct Player *startplayer = &player_o;

#define BOARDROW 3
#define BOARDSIZE 9
struct Field {
	struct Player *owner;
	HWND visual;
	};
struct Field board[BOARDSIZE];
unsigned int nfreefields;

BOOL gamestop;

BOOL rsvisually;

char initwcount[] = "0";

void updlabel (HWND hw, char *newtext) {
	SetWindowText (hw, newtext);
	/* ¯eby staryb tekst nie by³ widoczny pod nowym */
	ShowWindow (hw, SW_HIDE);
	ShowWindow (hw, SW_RESTORE);
	}

void rststats () {
	player_o.wincount = player_x.wincount = 0;
	if (rsvisually) {
		updlabel (player_o.visualcount, initwcount);
		updlabel (player_x.visualcount, initwcount);
		}
	}

struct Player *toggleplayer (struct Player *pla) {
	return (pla == &player_o) ? &player_x : &player_o;
	}

BOOL freshgame;

HWND statusbar;

void rstgame () {
	struct Field *fld;
	unsigned int flcnt;

	startplayer = toggleplayer (currplayer = startplayer);

	fld = board;
	nfreefields = flcnt = BOARDSIZE;
	while (flcnt--) {
		fld->owner = NULL;

		if (rsvisually)
			SetWindowText (fld->visual, neutralsymb);
		++fld;
		}

	if (rsvisually)
		updlabel (statusbar, currplayer->turnstatus);

	gamestop = FALSE;
	freshgame = TRUE;
	}

BOOL draw;
BOOL winning () {
	struct Field *fi, *fj;
	unsigned int i, j;
	BOOL won;

	draw = FALSE;

	/* Poziome */
	fi = board;
	i = BOARDROW;
	while (i--) {
		fj = fi;
		j = BOARDROW;
		won = TRUE;
		while (j--) {
			if (fj->owner != currplayer) {
				won = FALSE;
				break;
				}
			++fj;
			}
		if (won) return TRUE;
		fi += BOARDROW;
		}

	/* Pionowe */
	fi = board;
	i = BOARDROW;
	while (i--) {
		fj = fi;
		j = BOARDROW;
		won = TRUE;
		while (j--) {
			if (fj->owner != currplayer) {
				won = FALSE;
				break;
				}
			fj += BOARDROW;
			}
		if (won) return TRUE;
		++fi;
		}

	/* Ukoœne */
	if ((board + 4)->owner == currplayer) {
		if ((board->owner == currplayer &&
				(board + 8)->owner == currplayer)
			|| ((board + 2)->owner == currplayer &&
				(board + 6)->owner == currplayer))
			return TRUE;
		}

	if (nfreefields)
		return FALSE;
	return draw = TRUE;
	}

HWND mainwnd;
char wndtitle[] = "Kolko i krzyzyk v1";

void occupyfld (struct Field *fld) {
	fld->owner = currplayer;
	SetWindowText (fld->visual, currplayer->symbol);
	--nfreefields;
	freshgame = FALSE;

	if (winning ()) {
		gamestop = TRUE;
		freshgame = TRUE;
		if (draw)
			updlabel (statusbar, "Remis");
		else {
			updlabel (statusbar, currplayer->winstatus);
			wsprintf (currplayer->wincntstr, "%u",
					++currplayer->wincount);
			updlabel (currplayer->visualcount,
					currplayer->wincntstr);
				}
		}
	else {
		currplayer = toggleplayer (currplayer);
		updlabel (statusbar, currplayer->turnstatus);
		}
	}

void clickfield (struct Field *clkfld) {
	if (gamestop || clkfld->owner)
		MessageBeep (MB_ICONEXCLAMATION);
	else
		occupyfld (clkfld);
	}

BOOL findfieldbtn (HWND button) {
	struct Field *fld;
	unsigned int fldc;

	fld = board;
	fldc = BOARDSIZE;
	while (fldc--) {
		if (fld->visual == button) {
			clickfield (fld);
			return TRUE;
			}
		++fld;
		}
	return FALSE;
	}

HWND newgamebtn, zerobtn;

BOOL abandon () {
	return freshgame ||
		(MessageBox (mainwnd, "Gra trwa! Czy chcesz ja przerwac?",
				wndtitle, MB_YESNO | MB_APPLMODAL))
				== IDYES;
	}

void clickbtn (HWND button) {
	if (button == newgamebtn) {
		if (abandon ())
			rstgame ();
		return;
		}
	if (button == zerobtn) {
		rststats ();
		return;
		}

	findfieldbtn (button);
	}

LRESULT CALLBACK WindowProc (HWND hwnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
	case WM_COMMAND:
		clickbtn ((HWND) lParam);
		break;
	case WM_CLOSE:
		if (abandon ())
			DestroyWindow (hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage (0);
		break;
	default:
		DefWindowProc (hwnd, uMsg, wParam, lParam);
		}
	}

HINSTANCE hInstance;
WNDCLASS mainwndclass;
char mainwndclsname[] = "JMKolKrzyz";

HWND setupbutton (char *caption, unsigned int x, unsigned int y,
		unsigned int w, unsigned int h) {
	return CreateWindow ("BUTTON", caption,
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			x, y, w, h,
			mainwnd,
			NULL, hInstance, NULL);
	}

void setupfields () {
	unsigned int fldidx;
	unsigned int fldx, fldy;
	unsigned int fldinrow;

	fldidx = 0;
	fldy = FIELDS_Y;
	while (fldidx < BOARDSIZE) {
		fldx = FIELDS_X;

		fldinrow = BOARDROW;
		while (fldinrow--) {
			(board + fldidx++)->visual = setupbutton (
					neutralsymb, fldx, fldy,
					FIELDSIZE, FIELDSIZE);
			fldx += FIELDSPC;
			}
		fldy += FIELDSPC;
		}
	}

HWND setuplabel (char *text, unsigned int x, unsigned int y,
		unsigned int w, unsigned int h, unsigned int styles) {
	return CreateWindow ("STATIC", text,
			styles | WS_CHILD | WS_VISIBLE,
			x, y, w, h,
			mainwnd,
			NULL, hInstance, NULL);
	}

int WINAPI WinMain (HINSTANCE hInst, HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow) {
	MSG msg;

	hInstance = hInst;

	mainwndclass.style = 0;
	mainwndclass.lpfnWndProc = WindowProc;
	mainwndclass.cbClsExtra = mainwndclass.cbWndExtra = 0;
	mainwndclass.hInstance = hInstance;
	mainwndclass.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	mainwndclass.hCursor = LoadCursor (NULL, IDC_ARROW);
	mainwndclass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	mainwndclass.lpszMenuName = NULL;
	mainwndclass.lpszClassName = mainwndclsname;
	RegisterClass (&mainwndclass);

	mainwnd = CreateWindow (mainwndclsname,
			wndtitle,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			MAINWND_W, MAINWND_HE,
			NULL,
			NULL, hInstance, NULL);

	newgamebtn = setupbutton ("Nowa gra",
			RSIDE_X, NEWBTN_Y, RSIDE_W, RSBTN_HE);
	zerobtn = setupbutton ("Zeruj",
			RSIDE_X, ZEROBTN_Y, RSIDE_W, RSBTN_HE);
	setupfields ();
	setuplabel ("Jan Mleczko, 2024",
			LSIDE_X, AUTHORBAR_Y, BAR_W, BAR_HE,
			SS_RIGHT);
	statusbar = setuplabel (initstatus,
			LSIDE_X, STATUSBAR_Y, BAR_W, BAR_HE,
			0);
	setuplabel ("Wygrane O:",
			RSIDE_X, OCOUNTLBL_Y, RSIDE_W, STAT_HE,
			0);
	player_o.visualcount = setuplabel (initwcount,
			RSIDE_X, OCOUNT_Y, RSIDE_W, STAT_HE,
			SS_RIGHT);

	setuplabel ("Wygrane X:",
			RSIDE_X, XCOUNTLBL_Y, RSIDE_W, STAT_HE,
			0);
	player_x.visualcount = setuplabel (initwcount,
			RSIDE_X, XCOUNT_Y, RSIDE_W, STAT_HE,
			SS_RIGHT);

	rsvisually = FALSE;
	rststats ();
	rstgame ();
	rsvisually = TRUE;

	ShowWindow (mainwnd, SW_RESTORE);

	while (GetMessage (&msg, NULL, 0, 0)) {
		TranslateMessage (&msg);
		DispatchMessage (&msg);
		}
	ExitProcess (msg.wParam);
	}

