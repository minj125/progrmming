#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "io.h"
#include "display.h"
#include <Windows.h>
#include <stdio.h>
void init(void);
void intro(void);
void outro(void);
void cursor_move(DIRECTION dir);
void sample_obj_move(void);
POSITION sample_obj_next_position(void);

/*  1)���� �ۼ�  * /







/* ================= control =================== */
int sys_clock = 0;		// system-wide clock(ms)
CURSOR cursor = { { 1, 1 }, {1, 1} };

void drawWithBackground(int x, int y, char ch, int backgroundColor) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	int colorCode = 0 + (backgroundColor * 16); 
	SetConsoleTextAttribute(hConsole, colorCode);

	POSITION pos_1 = { x,y };
	gotoxy(pos_1);
	printf("%c", ch);

	SetConsoleTextAttribute(hConsole, 15);
}

/* ================= game data =================== */

char map[N_LAYER][MAP_HEIGHT][MAP_WIDTH] = { 0 };

RESOURCE resource = {
	.spice = 0,
	.spice_max = 0,
	.population = 0,
	.population_max = 0
};

OBJECT_SAMPLE obj = {
	.pos = {1, 1},
	.dest = {MAP_HEIGHT - 2, MAP_WIDTH - 2},
	.repr = 'o',
	.speed = 300,
	.next_move_time = 300
};
/* ================= main() =================== */
int main(void) {

	srand((unsigned int)time(NULL));

	init();

	intro();
	display(resource, map, cursor);

	drawWithBackground(17,1, 'B', 3);
	drawWithBackground(16, 1, ' ', 3);
	drawWithBackground(17, 2, ' ', 3);
	drawWithBackground(16, 2, ' ', 3);

	drawWithBackground(15, 1, 'H', 3);


	drawWithBackground(17, 3, 'P', 8);
	drawWithBackground(17, 4, ' ', 8);
	drawWithBackground(16, 3, ' ', 8);
	drawWithBackground(16, 4, ' ', 8);

	drawWithBackground(2, 58, 'B', 12);
	drawWithBackground(2, 57, ' ', 12);
	drawWithBackground(3, 58, ' ', 12);
	drawWithBackground(3, 57, ' ', 12);

	drawWithBackground(4, 58, 'H', 12);

	drawWithBackground(2,56, 'P', 8);
	drawWithBackground(3,56, ' ', 8);
	drawWithBackground(2,55, ' ', 8);
	drawWithBackground(3,55, ' ', 8);

	drawWithBackground(13, 1, '5', 6);
	drawWithBackground(6,58, '5', 6);

	drawWithBackground(5,11, 'W', 14);
	drawWithBackground(12,38, 'W', 14);

	drawWithBackground(11, 18, 'R', 7);
	drawWithBackground(15, 45, 'R', 7);
	drawWithBackground(6, 45, 'R', 7);

	drawWithBackground(6, 20, 'R', 7);
	drawWithBackground(7, 20, ' ', 7);
	drawWithBackground(6, 21, ' ', 7);
	drawWithBackground(7, 21, ' ', 7);

	drawWithBackground(14, 20, 'R', 7);
	drawWithBackground(15, 20, ' ', 7);
	drawWithBackground(14, 21, ' ', 7);
	drawWithBackground(15, 21, ' ', 7);



	

	while (1) {
		// loop �� ������(��, TICK==10ms����) Ű �Է� Ȯ��
		KEY key = get_key();

		// Ű �Է��� ������ ó��
		if (is_arrow_key(key)) {
			cursor_move(ktod(key));
		}
		else {
			// ����Ű ���� �Է�
			switch (key) {
			case k_quit: outro(); break;
			case k_none:
			case k_undef:
			default: break;
			}
		}

		// ���� ������Ʈ ����
		sample_obj_move();

		// ȭ�� ���
		display(resource, map, cursor);
		Sleep(TICK);
		sys_clock += 10;
	}
}

/* ================= subfunctions =================== */
void intro(void) {
	POSITION posxy = { 8,20 };
	gotoxy(posxy);
	printf("@@@@DUNE 1.5@@@@\n");
	Sleep(2000);
	system("cls");
}

void outro(void) {
	POSITION posxy_1 = { 9,23 };
	gotoxy(posxy_1);
	printf("exiting...\n");
	exit(0);
}

