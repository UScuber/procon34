#pragma once
# include <Siv3D.hpp>

class MenuRect{
private:
	// 四角形の左上のyx座標
	Point coordinate;
	// 四角形の大きさ
	Size size;
	// 四角形のフォント
	ColorF colorf;
	// 表示する文字
	String label;
	// 四角形
	Rect rect;
	// フォント
	Font& font;
public:
	MenuRect(Point coordinate, Size size, ColorF colorf, String label, Font &font);
	// クリックされたか
	bool isClicked(void);
	// 描画
	void DrawMenuRect(void);
};

// ウィンドウ上部のメニュータブ
class MenuTab {
private:
	// メニュータブの個数
	size_t number =  1;
	// 幅はウィンドウサイズの幅で固定
	size_t height;
	// フォント
	Font& font;
	// 四角形配列
	Array<MenuRect> menurects;
public:
	MenuTab(size_t number, size_t height, Font &font);
	void DrawMenuTab(void);
	size_t whatClicked(void);
};
