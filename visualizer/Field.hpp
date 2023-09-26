# pragma once
# include <Siv3D.hpp>
# include "Base.hpp"

extern int HEIGHT;
extern int WIDTH;
extern int CELL_SIZE;
extern int BLANK_LEFT;
extern int BLANK_TOP;

// フィールドの座標を引数に、セルの中心の画面上の座標を返す
Point get_cell_center(const Point p) {
	return Point(p.x * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, p.y * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2);
}
Rect get_grid_rect(const Point p) {
	return Rect{ p.x * CELL_SIZE + BLANK_LEFT, p.y * CELL_SIZE + BLANK_TOP, CELL_SIZE };
}
Circle get_grid_circle(const Point p) {
	return Circle(Arg::center(get_cell_center(p)), CELL_SIZE * 0.3);
}

Optional<Point> get_clicked_pos(const Point p, const Array<Point>& dydx) {
	for (const Point q : dydx) {
		if (is_in_field(p + q) and get_grid_rect(p + q).leftClicked()) {
			return q;
		}
	}
	return none;
}

Optional<Point> get_pressed_pos(void) {
	Point direction = {100, 100};
	if (KeyUp.pressed() and KeyLeft.pressed()) {
		direction = { -1, -1 };
	}else if (KeyLeft.pressed() and KeyDown.pressed()) {
		direction = { -1, 1 };
	}else if (KeyDown.pressed() and KeyRight.pressed()) {
		direction = { 1,1 };
	}else if (KeyRight.pressed() and KeyUp.pressed()) {
		direction = { 1, -1 };
	}else if (KeyUp.pressed()) {
		direction = { 0, -1 };
	}else if (KeyLeft.pressed()) {
		direction = { -1, 0 };
	}else if (KeyDown.pressed()) {
		direction = { 0, 1 };
	}else if (KeyRight.pressed()) {
		direction = { 1, 0 };
	}
	if (direction == Point(100,100)) {
		return none;
	}
	return  direction;

}
Optional<ACT> get_pressed_mode(void) {
	if (KeyZ.pressed()) {
		return ACT::MOVE;
	}else if (KeyX.pressed()) {
		return ACT::BUILD;
	}else if (KeyC.pressed()) {
		return ACT::DESTROY;
	}
	return none;
}

class Field {
public:
	// 指定したcsvの盤面をセット
	Field(String path);
	// 18種の内のランダムな盤面をセット
	Field(void);
	// 池、城、職人の座標配列を返す
	Array<Point> get_ponds(void) const;
	Array<Point> get_castles(void) const;
	Array<Point> get_craftsmen(TEAM team) const;
	// セルの情報を取得する
	CELL get_cell(const int y, const int x) const;
	CELL get_cell(const Point p) const;
	// セルの情報を追加する
	void set_bit(const int y, const int x, const CELL new_bit);
	void set_bit(const Point p, const CELL new_bit);
	// セルの情報を削除する
	void delete_bit(const int y, const int x, const CELL delete_bit);
	void delete_bit(const Point p, const CELL delete_bit);
	// 盤面のグリッドを表示させる
	void display_grid(void) const;
	// 盤面を表示させる
	void display_actors(void) const;
	// 陣地計算
	void calc_area(void);
	// ポイント計算
	void calc_point(TEAM team);
	// ポイント取得
	int get_point(TEAM team);
	// 建設物の数取得
	Array<int> get_building(TEAM team);
	// フィールド表示
	void output_field(void);

private:
	// 陣地計算のためのBFS
	void wall_bfs(Point start, Array<Array<bool>>& visited, TEAM team);
	// 盤面情報
	Array<Array<CELL>> grid;
	// 池、城、職人の座標配列
	Array<Point> ponds;
	Array<Point> castles;
	Array<Array<Point>> craftsmen;
	// それぞれの係数
	int point_castle = 100;
	int point_area = 30;
	int point_wall = 10;
	// チームポイント
	int point_red = 0;
	int point_blue = 0;
	// 城壁、陣地、城の数(
	Array<int> building_red = { 0,0,0 };
	Array<int> building_blue = { 0,0,0 };
};


