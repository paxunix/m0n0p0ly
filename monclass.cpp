#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mem.h>
#include "monopoly.h"
#include "mondlg.h"
#include "monmenu.h"
#include "monstr.h"

ModalDialog::ModalDialog(HANDLE hInst, int resource, HWND hwnd, FARPROC lpdlgfn, DWORD param)
{
	lpfn = MakeProcInstance(lpdlgfn, hInst);
	DialogBoxParam(hInst, MAKEINTRESOURCE(resource), hwnd, lpfn, param);
}

ModalDialog::ModalDialog(HANDLE hInst, int resource, HWND hwnd, FARPROC lpdlgfn)
{
	lpfn = MakeProcInstance(lpdlgfn, hInst);
	DialogBox(hInst, MAKEINTRESOURCE(resource), hwnd, lpfn);
}

ModalDialog::~ModalDialog(void)
{
	FreeProcInstance(lpfn);
}