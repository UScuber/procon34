﻿# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.10
# include "Base.hpp"
# include "Field.hpp"
# include "Actor.hpp"
# include "Connect.hpp"

// +--------------------+
//  |  solver.exe : 赤  |
//  |  server         : 青  |
// +--------------------+

// フィールドの縦横
int HEIGHT;
int WIDTH;
// 一つのセルの大きさ(正方形)
int CELL_SIZE = 25;
// フィールドの左上に開ける余白
int BLANK_LEFT = 50;
int BLANK_TOP = 50;


using App = SceneManager<String, Field>;

Array<Input> keyboard_craftsman = {Key0, Key1, Key2, Key3, Key4, Key5, Key6, Key7, Key8 };

class Game {
protected:
	// GUIによる操作
	void operate_gui(Field& field);
	// マウスクリックによる操作
	void operate_craftsman(TEAM team, Field& field);
	// フィールドの表示
	void display_field(void) const ;
	// 詳細表示
	void display_details(Field& field) const;
	// solverの初期化(引数にはsolverのチーム)
	void give_solver_initialize(bool is_first, Field& field);
	// solverに職人の行動を渡す(引数にはsolverのチーム)
	void give_solver(TEAM team);
	// solverから行動を受け取る(引数にはsolverのチーム)
	void receive_solver(TEAM team, Field& field);
	// GUIで建築予定の場所を受け取る
	void receive_build_plan(Field &field);
	void give_solver_build_plan(void);
	// 職人の配列
	Array<Array<Craftsman>> craftsmen;
	// solverプログラム
	 ChildProcess child{ U"solver.exe", Pipe::StdInOut };
	// 現在のターン、ポイント数のフォント
	const Font normal_font = Font{ 50, U"SourceHanSansJP-Medium.otf" };
	// 建築物の数のフォント
	const Font small_font = Font{ 25, U"SourceHanSansJP-Medium.otf" };
	// 職人の番号のフォント
	const Font craftsman_font = Font( (int32)CELL_SIZE,  U"SourceHanSansJP-Medium.otf");
	// 試合のターン数
	int turn_num = 200;
	// 現在のターン数
	int turn_num_now = 0;
	// 現在のターン
	TEAM now_turn = TEAM::RED;
	// 持ち時間(ms)
	int time = 3000;
	// 建築予定
	Array<Array<bool>> is_build_plan;
};

