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


Craftsman::Craftsman(Field& field, size_t y, size_t x){
	this->y_coordinate = y;
	this->x_coordinate = x;
	field.grid[y][x] |= CELL::ALLY;
}

// 城壁を建築
bool Craftsman::Build(Field& field, size_t y, size_t x) {
	// 建築可能範囲
	if (AbsDiff(y, this->y_coordinate) + AbsDiff(x, this->x_coordinate) > 1) {
		return false;
	}
	// 建築可能な場所か
	char& TargetCell = field.grid[y][x];
	if (TargetCell & CELL::POND or TargetCell & CELL::WALL_ENEM or TargetCell & CELL::WALL_ALLY or
		TargetCell & CELL::ENEM or TargetCell & CELL::CASTLE) {
		return false;
	}
	// bit変化
	TargetCell |= CELL::WALL_ALLY;
	return true;
}

// 城壁を破壊
bool Craftsman::Break(Field& field, size_t y, size_t x) {
	// 破壊可能範囲
	if (AbsDiff(y, this->y_coordinate) + AbsDiff(x, this->x_coordinate) > 1) {
		return false;
	}
	// 破壊する城壁があるか
	char& TargetCell = field.grid[y][x];
	if (not (TargetCell & CELL::WALL_ENEM or TargetCell & CELL::WALL_ALLY)) {
		return false;
	}
	// bit変化
	TargetCell &= ~CELL::WALL_ENEM;
	TargetCell &= ~CELL::WALL_ALLY;
	return true;
}

// 移動
bool Craftsman::Move(Field& field, int dy, int dx) {
	// 移動可能範囲
	if (Abs(dy) > 1 or Abs(dx) > 1) {
		return false;
	}
	// 移動先がフィールド内か
	int next_y = this->y_coordinate + dy;
	int next_x = this->x_coordinate + dx;
	if (next_y < 0 or next_x < 0 or next_y >= HEIGHT or next_x >= WIDTH) {
		return false;
	}
	// 移動可能な場所か
	char& TargetCell = field.grid[next_y][next_x];
	if (TargetCell & CELL::POND or TargetCell & CELL::WALL_ENEM or
		TargetCell & CELL::ENEM or TargetCell & CELL::ALLY) {
		return false;
	}
	// 座標変化とbit変化
	field.grid[x_coordinate][y_coordinate] &= ~CELL::ALLY;
	this->y_coordinate = next_y;
	this->x_coordinate = next_x;
	field.grid[next_y][next_x] |= CELL::ALLY;
	return true;
}

