# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.10
# include "Base.hpp"
# include "Field.hpp";
# include "Actor.hpp"

// +--------------+
//  |  先攻 : 赤   |
//  |  後攻 : 青   |
// +--------------+

// フィールドの縦横
size_t HEIGHT;
size_t WIDTH;
// 一つのセルの大きさ(正方形)
size_t CELL_SIZE = 25;
// フィールドの左上に開ける余白
size_t BLANK_LEFT = 50;
size_t BLANK_TOP = 50;

using App = SceneManager<String, Field>;

class Game {
protected:
	// GUIによる操作
	void operate_gui(Field& field);
	// マウスクリックによる操作
	void operate_craftsman(bool team, Field& field);
	// フィールドの表示
	void display_field(void) const ;
	// 詳細表示
	void display_details(Field& field) const;
	// solverの初期化(引数にはsolverのチーム)
	void give_solver_initialize(bool team, Field& field);
	// solverに職人の行動を渡す(引数にはsolverのチーム)
	void give_solver(bool team, Field& field);
	// solverから行動を受け取る(引数にはsolverのチーム)
	void receive_solver(bool team, Field& field);
	// 職人の配列
	Array<Array<Craftsman>> craftsmen;
	// solverプログラム
	 ChildProcess child{ U"solver.exe", Pipe::StdInOut };
	// 現在のターン、ポイント数のフォント
	Font normal_font{ 50, U"SourceHanSansJP-Medium.otf" };
	// 建築物の数のフォント
	Font small_font{ 25, U"SourceHanSansJP-Medium.otf" };
	// ターン数
	size_t turn_num = 200;
	// 現在のターン
	bool now_turn = TEAM::RED;
	// 操作モード
	char operation_mode = ACT::MOVE;
	// 職人を選択しているか
	bool is_targeting = false;
};

void Game::operate_gui(Field& field) {
	// 操作モードの変更
	if (SimpleGUI::Button(U"移動", { 700, 100 }) or Key1.down()) {
		operation_mode = ACT::MOVE;
	}if (SimpleGUI::Button(U"建設", { 700, 200 }) or Key2.down()) {
		operation_mode = ACT::BUILD;
	}if (SimpleGUI::Button(U"破壊", { 700, 300 }) or Key3.down()) {
		operation_mode = ACT::DESTROY;
	}
	// ターン終了
	if (SimpleGUI::Button(U"ターン終了", { 700, 500 }) or KeyE.down()) {
		field.calc_area();
		field.calc_point(TEAM::RED);
		field.calc_point(TEAM::BLUE);
		for (Array<Craftsman>& ary : craftsmen) {
			for (Craftsman& craftsman : ary) {
				craftsman.initialize();
			}
		}
		now_turn ^= true;
		is_targeting = false;
	}
}
void Game::operate_craftsman(bool team, Field& field) {
	for (Craftsman& craftsman : craftsmen[team]) {
		// 行動済みならcontinue
		if (craftsman.is_acted) {
			continue;
		}
		// 動かす対象の職人を決める
		if ((not is_targeting or craftsman.is_target) and get_grid_rect(craftsman.pos).leftClicked()) {
			craftsman.is_target ^= true;
			is_targeting ^= true;
		}
		// 動かす対象でなければcontinue
		if (not craftsman.is_target) {
			continue;
		}
		// クリックされた移動方向
		Optional<Point> direction;
		switch (operation_mode) {
			// 移動モード
		case ACT::MOVE:
			direction = get_clicked_pos(craftsman.pos, range_move);
			if (direction and craftsman.move(field, *direction)) {
				is_targeting = false;
			}
			break;
			// 建築モード
		case ACT::BUILD:
			direction = get_clicked_pos(craftsman.pos, range_wall);
			if (direction and craftsman.build(field, *direction)) {
				is_targeting = false;
			}
			break;
			// 破壊モード
		case ACT::DESTROY:
			direction = get_clicked_pos(craftsman.pos, range_wall);
			if (direction and craftsman.destroy(field, *direction)) {
				is_targeting = false;
			}
			break;
		}
	}
}
void Game::display_field(void) const {
	for (const Array<Craftsman>& ary : craftsmen) {
		for(const Craftsman &craftsman : ary){
			if (craftsman.is_acted) {
				get_grid_circle(craftsman.pos).draw(Palette::Gray);
			}else if (craftsman.is_target) {
				get_grid_rect(craftsman.pos).drawFrame(2, 2, Palette::Yellow);
			}
		}
	}
}
void Game::display_details(Field &field) const {
	if (now_turn == TEAM::RED) {
		normal_font(U"赤チームの手番").draw(100, 600, Palette::Red);
	}else {
		normal_font(U"青チームの手番").draw(100, 600, Palette::Blue);
	}
	Array<size_t> building_red = field.get_building(TEAM::RED);
	Array<size_t> building_blue = field.get_building(TEAM::BLUE);
	normal_font(U"赤ポイント:{}"_fmt(field.get_point(TEAM::RED))).draw(800, 100, Palette::Black);
	small_font(U"城壁:{}  陣地:{}  城:{}"_fmt(building_red[0], building_red[1], building_red[2])).draw(850, 175, Palette::Black);
	normal_font(U"青ポイント:{}"_fmt(field.get_point(TEAM::BLUE))).draw(800, 250, Palette::Black);
	small_font(U"城壁:{}  陣地:{}  城:{}"_fmt(building_blue[0], building_blue[1], building_blue[2])).draw(850, 325, Palette::Black);
}
void Game::give_solver_initialize(bool team, Field& field) {
	// フィールドの縦横
	child.ostream() << HEIGHT << std::endl << WIDTH << std::endl;
	// プログラム側を赤色とする
	child.ostream() << ((team == TEAM::RED) ? 0 : 1) << std::endl;
	// ターン数
	child.ostream() << turn_num << std::endl;
	// 池の数と座標
	child.ostream() << field.get_ponds().size() << std::endl;
	for (Point& p : field.get_ponds()) {
		child.ostream() << p.y << std::endl << p.x << std::endl;
	}
	// 城の数と座標
	child.ostream() << field.get_castles().size() << std::endl;
	for (Point& p : field.get_castles()) {
		child.ostream() << p.y << std::endl << p.x << std::endl;
	}
	// solverのチームの職人
	child.ostream() << field.get_craftsmen(team).size() << std::endl;
	for (Point& p : field.get_craftsmen(team)) {
		child.ostream() << p.y << std::endl << p.x << std::endl;
	}
	// solverでないチーム職人
	child.ostream() << field.get_craftsmen(not team).size() << std::endl;
	for (Point& p : field.get_craftsmen(not team)) {
		child.ostream() << p.y << std::endl << p.x << std::endl;
	}
}
void Game::give_solver(bool team, Field& field) {
	for (Craftsman& craftsman : craftsmen[not team]) {
		craftsman.output_act(child);
	}
}
void Game::receive_solver(bool team, Field& field) {
	for (Craftsman& craftsman : craftsmen[team]) {
		craftsman.input_act(child, field);
	}
}


