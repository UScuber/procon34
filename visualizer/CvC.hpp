# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.10
# include "Game.hpp"
# include "ProgressBar.hpp"

// solver.exe対サーバー
class CvC : public App::Scene, public Game {
public:
	CvC(const InitData &init);
	void update() override;
	void draw() const override;

private:
	// 通信を行うクラス
	Connect connect;
	// 試合のid
	int match_id = 0;
	// solver.exeが先手か
	bool is_first = false;
	// ストップウォッチ
	Stopwatch stopwatch;
	double time_limit_rate = 0.7;
	// 試合を開始する
	void execute_match(void);
	// 職人の行動をActionPlanに変換
	ActionPlan team2actionplan(const TEAM team);
	//  MatchStatusから職人情報を上塗り
	void set_craftsman(Array<Craftsman> &tmp_craftsmen, const int turn, const MatchStatus &matchstatus);
	// server.exe, serverのターンの処理
	bool turn_solver(void);
	bool turn_server(void);
	// 詳細情報の描画
	void display_details(const Field &field) const;
};

CvC::CvC(const InitData &init) : IScene(init){
	// サーバーから試合情報一覧を取得
	Optional<MatchDataMatch> tmp_matchdatamatch = connect.get_matches_list();
	if(tmp_matchdatamatch == none){
		throw Error{ U"Failed to get matches list" };
	}
	const MatchDataMatch matchdatamatch = tmp_matchdatamatch.value();
	craftsmen.resize(2, Array<Craftsman>(matchdatamatch.get_mason_num()));
	// フィールド情報をセット
	//getData().initialize(matchdatamatch);
	// 基本情報をセット
	is_build_plan.clear();
	is_build_plan.resize(HEIGHT, Array<bool>(WIDTH, false));
	is_first = matchdatamatch.get_first();
	if(is_first){
		now_turn = TEAM::RED;
	}else{
		now_turn = TEAM::BLUE;
	}
	time = matchdatamatch.get_turnSeconds() * 1000;
	turn_num = matchdatamatch.get_turn();

	for(Array<Craftsman> &craftsmen_ary : craftsmen){
		craftsmen_ary.resize(matchdatamatch.get_mason_num(), Craftsman());
	}
	for (int h = 0; h < HEIGHT; h++) {
		for (int w = 0; w < WIDTH; w++) {
			const int mason_num = matchdatamatch.get_masons()[h][w];
			if (mason_num == 0) {
				continue;
			}
			const TEAM team = (mason_num > 0) ? TEAM::RED : TEAM::BLUE;
			craftsmen[team][Abs(mason_num) - 1] = Craftsman(h, w, team);
		}
	}
	stopwatch.reset();
	// solver.exeの初期化
	give_solver_initialize(is_first, getData());
}


ActionPlan CvC::team2actionplan(const TEAM team){
	ActionPlan tmp_actionplan(turn_num_now + 1);
	for(const Craftsman &craftsman : craftsmen[team]){
		tmp_actionplan.push_back_action((int)craftsman.act, to_direction_server(craftsman.direction) + 1);
	}
	return tmp_actionplan;
}

void CvC::set_craftsman(Array<Craftsman> &tmp_craftsmen, const int turn, const MatchStatus &matchstatus){
	const MatchStatusLog &log = matchstatus.get_log(turn);
	int i = -1;
	for(Craftsman &craftsman : tmp_craftsmen){
		i++;
		craftsman.act = (ACT)log.get_action(i).type;
		if(craftsman.act == ACT::NOTHING){
			continue;
		}
		craftsman.direction = to_direction_client(log.get_action(i).dir - 1);
	}

	for(int h = 0; h < HEIGHT; h++){
		for(int w = 0; w < WIDTH; w++){
			const int craftsman_num = matchstatus.get_masons()[h][w];
			if(craftsman_num == 0){
				continue;
			}
			const TEAM team = (craftsman_num > 0) ? TEAM::RED : TEAM::BLUE;
			craftsmen[team][Abs(craftsman_num) - 1].pos = Point(w, h);
		}
	}
}