void Game::operate_gui(Field& field) {
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
		now_turn = (not now_turn);
	}
}
void Game::operate_craftsman(TEAM team, Field& field) {
	int craftsman_num = 0;
	for (Craftsman& craftsman : craftsmen[(int)team]) {
		craftsman_num++;
		// 行動済みならcontinue
		if (craftsman.is_acted) {
			continue;
		}
		// 動かす対象の職人を決める
		if (keyboard_craftsman[craftsman_num].down()) {
			if (not craftsman.is_target) {
				for (Craftsman& craftsman_tmp : craftsmen[(int)team]) {
					craftsman_tmp.is_target = false;
				}
			}
			craftsman.is_target ^= true;
		}

		// 動かす対象でなければcontinue
		if (not craftsman.is_target) {
			continue;
		}

		// 押された矢印キーに対する操作方向
		Optional<Point> direction = get_pressed_pos();
		// 押されたz,x,cに対する操作モード
		Optional<ACT> mode = get_pressed_mode();

		if (direction != none) {
			Line{ get_cell_center(craftsman.pos), get_cell_center(craftsman.pos + direction.value()) }.draw(LineStyle::RoundCap, 5, Palette::Orange);
		}

		if(direction != none and mode != none){
			bool acted = mode.has_value();

			switch (mode.value()) {
				// 移動モード
			case ACT::MOVE:
				acted &= craftsman.move(field, *direction);
				break;
				// 建築モード
			case ACT::BUILD:
				acted &= craftsman.build(field, *direction);
				break;
				// 破壊モード
			case ACT::DESTROY:
				acted &= craftsman.destroy(field, *direction);
				break;
			}
			if(acted){
				craftsman.is_target = false;
				for (int i = 1; i < (int)craftsmen[(int)team].size(); i++) {
					if (!craftsmen[(int)team][(craftsman_num + i - 1) % (int)craftsmen[(int)team].size()].is_acted) {
						craftsmen[(int)team][(craftsman_num + i - 1) % (int)craftsmen[(int)team].size()].is_target = true;
						break;
					}
				}
			}
			break;
		}
	}
}
void Game::display_field(void) const {
	for (const Array<Craftsman>& ary : craftsmen) {
		int craftsman_num = 0;
		for(const Craftsman &craftsman : ary){
			if (craftsman.is_acted) {
				get_grid_circle(craftsman.pos).draw(Palette::Darkgray);
			}else if (craftsman.is_target) {
				get_grid_rect(craftsman.pos).drawFrame(2, 2, Palette::Yellow);
			}
			craftsman_font(craftsman_num++).drawAt(get_cell_center(craftsman.pos));
		}
	}
}
void Game::display_details(Field &field) const {
	if (now_turn == TEAM::RED) {
		normal_font(U"赤チームの手番").draw(850, 600, Palette::Red);
	}else {
		normal_font(U"青チームの手番").draw(850, 600, Palette::Blue);
	}
	const Array<int> building_red = field.get_building(TEAM::RED);
	const Array<int> building_blue = field.get_building(TEAM::BLUE);
	normal_font(U"赤ポイント:{}"_fmt(field.get_point(TEAM::RED))).draw(800, 100, ((now_turn == TEAM::RED) ? Palette::Red : Palette::Black));
	small_font(U"城壁:{}  陣地:{}  城:{}"_fmt(building_red[0], building_red[1], building_red[2])).draw(850, 175, ((now_turn == TEAM::RED) ? Palette::Red : Palette::Black));
	normal_font(U"青ポイント:{}"_fmt(field.get_point(TEAM::BLUE))).draw(800, 250, ((now_turn == TEAM::BLUE) ? Palette::Blue: Palette::Black));
	small_font(U"城壁:{}  陣地:{}  城:{}"_fmt(building_blue[0], building_blue[1], building_blue[2])).draw(850, 325, ((now_turn == TEAM::BLUE) ? Palette::Blue: Palette::Black));
	normal_font(U"ターン数:{}/{}"_fmt(turn_num_now, turn_num)).draw(850, 400, Palette::Black);
}
void Game::give_solver_initialize(bool is_first, Field& field) {
	// フィールドの縦横
	child.ostream() << HEIGHT << std::endl << WIDTH << std::endl;
	// プログラム側を赤色とする
	child.ostream() << ((is_first) ? 0 : 1) << std::endl;
	// ターン数
	child.ostream() << turn_num << std::endl;
	// 持ち時間
	child.ostream() << time << std::endl;
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
	// REDのチームの職人
	child.ostream() << field.get_craftsmen(TEAM::RED).size() << std::endl;
	for (Point& p : field.get_craftsmen(TEAM::RED)) {
		child.ostream() << p.y << std::endl << p.x << std::endl;
	}
	// BLUEチームの職人
	child.ostream() << field.get_craftsmen(TEAM::BLUE).size() << std::endl;
	for (Point& p : field.get_craftsmen(TEAM::BLUE)) {
		child.ostream() << p.y << std::endl << p.x << std::endl;
	}
}
void Game::give_solver(TEAM team) {
	for (Craftsman& craftsman : craftsmen[(int)(not team)]) {
		craftsman.output_act(child);
	}
}
void Game::receive_solver(TEAM team, Field& field) {
	for (Craftsman& craftsman : craftsmen[(int)team]) {
		craftsman.input_act(child, field);
	}
}
void Game::receive_build_plan(Field& field) {
	for (int h = 0; h < HEIGHT; h++) {
		for (int w = 0; w < WIDTH; w++) {
			if (get_grid_rect({ w, h }).rightClicked()) {
				if (field.get_cell(h, w) & CELL::CASTLE) {
					continue;
				}
				bool is_around_wall = true;
				for (auto& dydx : range_wall) {
					if (not is_in_field(h + dydx.y, w + dydx.x)) {
						continue;
					}
					if (not (field.get_cell(h + dydx.y, w + dydx.x) & CELL::POND)) {
						is_around_wall = false;
						break;
					}
				}
				if (is_around_wall) {
					continue;
				}
				is_build_plan[h][w] ^= true;
			}
		}
	}
}
void Game::give_solver_build_plan(void) {
	int n = 0;
	for (auto& ary : is_build_plan) {
		for (auto& cell : ary) {
			if (cell) {
				n++;
			}
		}
	}
	//Console << n << '\n';
	//for (int h = 0; h < HEIGHT; h++) {
	//	for (int w = 0; w < WIDTH; w++) {
	//		if (is_build_plan[h][w]) {
	//			Console << h << ' ' << w << '\n';
	//		}
	//	}
	//}
	child.ostream() << n << std::endl;
	for (int h = 0; h < HEIGHT; h++) {
		for (int w = 0; w < WIDTH; w++) {
			if (is_build_plan[h][w]) {
				child.ostream() << h << std::endl;
				child.ostream() << w << std::endl;
			}
		}
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
	for (int h = 0; h < HEIGHT; h++) {
		for (int w = 0; w < WIDTH; w++) {
			if (getData().get_cell(h, w) & CELL::CRAFTSMAN_RED) {
				craftsmen[(int)TEAM::RED].push_back(Craftsman(h, w, TEAM::RED));
			}else if (getData().get_cell(h, w) & CELL::CRAFTSMAN_BLUE) {
				craftsmen[(int)TEAM::BLUE].push_back(Craftsman(h, w, TEAM::BLUE));
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
	display_details(getData());

}



// 人対solver
class PvC : public App::Scene, public Game{
public:
	PvC(const InitData& init);
	void operate_gui(Field& field);
	void display_field(void) const;
	void update() override;
	void draw() const override;
private:
	TEAM team_solver = TEAM::RED;
};
PvC::PvC(const InitData& init) : IScene{ init } {
	craftsmen.resize(2);
	is_build_plan.resize(HEIGHT, Array<bool>(WIDTH, false));
	for (int h = 0; h < HEIGHT; h++) {
		for (int w = 0; w < WIDTH; w++) {
			if (getData().get_cell(h, w) & CELL::CRAFTSMAN_RED) {
				craftsmen[(int)TEAM::RED].push_back(Craftsman(h, w, TEAM::RED));
			}else if (getData().get_cell(h, w) & CELL::CRAFTSMAN_BLUE) {
				craftsmen[(int)TEAM::BLUE].push_back(Craftsman(h, w, TEAM::BLUE));
			}
		}
	}
	give_solver_initialize(team_solver == TEAM::RED, getData());

	// Computerが先手の場合
	if (team_solver == TEAM::RED) {
		System::Sleep(1.5s);
		now_turn = (not now_turn);
		receive_solver(team_solver, getData());
		for (Array<Craftsman>& ary : craftsmen) {
			for (Craftsman& craftsman : ary) {
				craftsman.initialize();
			}
		}
		getData().calc_area();
		getData().calc_point(TEAM::RED);
		getData().calc_point(TEAM::BLUE);
	}
}

void PvC::operate_gui(Field& field) {
	// ターン終了
	if (SimpleGUI::Button(U"ターン終了", { 700, 500 }) or KeyE.down()) {
		field.calc_area();
		field.calc_point(TEAM::RED);
		field.calc_point(TEAM::BLUE);
		give_solver(team_solver);
		//give_solver_build_plan();
		System::Sleep(1.5s);
		now_turn = not now_turn;
		receive_solver(team_solver, field);
		for (Array<Craftsman>& ary : craftsmen) {
			for (Craftsman& craftsman : ary) {
				craftsman.initialize();
			}
		}
		now_turn = not now_turn;
		field.calc_area();
		field.calc_point(TEAM::RED);
		field.calc_point(TEAM::BLUE);
	}
}
void PvC::display_field(void) const {
	for (const Array<Craftsman>& ary : craftsmen) {
		int craftsman_num = 1;
		for (const Craftsman& craftsman : ary) {
			if (craftsman.is_acted) {
				get_grid_circle(craftsman.pos).draw(Palette::Darkgray);
			}else if (craftsman.is_target) {
				get_grid_rect(craftsman.pos).drawFrame(2, 2, Palette::Yellow);
			}
			craftsman_font(craftsman_num++).drawAt(get_cell_center(craftsman.pos), Palette::Black);
		}
	}
	for (int h = 0; h < HEIGHT; h++) {
		for (int w = 0; w < WIDTH; w++) {
			if (is_build_plan[h][w]) {
				get_grid_rect({ h,w }).drawFrame(1, 1, Palette::Darkviolet);
			}
		}
	}
}
void PvC::update() {
	operate_gui(getData());
	operate_craftsman(now_turn, getData());
	receive_build_plan(getData());
}
void PvC::draw() const {
	getData().display_grid();
	getData().display_actors();
	display_field();
	display_details(getData());
}


// solver.exe対サーバー
class CvC : public App::Scene, public Game {
public:
	CvC(const InitData& init);
	void update() override;
	void draw() const override;
private:
	// 通信を行うクラス
	Connect connect;
	// 通信を行う際に使用する構造体
	MatchDataMatch matchdatamatch;
	MatchStatus matchstatus;
	ActionPlan actionplan;
	// 試合のid
	int match_id = 0;
	// solver.exeが先手か
	bool is_first = false;
	// ストップウォッチ
	Stopwatch stopwatch;
	// 試合を開始する
	 void execute_match(void);
	// 職人の行動をActionPlanに変換
	ActionPlan team2actionplan(TEAM team);
	//  MatchStatusから職人情報を上塗り
	void set_craftsman(Array<Craftsman>& tmp_craftsmen, int turn);
	// server.exe, serverのターンの処理
	bool turn_solver(void);
	bool turn_server(void);
	// 盤面描画
	void display_field(void) const;
};
CvC::CvC(const InitData& init) : IScene{ init } {
	// サーバーから試合情報一覧を取得
	Optional<MatchDataMatch> tmp_matchdatamatch = connect.get_matches_list();
	if (tmp_matchdatamatch == none) {
		throw Error{ U"Failed to get matches list" };
	}else {
		this->matchdatamatch = tmp_matchdatamatch.value();
	}
	craftsmen.resize(2, Array<Craftsman>(matchdatamatch.board.mason));
	// フィールド情報をセット
	getData().initialize(matchdatamatch);
	// 基本情報をセット
	is_build_plan.clear();
	is_build_plan.resize(HEIGHT, Array<bool>(WIDTH, false));
	is_first = matchdatamatch.first;
	if (is_first) {
		now_turn = TEAM::RED;
	}else {
		now_turn = TEAM::BLUE;
	}
	time = matchdatamatch.turnSeconds * 1000;
	time = 2000;
	for (Array<Craftsman>& craftsmen_ary : craftsmen) {
		craftsmen_ary.resize(matchdatamatch.board.mason, Craftsman());
	}
	for (int h = 0; h < HEIGHT; h++) {
		for (int w = 0; w < WIDTH; w++) {
			const int mason_num = matchdatamatch.board.masons[h][w];
			if (mason_num == 0) {
				continue;
			}
			TEAM team = (mason_num > 0) ? TEAM::RED : TEAM::BLUE;
			craftsmen[(int)team][Abs(mason_num) - 1] = Craftsman(h, w, team);
		}
	}
	// solver.exeの初期化
	give_solver_initialize(is_first, getData());
}
ActionPlan CvC::team2actionplan(TEAM team) {
	ActionPlan tmp_actionplan(turn_num_now + 1);
	for (const Craftsman& craftsman : craftsmen[(int)team]) {
		tmp_actionplan.push_back_action((int)craftsman.act, to_direction_server(craftsman.direction) + 1);
	}
	return tmp_actionplan;
}
void CvC::set_craftsman(Array<Craftsman>& tmp_craftsmen, int turn) {
	for (const MatchStatusLog& log : matchstatus.logs) {
		if (log.turn == turn) {
			int i = 0;
			for (Craftsman& craftsman : tmp_craftsmen) {
				craftsman.act = (ACT)log.actions[i].type;
				if (craftsman.act == ACT::NOTHING) {
					continue;
				}
				craftsman.direction = to_direction_client(log.actions[i].dir - 1);
				if (log.actions[i].succeeded and craftsman.act == ACT::MOVE) {
					craftsman.pos += range_move[craftsman.direction];
				}
				i++;
			}
		}
	}
}
void CvC::execute_match(void) {
	Console << turn_num_now << U" " << (int)now_turn;
	if (turn_num_now <= turn_num) {
		if (now_turn == TEAM::RED) {
			turn_solver();
		}else {
			turn_server();
		}
	}
}
bool CvC::turn_solver(void){
	if (not stopwatch.isStarted()) {
		stopwatch.start();
		return false;
	}else {
		if (stopwatch.ms() < time * 0.9) {
			return false;
		}else {
			stopwatch.reset();
		}
	}
	// solver.exeから行動情報を受け取る
	receive_solver(TEAM::RED, getData());
	connect.post_action_plan(team2actionplan(TEAM::RED));
	turn_num_now++;
	now_turn = TEAM::BLUE;
	return true;
}
bool CvC::turn_server(void) {
	// 次のターンが来るまで待機
	Optional<MatchStatus> tmp_matchstatus;
	tmp_matchstatus = connect.get_match_status();
	if (tmp_matchstatus == none) {
		Console << U"Cannot get match status! \t get again now ...";
		return false;
	}else if (tmp_matchstatus.value().turn != turn_num_now + 1) {
		Console << U"this turn is server's";
		return false;
	}
	matchstatus = tmp_matchstatus.value();
	turn_num_now++;
	now_turn = TEAM::RED;
	// フィールド更新
	getData().update(matchstatus);
	// 職人情報更新
	set_craftsman(craftsmen[(int)TEAM::BLUE], turn_num_now);
	// solver.exeに行動情報を渡す
	give_solver(TEAM::RED);
	// solver.exeに建築予定の壁を渡す
	give_solver_build_plan();
	return true;
}
void CvC::display_field(void) const {
	for (const Array<Craftsman>& ary : craftsmen) {
		int craftsman_num = 1;
		for (const Craftsman& craftsman : ary) {
			craftsman_font(craftsman_num++).drawAt(get_cell_center(craftsman.pos), Palette::Black);
		}
	}
	for (int h = 0; h < HEIGHT; h++) {
		for (int w = 0; w < WIDTH; w++) {
			if (is_build_plan[h][w]) {
				get_grid_rect({ w,h }).drawFrame(1, 1, Palette::Darkviolet);
			}
		}
	}
}

void CvC::update(){
	execute_match();
	receive_build_plan(getData());
}
void CvC::draw() const {
	getData().display_grid();
	getData().display_actors();
	display_field();
	display_details(getData());
}

class Start : public App::Scene {
public:
	Start(const InitData& init) : IScene{ init } {};
	void update() override {
		if (SimpleGUI::Button(U"Start", { 100, 100 })) {
			const URL url = U"localhost:5000/start";
			const HashTable<String, String> headers{ {U"Content-Type", U"application/json"} };
			SimpleHTTP::Get(url, headers, U"./tmp");
			changeScene(U"CvC", 0s);
		}
	}
};


void Main(){
	// ウィンドウ設定
	Scene::SetBackground(Palette::Lightgreen);
	Window::Resize(1280, 720);

	App manager;
	manager.add<PvP>(U"PvP");
	manager.add<PvC>(U"PvC");
	manager.add<CvC>(U"CvC");
	manager.add<Start>(U"Start");

	manager.init(U"Start");
	
	while (System::Update()){
		if (not manager.update()) {
			break;
		}
	}

}
