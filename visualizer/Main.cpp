# include <Siv3D.hpp> // OpenSiv3D v0.6.9
# include "Field.hpp"
# include "Actor.hpp"

void Main(){

	Scene::SetBackground(Palette::Lightsteelblue);
	Window::Resize(1280, 720);

	Field tmp(25, 25);
	Actor actor(25, 25, 4, 3, Texture{ U"👷"_emoji });

	Craftsman craftman(25, 25, 2, 3, Texture{ U"👷"_emoji });

	while (System::Update()){
		tmp.DisplayGrid();
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
