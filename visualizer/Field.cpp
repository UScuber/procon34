# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Field.hpp"

enum CELL{
	POND = 1 << 0,
	WALL_ENEM = 1 << 1,
	WALL_ALLY = 1 << 2,
	AREA_ENEM = 1 << 3,
	AREA_ALLY = 1 << 4,
	ENEM = 1 << 5,
	ALLY = 1 << 6,
	CASTLE = 1 << 7
};

enum TEAM {
	RED = false,
	BLUE = true
};


extern size_t HEIGHT;
extern size_t WIDTH;
extern size_t CELL_SIZE;
extern size_t BLANK_LEFT;
extern size_t BLANK_TOP;


char SwitchCELL(String s, bool team) {
	if (s == U"WALL") {
		return (team == TEAM::BLUE) ? CELL::WALL_ALLY : CELL::WALL_ENEM;
	}else if (s == U"AREA") {
		return (team == TEAM::BLUE) ? CELL::AREA_ALLY : CELL::AREA_ENEM;
	}else if (s == U"CRAFTSMAN") {
		return (team == TEAM::BLUE) ? CELL::ALLY : CELL::ENEM;
	}
}

Rect GetGridRect(size_t y, size_t x) {
	return Rect{ x * CELL_SIZE + BLANK_LEFT, y * CELL_SIZE + BLANK_TOP, CELL_SIZE };
}
Circle GetGridCircle(size_t y, size_t x) {
	return Circle(Arg::center(x * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, y * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2), CELL_SIZE * 0.4);
}




Field::Field(void) {
	this->grid.resize(HEIGHT, Array<char>(WIDTH));
}

void Field::DisplayGrid(void) {
	for (size_t i = 0; i < (HEIGHT * WIDTH); i++) {
		GetGridRect(i / WIDTH, i % WIDTH).drawFrame(1, 1, Palette::Black);
	}
}

void Field::DrawActors(void) {
	for (size_t i = 0; i < (HEIGHT * WIDTH); i++) {
		char TargetCell = grid[i / WIDTH][i % WIDTH];
		if (TargetCell & CELL::POND) {
			Rect((i % WIDTH) * CELL_SIZE + BLANK_LEFT, (i / WIDTH) * CELL_SIZE + BLANK_TOP, CELL_SIZE).draw(Palette::Black);
		}
		if (TargetCell & CELL::WALL_ENEM) {
			Rect(Arg::center((i % WIDTH) * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, (i/WIDTH) * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2), CELL_SIZE * 0.6).draw(Palette::Red);
		}
		if (TargetCell & CELL::WALL_ALLY) {
			Rect(Arg::center((i % WIDTH) * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, (i / WIDTH) * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2), CELL_SIZE * 0.6).draw(Palette::Blue);
		}
		if (TargetCell & CELL::AREA_ENEM and TargetCell & CELL::AREA_ALLY) {
			Rect((i % WIDTH) * CELL_SIZE + BLANK_LEFT, (i / WIDTH) * CELL_SIZE + BLANK_TOP, CELL_SIZE).draw(ColorF(1.0, 0.0, 1.0, 0.5));
		}else 	if (TargetCell & CELL::AREA_ENEM) {
			Rect((i % WIDTH) * CELL_SIZE + BLANK_LEFT, (i / WIDTH) * CELL_SIZE + BLANK_TOP, CELL_SIZE).draw(ColorF(1.0, 0.0, 0.0, 0.25));
		}else 	if (TargetCell & CELL::AREA_ALLY) {
			Rect((i % WIDTH) * CELL_SIZE + BLANK_LEFT, (i / WIDTH) * CELL_SIZE + BLANK_TOP, CELL_SIZE).draw(ColorF(0.0, 0.0, 1.0, 0.25));
		}
		if (TargetCell & CELL::ENEM) {
			Circle(Arg::center((i % WIDTH) * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, (i / WIDTH) * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2), CELL_SIZE * 0.4).draw(Palette::Red);
		}
		if (TargetCell & CELL::ALLY) {
			Circle(Arg::center((i % WIDTH) * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, (i / WIDTH) * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2), CELL_SIZE * 0.4).draw(Palette::Blue);
		}
		if (TargetCell & CELL::CASTLE) {
			Shape2D::Star(CELL_SIZE * 0.6, Vec2{ (i % WIDTH) * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, (i / WIDTH) * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2 }).draw(Palette::Black);
		}
	}
}



/*
void WallDFS(size_t y, size_t x, Array<Array<bool>> &flgs, Array<std::pair<size_t, size_t>> &ary, Array<Array<char>> &grid) {
	ary.push_back({y, x});
	flgs[y][x] = true;
	for (auto &d: d_ary) {
		int next_y = y + d.first;
		int next_x = x + d.second;
		if (next_y < 0 or HEIGHT <= next_y	or	next_x < 0 or HEIGHT <= next_x) {
			continue;
		}
		if (not flgs[next_y][next_x] and grid[next_y][next_x] & WALL_ALLY) {
			WallDFS(next_y, next_x, flgs, ary, grid);
		}
	}
}
*/

const Array<std::pair<int, int>> d_ary = { {0,1},{-1,0},{0,-1},{1,0} };
void WallBFS(size_t y, size_t x, Array<Array<bool>>& visited, Array<Array<char>>& grid, bool team) {
	std::queue<std::pair<size_t, size_t>> que;
	que.push({ y, x });
	visited[y][x] = true;
	while (not que.empty()) {
		std::pair<size_t, size_t> now = que.front();
		que.pop();
		for (auto& d : d_ary) {
			std::pair<int, int> next = { now.first + d.first , now.second + d.second };
			if (next.first < 0 or HEIGHT <= next.first or next.second < 0 or WIDTH <= next.second) {
				continue;
			}
			if (grid[next.first][next.second] & SwitchCELL(U"WALL", team) or visited[next.first][next.second]) {
				continue;
			}
			que.push(next);
			visited[next.first][next.second] = true;
		}
	}
}

size_t Field::SearchArea(bool team) {
	Array<Array<bool>> visited(HEIGHT, Array<bool>(WIDTH, false));
	for (size_t h = 0; h < HEIGHT; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			if ((h == 0 or h == HEIGHT - 1 or w == 0 or w == WIDTH - 1) and not(this->grid[h][w] & SwitchCELL(U"WALL", team))) {
				WallBFS(h, w, visited, this->grid, team);
			}
		}
	}

	for (size_t h = 0; h < HEIGHT ; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			if (not(visited[h][w]) and not(this->grid[h][w] & SwitchCELL(U"WALL", team))) {
				this->grid[h][w] |= SwitchCELL(U"AREA", team);
			}
			else if (this->grid[h][w] & SwitchCELL(U"WALL", team)) {
				this->grid[h][w] &= ~SwitchCELL(U"AREA", team);
			}
		}
	}
	return 67;
}

