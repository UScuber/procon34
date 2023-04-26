# pragma once
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

enum OPERATION_MODE {
	MOVE = 0,
	BUILD = 1,
	BREAK = 2
};

// フィールドの縦横
size_t HEIGHT;
size_t WIDTH;
size_t CELL_SIZE;

void Main(){
	Scene::SetBackground(Palette::Lightsteelblue);
	Window::Resize(1280, 720);

	HEIGHT = 25;
	WIDTH = 25;
	CELL_SIZE = 20;

	Field field;

	size_t NumCraftsman = 6;
	size_t NUmTurn = 200;
	size_t MODE = 0;

	// trueなら青チーム, falseなら赤チーム
	bool TURN = true;

	
	Craftsman craftsman(field, 3, 5);


	field.grid[5][7] |= WALL_ALLY;
	field.grid[5][8] |= WALL_ALLY;
	field.grid[5][9] |= WALL_ALLY;
	field.grid[5][10] |= WALL_ALLY;
	field.grid[6][6] |= WALL_ALLY;
	field.grid[6][11] |= WALL_ALLY;
	field.grid[7][5] |= WALL_ALLY;
	field.grid[7][8] |= WALL_ALLY;
	field.grid[7][10] |= WALL_ALLY;
	field.grid[8][4] |= WALL_ALLY;
	field.grid[8][11] |= WALL_ALLY;
	field.grid[9][5] |= WALL_ALLY;
	field.grid[9][6] |= WALL_ALLY;
	field.grid[9][7] |= WALL_ALLY;
	field.grid[9][8] |= WALL_ALLY;
	field.grid[9][9] |= WALL_ALLY;
	field.grid[9][10] |= WALL_ALLY;
	

	field.grid[12][4] |= WALL_ALLY;
	field.grid[12][5] |= WALL_ALLY;

	while (System::Update()) {
		if (SimpleGUI::Button(U"移動", { 900, 100 })) {
			MODE = 0;
		}else if (SimpleGUI::Button(U"建設", { 900, 200 })) {
			MODE = 1;
		}else if (SimpleGUI::Button(U"破壊", { 900, 300 })) {
			MODE = 2;
		}
		if (SimpleGUI::Button(U"ターン終了", { 900, 500 })) {
			TURN ^= true;
		}

		if (SimpleGUI::Button(U"Debug", { 900, 600 })) {
			for (auto& ary : field.grid) {
				Console << ary;
			}
			field.SearchArea();
		}

		switch (MODE){
			case OPERATION_MODE::MOVE:

				break;
			case OPERATION_MODE::BUILD:
				break;
			case OPERATION_MODE::BREAK:
				break;
		}
		field.DisplayGrid();
		field.DrawActors();
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
