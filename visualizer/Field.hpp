# pragma once
# include <Siv3D.hpp>
# include "Base.hpp"

extern size_t HEIGHT;
extern size_t WIDTH;
extern size_t CELL_SIZE;
extern size_t BLANK_LEFT;
extern size_t BLANK_TOP;

Rect get_grid_rect(const Point p) {
	return Rect{ p.x * CELL_SIZE + BLANK_LEFT, p.y * CELL_SIZE + BLANK_TOP, CELL_SIZE };
}
Circle get_grid_circle(const Point p) {
	return Circle(Arg::center(p.x * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, p.y * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2), CELL_SIZE * 0.4);
}

Optional<Point> get_clicked_pos(const Point p, const Array<Point>& dydx) {
	for (const Point q : dydx) {
		if (is_in_field(p + q) and get_grid_rect(p + q).leftClicked()) {
			return q;
		}
	}
	return none;
}

class Field {
public:
	// 指定したcsvの盤面をセット
	Field(String path);
	// 18種の内のランダムな盤面をセット
	Field(void);
	// セルの情報を取得する
	char get_cell(const int y, const int x) const;
	char get_cell(const Point p) const;
	// セルの情報を追加する
	void set_bit(const int y, const int x, const char new_bit);
	void set_bit(const Point p, const char new_bit);
	// セルの情報を削除する
	void delete_bit(const int y, const int x, const char delete_bit);
	void delete_bit(const Point p, const char delete_bit);
	// 盤面のグリッドを表示させる
	void display_grid(void) const;
	// 盤面を表示させる
	void display_actors(void) const;
	// 陣地計算
	void calc_area(void);
	// ポイント計算
	void calc_point(bool team);
	// ポイント取得
	size_t get_point(bool team);
	// 建設物の数取得
	Array<size_t> get_building(bool team);
	// フィールド表示
	void output_field(void);

private:
	// 陣地計算のためのBFS
	void wall_bfs(Point start, Array<Array<bool>>& visited, bool team);
	// 盤面情報
	Array<Array<char>> grid;
	// それぞれの係数
	size_t point_castle = 100;
	size_t point_area = 30;
	size_t point_wall = 10;
	// チームポイント
	size_t point_red = 0;
	size_t point_blue = 0;
	// 城壁、陣地、城の数
	Array<size_t> building_red = { 0,0,0 };
	Array<size_t> building_blue = { 0,0,0 };
};


// 指定したcsvファイルから盤面を読み込む
Field::Field(String path) {
	const CSV csv{ path };
	HEIGHT = csv.rows();
	WIDTH = csv.columns(0);
	this->grid.resize(HEIGHT, Array<char>(WIDTH, 0));
	for (int8 row = 0; row < csv.rows(); row++) {
		for (int8 col = 0; col < csv.columns(row); col++) {
			if (csv[row][col] == U"1") {
				set_bit(row, col, CELL::POND);
			}else if (csv[row][col] == U"2") {
				set_bit(row, col, CELL::CASTLE);
			}else if (csv[row][col] == U"a") {
				set_bit(row, col, CELL::CRAFTSMAN_RED);
			}else if (csv[row][col] == U"b") {
				set_bit(row, col, CELL::CRAFTSMAN_BLUE);
			}
		}
	}
	return;
}

// ランダムで選んだcsvファイルから盤面を読み込む
Field::Field(void) {
	FilePath path = FileSystem::DirectoryContents(U"field", Recursive::No).choice();
	const CSV csv{ path };
	HEIGHT = csv.rows();
	WIDTH = csv.columns(0);
	this->grid.resize(HEIGHT);
	for (int8 row = 0; row < csv.rows(); row++) {
		this->grid[row].resize(WIDTH);
		for (int8 col = 0; col < csv.columns(row); col++) {
			if (csv[row][col] == U"1") {
				this->grid[row][col] = CELL::POND;
			}
			else if (csv[row][col] == U"2") {
				this->grid[row][col] = CELL::CASTLE;
			}
			else if (csv[row][col] == U"a") {
				this->grid[row][col] = CELL::CRAFTSMAN_RED;
			}
			else if (csv[row][col] == U"b") {
				this->grid[row][col] = CELL::CRAFTSMAN_BLUE;
			}
		}
	}
	return;
}

// セルの情報を取得
char Field::get_cell(const int y, const int x) const {
	return this->grid[y][x];
}
char Field::get_cell(const Point p) const {
	return this->get_cell(p.y, p.x);
}

// セルの情報を変更
void Field::set_bit(const int y, const int x, const char new_bit) {
	this->grid[y][x] |= new_bit;
}
void Field::set_bit(const Point p, const char new_bit) {
	this->set_bit(p.y, p.x, new_bit);
}

// セルの情報を削除
void Field::delete_bit(const int y, const int x, const char delete_bit) {
	this->grid[y][x] &= ~delete_bit;
}
void Field::delete_bit(const Point p, const char delete_bit) {
	this->delete_bit(p.y, p.x, delete_bit);
}

void Field::display_grid(void) const {
	for (size_t i = 0; i < HEIGHT * WIDTH; i++) {
		get_grid_rect(Point(i % WIDTH, i / WIDTH)).drawFrame(1, 1, Palette::Black);
	}
}

