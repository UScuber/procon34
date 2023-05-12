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

const Array<std::pair<int, int>> dydx = { {-1,0},{0, -1},{1,0},{0,1},{-1,-1},{1,-1},{1,1},{-1,1} };

// コンストラクタで座標、チームを取得しフィールドを書き換える
Craftsman::Craftsman(Field& field, size_t y, size_t x, bool team){
	this->y_coordinate = y;
	this->x_coordinate = x;
	this ->team = team;
	field.grid[y][x] |= SwitchCELL(U"CRAFTSMAN", team);
}

// 状態をリセット
void Craftsman::Initialize(void) {
	this->isActed = false;
	this->isTarget = false;
	this->Direction = 0;
	this->Act = U"";
}


// 城壁を建築
bool Craftsman::Build(Field& field, int dy, int dx) {
	// 建築可能範囲
	if (not isInField(this->y_coordinate + dy, this->x_coordinate + dx) or not((Abs(dy) == 1) xor (Abs(dx) == 1))) {
		return false;
	}
	// 建築可能な場所か
	char& TargetCell = field.grid[this->y_coordinate + dy][this->x_coordinate + dx];
	if (TargetCell & CELL::WALL_ENEM or TargetCell & CELL::WALL_ALLY or
		TargetCell & SwitchCELL(U"CRAFTSMAN", not team) or TargetCell & CELL::CASTLE) {
		return false;
	}
	// bit変化
	TargetCell |= SwitchCELL(U"WALL", team);
	this->isActed = true;
	this->isTarget = false;
	for (size_t i = 0; i < dydx.size(); i++) {
		if (dydx[i].first == dy and dydx[i].second == dx) {
			this->Direction = i;
			break;
		}
	}
	this->Act = U" build";
	return true;
}

// 城壁を破壊
bool Craftsman::Break(Field& field, int dy, int dx) {
	// 建築可能範囲
	if (not isInField(this->y_coordinate + dy, this->x_coordinate + dx) or not((Abs(dy) == 1) xor (Abs(dx) == 1))) {
		return false;
	}
	// 建築可能な場所か
	char& TargetCell = field.grid[this->y_coordinate + dy][this->x_coordinate + dx];
	if (not (TargetCell & CELL::WALL_ENEM or TargetCell & CELL::WALL_ALLY)) {
		return false;
	}
	// bit変化
	TargetCell &= ~CELL::WALL_ENEM;
	TargetCell &= ~CELL::WALL_ALLY;
	this->isActed = true;
	this->isTarget = false;
	for (size_t i = 0; i < dydx.size(); i++) {
		if (dydx[i].first == dy and dydx[i].second == dx) {
			this->Direction = i;
			break;
		}
	}
	this->Act = U" break";
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
	if (not isInField(next_y, next_x)){
		return false;
	}
	// 移動可能な場所か
	char& TargetCell = field.grid[next_y][next_x];
	if (TargetCell & CELL::POND or TargetCell & SwitchCELL(U"WALL", not team) or
		TargetCell & CELL::ENEM or TargetCell & CELL::ALLY) {
		return false;
	}
	// 座標変化とbit変化
	field.grid[y_coordinate][x_coordinate] &= ~SwitchCELL(U"CRAFTSMAN", team);
	this->y_coordinate = next_y;
	this->x_coordinate = next_x;
	field.grid[next_y][next_x] |= SwitchCELL(U"CRAFTSMAN", team);
	this->isActed = true;
	this->isTarget = false;
	for (size_t i = 0; i < dydx.size(); i++) {
		if (dydx[i].first == dy and dydx[i].second == dx) {
			this->Direction = i;
			break;
		}
	}
	this->Act = U" move";
	return true;
}

