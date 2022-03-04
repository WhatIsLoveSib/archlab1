#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define OD 0b00000001 // перевыполнение при выполнении операции 
					 // overfulfillment during operation
#define DE 0b00000010 // ошибка деления на 0
					 // division error by 0
#define EG 0b00000100 // ошибка выхода за границы
					 // error of going beyond borders
#define CI 0b00001000 // игнорирование тактовых импульсов
					 // clock ignoring
#define IC 0b00010000 // неверная команда
					 // invalid command
#define MEMORY_SIZE 100
#define CELL_SIZE sizeof(int)


int ptr_str[MEMORY_SIZE];

unsigned short int flags;

int sc_memoryInit();
int sc_memorySet(int address, int value);
int sc_memoryGet(int address, int *value);
int sc_memorySave(char *filename);
int sc_memoryLoad(char* filename);
void sm_printMemory();
int sc_regInit();
int sc_regSet(int reg, int value);
int sc_regGet(int register, int *value);
int sc_commandEncode(int command, int operand, int *value);
int sc_commandDecode(int value, int *command, int *operand);

enum colors {
	black = 0,
	red = 1,
	green = 2,
	yellow = 3,
	blue = 4,
	purple = 5,
	cyan = 6,
	white = 7
};

int mt_clrscr();
int mt_gotoXY(int y, int x);
int mt_getscreensize(int *rows, int *cols);
int mt_ssetfgcolor(enum colors color);
int mt_ssetbgcolor(enum colors color);
int mt_stopcolor();


int sc_memoryInit()
{
	for (int i = 0; i < MEMORY_SIZE; i++) {
		ptr_str[i] = 0;
	}
	
	return 0;
}

int sc_memorySet(int address, int value)
{
	if (address < 0 || address > 99) {
		//flag error;
		return 1;
	}
	ptr_str[address] = value;
	
	return 0;
}

int sc_memoryGet(int address, int *value)
{
	if (address < 0 || address > 99) {
		return 1;
	}
	
	*value = ptr_str[address];
	
	return 0;
} 

int sc_memorySave(char *filename)
{
	FILE *ptrFile = fopen(filename, "wb");

	fwrite(ptr_str, CELL_SIZE, MEMORY_SIZE, ptrFile);

	fclose(ptrFile);

	return 0;
}

int sc_memoryLoad(char* filename)
{
	FILE *ptrFile = fopen(filename, "rb");
	
	fread(ptr_str, CELL_SIZE, MEMORY_SIZE, ptrFile);
	
	fclose(ptrFile);

	return 0;
}

void sm_printMemory()
{
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 10; j++) {
			printf("+%04x ", ptr_str[i * 10 + j]);
		}
		printf("\n");
	}
}

int sc_regInit()
{
	flags = 0;
	return 0;
}

int sc_regSet(int reg, int value)
{
	if (reg == OD || reg == DE || reg == EG || reg == CI || reg == IC) {
		if (value == 0) {
			flags = flags & (~(reg));
		} if (value == 1) {
			flags = flags | reg;
		} else {
			return 1;
		}
	} else {
		return 1;
	}

	return 0;
}

int sc_regGet(int reg, int *value)
{
	if (!value) {
		return 1;
	}

	if (reg == OD) {
		*value = flags & 0x1;
	} else if (reg == DE) {
		*value = (flags >> 1) & 0x1;
	} else if (reg == EG) {
		*value = (flags >> 2) & 0x1;
	} else if (reg == CI) {
		*value = (flags >> 3) & 0x1;
	} else if (reg == IC) {
		*value = (flags >> 4) & 0x1;
	} else {
		return 1;
	}

	return 0;
}

int sc_commandEncode(int command, int operand, int *value)
{
	if (!value) {
		return 1;
	}

	if (command < 10 ||
		(command > 11 && command < 20) ||
		(command > 21 && command < 30) ||
		(command > 33 && command < 40) ||
		(command > 43 && command < 51) ||
		command > 76) {
		return 1;
	}

	if (operand < 0 || operand > 127) {
		return 1;
	}

	*value = *value | (command << 7);
	*value = *value | operand;

	return 0;
}

int sc_commandDecode(int value, int *command, int *operand)
{
	if (!command || !operand) {
		return 1;
	}

	if ((value >> 14) != 0) {
		return 1;
	}

	*command = value >> 7;
	*operand = value & 0b1111111;

	return 0;
}

int mt_clrscr()
{
	printf("\E[H\E[J");

	return 0;
}

int mt_gotoXY(int y, int x)
{

	printf("\E[%d;%dH", y, x);

	return 0;
}

int mt_getscreensize(int *rows, int *cols)
{
	struct winsize ws;

	if (ioctl(1, TIOCGWINSZ, &ws)) {
		return 1;
	} else {
		*rows = ws.ws_row;
		*cols = ws.ws_col;
	}

	return 0;
}

int mt_ssetfgcolor(enum colors color)
{
	printf("\E[3%dm", color);

	return 0;
}

int mt_ssetbgcolor(enum colors color)
{
	printf("\E[4%dm", color);

	return 0;
}

int mt_stopcolor()
{
	printf("\E[0m");

	return 0;
}

int main()
{
	sc_memoryInit();
	//sm_printMemory();
	
	sc_memorySet(0, 65535);
	
	//int tmp = 0;
	//sc_memoryGet(55, &tmp);
	//printf("tmp = %d\n", tmp);
	//sc_memorySave("test.bin");
	
	//sc_memoryLoad("test.bin");
	// system("clear");
	// sm_printMemory();

	printf("flags = ");
	for(int i = 7; i >= 0; --i) {
		printf("%d", (flags >> i) & 1);
		if (i == 4) {
			printf(" ");
		}
	}
	printf("\n");

	int *val = malloc(sizeof(int));

	sc_commandEncode(26, 3, val);

	printf("value = ");
	for(int i = 14; i >= 0; --i) {
		printf("%d", (*val >> i) & 1);
		if (i % 7 == 0) {
			printf(" ");
		}
	}
	printf("\n");

	int *command = malloc(sizeof(int));
	int *operand = malloc(sizeof(int));
	int value = 0b001100111011001;
	sc_commandDecode(value, command, operand);

	printf("value = ");
	for(int i = 14; i >= 0; --i) {
		printf("%d", (value >> i) & 1);
		if (i % 7 == 0) {
			printf(" ");
		}
	}
	printf("\n");

	printf("command = ");
	for(int i = 7; i >= 0; --i) {
		printf("%d", (*command >> i) & 1);
		if (i % 4 == 0) {
			printf(" ");
		}
	}
	printf("\n");

	printf("operand = ");
	for(int i = 7; i >= 0; --i) {
		printf("%d", (*operand >> i) & 1);
		if (i % 4 == 0) {
			printf(" ");
		}
	}
	printf("\n");

	sm_printMemory();

	// mt_clrscr();

	mt_gotoXY(3, 2);


	int *r = malloc(sizeof(int));
	int *c = malloc(sizeof(int));

	mt_getscreensize(r, c);

	printf("%d :: %d\n", *r, *c);

	enum colors f = red;

	mt_ssetfgcolor(f);

	enum colors a = white;

	mt_ssetbgcolor(a);

	printf("HELLO\n");

	mt_stopcolor();

	return 0;
}