# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Actor.hpp"
# include "Field.hpp"

enum CELL {
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

// 初期化
Actor::Actor(size_t y, size_t x) :
	y_coordinate(y), x_coordinate(x) {}


bool Craftsman::Build(Field& field, size_t y, size_t x) {
	if (AbsDiff(y, this->y_coordinate) + AbsDiff(x, this->x_coordinate) > 1) {
		return false;
	}
	if (not field[y][x] & CELL::POND and
		not field[y][x] & CELL::WALL_ENEM and
		not field[y][x] & CELL::WALL_ALLY and
		not field[y][x] & CELL::ENEM and
		not field[y][x] & CELL::CASTLE) {
		field[y][x] |= CELL::WALL_ALLY;
	}
}