void init(void) {
	// layer 0(map[0])�� ���� ����
	for (int j = 0; j < MAP_WIDTH; j++) {
		map[0][0][j] = '#';
		map[0][MAP_HEIGHT - 1][j] = '#';
	}

	for (int i = 1; i < MAP_HEIGHT - 1; i++) {
		map[0][i][0] = '#';
		map[0][i][MAP_WIDTH - 1] = '#';
		for (int j = 1; j < MAP_WIDTH - 1; j++) {
			map[0][i][j] = ' ';
		}
	}

	// layer 1(map[1])�� ��� �α�(-1�� ä��)
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[1][i][j] = -1;
		}
	}


	// object sample



}
int can_build(POSITION pos) {
	if (map[0][pos.row][pos.column] == 'P') { //�������� �Ǽ�
		return 1; 
	}
	return 0; 
}

// (�����ϴٸ�) ������ �������� Ŀ�� �̵�
void cursor_move(DIRECTION dir) {
	POSITION curr = cursor.current;
	POSITION new_pos = pmove(curr, dir);

	// validation check
	if (1 <= new_pos.row && new_pos.row <= MAP_HEIGHT - 2 && \
		1 <= new_pos.column && new_pos.column <= MAP_WIDTH - 2) {

		cursor.previous = cursor.current;
		cursor.current = new_pos;

		display(resource, map, cursor);
		POSITION pos_1 = { 0, MAP_HEIGHT + 1 };
		gotoxy(pos_1);
		printf("Cursor Position: (%d, %d)\n", new_pos.row, new_pos.column);
	}
}

/* ================= sample object movement =================== */
POSITION sample_obj_next_position(void) {
	// ���� ��ġ�� �������� ���ؼ� �̵� ���� ����	
	POSITION diff = psub(obj.dest, obj.pos);
	DIRECTION dir;

	// ������ ����. ������ �ܼ��� ���� �ڸ��� �պ�
	if (diff.row == 0 && diff.column == 0) {
		if (obj.dest.row == 1 && obj.dest.column == 1) {
			// topleft --> bottomright�� ������ ����
			POSITION new_dest = { MAP_HEIGHT - 2, MAP_WIDTH - 2 };
			obj.dest = new_dest;
		}
		else {
			// bottomright --> topleft�� ������ ����
			POSITION new_dest = { 1, 1 };
			obj.dest = new_dest;
		}
		return obj.pos;
	}

	// ������, ������ �Ÿ��� ���ؼ� �� �� �� ������ �̵�
	if (abs(diff.row) >= abs(diff.column)) {
		dir = (diff.row >= 0) ? d_down : d_up;
	}
	else {
		dir = (diff.column >= 0) ? d_right : d_left;
	}

	// validation check
	// next_pos�� ���� ����� �ʰ�, (������ ������)��ֹ��� �ε����� ������ ���� ��ġ�� �̵�
	// ������ �浹 �� �ƹ��͵� �� �ϴµ�, ���߿��� ��ֹ��� ���ذ��ų� ���� ������ �ϰų�... ���
	POSITION next_pos = pmove(obj.pos, dir);
	if (1 <= next_pos.row && next_pos.row <= MAP_HEIGHT - 2 && \
		1 <= next_pos.column && next_pos.column <= MAP_WIDTH - 2) {

		// ���� 'R'�� �ִ� ��ġ��� ������ ����
		if (map[0][next_pos.row][next_pos.column] == 'R') {
			// ����, ���� ���� �ݴ�� ����
			dir = (dir == d_up || dir == d_down) ? ((diff.column >= 0) ? d_right : d_left)
				: ((diff.row >= 0) ? d_down : d_up);
			next_pos = pmove(obj.pos, dir);

			// ���� ���� �Ŀ��� �浹�� �ִٸ� ���ڸ� ����
			if (map[0][next_pos.row][next_pos.column] == 'R') {
				return obj.pos;
			}
		}

		// ��ֹ��̳� ��迡 �ε����� �ʴ� ��쿡�� �̵�
		if (map[1][next_pos.row][next_pos.column] < 0) {
			return next_pos;
		}
	}

	// ���ڸ� ����
	return obj.pos;
}

void sample_obj_move(void) {
	if (sys_clock < obj.next_move_time) {
		// ���� �ð��� �� ����
		return;
	}

	// ������Ʈ(�ǹ�, ���� ��)�� layer1(map[1])�� ����
	map[1][obj.pos.row][obj.pos.column] = -1;
	obj.pos = sample_obj_next_position();
	map[1][obj.pos.row][obj.pos.column] = obj.repr;

	obj.next_move_time = sys_clock + obj.speed;
}