void Field::display_actors(void) const {
	for (size_t i = 0; i < (HEIGHT * WIDTH); i++) {
		char target_cell = grid[i / WIDTH][i % WIDTH];
		if (target_cell & CELL::POND) {
			Rect((i % WIDTH) * CELL_SIZE + BLANK_LEFT, (i / WIDTH) * CELL_SIZE + BLANK_TOP, CELL_SIZE).draw(Palette::Black);
		}
		if (target_cell & CELL::WALL_RED) {
			Rect(Arg::center((i % WIDTH) * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, (i / WIDTH) * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2), CELL_SIZE * 0.6).draw(Palette::Red);
		}
		if (target_cell & CELL::WALL_BLUE) {
			Rect(Arg::center((i % WIDTH) * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, (i / WIDTH) * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2), CELL_SIZE * 0.6).draw(Palette::Blue);
		}
		if (target_cell & CELL::AREA_RED and target_cell & CELL::AREA_BLUE) {
			Rect((i % WIDTH) * CELL_SIZE + BLANK_LEFT, (i / WIDTH) * CELL_SIZE + BLANK_TOP, CELL_SIZE).draw(ColorF(1.0, 0.0, 1.0, 0.5));
		}
		else 	if (target_cell & CELL::AREA_RED) {
			Rect((i % WIDTH) * CELL_SIZE + BLANK_LEFT, (i / WIDTH) * CELL_SIZE + BLANK_TOP, CELL_SIZE).draw(ColorF(1.0, 0.0, 0.0, 0.25));
		}
		else 	if (target_cell & CELL::AREA_BLUE) {
			Rect((i % WIDTH) * CELL_SIZE + BLANK_LEFT, (i / WIDTH) * CELL_SIZE + BLANK_TOP, CELL_SIZE).draw(ColorF(0.0, 0.0, 1.0, 0.25));
		}
		if (target_cell & CELL::CRAFTSMAN_RED) {
			Circle(Arg::center((i % WIDTH) * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, (i / WIDTH) * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2), CELL_SIZE * 0.4).draw(Palette::Red);
		}
		if (target_cell & CELL::CRAFTSMAN_BLUE) {
			Circle(Arg::center((i% WIDTH)* CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, (i / WIDTH)* CELL_SIZE + BLANK_TOP + CELL_SIZE / 2), CELL_SIZE * 0.4).draw(Palette::Blue);
		}
		if (target_cell & CELL::CASTLE) {
			Shape2D::Star(CELL_SIZE * 0.6, Vec2{ (i % WIDTH) * CELL_SIZE + BLANK_LEFT + CELL_SIZE / 2, (i / WIDTH) * CELL_SIZE + BLANK_TOP + CELL_SIZE / 2 }).draw(Palette::Black);
		}
	}
}

// startからBFSを開始
const Array<Point> range_area = { {0,-1},{-1,0},{0,1},{1,0} };
void Field::wall_bfs(Point start, Array<Array<bool>>& visited, bool team) {
	std::queue<Point> que;
	que.push(start);
	visited[start.y][start.x] = true;
	if (get_cell(start) & switch_cell(CELL_TYPE::WALL, team)) {
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
			if (get_cell(next) & switch_cell(CELL_TYPE::WALL, team) or
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
	for (size_t h = 0; h < HEIGHT; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			if (not (h == 0 or h == HEIGHT - 1 or w == 0 or w == WIDTH - 1)) {
				continue;
			}
			wall_bfs({ h,w }, red_visited, TEAM::RED);
			wall_bfs({ h, w }, blue_visited, TEAM::BLUE);
		}
	}

	// BFSに到達できたかで陣地塗り
	for (size_t h = 0; h < HEIGHT; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			// 両方の陣地になる場合
			if ((not red_visited[h][w]) and (not blue_visited[h][w])) {
				set_bit(h, w, CELL_TYPE::AREA);
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
	for (size_t h = 0; h < HEIGHT; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			if (get_cell(h, w) & CELL::WALL_RED) {
				delete_bit(h, w, CELL::AREA_RED);
			}
			if (get_cell(h, w) & CELL::WALL_BLUE) {
				delete_bit(h, w, CELL::AREA_BLUE);
			}
		}
	}

}


void Field::calc_point(bool team) {
	size_t point = 0;
	Array<size_t> building = { 0,0,0 };
	for (size_t h = 0; h < HEIGHT; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			if (get_cell(h, w) & switch_cell(CELL_TYPE::AREA, team)) {
				if (get_cell(h, w) & CELL::CASTLE) { 
					point += this->point_castle;
					building[2]++;
				}
				point += this->point_area;
				building[1]++;
			}else if (get_cell(h, w) & switch_cell(CELL_TYPE::WALL, team)) {
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

size_t Field::get_point(bool team) {
	if (team == TEAM::RED) {
		return point_red;
	}else if (team == TEAM::BLUE) {
		return point_blue;
	}
}

Array<size_t> Field::get_building(bool team) {
	if (team == TEAM::RED) {
		return building_red;
	}else if (team == TEAM::BLUE) {
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
