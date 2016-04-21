#include "text_queue.h"
#include <windows.h>
#include <string.h>
#include <stdlib.h>

#define QUEUE_SZ     15
#define QUEUE_BUF_SZ 48

static char textbuf[QUEUE_SZ][QUEUE_BUF_SZ] = {0,};
static size_t index = 0; // maximum: QUEUE_SZ
static HANDLE h_stdout = NULL;

void tq_enqueue(char *string) {
	int i;
	if (index == QUEUE_SZ) {
		for (i=1; i<QUEUE_SZ; i++) {
			memcpy(textbuf[i-1], textbuf[i], QUEUE_BUF_SZ);
		}
		index--;
	}

	int copied = 0;
	char *src = string;
	char *dest = textbuf[index];
	while (*src && copied < QUEUE_BUF_SZ) {
		*dest = *src;
		src++;
		dest++;
		copied++;
	}
	while (dest < textbuf[index+1]) {
		*dest = ' ';
		dest++;
	}
	// textbuf[index][QUEUE_BUF_SZ-2] = '\n';
	// textbuf[index][QUEUE_BUF_SZ-1] = 0;

	index++;
}
void tq_print(void) {
	if (h_stdout == NULL){
		h_stdout = GetStdHandle(STD_OUTPUT_HANDLE);
	}

	COORD zero;
	zero.X = 0;
	zero.Y = 0;

	int i, written;
	for (i=0; i<index; i++){
		SetConsoleCursorPosition(h_stdout, zero);
		WriteConsole(h_stdout, textbuf[i], QUEUE_BUF_SZ, &written, NULL);
		zero.Y++;
	}
}