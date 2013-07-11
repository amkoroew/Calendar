/**
 * Copyright 2013 Matthias Gugel <mail@matthias-gugel.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "../include/stdafx.h"

void printMonth(int, int);
void printYear(int);
int calculateDayOfWeek(int, int, int, bool);
void gotoxy(int, int);
COORD getCursorPosition();
void clearUsedScreenArea();
bool keyEventProc(KEY_EVENT_RECORD, int &, bool &);

int main(int argc, char * argv[]) {
	int year = 0;
	bool help = false, changed = true;
	HANDLE stdInputHandle;
	DWORD saveOldMode, cNumRead, i;
	INPUT_RECORD irInBuf[128];
	if(argc == 2) {
		year = strtol(argv[1], NULL, 10);
	} else {
		printf("Usage: %s year", argv[0]);
		return EXIT_FAILURE;
	}
	stdInputHandle = GetStdHandle(STD_INPUT_HANDLE);
	if(stdInputHandle == INVALID_HANDLE_VALUE) {
		return EXIT_FAILURE;
	}
	if(!GetConsoleMode(stdInputHandle, &saveOldMode)) {
		return EXIT_FAILURE;
	}
	
	system("cls");
	
	do {
		if(changed) {
			changed = false;
			if(help) {
				clearUsedScreenArea();
				gotoxy(0, 0);
				printf_s("\n\n\n\tHelp\n\t----\n\n\t[y] next Year\t\t[Y] previous Year\n\t[d] next Decade\t\t[D] previous Decade\n\t[c] next Century\t[C] previous Century\n\t[m] next Millenium\t[M] previous Millenium\n\t[h\\H] toggle help");
			} else {
				printYear(year);
				gotoxy(30, 38);
				printf_s("[h\\H] toggle help");
			}
		}
		if(!ReadConsoleInput(stdInputHandle, irInBuf, 128, &cNumRead)) {
			return EXIT_FAILURE;
		}
		for(i=0; i<cNumRead; ++i) {
			switch(irInBuf[i].EventType) {
				case KEY_EVENT:
					changed = keyEventProc(irInBuf[i].Event.KeyEvent, year, help);
				break;
				case MOUSE_EVENT:
				case WINDOW_BUFFER_SIZE_EVENT:
				case FOCUS_EVENT:
				case MENU_EVENT:
				break;
				default:
					return EXIT_FAILURE;
				break;
			}
			if(changed) {
				break;
			}
		}
	} while(true);

	SetConsoleMode(stdInputHandle, saveOldMode);
	return 0;
}

void printMonth(int month, int year) {
	int day, firstDayOffset, maxDayOfMonth;
	char weekday[7]={'M','D','M','D','F','S','S'};
	bool isLeapYear;
	short colOffset;
	colOffset = getCursorPosition().X;

	isLeapYear = year%400 == 0 || (year%100 != 0 && year%4 == 0);

	firstDayOffset = (calculateDayOfWeek(year, month, 1, isLeapYear)+5)%7;
	maxDayOfMonth = 31 - ((month%7)%2)-(month==1)*(2-isLeapYear);
	
	for(day=0; day<7; ++day) {
		printf_s("| %c", weekday[day]);
	}
	printf_s("|      \n");
	gotoxy(colOffset, getCursorPosition().Y);

	for(day=0; day<firstDayOffset; ++day) {
		printf_s("|  ");
	}
	day=0;
	while(day<36) {
		do {
			if(++day>maxDayOfMonth) {
				printf_s("|  ");
			} else {
				printf_s("|%2i", day);
			}
		} while((firstDayOffset+day)%7);
		printf("|      \n");
		gotoxy(colOffset, getCursorPosition().Y);
	}
	gotoxy(colOffset, getCursorPosition().Y);
	printf_s("                            ");
}

void printYear(int year) {
	int month;
	char * monthName[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
	gotoxy(34, 0);
	printf_s("Year %4i", year);
	for(month=0; month<12; ++month) {
		gotoxy(23*(month%3)+5, 8*(month/3)+(month/3)+2);
		printf_s("     %s        ", monthName[month]);
		gotoxy(23*(month%3)+5, 8*(month/3)+(month/3)+3);
		printMonth(month, year);
	}
}

int calculateDayOfWeek(int year, int month, int day, bool leapYear) {
	int dayOfWeek, centuryOffset;
	static int monthValue[12] = {1,4,4,0,2,5,0,3,6,1,4,6};
	static int centuryOffsetValue[4] = {-1,4,2,0};

	centuryOffset = centuryOffsetValue[(year % 400) / 100];
	year %= 100;
	dayOfWeek = day + monthValue[month] + year/12 + year%12 + (year%12)/4 + centuryOffset;
	if(leapYear && month < 2) {
		--dayOfWeek;
	}
	return dayOfWeek % 7;
}

void gotoxy(int col, int row) {
	COORD c;
	c.X = col;
	c.Y = row;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

COORD getCursorPosition() {
	PCONSOLE_SCREEN_BUFFER_INFO info = (PCONSOLE_SCREEN_BUFFER_INFO)malloc(sizeof(_CONSOLE_SCREEN_BUFFER_INFO));
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), info);
	return info->dwCursorPosition;
}

void clearUsedScreenArea() {
	int row;
	char spaceLine[81]="                                                                               \0";
	gotoxy(0, 0);
	for(row=0; row<40; ++row) {
		printf("%s\n", spaceLine);
	}
}

bool keyEventProc(KEY_EVENT_RECORD ker, int & year, bool & help) {
	bool changed = true;
	if(ker.bKeyDown) {
		switch(ker.uChar.AsciiChar) {
			case 'y': year += ker.wRepeatCount;
			break;
			case 'Y': year -= ker.wRepeatCount;
			break;
			case 'd': year += 10*ker.wRepeatCount;
			break;
			case 'D': year -= 10*ker.wRepeatCount;
			break;
			case 'c': year += 100*ker.wRepeatCount;
			break;
			case 'C': year -= 100*ker.wRepeatCount;
			break;
			case 'm': year += 1000*ker.wRepeatCount;
			break;
			case 'M': year -= 1000*ker.wRepeatCount;
			break;
			case 'h':
			case 'H':
				help = !help;
			break;
			default: changed = false;
			break;
		}
		year = max(year, 0);
	} else {
		changed = false;
	}
	return changed;
}