# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.10
# include "Game.hpp"


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
	// 試合を開始する
	void execute_match(void);
	// 職人の行動をActionPlanに変換
	ActionPlan team2actionplan(const TEAM team);
	//  MatchStatusから職人情報を上塗り
	void set_craftsman(Array<Craftsman> &tmp_craftsmen, const int turn, const MatchStatus &matchstatus);
	// server.exe, serverのターンの処理
	bool turn_solver(void);
	bool turn_server(void);
	// 盤面描画
	void display_field(void) const;
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
	getData().initialize(matchdatamatch);
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
	for(int h = 0; h < HEIGHT; h++){
		for(int w = 0; w < WIDTH; w++){
			const int mason_num = matchdatamatch.get_masons()[h][w];
			if(mason_num == 0){
				continue;
			}
			const TEAM team = (mason_num > 0) ? TEAM::RED : TEAM::BLUE;
			craftsmen[team][Abs(mason_num) - 1] = Craftsman(h, w, team);
		}
	}
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
		//Console << U"------------------------------------------------------------------------------------";
		//for (auto& ary : matchstatus.board.masons) {
		//	Console << ary;
		//}
		//Console << U"------------------------------------------------------------------------------------";
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
		stopwatch.start();
		return false;
	}else{
		if(stopwatch.ms() < time * 0.9){
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
	// 次のターンが来るまで待機
	Optional<MatchStatus> tmp_matchstatus;
	tmp_matchstatus = connect.get_match_status();
	if(tmp_matchstatus == none){
		Console << U"Cannot get match status! \t get again now ...";
		return false;
	}else if(tmp_matchstatus.value().get_turn() != turn_num_now + 1){
		Console << U"this turn is server's";
		return false;
	}
	const MatchStatus matchstatus = tmp_matchstatus.value();
	turn_num_now++;
	now_turn = TEAM::RED;
	// フィールド更新
	getData().update(matchstatus);
	// 職人情報更新
	set_craftsman(craftsmen[TEAM::BLUE], turn_num_now, matchstatus);
	// solver.exeに行動情報を渡す
	give_solver(TEAM::RED);
	// solver.exeに建築予定の壁を渡す
	give_solver_build_plan();
	return true;
}

void CvC::display_field(void) const {
	for(const Array<Craftsman> &ary : craftsmen){
		int craftsman_num = 1;
		for(const Craftsman &craftsman : ary){
			craftsman_font(craftsman_num++).drawAt(get_cell_center(craftsman.pos), Palette::Black);
		}
	}
	int craftsman_num = 0;
	for(const Craftsman &craftsman : craftsmen[TEAM::RED]){
		craftsman_num++;
		if(craftsman.act != ACT::NOTHING) continue;
		craftsman_font(craftsman_num).drawAt(get_cell_center(craftsman.pos), Palette::Gray);
	}
	for(int h = 0; h < HEIGHT; h++){
		for(int w = 0; w < WIDTH; w++){
			if(is_build_plan[h][w]){
				get_grid_rect(h, w).drawFrame(3, 0, Palette::Orange);
			}
		}
	}
}

void CvC::update(){
	execute_match();
	receive_build_plan(getData());
}

void CvC::draw() const {
	getData().display_actors();
	display_field();
	getData().display_grid();
	display_details(getData());
}
