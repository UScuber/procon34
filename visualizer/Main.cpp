# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.10
# include "Base.hpp"
# include "Field.hpp";
# include "Actor.hpp"

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
	void operate_gui(Field& field);
	void operate_craftsman(Array<Craftsman>& craftsmen, Field& field);
	void display_field(void) const ;
	void display_details(Field& field) const;
	// 職人の配列
	Array<Craftsman> craftsmen_red;
	Array<Craftsman> craftsmen_blue;
	// solverプログラム
	// ChildProcess child{ U"solver.exe", Pipe::StdInOut };
	// 現在のターン、ポイント数のフォント
	Font normal_font{ 50, U"SourceHanSansJP-Medium.otf" };
	// 建築物の数のフォント
	Font small_font{ 25, U"SourceHanSansJP-Medium.otf" };
	// 現在のターン
	bool now_turn = TEAM::RED;
	// 操作モード
	char operation_mode = ACT::MOVE;
	// 職人を選択しているか
	bool is_targeting = false;
};

// GUIによる操作
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
		for (Craftsman& craftsman : craftsmen_red) {
			craftsman.initialize();
		}
		for (Craftsman& craftsman : craftsmen_blue) {
			craftsman.initialize();
		}
		now_turn ^= true;
		is_targeting = false;
	}
}

// マウスクリックによる操作
void Game::operate_craftsman(Array<Craftsman>& craftsmen, Field& field) {
	for (Craftsman& craftsman : craftsmen) {
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

// フィールドの表示
void Game::display_field(void) const {
	for (const Craftsman& craftsman : craftsmen_red) {
		if (craftsman.is_acted) {
			get_grid_circle(craftsman.pos).draw(Palette::Gray);
		}else if (craftsman.is_target) {
			get_grid_rect(craftsman.pos).drawFrame(2, 2, Palette::Yellow);
		}
	}
	for (const Craftsman& craftsman : craftsmen_blue) {
		if (craftsman.is_acted) {
			get_grid_circle(craftsman.pos).draw(Palette::Gray);
		}else if (craftsman.is_target) {
			get_grid_rect(craftsman.pos).drawFrame(2, 2, Palette::Yellow);
		}
	}
}

// 詳細表示
void Game::display_details(Field &field) const {
	if (now_turn == TEAM::RED) {
		normal_font(U"赤チームの手番").draw(100, 600, Palette::Red);
	}
	else {
		normal_font(U"青チームの手番").draw(100, 600, Palette::Blue);
	}
	Array<size_t> building_red = field.get_building(TEAM::RED);
	Array<size_t> building_blue = field.get_building(TEAM::BLUE);
	normal_font(U"赤ポイント:{}"_fmt(field.get_point(TEAM::RED))).draw(800, 100, Palette::Black);
	small_font(U"城壁:{}  陣地:{}  城:{}"_fmt(building_red[0], building_red[1], building_red[2])).draw(850, 175, Palette::Black);
	normal_font(U"青ポイント:{}"_fmt(field.get_point(TEAM::BLUE))).draw(800, 250, Palette::Black);
	small_font(U"城壁:{}  陣地:{}  城:{}"_fmt(building_blue[0], building_blue[1], building_blue[2])).draw(850, 325, Palette::Black);
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
	for (size_t h = 0; h < HEIGHT; h++) {
		for (size_t w = 0; w < WIDTH; w++) {
			if (getData().get_cell(h, w) & CELL::CRAFTSMAN_RED) {
				craftsmen_red.push_back(Craftsman(h, w, TEAM::RED));
			}
			else if (getData().get_cell(h, w) & CELL::CRAFTSMAN_BLUE) {
				craftsmen_blue.push_back(Craftsman(h, w, TEAM::BLUE));
			}
		}
	}
}

void PvP::update() {
	operate_gui(getData());
	if (now_turn == TEAM::RED) {
		operate_craftsman(craftsmen_red, getData());
	}
	else if (now_turn == TEAM::BLUE) {
		operate_craftsman(craftsmen_blue, getData());
	}
}

void PvP::draw() const {
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

	while (System::Update()){
		if (not manager.update()) {
			break;
		}
	}

}
