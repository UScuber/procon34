﻿# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Field.hpp"
# include "Actor.hpp"

enum CELL{
	POND = 1 << 0,
	WALL_ENEM = 1 << 1,
	WALL_ALLY = 1 << 2,
	AREA_ENEM = 1 << 3,
	AREA_ALLY = 1 << 4,
	ENEM = 1 << 5,
	ALLY = 1 << 6,
	CASTLE = 1 << 7
};

enum TEAM {
	RED = false,
	BLUE = true
};

enum OPERATION_MODE {
	MOVE = 0,
	BUILD = 1,
	BREAK = 2
};



// フィールドの縦横
size_t HEIGHT;
size_t WIDTH;
// 一つのセルの大きさ(正方形)
size_t CELL_SIZE;
// フィールドの左上に開ける空白
size_t BLANK_LEFT;
size_t BLANK_TOP;

// 職人の移動範囲
const Array<std::pair<int,int>> dydx_craftsman = { {0,1},{-1,1},{-1,0},{-1,-1},{0,-1},{1,-1},{1,0},{1,1} };
// 城壁の建築範囲
const Array<std::pair<int, int>> dydx_wall = { {0,1},{0,-1},{1,0},{-1,0} };

void Main() {
	Scene::SetBackground(Palette::Lightsteelblue);
	Window::Resize(1280, 720);

	HEIGHT = 25;
	WIDTH = 25;
	CELL_SIZE = 20;
	BLANK_LEFT = 100;
	BLANK_TOP = 100;

	Field field;

	size_t NumCraftsman = 6;
	size_t MODE = 0;


	size_t NumTurn = 0;
	size_t BlueArea = 0;
	size_t RedArea = 0;

	// trueなら味方(青)チーム, falseなら敵(赤)チーム
	bool TURN = TEAM::BLUE;

	Array<Craftsman> craftsmen_blue;
	Array<Craftsman> craftsmen_red;
	craftsmen_blue << Craftsman(field, 0, 0, TEAM::BLUE);
	craftsmen_blue << Craftsman(field, 24, 24, TEAM::BLUE);
	craftsmen_blue << Craftsman(field, 6, 7, TEAM::BLUE);
	craftsmen_blue << Craftsman(field, 7, 6, TEAM::BLUE);
	craftsmen_blue << Craftsman(field, 10, 15, TEAM::BLUE);
	craftsmen_blue << Craftsman(field, 2, 20, TEAM::BLUE);

	craftsmen_red << Craftsman(field, 3, 6, TEAM::RED);
	craftsmen_red << Craftsman(field, 3, 12, TEAM::RED);
	craftsmen_red << Craftsman(field, 20, 21, TEAM::RED);
	craftsmen_red << Craftsman(field, 15, 6, TEAM::RED);
	craftsmen_red << Craftsman(field, 11, 17, TEAM::RED);
	craftsmen_red << Craftsman(field, 22, 9, TEAM::RED);

	// 職人を選択中かどうか
	bool isTargeting = false;

	const Font font{75, U"SourceHanSansJP-Medium.otf"};

	Array<Craftsman > pre_craftsmen = craftsmen_blue;
	Field pre_field = field;

	while (System::Update()) {

		//Console << Scene::DeltaTime();

		if (TURN == TEAM::BLUE) {
			font(U"青チームの手番").draw(100, 600, Palette::Blue);
		}else{ 
			font(U"赤チームの手番").draw(100, 600, Palette::Red);
		}

		font(U"ターン数:{}"_fmt(NumTurn)).draw(800, 50, Palette::Black);
		font(U"青エリア:{}"_fmt(BlueArea)).draw(800, 150, Palette::Black);
		font(U"赤エリア:{}"_fmt(RedArea)).draw(800, 250, Palette::Black);


		// 上書きする場合もあるため始めに描画する
		field.DisplayGrid();
		field.DrawActors();

		if (SimpleGUI::Button(U"移動", { 700, 100 })) {
			MODE = OPERATION_MODE::MOVE;
		}else if (SimpleGUI::Button(U"建設", { 700, 200 })) {
			MODE = OPERATION_MODE::BUILD;
		}else if (SimpleGUI::Button(U"破壊", { 700, 300 })) {
			MODE = OPERATION_MODE::BREAK;
		}else if (SimpleGUI::Button(U"手番開始時に戻す", { 700,400 })) {
			(TURN == TEAM::BLUE ? craftsmen_blue : craftsmen_red) = pre_craftsmen;
			field = pre_field;
		}
		if (SimpleGUI::Button(U"ターン終了", { 700, 500 })) {
			for (Craftsman& craftsman : (TURN == TEAM::BLUE ? craftsmen_blue : craftsmen_red)) {
				craftsman.Initialize();
			}
			BlueArea = field.SearchArea(TEAM::BLUE);
			RedArea = field.SearchArea(TEAM::RED);
			TURN ^= true;
			NumTurn++;
			pre_craftsmen = (TURN == TEAM::BLUE ? craftsmen_blue : craftsmen_red);
			pre_field = field;
		}

		if (SimpleGUI::Button(U"Debug", { 700, 600 })) {
			for (auto& ary : field.grid) {
				Console << ary;
			}
		}


		// 職人の行動
		for (Craftsman& craftsman : (TURN == TEAM::BLUE ? craftsmen_blue : craftsmen_red)) {
			// 敵の職人はスキップ
			if (TURN != craftsman.team) {
				continue;
			}
			// 行動済みの職人は灰色で上塗り
			if (craftsman.isActed) {
				GetGridCircle(craftsman.y_coordinate, craftsman.x_coordinate).draw(Palette::Grey);
				continue;
			}
			// 動かす対象の職人を決める
			if ((not isTargeting or craftsman.isTarget) and GetGridRect(craftsman.y_coordinate, craftsman.x_coordinate).leftClicked()) {
				craftsman.isTarget ^= true;
				isTargeting ^= true;
			}
			// 対象の職人は黄色の枠で囲う
			if (craftsman.isTarget) {
				GetGridRect(craftsman.y_coordinate, craftsman.x_coordinate).drawFrame(2, 2, Palette::Yellow);
			}else {
				continue;
			}
			switch (MODE) {
			// 移動モード
			case OPERATION_MODE::MOVE:
					// 職人の周囲8マスがクリックされたらそこに移動
					for (auto& d : dydx_craftsman) {
						std::pair<int, int> next = { craftsman.y_coordinate + d.first, craftsman.x_coordinate + d.second };
						if (next.first < 0 or HEIGHT <= next.first or next.second < 0 or WIDTH <= next.second) {
							continue;
						}
						if (GetGridRect(next.first, next.second).leftClicked() and craftsman.Move(field, d.first, d.second)) {
							isTargeting = false;
						}
					}
				break;

			// 城壁の建設
			case OPERATION_MODE::BUILD:
				for (auto& d : dydx_wall) {
					std::pair<int, int> next = { craftsman.y_coordinate + d.first, craftsman.x_coordinate + d.second };
					if (next.first < 0 or HEIGHT <= next.first or next.second < 0 or WIDTH <= next.second) {
						continue;
					}
					if (GetGridRect(next.first, next.second).leftClicked() and craftsman.Build(field, next.first, next.second)) {
						isTargeting = false;
					}
				}
				break;


			// 城壁の破壊
			case OPERATION_MODE::BREAK:
				for (auto& d : dydx_wall) {
					std::pair<int, int> next = { craftsman.y_coordinate + d.first, craftsman.x_coordinate + d.second };
					if (next.first < 0 or HEIGHT <= next.first or next.second < 0 or WIDTH <= next.second) {
						continue;
					}
					if (GetGridRect(next.first, next.second).leftClicked() and craftsman.Break(field, next.first, next.second)) {
						isTargeting = false;
					}
				}
				break;
			}
		}
	}
}

//
// - Debug ビルド: プログラムの最適化を減らす代わりに、エラーやクラッシュ時に詳細な情報を得られます。
//
// - Release ビルド: 最大限の最適化でビルドします。
//
// - [デバッグ] メニュー → [デバッグの開始] でプログラムを実行すると、[出力] ウィンドウに詳細なログが表示され、エラーの原因を探せます。
//
// - Visual Studio を更新した直後は、プログラムのリビルド（[ビルド]メニュー → [ソリューションのリビルド]）が必要な場合があります。
//
