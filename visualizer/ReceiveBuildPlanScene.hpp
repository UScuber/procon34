# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.10
# include "Game.hpp"
# include "Connect.hpp"

class ReceiveBuildPlanScene : public App::Scene, public Game {
private:
	Connect connect;
	Optional<MatchDataMatch> tmp_matchdatamatch = none;
	Optional<MatchStatus> tmp_matchstatus = none;
	Stopwatch stopwatch;
	bool can_draw = false;

public:
	ReceiveBuildPlanScene(const InitData &init) : IScene(init){}
	void update() override {
		// 試合情報が手に入るまで100msおきにリクエスト
		if(tmp_matchdatamatch == none){
			tmp_matchdatamatch = connect.get_matches_list();
			if(tmp_matchdatamatch == none){
				Console << U"Failed to get match data";
				System::Sleep(100ms);
				return;
			}else{
				// リクエストが受理されたら盤面を初期化
				getData().initialize(tmp_matchdatamatch.value());
				is_build_plan.resize(HEIGHT, Array<bool>(WIDTH, false));
				can_draw = true;
			}
		}
		// 建築予定場所を受け取る
		getData().receive_build_plan();
		// 100msおきにフィールド情報をリクエスト
		if(not stopwatch.isStarted()){
			stopwatch.start();
		}else if(stopwatch.ms() < 100){
			return;
		}
		// フィールド情報を受け取ったら
		tmp_matchstatus = connect.get_match_status();
		stopwatch.restart();
		if(tmp_matchstatus == none){
			Console << U"Failed to get match status";
			return;
		}else{
			changeScene(U"CvC", 0s);
		}
	}
	void draw() const {
		if(can_draw){
			getData().display_actors();
			getData().display_grid();
			getData().display_build_plan();
			display_details(getData());
		}
	}

};
