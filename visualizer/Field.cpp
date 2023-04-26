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


extern size_t HEIGHT;
extern size_t WIDTH;
extern size_t CELL_SIZE;


Field::Field(void) {
	this->grid.resize(HEIGHT, Array<char>(WIDTH));
}

void Field::DisplayGrid(void) {
	for (size_t i = 0; i < (HEIGHT * WIDTH); i++) {
		Rect((i % WIDTH) * CELL_SIZE + 100, (i / WIDTH) * CELL_SIZE + 100, CELL_SIZE).drawFrame(1, 1, Palette::Black);
	}
}

void Field::DrawActors(void) {
	for (size_t i = 0; i < (HEIGHT * WIDTH); i++) {
		char TargetCell = grid[i / WIDTH][i % WIDTH];
		if (TargetCell & CELL::POND) {
			Rect((i % WIDTH) * CELL_SIZE + 100, (i / WIDTH) * CELL_SIZE + 100, CELL_SIZE).draw(Palette::Black);
		}
		if (TargetCell & CELL::WALL_ENEM) {
			Rect(Arg::center((i % WIDTH) * CELL_SIZE + 100 + CELL_SIZE / 2, (i/WIDTH) * CELL_SIZE + 100 + CELL_SIZE / 2), CELL_SIZE * 0.6).draw(Palette::Red);
		}
		if (TargetCell & CELL::WALL_ALLY) {
			Rect(Arg::center((i % WIDTH) * CELL_SIZE + 100 + CELL_SIZE / 2, (i / WIDTH) * CELL_SIZE + 100 + CELL_SIZE / 2), CELL_SIZE * 0.6).draw(Palette::Blue);
		}
		if (TargetCell & CELL::AREA_ENEM) {
			Rect((i % WIDTH) * CELL_SIZE + 100, (i / WIDTH) * CELL_SIZE + 100, CELL_SIZE).draw(ColorF(1.0, 0.0, 0.0, 0.25));
		}
		if (TargetCell & CELL::AREA_ALLY) {
			Rect((i % WIDTH) * CELL_SIZE + 100, (i / WIDTH) * CELL_SIZE + 100, CELL_SIZE).draw(ColorF(0.0, 0.0, 1.0, 0.25));
		}
		if (TargetCell & CELL::ENEM) {
			Circle(Arg::center((i % WIDTH) * CELL_SIZE + 100 + CELL_SIZE / 2, (i / WIDTH) * CELL_SIZE + 100 + CELL_SIZE / 2), CELL_SIZE * 0.4).draw(Palette::Red);
		}
		if (TargetCell & CELL::ALLY) {
			Circle(Arg::center((i % WIDTH) * CELL_SIZE + 100 + CELL_SIZE / 2, (i / WIDTH) * CELL_SIZE + 100 + CELL_SIZE / 2), CELL_SIZE * 0.4).draw(Palette::Blue);
		}
		if (TargetCell & CELL::CASTLE) {
			Shape2D::Star(CELL_SIZE * 0.6, Vec2{ (i % WIDTH) * CELL_SIZE + 100 + CELL_SIZE / 2, (i / WIDTH) * CELL_SIZE + 100 + CELL_SIZE / 2 }).draw(Palette::Black);
		}
	}
}



const Array<std::pair<int, int>> d_ary = { {0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0},{1,1} };
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

Array<Array<std::pair<size_t, size_t>>> SearchWall(Array<Array<char>>& grid) {
	Array<Array<std::pair<size_t, size_t>>> res;
	Array<Array<bool>> flgs(HEIGHT, Array<bool>(WIDTH, false));
	for (size_t i = 0; i < HEIGHT; i++) {
		for (size_t j = 0; j < WIDTH; j++) {
			if (not flgs[i][j] and grid[i][j] & WALL_ALLY) {
				Array<std::pair<size_t, size_t>> ary;
				WallDFS(i, j, flgs, ary, grid);
				res.push_back(ary);
			}
		}
	}
	return res;
}

size_t Field::SearchArea(void) {
	Array<Array<std::pair<size_t, size_t>>> WallArays = SearchWall(this->grid);
	for (auto& ary : WallArays) {
		Array<Array<size_t>> EachRowWalls(HEIGHT);
		for (auto& wall : ary) {
			EachRowWalls[wall.first].push_back(wall.second);
		}
		for (size_t y = 0; y < HEIGHT; y++) {
			Array<size_t>& walls = EachRowWalls[y];
			if (walls.size() == 0) {
				continue;
			}
			auto iters = std::minmax_element(walls.begin(), walls.end());
			for (size_t x = *iters.first; x <= *iters.second; x++) {
				Print << x;
				if (not (this->grid[y][x] & CELL::WALL_ALLY)) {
					this->grid[y][x] |= AREA_ALLY;
				}
			}
		}
	}
	return 67;
}

