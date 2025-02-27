/******************************************************************************
 * PartDialog.c
 *
 * Copyright (c) 2020, longpanda <admin@ventoy.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <Windows.h>
#include <commctrl.h>
#include "resource.h"
#include "Language.h"
#include "Ventoy2Disk.h"

static BOOL g_enable_reserve_space = FALSE;
static BOOL g_enable_reserve_space_tmp = FALSE;
static BOOL g_align_part_4KB = TRUE;
static BOOL g_align_part_4KB_tmp = TRUE;
static int g_unit_sel = 1;
static int g_unit_sel_tmp = 1;
static int g_reserve_space = -1;
static int g_fs_type = 0;
static int g_fs_type_tmp = 0;
static int g_fs_radio_id[] =
{
    IDC_RADIO1, IDC_RADIO2, IDC_RADIO3
};

static char* g_fs_name[] =
{
    "exFAT", "NTFS", "FAT32"
};

int IsPartNeed4KBAlign(void)
{
    return g_align_part_4KB;
}

void SetVentoyFsType(int fs)
{
    g_fs_type = g_fs_type_tmp = fs;
}


int GetVentoyFsType(void)
{
    return g_fs_type;
}

const char* GetVentoyFsName(void)
{
    return g_fs_name[g_fs_type];
}

int GetReservedSpaceInMB(void)
{
    if (g_enable_reserve_space)
    {
        if (g_unit_sel == 0)
        {
            return g_reserve_space;
        }
        else
        {
            return g_reserve_space * 1024;
        }
    }
    else
    {
        return 0;
    }
}


static VOID UpdateControlStatus(HWND hWnd)
{
	HWND hComobox;
	HWND hCheckbox;
    HWND hCheckbox4KB;
	HWND hEdit;

	hCheckbox = GetDlgItem(hWnd, IDC_CHECK_RESERVE_SPACE);
    hCheckbox4KB = GetDlgItem(hWnd, IDC_CHECK_PART_ALIGN_4KB);
	hComobox = GetDlgItem(hWnd, IDC_COMBO_SPACE_UNIT);
	hEdit = GetDlgItem(hWnd, IDC_EDIT_RESERVE_SPACE_VAL);

	SendMessage(hComobox, CB_SETCURSEL, g_unit_sel, 1);
	SendMessage(hCheckbox, BM_SETCHECK, g_enable_reserve_space_tmp ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessage(hCheckbox4KB, BM_SETCHECK, g_align_part_4KB_tmp ? BST_CHECKED : BST_UNCHECKED, 0);
	
    EnableWindow(hEdit, g_enable_reserve_space_tmp);
	EnableWindow(hComobox, g_enable_reserve_space_tmp);
}

static BOOL PartInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
    HWND hComobox;
    HWND hCheckbox;
    HWND hCheckbox4KB;
    HWND hEdit;
    WCHAR buf[64];

    hCheckbox = GetDlgItem(hWnd, IDC_CHECK_RESERVE_SPACE);
    hCheckbox4KB = GetDlgItem(hWnd, IDC_CHECK_PART_ALIGN_4KB);
    hComobox = GetDlgItem(hWnd, IDC_COMBO_SPACE_UNIT);
    hEdit = GetDlgItem(hWnd, IDC_EDIT_RESERVE_SPACE_VAL);

    SetWindowText(hCheckbox, _G(STR_PRESERVE_SPACE));
    SetWindowText(hCheckbox4KB, _G(STR_PART_ALIGN_4KB));
    SetWindowText(GetDlgItem(hWnd, ID_PART_OK), _G(STR_BTN_OK));
    SetWindowText(GetDlgItem(hWnd, ID_PART_CANCEL), _G(STR_BTN_CANCEL));
    SetWindowText(GetDlgItem(hWnd, IDC_VENTOY_PART_FS), _G(STR_PART_VENTOY_FS));


    SetWindowText(hWnd, _G(STR_MENU_PART_CFG));

	SendMessage(hEdit, EM_LIMITTEXT, 9, 0);

    SendMessage(hComobox, CB_ADDSTRING, 0, (LPARAM)TEXT(" MB"));
    SendMessage(hComobox, CB_ADDSTRING, 0, (LPARAM)TEXT(" GB"));
    
    SendMessage(hCheckbox, WM_SETFONT, (WPARAM)g_language_normal_font, TRUE);
    SendMessage(hCheckbox4KB, WM_SETFONT, (WPARAM)g_language_normal_font, TRUE);
    SendMessage(GetDlgItem(hWnd, ID_PART_OK), WM_SETFONT, (WPARAM)g_language_normal_font, TRUE);
    SendMessage(GetDlgItem(hWnd, ID_PART_CANCEL), WM_SETFONT, (WPARAM)g_language_normal_font, TRUE);

	if (g_reserve_space >= 0)
    {
		swprintf_s(buf, 64, L"%lld", (long long)g_reserve_space);
    }
    else
    {
        buf[0] = 0;
    }

    SetWindowText(hEdit, buf);

	g_enable_reserve_space_tmp = g_enable_reserve_space;
    g_align_part_4KB_tmp = g_align_part_4KB;
    g_fs_type_tmp = g_fs_type;
	g_unit_sel_tmp = g_unit_sel;

    CheckRadioButton(hWnd, IDC_RADIO1, IDC_RADIO3, g_fs_radio_id[g_fs_type]);

	UpdateControlStatus(hWnd);

    return TRUE;
}

static VOID OnPartBtnOkClick(HWND hWnd)
{
    int code;
	HWND hEdit;
	ULONG SpaceVal = 0;
	CHAR Value[64] = { 0 };

	hEdit = GetDlgItem(hWnd, IDC_EDIT_RESERVE_SPACE_VAL);

	GetWindowTextA(hEdit, Value, sizeof(Value) - 1);
	
	SpaceVal = strtoul(Value, NULL, 10);

    if (g_enable_reserve_space_tmp)
    {
        if (g_unit_sel_tmp == 0)
        {
            if (SpaceVal == 0 || SpaceVal >= 1024 * 1024 * 2000)
            {
                MessageBox(hWnd, _G(STR_SPACE_VAL_INVALID), _G(STR_ERROR), MB_OK | MB_ICONERROR);
                return;
            }
        }
        else
        {
            if (SpaceVal == 0 || SpaceVal >= 1024 * 2000)
            {
                MessageBox(hWnd, _G(STR_SPACE_VAL_INVALID), _G(STR_ERROR), MB_OK | MB_ICONERROR);
                return;
            }
        }
    }


    code = (INT)SendMessage(GetDlgItem(hWnd, IDC_RADIO1), BM_GETSTATE, 0, 0);
    if (BST_CHECKED & code)
    {
        g_fs_type_tmp = 0;
    }
    else
    {
        code = (INT)SendMessage(GetDlgItem(hWnd, IDC_RADIO2), BM_GETSTATE, 0, 0);
        if (BST_CHECKED & code)
        {
            g_fs_type_tmp = 1;
        }
        else
        {
            g_fs_type_tmp = 2;
        }
    }

	g_reserve_space = (int)SpaceVal;
	g_enable_reserve_space = g_enable_reserve_space_tmp;
    g_align_part_4KB = g_align_part_4KB_tmp;
    g_fs_type = g_fs_type_tmp;
	g_unit_sel = g_unit_sel_tmp;

	SendMessage(hWnd, WM_CLOSE, 0, 0);
}

static VOID OnPartBtnCancelClick(HWND hWnd)
{
	SendMessage(hWnd, WM_CLOSE, 0, 0);
}

INT_PTR CALLBACK PartDialogProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	INT index;
    WORD NotifyCode;
    WORD CtrlID;

    switch (Message)
    {
        case WM_COMMAND:
        {
            NotifyCode = HIWORD(wParam);
            CtrlID = LOWORD(wParam);
            
            if (CtrlID == ID_PART_OK && NotifyCode == BN_CLICKED)
            {
				OnPartBtnOkClick(hWnd);
            }
            else if (CtrlID == ID_PART_CANCEL && NotifyCode == BN_CLICKED)
            {
				OnPartBtnCancelClick(hWnd);
            }
			else if (CtrlID == IDC_CHECK_RESERVE_SPACE && NotifyCode == BN_CLICKED)
			{
				g_enable_reserve_space_tmp = !g_enable_reserve_space_tmp;
				UpdateControlStatus(hWnd);
			}
            else if (CtrlID == IDC_CHECK_PART_ALIGN_4KB && NotifyCode == BN_CLICKED)
            {
                g_align_part_4KB_tmp = !g_align_part_4KB_tmp;
                UpdateControlStatus(hWnd);
            }
			else if (CtrlID == IDC_COMBO_SPACE_UNIT && NotifyCode == CBN_SELCHANGE)
			{
				index = (INT)SendMessage(GetDlgItem(hWnd, IDC_COMBO_SPACE_UNIT), CB_GETCURSEL, 0, 0);
				if (index != CB_ERR)
				{
					g_unit_sel_tmp = index;
				}
			}            
            else
            {
                return TRUE;
            }

            break;
        }
        case WM_INITDIALOG:
        {
            PartInitDialog(hWnd, wParam, lParam);
            return FALSE;
        }        
        case WM_CLOSE:
        {            
            EndDialog(hWnd, 0);
            return TRUE;
        }
    }

    return 0;
}