// 指定したcsvファイルから盤面を読み込む
Field::Field(String path) {
	craftsmen.resize(2);
	const CSV csv{ path };
	HEIGHT = (int)csv.rows();
	WIDTH = (int)csv.columns(0);
	this->grid.resize(HEIGHT, Array<CELL>(WIDTH, CELL::NONE));
	for (int row = 0; row < csv.rows(); row++) {
		this->grid[row].resize(WIDTH);
		for (int col = 0; col < csv.columns(row); col++) {
			if (csv[row][col] == U"1") {
				set_bit(row, col, CELL::POND);
				ponds.push_back({ col, row });
			}else if (csv[row][col] == U"2") {
				set_bit(row, col, CELL::CASTLE);
				castles.push_back({ col, row });
			}else if (csv[row][col] == U"a") {
				set_bit(row, col, CELL::CRAFTSMAN_RED);
				craftsmen[(int)TEAM::RED].push_back({ col, row });
			}else if (csv[row][col] == U"b") {
				set_bit(row, col, CELL::CRAFTSMAN_BLUE);
				craftsmen[(int)TEAM::BLUE].push_back({ col, row });
			}
		}
	}
	return;
}

// ランダムで選んだcsvファイルから盤面を読み込む
Field::Field(void) {
	craftsmen.resize(2);
	FilePath path = FileSystem::DirectoryContents(U"field", Recursive::No).choice();
	const CSV csv{ path };
	HEIGHT = (int)csv.rows();
	WIDTH = (int)csv.columns(0);
	this->grid.resize(HEIGHT);
	for (int row = 0; row < csv.rows(); row++) {
		this->grid[row].resize(WIDTH);
		for (int col = 0; col < csv.columns(row); col++) {
			if (csv[row][col] == U"1") {
				set_bit(row, col, CELL::POND);
				ponds.push_back({ col, row });
			}else if (csv[row][col] == U"2") {
				set_bit(row, col, CELL::CASTLE);
				castles.push_back({ col, row });
			}else if (csv[row][col] == U"a") {
				set_bit(row, col, CELL::CRAFTSMAN_RED);
				craftsmen[(int)TEAM::RED].push_back({ col, row });
			}else if (csv[row][col] == U"b") {
				set_bit(row, col, CELL::CRAFTSMAN_BLUE);
				craftsmen[(int)TEAM::BLUE].push_back({ col, row });
			}
		}
	}
	return;
}

Array<Point> Field::get_ponds(void) const{
	return ponds;
}
Array<Point> Field::get_castles(void) const {
	return castles;
}
Array<Point> Field::get_craftsmen(TEAM team) const {
	return craftsmen[(int)team];
}

// セルの情報を取得
CELL Field::get_cell(const int y, const int x) const {
	return this->grid[y][x];
}
CELL Field::get_cell(const Point p) const {
	return this->get_cell(p.y, p.x);
}

// セルの情報を変更
void Field::set_bit(const int y, const int x, const CELL new_bit) {
	this->grid[y][x] |= new_bit;
}
void Field::set_bit(const Point p, const CELL new_bit) {
	this->set_bit(p.y, p.x, new_bit);
}

// セルの情報を削除
void Field::delete_bit(const int y, const int x, const CELL delete_bit) {
	this->grid[y][x] &= ~delete_bit;
}
void Field::delete_bit(const Point p, const CELL delete_bit) {
	this->delete_bit(p.y, p.x, delete_bit);
}

void Field::display_grid(void) const {
	for (int i = 0; i < HEIGHT * WIDTH; i++) {
		get_grid_rect(Point(i % WIDTH, i / WIDTH)).drawFrame(1, 1, Palette::Black);
	}
}

void Field::display_actors(void) const {
	for (int i = 0; i < (HEIGHT * WIDTH); i++) {
		int y = i / WIDTH;
		int x = i % WIDTH;
		Point p = { x,y };
		CELL target_cell = grid[y][x];

		if (target_cell & CELL::POND) {
			get_grid_rect(p).draw(Palette::Black);
		}
		if (target_cell & CELL::CASTLE) {
			Shape2D::Star(CELL_SIZE * 0.6, get_cell_center(p)).draw(Palette::Black);
		}
		if (target_cell & CELL::AREA_RED and target_cell & CELL::AREA_BLUE) {
			get_grid_rect(p).draw(ColorF(1.0, 0.0, 1.0, 0.5));
		}else 	if (target_cell & CELL::AREA_RED) {
			get_grid_rect(p).draw(ColorF(1.0, 0.0, 0.0, 0.25));
		}else 	if (target_cell & CELL::AREA_BLUE) {
			get_grid_rect(p).draw(ColorF(0.0, 0.0, 1.0, 0.25));
		}
		if (target_cell & CELL::WALL_RED) {
			Rect(Arg::center(get_cell_center(p)), (int)(CELL_SIZE*0.7)).draw(Palette::Red);
		}
		if (target_cell & CELL::WALL_BLUE) {
			Rect(Arg::center(get_cell_center(p)), (int)(CELL_SIZE * 0.7)).draw(Palette::Blue);
		}
		if (target_cell & CELL::CRAFTSMAN_RED) {
			Circle(Arg::center(get_cell_center(p)), CELL_SIZE * 0.3).draw(ColorF(1.0, 0.5, 0.5));
			Circle(Arg::center(get_cell_center(p)), CELL_SIZE * 0.3).drawFrame(1,1,Palette::White);
		}
		if (target_cell & CELL::CRAFTSMAN_BLUE) {
			Circle(Arg::center(get_cell_center(p)), CELL_SIZE * 0.3).draw(ColorF(0.5, 0.5, 1.0));
			Circle(Arg::center(get_cell_center(p)), CELL_SIZE * 0.3).drawFrame(1, 1, Palette::White);
		}
	}
}

