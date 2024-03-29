#pragma once


#ifdef _WIN64
INT_PTR ConfigDlgWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#else
int CALLBACK ConfigDlgWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif


//! Initialize dialog controls, fill it with data from config structure
void ConfigDlgInitDialog(HWND hWnd);

//! Copy values from controls to config structure
void UpdateConfig(HWND hWnd);

//! Procedure handling selection of song length file
void SelectHvscFile(HWND hWnd);

//! Procedure handling selection of HVSC directory
void SelectHvscDirectory(HWND hWnd);