void CvC::execute_match(void){
	if(turn_num_now < turn_num){
		if(now_turn == TEAM::RED){
			if(turn_solver()){
				getData().calc_area();
				getData().calc_point(TEAM::RED);
				getData().calc_point(TEAM::BLUE);
			}
		}else{
			if(turn_server()){
				getData().calc_point(TEAM::RED);
				getData().calc_point(TEAM::BLUE);
			}
		}
	}
}

bool CvC::turn_solver(void){
	if(not stopwatch.isStarted()){
		// solver.exeに建築予定の壁を渡す
		getData().give_solver_build_plan(child);
		stopwatch.start();
		return false;
	}else{
		if(stopwatch.ms() < time * time_limit_rate){
			Console << U"this turn is solver's";	
			return false;
		}else{
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

bool CvC::turn_server(void){
	Optional<MatchStatus> tmp_matchstatus;
	if(not stopwatch.isStarted()){
		stopwatch.start();
		return false;
	}else{
		if(stopwatch.ms() <= 100){
			return false;
		}else{
			stopwatch.restart();
			// 次のターンが来るまで待機
			tmp_matchstatus = connect.get_match_status();
			if(tmp_matchstatus == none){
				Console << U"Cannot get match status! \t get again now ...";
				return false;
			}else if(tmp_matchstatus.value().get_turn() != turn_num_now + 1){
				Console << U"this turn is server's";
				return false;
			}
		}
	}
	stopwatch.reset();
	const MatchStatus matchstatus = tmp_matchstatus.value();
	turn_num_now++;
	now_turn = TEAM::RED;
	// フィールド更新
	getData().update(matchstatus);
	// 職人情報更新
	set_craftsman(craftsmen[TEAM::BLUE], turn_num_now, matchstatus);
	// solver.exeに行動情報を渡す
	give_solver(TEAM::RED);
	return true;
}

void CvC::display_details(const Field &field) const {
	if (now_turn == TEAM::RED) {
		ProgressBar(Point(0, 0), 40, 1280).draw(this->time * time_limit_rate - stopwatch.ms(), this->time * time_limit_rate);
		small_font(U"{} / {}"_fmt(this->time * time_limit_rate - stopwatch.ms(), this->time * time_limit_rate)).draw(Arg::center(1280 / 2, 40 / 2), Palette::Black);
	}
	const Array<int> building_red = field.get_building(TEAM::RED);
	const Array<int> building_blue = field.get_building(TEAM::BLUE);
	normal_font(U"赤ポイント:{}"_fmt(field.get_point(TEAM::RED))).draw(800, 50, ((now_turn == TEAM::RED) ? Palette::Red : Palette::Black));
	small_font(U"城壁:{}  陣地:{}  城:{}"_fmt(building_red[0], building_red[1], building_red[2])).draw(850, 125, ((now_turn == TEAM::RED) ? Palette::Red : Palette::Black));
	normal_font(U"青ポイント:{}"_fmt(field.get_point(TEAM::BLUE))).draw(800, 200, ((now_turn == TEAM::BLUE) ? Palette::Blue : Palette::Black));
	small_font(U"城壁:{}  陣地:{}  城:{}"_fmt(building_blue[0], building_blue[1], building_blue[2])).draw(850, 275, ((now_turn == TEAM::BLUE) ? Palette::Blue : Palette::Black));
	const int point_diff = field.get_point(TEAM::RED) - field.get_point(TEAM::BLUE);
	normal_font(U"点差:{}"_fmt(point_diff)).draw(800, 350, (point_diff >= 0) ? ((point_diff == 0) ? Palette::Black : Palette::Red) : Palette::Blue);
	normal_font(U"ターン数:{}/{}"_fmt(turn_num_now + 1, turn_num)).draw(800, 450, Palette::Black);
	normal_font(U"持ち時間:{}ms"_fmt(time)).draw(800, 550, Palette::Black);
}

void CvC::update(){
	execute_match();
	getData().receive_build_plan();
}

void CvC::draw() const {
	getData().display_actors();
	display_field();
	getData().display_grid();
	getData().display_build_plan();
	display_details(getData());
}