// startからBFSを開始
const Array<Point> range_area = { {0,-1},{-1,0},{0,1},{1,0} };
void Field::wall_bfs(Point start, Array<Array<bool>>& visited, TEAM team) {
	std::queue<Point> que;
	que.push(start);
	visited[start.y][start.x] = true;
	if (get_cell(start) & switch_cell(CELL::WALL, team)) {
		return;
	}

	while (not que.empty()) {
		Point now = que.front();
		que.pop();
		for (auto& direction : range_area) {
			Point next = now + direction;
			if (not is_in_field(next)) {
				continue;
			}
			if (get_cell(next) & switch_cell(CELL::WALL, team) or
				visited[next.y][next.x]) {
				continue;
			}
			que.push(next);
			visited[next.y][next.x] = true;
		}
	}
}

// 陣地計算
void Field::calc_area(void) {
	Array<Array<bool>> red_visited(HEIGHT, Array<bool>(WIDTH, false));
	Array<Array<bool>> blue_visited(HEIGHT, Array<bool>(WIDTH, false));
	// フィールドの上下左右の辺からBFSを開始
	for (int h = 0; h < HEIGHT; h++) {
		for (int w = 0; w < WIDTH; w++) {
			if (not (h == 0 or h == HEIGHT - 1 or w == 0 or w == WIDTH - 1)) {
				continue;
			}
			wall_bfs({ h,w }, red_visited, TEAM::RED);
			wall_bfs({ h, w }, blue_visited, TEAM::BLUE);
		}
	}

	// BFSに到達できたかで陣地塗り
	for (int h = 0; h < HEIGHT; h++) {
		for (int w = 0; w < WIDTH; w++) {
			// 両方の陣地になる場合
			if ((not red_visited[h][w]) and (not blue_visited[h][w])) {
				set_bit(h, w, CELL::AREA);
			}
			// 赤の陣地となる場合
			else if (not red_visited[h][w]) {
				set_bit(h, w, CELL::AREA_RED);
				delete_bit(h, w, CELL::AREA_BLUE);
			}
			// 青の陣地となる場合
			else if (not blue_visited[h][w]) {
				set_bit(h, w, CELL::AREA_BLUE);
				delete_bit(h, w, CELL::AREA_RED);
			}
		}
	}

	// 壁は陣地外
	for (int h = 0; h < HEIGHT; h++) {
		for (int w = 0; w < WIDTH; w++) {
			if (get_cell(h, w) & CELL::WALL_RED) {
				delete_bit(h, w, CELL::AREA_RED);
			}
			if (get_cell(h, w) & CELL::WALL_BLUE) {
				delete_bit(h, w, CELL::AREA_BLUE);
			}
		}
	}

}


void Field::calc_point(TEAM team) {
	int point = 0;
	Array<int> building = { 0,0,0 };
	for (int h = 0; h < HEIGHT; h++) {
		for (int w = 0; w < WIDTH; w++) {
			if (get_cell(h, w) & switch_cell(CELL::AREA, team)) {
				if (get_cell(h, w) & CELL::CASTLE) { 
					point += this->point_castle;
					building[2]++;
				}
				point += this->point_area;
				building[1]++;
			}else if (get_cell(h, w) & switch_cell(CELL::WALL, team)) {
				point += this->point_wall;
				building[0]++;
			}
		}
	}
	if (team == TEAM::RED) {
		point_red = point;
		building_red = building;
	}else if(team == TEAM::BLUE){
		point_blue = point;
		building_blue = building;
	}


}

int Field::get_point(TEAM team) {
	if (team == TEAM::RED) {
		return point_red;
	}else{
		return point_blue;
	}
}

Array<int> Field::get_building(TEAM team) {
	if (team == TEAM::RED) {
		return building_red;
	}else{
		return building_blue;
	}
}

void Field::output_field(void) {
	for (int h = 0; h < HEIGHT; h++) {
		String str = U"";
		for (int w = 0; w < WIDTH; w++) {
			str += U"{:3X} "_fmt((unsigned char)grid[h][w]);
		}
		Console << str;
	}
	Console << '\n';
}
