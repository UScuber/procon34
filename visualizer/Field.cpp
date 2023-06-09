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


char SwitchCELL(const String& s, const bool team) {
	if (s == U"WALL") {
		return (team == TEAM::BLUE) ? CELL::WALL_ALLY : CELL::WALL_ENEM;
	}else if (s == U"AREA") {
		return (team == TEAM::BLUE) ? CELL::AREA_ALLY : CELL::AREA_ENEM;
	}else if (s == U"CRAFTSMAN") {
		return (team == TEAM::BLUE) ? CELL::ALLY : CELL::ENEM;
	}
}

Rect GetGridRect(const Point p) {
	return Rect{ p.x * CELL_SIZE + BLANK_LEFT, p.y * CELL_SIZE + BLANK_TOP, CELL_SIZE };
}
Circle GetGridCircle(const Point p) {
	return Circle(Arg::center(p.x * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, p.y * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2), CELL_SIZE * 0.4);
}

bool isInField(int y, int x) {
	return (0 <= y and y < HEIGHT and 0 <= x and x < WIDTH);
}
bool isInField(const Point& p) {
	return (0 <= p.y and p.y < HEIGHT and 0 <= p.x and p.x < WIDTH);
}

Optional<Point> get_clicked_pos(const Point p, const Array<Point>& dydx) {
	for (const Point q : dydx) {
		if (isInField(p + q) and GetGridRect(p + q).leftClicked()) {
			return q;
		}
	}
	return none;
}


Field::Field(void) {
	this->grid.resize(HEIGHT, Array<char>(WIDTH));
}

void Field::Initialize(size_t pond, size_t castle, size_t craftsman){
	size_t y, x;
	for (size_t i = 0; i < (pond  + craftsman * 2); i++) {
		do {
			y = Random(HEIGHT-1);
			x = Random(WIDTH-1);
		} while (this->grid[y][x] != 0);
		if (i < pond) {
			this->grid[y][x] |= CELL::POND;
		}else if (i < pond + craftsman) {
			this->grid[y][x] |= CELL::ALLY;
		}else {
			this->grid[y][x] |= CELL::ENEM;
		}
	}
	for (size_t i = 0; i < castle; i++) {
		do {
			y = Random(1, (int)HEIGHT - 2);
			x = Random(1, (int)WIDTH - 2);
		} while (this->grid[y][x] != 0);
		this->grid[y][x] |= CELL::CASTLE;
	}
	return;
}

/*
void Field::Initialize(size_t pond, size_t castle, size_t craftsman) {
	size_t y, x;
	for (size_t i = 0; i < (pond + castle + craftsman * 2); i++) {
		do {
			y = Random(HEIGHT - 1);
			x = Random(WIDTH - 1);
		} while (this->grid[y][x] != 0);
		if (i < pond) {
			this->grid[y][x] |= CELL::POND;
		}
		else if (i < pond + castle) {
			this->grid[y][x] |= CELL::CASTLE;
		}
		else if (i < pond + castle + craftsman) {
			this->grid[y][x] |= CELL::ALLY;
		}
		else {
			this->grid[y][x] |= CELL::ENEM;
		}
	}
	return;
}
*/


char Field::getGrid(const int y, const int x) const {
	return grid[y][x];
}

char Field::getGrid(const Point p) const {
	return grid[p.y][p.x];
}

void Field::setGrid(const int y, const int x, const char v) {
	grid[y][x] = v;
}

void Field::setGrid(const Point p, const char v) {
	grid[p.y][p.x] = v;
}

void Field::DisplayGrid(void) const {
	for (size_t i = 0; i < (HEIGHT * WIDTH); i++) {
		GetGridRect(Point(i % WIDTH, i / WIDTH)).drawFrame(1, 1, Palette::Black);
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



// {y,x}からBFSを始める
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
			if (not isInField(next.first, next.second)) {
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

// 陣地計算
void Field::SearchArea(void) {
	Array<Array<bool>> blue_visited(HEIGHT, Array<bool>(WIDTH, false));
	Array<Array<bool>> red_visited(HEIGHT, Array<bool>(WIDTH, false));
	// フィールドの辺からBFSを開始
	for (size_t h = 0; h < HEIGHT; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			if ((h == 0 or h == HEIGHT - 1 or w == 0 or w == WIDTH - 1) and not(this->grid[h][w] & SwitchCELL(U"WALL", TEAM::BLUE))) {
				WallBFS(h, w, blue_visited, this->grid, TEAM::BLUE);
			}
			if ((h == 0 or h == HEIGHT - 1 or w == 0 or w == WIDTH - 1) and not(this->grid[h][w] & SwitchCELL(U"WALL", TEAM::RED))) {
				WallBFS(h, w, red_visited, this->grid, TEAM::RED);
			}
		}
	}
	// BFSで到達できたかで陣地塗り
	for (size_t h = 0; h < HEIGHT ; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			// 両方の陣地となる場合
			if ((not blue_visited[h][w]) and (not red_visited[h][w])) {
				grid[h][w] |= (CELL::AREA_ALLY | CELL::AREA_ENEM);
			}
			// 片方の陣地となる場合
			else if (blue_visited[h][w] xor red_visited[h][w]) {
				if (not blue_visited[h][w]) {
					grid[h][w] |= SwitchCELL(U"AREA", TEAM::BLUE);
					grid[h][w] &= ~SwitchCELL(U"AREA", TEAM::RED);
				}
				else if (not red_visited[h][w]) {
					grid[h][w] |= SwitchCELL(U"AREA", TEAM::RED);
					grid[h][w] &= ~SwitchCELL(U"AREA", TEAM::BLUE);
				}
			}
		}
	}

	// 壁は陣地外
	for (size_t h = 0; h < HEIGHT; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			if (grid[h][w] & SwitchCELL(U"WALL", TEAM::BLUE)) {
				grid[h][w] &= ~SwitchCELL(U"AREA", TEAM::BLUE);
			}
			if (grid[h][w] & SwitchCELL(U"WALL", TEAM::RED)) {
				grid[h][w] &= ~SwitchCELL(U"AREA", TEAM::RED);
			}
		}
	}
}

size_t Field::CountArea(bool team) {
	size_t count = 0;
	for (size_t h = 0; h < HEIGHT; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			if (grid[h][w] & SwitchCELL(U"AREA", team)) {
				count++;
			}
		}
	}
	return count;
}