// 人対人
class PvP : public App::Scene, public Game {
public:
	PvP(const InitData& init);
	void update() override;
	void draw() const override;
};

// コンストラクタで職人配列をセット
PvP::PvP(const InitData& init) : IScene{ init } {
	craftsmen.resize(2);
	for (size_t h = 0; h < HEIGHT; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			if (getData().get_cell(h, w) & CELL::CRAFTSMAN_RED) {
				craftsmen[TEAM::RED].push_back(Craftsman(h, w, TEAM::RED));
			}else if (getData().get_cell(h, w) & CELL::CRAFTSMAN_BLUE) {
				craftsmen[TEAM::BLUE].push_back(Craftsman(h, w, TEAM::BLUE));
			}
		}
	}
}
void PvP::update() {
	operate_gui(getData());
	operate_craftsman(now_turn, getData());
}
void PvP::draw() const {
	getData().display_grid();
	getData().display_actors();
	display_field();
	display_details(getData());
}



// 人対solver
class PvC : public App::Scene, public Game{
public:
	PvC(const InitData& init);
	void operate_gui(Field& field);
	void update() override;
	void draw() const override;
private:
	bool team_solver = TEAM::BLUE;
};
PvC::PvC(const InitData& init) : IScene{ init } {
	craftsmen.resize(2);
	for (size_t h = 0; h < HEIGHT; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			if (getData().get_cell(h, w) & CELL::CRAFTSMAN_RED) {
				craftsmen[TEAM::RED].push_back(Craftsman(h, w, TEAM::RED));
			}else if (getData().get_cell(h, w) & CELL::CRAFTSMAN_BLUE) {
				craftsmen[TEAM::BLUE].push_back(Craftsman(h, w, TEAM::BLUE));
			}
		}
	}
	give_solver_initialize(team_solver, getData());
}
void PvC::operate_gui(Field& field) {
	// 操作モードの変更
	if (SimpleGUI::Button(U"移動", { 700, 100 }) or Key1.down()) {
		operation_mode = ACT::MOVE;
	}if (SimpleGUI::Button(U"建設", { 700, 200 }) or Key2.down()) {
		operation_mode = ACT::BUILD;
	}if (SimpleGUI::Button(U"破壊", { 700, 300 }) or Key3.down()) {
		operation_mode = ACT::DESTROY;
	}
	// ターン終了
	if (SimpleGUI::Button(U"ターン終了", { 700, 500 }) or KeyE.down()) {
		field.calc_area();
		field.calc_point(TEAM::RED);
		field.calc_point(TEAM::BLUE);
		give_solver(team_solver, field);
		System::Sleep(1.5s);
		now_turn ^= true;
		receive_solver(team_solver, field);
		for (Array<Craftsman>& ary : craftsmen) {
			for (Craftsman& craftsman : ary) {
				craftsman.initialize();
			}
		}
		now_turn ^= true;
		is_targeting = false;
	}
}
void PvC::update() {
	operate_gui(getData());
	operate_craftsman(now_turn, getData());
}
void PvC::draw() const {
	getData().display_grid();
	getData().display_actors();
	display_field();
	display_details(getData());
}

void Main(){
	// ウィンドウ設定
	Scene::SetBackground(Palette::Lightgreen);
	Window::Resize(1280, 720);

	App manager;
	manager.add<PvP>(U"PvP");
	manager.add<PvC>(U"PvC");


	manager.init(U"PvC");
	while (System::Update()){
		if (not manager.update()) {
			break;
		}
	}

}
