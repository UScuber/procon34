# pragma once
# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Field.hpp"
# include "Actor.hpp"

enum CELL {
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

enum GAME_MODE {
	PlayerVSPlayer = 0,
	PlayerVSComputer = 1,
	ComputerVSComputer = 2
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
const Array<std::pair<int, int>> dydx_craftsman = { {-1,0},{0, -1},{1,0},{0,1},{-1,-1},{1,-1},{1,1},{-1,1} };
// 城壁の建築範囲
const Array<std::pair<int, int>> dydx_wall = { {0,1},{0,-1},{1,0},{-1,0} };


void Main() {
	Scene::SetBackground(Palette::Lightsteelblue);
	Window::Resize(1280, 720);

	int GAME_MODE = 0;
	while (System::Update()) {
		if (SimpleGUI::Button(U"PlayerVSPlayer", {100, 100})) {
			GAME_MODE = GAME_MODE::PlayerVSPlayer;
			break;
		}
		if (SimpleGUI::Button(U"PlayerVSComputer", { 100, 200 })) {
			GAME_MODE = GAME_MODE::PlayerVSComputer;
			break;
		}
		if (SimpleGUI::Button(U"ComputerVSComputer", { 100, 300 })) {
			GAME_MODE = GAME_MODE::ComputerVSComputer;
			break;
		}
	}

	HEIGHT = Random(11, 25);
	WIDTH = Random(11, 25);
	CELL_SIZE = 20;
	BLANK_LEFT = 100;
	BLANK_TOP = 100;

	Field field;

	size_t MODE = 0;


	size_t CountTurn = 0;
	size_t BlueArea = 0;
	size_t RedArea = 0;

	size_t NumTurn = Random(15, 100) * 2;
	size_t NumPond = Random(0, 20);
	size_t NumCastle = Random(1, 6);
	size_t NumCraftsman = Random(2, 6);

	// trueなら味方(青)チーム, falseなら敵(赤)チーム
	bool TURN = Random(0, 1);

	Array<Craftsman> craftsmen;
	Array<Craftsman> craftsmen_blue;
	Array<Craftsman> craftsmen_red;

	Array<std::pair<size_t, size_t>> ponds;
	Array<std::pair<size_t, size_t>> castles;

	field.Initialize(NumPond, NumCastle, NumCraftsman);

	for (size_t i = 0; i < HEIGHT; i++) {
		for (size_t j = 0; j < WIDTH; j++) {
			if (field.grid[i][j] & CELL::ALLY) {
				craftsmen_blue << Craftsman(field, i, j, TEAM::BLUE);
				craftsmen << Craftsman(field, i, j, TEAM::BLUE);
			}
			else if (field.grid[i][j] & CELL::ENEM) {
				craftsmen_red << Craftsman(field, i, j, TEAM::RED);
				craftsmen << Craftsman(field, i, j, TEAM::RED);
			}
			else if (field.grid[i][j] & CELL::POND) {
				ponds << std::make_pair(i, j);
			}
			else if (field.grid[i][j] & CELL::CASTLE) {
				castles << std::make_pair(i, j);
			}
		}
	}


	// 職人を選択中かどうか
	bool isTargeting = false;

	const Font font{ 50, U"SourceHanSansJP-Medium.otf" };

	while (System::Update()) {

		if (CountTurn >= NumTurn) {
			break;
		}

		if (TURN == TEAM::BLUE) {
			font(U"青チームの手番").draw(100, 600, Palette::Blue);
		}
		else {
			font(U"赤チームの手番").draw(100, 600, Palette::Red);
		}

		font(U"ターン数:{}/{}"_fmt(CountTurn, NumTurn)).draw(800, 50, Palette::Black);
		font(U"青エリア:{}"_fmt(BlueArea)).draw(800, 150, Palette::Black);
		font(U"赤エリア:{}"_fmt(RedArea)).draw(800, 250, Palette::Black);


		// 上書きする場合もあるため始めに描画する
		field.DisplayGrid();
		field.DrawActors();

		if (SimpleGUI::Button(U"移動", { 700, 100 })) {
			MODE = OPERATION_MODE::MOVE;
		}
		else if (SimpleGUI::Button(U"建設", { 700, 200 })) {
			MODE = OPERATION_MODE::BUILD;
		}
		else if (SimpleGUI::Button(U"破壊", { 700, 300 })) {
			MODE = OPERATION_MODE::BREAK;
		}

		if (SimpleGUI::Button(U"ターン終了", { 700, 500 })) {
			field.SearchArea();
			RedArea = field.CountArea(TEAM::RED);
			BlueArea = field.CountArea(TEAM::BLUE);
			for (Craftsman& craftsman : craftsmen) {
				craftsman.Initialize();
			}
			TURN ^= true;
			CountTurn++;
		}


		// 職人の行動
		for (Craftsman& craftsman : craftsmen) {
			// そのターンの職人でなければスルー
			if (craftsman.team != TURN) {
				continue;
			}
			// 行動済みの職人だったら灰色で上塗りしてcontinue
			if (craftsman.isActed) {
				GetGridCircle(craftsman.y_coordinate, craftsman.x_coordinate).draw(Palette::Gray);
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
			}
			else {
				continue;
			}
			switch (MODE) {
				// 移動モード
			case OPERATION_MODE::MOVE:
				// 職人の周囲8マスがクリックされたらそこに移動
				for (auto& d : dydx_craftsman) {
					std::pair<int, int> next = { craftsman.y_coordinate + d.first, craftsman.x_coordinate + d.second };
					if (not isInField(next.first, next.second)) {
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
					if (not isInField(next.first, next.second)) {
						continue;
					}
					if (GetGridRect(next.first, next.second).leftClicked() and craftsman.Build(field, d.first, d.second)) {
						isTargeting = false;
					}
				}
				break;


				// 城壁の破壊
			case OPERATION_MODE::BREAK:
				for (auto& d : dydx_wall) {
					std::pair<int, int> next = { craftsman.y_coordinate + d.first, craftsman.x_coordinate + d.second };
					if (not isInField(next.first, next.second)) {
						continue;
					}
					if (GetGridRect(next.first, next.second).leftClicked() and craftsman.Break(field, d.first, d.second)) {
						isTargeting = false;
					}
				}
				break;
			}
		}
	}

	while (System::Update()) {
		field.DisplayGrid();
		field.DrawActors();
		font(U"ターン数:{}/{}"_fmt(CountTurn, NumTurn)).draw(800, 50, Palette::Black);
		font(U"青エリア:{}"_fmt(BlueArea)).draw(800, 150, Palette::Black);
		font(U"赤エリア:{}"_fmt(RedArea)).draw(800, 250, Palette::Black);
	}

}


/*
void Main() {

	ChildProcess child{ U"solver.exe", Pipe::StdInOut };

	Scene::SetBackground(Palette::Lightsteelblue);
	Window::Resize(1280, 720);

	HEIGHT = Random(11, 25);
	WIDTH = Random(11, 25);
	CELL_SIZE = 20;
	BLANK_LEFT = 100;
	BLANK_TOP = 100;

	Field field;

	size_t MODE = 0;


	size_t CountTurn = 0;
	size_t BlueArea = 0;
	size_t RedArea = 0;

	size_t NumTurn = Random(15, 100) * 2;
	size_t NumPond = Random(0, 20);
	size_t NumCastle = Random(1, 6);
	size_t NumCraftsman = Random(2, 6);

	// trueなら味方(青)チーム, falseなら敵(赤)チーム
	bool TURN = Random(0, 1);

	Array<Craftsman> craftsmen_blue;
	Array<Craftsman> craftsmen_red;

	Array<std::pair<size_t, size_t>> ponds;
	Array<std::pair<size_t, size_t>> castles;

	field.Initialize(NumPond, NumCastle, NumCraftsman);

	for (size_t i = 0; i < HEIGHT; i++) {
		for (size_t j = 0; j < WIDTH; j++) {
			if (field.grid[i][j] & CELL::ALLY) {
				craftsmen_blue << Craftsman(field, i, j, TEAM::BLUE);
			}
			else if (field.grid[i][j] & CELL::ENEM) {
				craftsmen_red << Craftsman(field, i, j, TEAM::RED);
			}
			else if (field.grid[i][j] & CELL::POND) {
				ponds << std::make_pair(i, j);
			}
			else if (field.grid[i][j] & CELL::CASTLE) {
				castles << std::make_pair(i, j);
			}
		}
	}


	// solverに渡す
	child.ostream() << HEIGHT << std::endl << WIDTH << std::endl;
	child.ostream() << ((TURN == TEAM::RED) ? 0 : 1) << std::endl;
	child.ostream() << NumTurn << std::endl;
	child.ostream() << NumPond << std::endl;
	for (auto& p : ponds) {
		child.ostream() << p.first << std::endl << p.second << std::endl;
	}
	child.ostream() << NumCastle << std::endl;
	for (auto& p : castles) {
		child.ostream() << p.first << std::endl << p.second << std::endl;
	}
	child.ostream() << NumCraftsman << std::endl;
	for (auto& p : craftsmen_blue) {
		child.ostream() << p.y_coordinate << std::endl << p.x_coordinate << std::endl;
	}
	child.ostream() << NumCraftsman << std::endl;
	for (auto& p : craftsmen_red) {
		child.ostream() << p.y_coordinate << std::endl << p.x_coordinate << std::endl;
	}




	// 職人を選択中かどうか
	bool isTargeting = false;

	const Font font{ 50, U"SourceHanSansJP-Medium.otf" };

	Array<Craftsman > pre_craftsmen = craftsmen_blue;

	Field pre_field = field;

	while (System::Update()) {

		if (CountTurn >= NumTurn) {
			break;
		}

		if (TURN == TEAM::BLUE) {
			font(U"青チームの手番").draw(100, 600, Palette::Blue);
		}
		else {
			font(U"赤チームの手番").draw(100, 600, Palette::Red);
		}

		font(U"ターン数:{}/{}"_fmt(CountTurn, NumTurn)).draw(800, 50, Palette::Black);
		font(U"青エリア:{}"_fmt(BlueArea)).draw(800, 150, Palette::Black);
		font(U"赤エリア:{}"_fmt(RedArea)).draw(800, 250, Palette::Black);

		if (TURN == TEAM::RED) {
			// 手動の操作をプログラムに入力
			for (Craftsman& craftsman : craftsmen_blue) {
				if (craftsman.Act == U"") {
					child.ostream() << "0" << std::endl << "none" << std::endl;
				}
				else {
					child.ostream() << craftsman.Direction << std::endl << craftsman.Act << std::endl;
				}
				craftsman.Initialize();
			}
			// プログラムからの出力を受け取る
			for (Craftsman& craftsman : craftsmen_red) {
				child.istream() >> craftsman.Direction >> craftsman.Act;
				std::pair<int, int> dydx = dydx_craftsman[craftsman.Direction];
				if (craftsman.Act == U"move") {
					craftsman.Move(field, dydx.first, dydx.second);
				}
				else if (craftsman.Act == U"build") {
					craftsman.Build(field, dydx.first, dydx.second);
				}
				else if (craftsman.Act == U"break") {
					craftsman.Break(field, dydx.first, dydx.second);
				}
				craftsman.Initialize();
			}
			field.SearchArea(TEAM::BLUE);
			field.SearchArea(TEAM::RED);
			TURN = TEAM::BLUE;
			CountTurn++;
		}



		// 上書きする場合もあるため始めに描画する
		field.DisplayGrid();
		field.DrawActors();

		if (SimpleGUI::Button(U"移動", { 700, 100 })) {
			MODE = OPERATION_MODE::MOVE;
		}
		else if (SimpleGUI::Button(U"建設", { 700, 200 })) {
			MODE = OPERATION_MODE::BUILD;
		}
		else if (SimpleGUI::Button(U"破壊", { 700, 300 })) {
			MODE = OPERATION_MODE::BREAK;
		}
		else if (SimpleGUI::Button(U"手番開始時に戻す", { 700,400 })) {

		}
		if (SimpleGUI::Button(U"ターン終了", { 700, 500 })) {
			for (Craftsman& craftsman : craftsmen_blue) {
				craftsman.Initialize();
			}
			TURN = TEAM::RED;
			CountTurn++;
		}


		// 職人の行動
		for (Craftsman& craftsman : craftsmen_blue) {
			// 行動済みの職人だったら灰色で上塗りしてcontinue
			if (craftsman.isActed) {
				GetGridCircle(craftsman.y_coordinate, craftsman.x_coordinate).draw(Palette::Gray);
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
			}
			else {
				continue;
			}
			switch (MODE) {
				// 移動モード
			case OPERATION_MODE::MOVE:
				// 職人の周囲8マスがクリックされたらそこに移動
				for (auto& d : dydx_craftsman) {
					std::pair<int, int> next = { craftsman.y_coordinate + d.first, craftsman.x_coordinate + d.second };
					if (not isInField(next.first, next.second)) {
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
					if (not isInField(next.first, next.second)) {
						continue;
					}
					if (GetGridRect(next.first, next.second).leftClicked() and craftsman.Build(field, d.first, d.second)) {
						isTargeting = false;
					}
				}
				break;


				// 城壁の破壊
			case OPERATION_MODE::BREAK:
				for (auto& d : dydx_wall) {
					std::pair<int, int> next = { craftsman.y_coordinate + d.first, craftsman.x_coordinate + d.second };
					if (not isInField(next.first, next.second)) {
						continue;
					}
					if (GetGridRect(next.first, next.second).leftClicked() and craftsman.Break(field, d.first, d.second)) {
						isTargeting = false;
					}
				}
				break;
			}
		}
	}

	while (System::Update()) {
		field.DisplayGrid();
		field.DrawActors();
		font(U"ターン数:{}/{}"_fmt(CountTurn, NumTurn)).draw(800, 50, Palette::Black);
		font(U"青エリア:{}"_fmt(BlueArea)).draw(800, 150, Palette::Black);
		font(U"赤エリア:{}"_fmt(RedArea)).draw(800, 250, Palette::Black);
	}

}
*/
