# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.10
# include "Game.hpp"

class Start : public App::Scene {
public:
	Start(const InitData &init) : IScene(init), url_base(), token(), match_id(){
		sw.restart();
		read_file();
	}

	void read_file(){
		// 接続先のURLをローカルファイルから取得
		TextReader reader_url{ U"./url.env" };
		if(not reader_url){
			url_base = U"";
		}else{
			reader_url.readLine(url_base);
		}
		// solver.exeのチームトークンをローカルファイルから取得
		TextReader reader_token{ U"./token.env" };
		if(not reader_token){
			token = U"";
		}else{
			reader_token.readLine(token);
		}
		// 行う試合のidをローカルファイルから取得
		TextReader reader_id{ U"./id.env" };
		if(not reader_id){
			match_id = -1;
		}else{
			String tmp_id;
			reader_id.readLine(tmp_id);
			match_id = Parse<int>(tmp_id);
		}
	}

	void update() override {
		if(sw >= 0.5s){
			sw.restart();
			read_file();
		}
		if(SimpleGUI::Button(U"Start", { 100, 100 })){
			 //const URL url = U"localhost:5000/start";
			 //const HashTable<String, String> headers{ { U"Content-Type", U"application/json" } };
			 //SimpleHTTP::Get(url, headers, U"./tmp");
			changeScene(U"ReceiveBuildPlanScene", 0s);
		}
	}

	void draw() const override {
		font(U"URL: {}"_fmt(url_base.empty() ? U"None" : url_base)).draw(800, 50, url_base.empty() ? Palette::Red : Palette::Black);
		font(U"Token: {} chars"_fmt(token.empty() ? U"None" : ToString(token.size()))).draw(800, 125, token.empty() ? Palette::Red : Palette::Black);
		font(U"ID: {}"_fmt(match_id < 0 ? U"None" : ToString(match_id))).draw(800, 200, match_id < 0 ? Palette::Red : Palette::Black);
	}

private:
	String url_base;
	String token;
	int match_id;
	Stopwatch sw;
	const Font font = Font(30, U"SourceHanSansJP-Medium.otf");
};
