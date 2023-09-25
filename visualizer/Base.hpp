# pragma once
# include <Siv3D.hpp>

extern size_t HEIGHT;
extern size_t WIDTH;
extern size_t CELL_SIZE;
extern size_t BLANK_LEFT;
extern size_t BLANK_TOP;

enum CELL {
	POND = 1 << 0,
	WALL_RED = 1 << 1,
	WALL_BLUE = 1 << 2,
	AREA_RED = 1 << 3,
	AREA_BLUE = 1 << 4,
	CRAFTSMAN_RED = 1 << 5,
	CRAFTSMAN_BLUE = 1 << 6,
	CASTLE = 1 << 7
};

enum TEAM {
	RED = false,
	BLUE = true
};

enum ACT {
	NOTHING = 0,
	BUILD = 1,
	DESTROY = 2,
	MOVE = 3
};

enum CELL_TYPE {
	WALL = (1 << 1) + (1 << 2),
	AREA = (1 << 3) + (1 << 4),
	CRAFTSMAN = (1 << 5) + (1 << 6)
};

bool is_in_field(int y, int x) {
	return (0 <= y and y < HEIGHT and 0 <= x and x < WIDTH);
}
bool is_in_field(const Point p) {
	return is_in_field(p.y, p.x);
}

char switch_cell(char type, bool team) {
	if (type == CELL_TYPE::WALL) {
		return (team == TEAM::RED) ? CELL::WALL_RED : CELL::WALL_BLUE;
	}else if (type == CELL_TYPE::AREA) {
		return (team == TEAM::RED) ? CELL::AREA_RED : CELL::AREA_BLUE;
	}else if (type == CELL_TYPE::CRAFTSMAN) {
		return (team == TEAM::RED) ? CELL::CRAFTSMAN_RED : CELL::CRAFTSMAN_BLUE;
	}
}
