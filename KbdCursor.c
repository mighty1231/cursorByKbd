// Current version : 2.0

#include <windows.h>
#include "text_queue.h"
#pragma comment(lib, "User32")

#define HKPARAM_ON 30
#define HKPARAM_OFF 31
#define HKPARAM_EXIT 32

#define LCLICK_KEY 'A'
#define RCLICK_KEY 'S'

#define MOVE_INITIAL 1
#define MOVE_DELTA 2

static HHOOK hook_id;

static int curMode = 0;
static int repeat = MOVE_INITIAL;
static BOOL isActive = FALSE;

static int l_isUP = TRUE;
static int r_isUP = TRUE;

static char text_buffer[100]; // QUEUE_BUF_SZ
static char clock_buf[40];

#define printText(...) {sprintf(text_buffer, __VA_ARGS__); tq_enqueue(text_buffer); tq_print();}

void getTime(void) {
	SYSTEMTIME st;
	GetLocalTime(&st);
	sprintf(clock_buf, "%4d-%02d-%02d %02d:%02d:%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
}
void moveCursor( int x, int y ){
	INPUT input = {0};
	DWORD flag = MOUSEEVENTF_MOVE;
	if( GetAsyncKeyState( LCLICK_KEY ) ){ flag |= MOUSEEVENTF_LEFTDOWN; }
	if( GetAsyncKeyState( RCLICK_KEY ) ){ flag |= MOUSEEVENTF_RIGHTDOWN; }

	input.type = INPUT_MOUSE;
	input.mi.dx = x;
	input.mi.dy = y;
	input.mi.dwFlags = flag;
	input.mi.dwExtraInfo = GetMessageExtraInfo();
	SendInput( 1, &input, sizeof( INPUT ) );
}

LRESULT CALLBACK HOOKFUNC( int nCode, WPARAM wParam, LPARAM lParam ){
	if(nCode < 0){ return CallNextHookEx(0, nCode, wParam, lParam); }

	KBDLLHOOKSTRUCT *kbdstru;
	DWORD flag;
	INPUT input;

	getTime();
	if( isActive ){
		kbdstru = (KBDLLHOOKSTRUCT *) lParam;
		if( wParam == WM_KEYDOWN ){
			switch( kbdstru->vkCode ){
				case LCLICK_KEY:
					if( l_isUP ){
						l_isUP = FALSE;
						flag = MOUSEEVENTF_LEFTDOWN;
						printText("[%s] L Down", clock_buf);
						goto call_input;
					}
					printText("[%s] L Double", clock_buf); goto ignore_next_hook;
				case RCLICK_KEY:
					if( r_isUP ){
						r_isUP = FALSE;
						flag = MOUSEEVENTF_RIGHTDOWN;
						printText("[%s] R Down", clock_buf);
						goto call_input;
					}
					printText("[%s] R Double", clock_buf); goto ignore_next_hook;
				case VK_LEFT:
					if( curMode == VK_LEFT ) repeat += MOVE_DELTA;
					else{ curMode = VK_LEFT; repeat = 1; }
					moveCursor(-repeat, 0); goto ignore_next_hook;
				case VK_RIGHT:
					if( curMode == VK_RIGHT ) repeat += MOVE_DELTA;
					else{ curMode = VK_RIGHT; repeat = 1; }
					moveCursor(repeat, 0); goto ignore_next_hook;
				case VK_UP:
					if( curMode == VK_UP ) repeat += MOVE_DELTA;
					else{ curMode = VK_UP; repeat = 1; }
					moveCursor(0, -repeat); goto ignore_next_hook;
				case VK_DOWN:
					if( curMode == VK_DOWN ) repeat += MOVE_DELTA;
					else{ curMode = VK_DOWN; repeat = 1; }
					moveCursor(0, repeat); goto ignore_next_hook;
			}
		}
		else if( wParam == WM_KEYUP ){
			switch( kbdstru->vkCode ){
				case LCLICK_KEY:
					if( !l_isUP ){
						l_isUP = TRUE;
						flag = MOUSEEVENTF_LEFTUP;
						printText("[%s] L UP", clock_buf);
						goto call_input;
					}
					printText("[%s] should not be happened", clock_buf);
					goto ignore_next_hook;
				case RCLICK_KEY:
					if( !r_isUP ){
						r_isUP = TRUE;
						flag = MOUSEEVENTF_RIGHTUP;
						printText("[%s] R UP", clock_buf);
						goto call_input;
					} 
					printText("[%s] should not be happened", clock_buf);
					goto ignore_next_hook;
			}
		}
	}
	return CallNextHookEx(0, nCode, wParam, lParam);

	call_input:
		FillMemory( &input, sizeof(INPUT), 0 );
		input.type = INPUT_MOUSE;
		input.mi.dwFlags = flag;
		input.mi.dwExtraInfo = GetMessageExtraInfo();
		SendInput( 1, &input, sizeof(INPUT) );
	ignore_next_hook:
		return 1;
}
int main(){
	// ON with CTRL+SHIFT+X
	// OFF with CTRL+SHIFT+C
	// EXIT with CTRL+SHIFT+V
	if( !RegisterHotKey( NULL, HKPARAM_ON, MOD_CONTROL | MOD_SHIFT, (int)'X' ) ){
		MessageBox( NULL, "ctrl + shift + X error", NULL, MB_OK | MB_ICONSTOP);
		return 0;
	}
	printText("HotKey: Ctrl+Shift+X registered to turn on");
	if( !RegisterHotKey( NULL, HKPARAM_OFF, MOD_CONTROL | MOD_SHIFT, (int)'C' ) ){
		MessageBox( NULL, "ctrl + shift + C error", NULL, MB_OK | MB_ICONSTOP);
		return 0;
	}
	printText("HotKey: Ctrl+Shift+C registered to turn off");
	if( !RegisterHotKey( NULL, HKPARAM_EXIT, MOD_CONTROL | MOD_SHIFT, (int)'V' ) ){
		MessageBox( NULL, "ctrl + shift + V error", NULL, MB_OK | MB_ICONSTOP);
		return 0;
	}
	printText("HotKey: Ctrl+Shift+V registered to exit");

	hook_id = SetWindowsHookEx( WH_KEYBOARD_LL, HOOKFUNC, (HINSTANCE) GetModuleHandle(NULL), 0 );
	printText("Hook: Success with id = 0x%X", hook_id);

	MSG msg;
	while( GetMessage(&msg, NULL, 0, 0) != 0 ){
		switch( msg.wParam ){
			case HKPARAM_ON:
				getTime();
				isActive = TRUE;
				printText("[%s] ON", clock_buf);
				break;
			case HKPARAM_OFF:
				getTime();
				isActive = FALSE;
				printText("[%s] OFF", clock_buf);
				break;
			case HKPARAM_EXIT:
				UnhookWindowsHookEx( hook_id );
				UnregisterHotKey( NULL, HKPARAM_ON );
				UnregisterHotKey( NULL, HKPARAM_OFF );
				UnregisterHotKey( NULL, HKPARAM_EXIT );
				return 0;
		}
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}
}