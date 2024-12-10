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

/*  1)까지 작성  * /







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
		// loop 돌 때마다(즉, TICK==10ms마다) 키 입력 확인
		KEY key = get_key();

		// 키 입력이 있으면 처리
		if (is_arrow_key(key)) {
			cursor_move(ktod(key));
		}
		else {
			// 방향키 외의 입력
			switch (key) {
			case k_quit: outro(); break;
			case k_none:
			case k_undef:
			default: break;
			}
		}

		// 샘플 오브젝트 동작
		sample_obj_move();

		// 화면 출력
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
	// layer 0(map[0])에 지형 생성
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

	// layer 1(map[1])은 비워 두기(-1로 채움)
	for (int i = 0; i < MAP_HEIGHT; i++) {
		for (int j = 0; j < MAP_WIDTH; j++) {
			map[1][i][j] = -1;
		}
	}


	// object sample



}
int can_build(POSITION pos) {
	if (map[0][pos.row][pos.column] == 'P') { //장판위만 건설
		return 1; 
	}
	return 0; 
}

// (가능하다면) 지정한 방향으로 커서 이동
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
	// 현재 위치와 목적지를 비교해서 이동 방향 결정	
	POSITION diff = psub(obj.dest, obj.pos);
	DIRECTION dir;

	// 목적지 도착. 지금은 단순히 원래 자리로 왕복
	if (diff.row == 0 && diff.column == 0) {
		if (obj.dest.row == 1 && obj.dest.column == 1) {
			// topleft --> bottomright로 목적지 설정
			POSITION new_dest = { MAP_HEIGHT - 2, MAP_WIDTH - 2 };
			obj.dest = new_dest;
		}
		else {
			// bottomright --> topleft로 목적지 설정
			POSITION new_dest = { 1, 1 };
			obj.dest = new_dest;
		}
		return obj.pos;
	}

	// 가로축, 세로축 거리를 비교해서 더 먼 쪽 축으로 이동
	if (abs(diff.row) >= abs(diff.column)) {
		dir = (diff.row >= 0) ? d_down : d_up;
	}
	else {
		dir = (diff.column >= 0) ? d_right : d_left;
	}

	// validation check
	// next_pos가 맵을 벗어나지 않고, (지금은 없지만)장애물에 부딪히지 않으면 다음 위치로 이동
	// 지금은 충돌 시 아무것도 안 하는데, 나중에는 장애물을 피해가거나 적과 전투를 하거나... 등등
	POSITION next_pos = pmove(obj.pos, dir);
	if (1 <= next_pos.row && next_pos.row <= MAP_HEIGHT - 2 && \
		1 <= next_pos.column && next_pos.column <= MAP_WIDTH - 2) {

		// 만약 'R'이 있는 위치라면 방향을 변경
		if (map[0][next_pos.row][next_pos.column] == 'R') {
			// 가로, 세로 축을 반대로 변경
			dir = (dir == d_up || dir == d_down) ? ((diff.column >= 0) ? d_right : d_left)
				: ((diff.row >= 0) ? d_down : d_up);
			next_pos = pmove(obj.pos, dir);

			// 만약 변경 후에도 충돌이 있다면 제자리 유지
			if (map[0][next_pos.row][next_pos.column] == 'R') {
				return obj.pos;
			}
		}

		// 장애물이나 경계에 부딪히지 않는 경우에만 이동
		if (map[1][next_pos.row][next_pos.column] < 0) {
			return next_pos;
		}
	}

	// 제자리 유지
	return obj.pos;
}

void sample_obj_move(void) {
	if (sys_clock < obj.next_move_time) {
		// 아직 시간이 안 됐음
		return;
	}

	// 오브젝트(건물, 유닛 등)은 layer1(map[1])에 저장
	map[1][obj.pos.row][obj.pos.column] = -1;
	obj.pos = sample_obj_next_position();
	map[1][obj.pos.row][obj.pos.column] = obj.repr;

	obj.next_move_time = sys_clock + obj.speed;
}