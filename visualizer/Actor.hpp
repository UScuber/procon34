# pragma once
# include <Siv3D.hpp>
# include "Field.hpp"
# include "Base.hpp"

extern size_t HEIGHT;
extern size_t WIDTH;
extern size_t CELL_SIZE;
extern size_t BLANK_LEFT;
extern size_t BLANK_TOP;

class Craftsman {
public:
	// 盤面情報に合わせて職人を配置
	Craftsman(int y, int x, bool team);
	// 職人の行動情報を初期化
	void initialize(void);
	// 城壁の建築
	bool build(Field &field, const Point direction);
	// 城壁の破壊
	bool destroy(Field& field, const Point direction);
	// 職人の移動
	bool move(Field& field, const Point direction);
	// 行動情報の出力
	String output_act(void);

// private:
	// 職人の座標
	Point pos = { 0, 0 };
	// 行動済みかどうか
	bool is_acted = false;
	// 選択済みかどうか
	bool is_target = false;
	// 職人のチーム
	bool team = TEAM::RED;
	// 行動をした方向
	char direction = 0;
	// 行動の種類
	char act = ACT::NOTHING;
};

// 職人の移動範囲
const Array<Point> range_move = { {0,-1},{-1,0},{0,1},{1,0},{-1,-1},{-1,1},{1,1},{1,-1} };
// 職人の建設・破壊範囲
const Array<Point> range_wall = { {0,-1},{-1,0},{0,1},{1,0} };

// 職人の座標とチームを初期化
Craftsman::Craftsman(int y, int x, bool team) {
	this->pos = { x, y };
	this->team = team;
}

// 職人の行動情報を初期化
void Craftsman::initialize(void) {
	this->is_acted = false;
	this->is_target = false;
	this->direction = 0;
	this->act = ACT::NOTHING;
}

// 建築
bool Craftsman::build(Field& field, const Point direction) {
	const Point target_cell_pos = this->pos + direction;
	// 建設可能範囲内か
	if (not is_in_field(target_cell_pos) or direction.manhattanLength() != 1) {
		return false;
	}
	const char target_cell = field.get_cell(target_cell_pos);
	// 建設可能な場所か
	if (target_cell & CELL_TYPE::WALL or
		target_cell & switch_cell(CELL_TYPE::CRAFTSMAN, not team) or
		target_cell & CELL::CASTLE) {
		return false;
	}
	// フィールド変化
	field.set_bit(target_cell_pos, switch_cell(CELL_TYPE::WALL, team));
	// 行動情報
	this->is_acted = true;
	this->is_target = false;
	this->act = ACT::BUILD;
	for (int8 i = 0; i < range_wall.size(); i++) {
		if (range_wall[i] == direction) {
			this->direction = i;
			break;
		}
	}
	return true;
}

// 破壊
bool Craftsman::destroy(Field& field, const Point direction) {
	const Point target_cell_pos = this->pos + direction;
	// 破壊可能範囲内か
	if (not is_in_field(target_cell_pos) or direction.manhattanLength() != 1) {
		return false;
	}
	const char target_cell = field.get_cell(target_cell_pos);
	// 破壊可能な場所か
	if (not target_cell & CELL_TYPE::WALL) {
		return false;
	}
	// フィールド変化
	field.delete_bit(target_cell_pos, CELL_TYPE::WALL);
	// 行動情報
	this->is_acted = true;
	this->is_target = false;
	this->act = ACT::DESTROY;
	for (int8 i = 0; i < range_wall.size(); i++) {
		if (range_wall[i] == direction) {
			this->direction = i;
			break;
		}
	}
	return true;
}

// 移動
bool Craftsman::move(Field& field, const Point direction) {
	const Point target_cell_pos = this->pos + direction;
	// 移動可能範囲か
	if ((not is_in_field(target_cell_pos)) or Abs(direction.y) > 1 or Abs(direction.x) > 1) {
		return false;
	}
	// 移動可能な場所か
	const char target_cell = field.get_cell(target_cell_pos);
	if (target_cell & CELL::POND or
		target_cell & switch_cell(CELL_TYPE::WALL, not team) or
		target_cell & CELL_TYPE::CRAFTSMAN) {
		return false;
	}
	// フィールド変化
	field.delete_bit(this->pos, switch_cell(CELL_TYPE::CRAFTSMAN, team));
	this->pos = target_cell_pos;
	field.set_bit(this->pos, switch_cell(CELL_TYPE::CRAFTSMAN, team));
	// 行動情報
	this->is_acted = true;
	this->is_target = false;
	this->act = ACT::MOVE;
	for (int8 i = 0; i < range_move.size(); i++) {
		if (range_move[i] == direction) {
			this->direction = i;
			break;
		}
	}
	return true;
}
