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

const Array<Point> dydx = { {0,-1},{-1,0},{0,1},{1,0},{-1,-1},{-1,1},{1,1},{1,-1} };

// コンストラクタで座標、チームを取得しフィールドを書き換える
Craftsman::Craftsman(Field& field, int y, int x, bool team){
	this->pos = { x, y };
	this->team = team;
	field.setGrid(y, x, field.getGrid(y, x) | SwitchCELL(U"CRAFTSMAN", team));
}

// 状態をリセット
void Craftsman::Initialize(void) {
	this->isActed = false;
	this->isTarget = false;
	this->Direction = 0;
	this->Act = U"";
}


// 城壁を建築
bool Craftsman::Build(Field& field, const Point d) {
	const Point to_pos = this->pos + d;
	// 建築可能範囲
	if (not isInField(to_pos) or (Abs(d.y) + Abs(d.x) != 1)) {
		return false;
	}
	// 建築可能な場所か
	const char TargetCell = field.getGrid(to_pos);
	if (TargetCell & CELL::WALL_ENEM or TargetCell & CELL::WALL_ALLY or
		TargetCell & SwitchCELL(U"CRAFTSMAN", not team) or TargetCell & CELL::CASTLE) {
		return false;
	}
	// bit変化
	field.setGrid(to_pos, TargetCell | SwitchCELL(U"WALL", team));

	this->isActed = true;
	this->isTarget = false;
	for (size_t i = 0; i < dydx.size(); i++) {
		if (dydx[i] == d) {
			this->Direction = i;
			break;
		}
	}
	this->Act = U"build";
	return true;
}

// 城壁を破壊
bool Craftsman::Break(Field& field, const Point d) {
	const Point to_pos = this->pos + d;
	// 建築可能範囲
	if (not isInField(to_pos) or (Abs(d.y) + Abs(d.x) != 1)) {
		return false;
	}
	// 建築可能な場所か
	const char TargetCell = field.getGrid(to_pos);
	if (not (TargetCell & CELL::WALL_ENEM or TargetCell & CELL::WALL_ALLY)) {
		return false;
	}
	// bit変化
	field.setGrid(to_pos, TargetCell & ~(CELL::WALL_ENEM | CELL::WALL_ALLY));

	this->isActed = true;
	this->isTarget = false;
	for (size_t i = 0; i < dydx.size(); i++) {
		if (dydx[i] == d) {
			this->Direction = i;
			break;
		}
	}
	this->Act = U"break";
	return true;
}

// 移動
bool Craftsman::Move(Field& field, const Point d) {
	// 移動可能範囲
	if (Abs(d.y) > 1 or Abs(d.x) > 1) {
		return false;
	}
	// 移動先がフィールド内か
	const Point next = this->pos + d;
	if (not isInField(next)){
		return false;
	}
	// 移動可能な場所か
	const char TargetCell = field.getGrid(next);
	if (TargetCell & CELL::POND or TargetCell & SwitchCELL(U"WALL", not team) or
		TargetCell & CELL::ENEM or TargetCell & CELL::ALLY) {
		return false;
	}
	// 座標変化とbit変化
	field.setGrid(this->pos, field.getGrid(this->pos) & ~SwitchCELL(U"CRAFTSMAN", team));
	this->pos = next;
	field.setGrid(next, field.getGrid(next) | SwitchCELL(U"CRAFTSMAN", team));
	this->isActed = true;
	this->isTarget = false;
	for (size_t i = 0; i < dydx.size(); i++) {
		if (dydx[i] == d) {
			this->Direction = i;
			break;
		}
	}
	this->Act = U"move";
	return true;
}

