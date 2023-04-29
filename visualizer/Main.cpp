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

	Array<Craftsman> craftsmen;
	craftsmen << Craftsman(field, 0, 0);
	craftsmen << Craftsman(field, 24, 24);
	craftsmen << Craftsman(field, 6, 7);
	craftsmen << Craftsman(field, 7, 6);
	craftsmen << Craftsman(field, 10, 15);
	craftsmen << Craftsman(field, 2, 20);

	while (System::Update()) {

		if (SimpleGUI::Button(U"移動", { 900, 100 })) {
			MODE = OPERATION_MODE::MOVE;
		}else if (SimpleGUI::Button(U"建設", { 900, 200 })) {
			MODE = OPERATION_MODE::BUILD;
		}else if (SimpleGUI::Button(U"破壊", { 900, 300 })) {
			MODE = OPERATION_MODE::BREAK;
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
				for (int i = 0; i < HEIGHT; i++) {
					for (int j = 0; j < WIDTH; j++) {
						if (Rect{ j * CELL_SIZE + 100, i * CELL_SIZE + 100, CELL_SIZE }.leftPressed()) {

						}
					}
				}
				break;
			case OPERATION_MODE::BUILD:
				for (int i = 0; i < HEIGHT; i++) {
					for (int j = 0; j < WIDTH; j++) {
						if (Rect{ j * CELL_SIZE + 100, i * CELL_SIZE + 100, CELL_SIZE }.leftPressed()) {
							field.grid[i][j] |= CELL::WALL_ALLY;
						}
					}
				}
				break;
			case OPERATION_MODE::BREAK:
				for (int i = 0; i < HEIGHT; i++) {
					for (int j = 0; j < WIDTH; j++) {
						if (Rect{ j * CELL_SIZE + 100, i * CELL_SIZE + 100, CELL_SIZE }.leftPressed()) {
							field.grid[i][j] &= ~CELL::WALL_ALLY;
						}
					}
				}
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
