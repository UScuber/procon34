# pragma once
# include <Siv3D.hpp>

//　↖　↑　↗　→　↘　↓　↙　←　に変更
const Array<int> direction_server = { 4, 0, 7, 3, 6, 2, 5, 1 };
// 　↑　←　↓　→　↖　↙　↘　↗　に変更
const Array<int> direction_client = { 1, 7, 5, 3, 0, 6, 4, 2 };

// クライアントのdirectionの番号からサーバーの番号に変更
int to_direction_server(int direction_num_client) {
	return direction_server[direction_num_client];
}
// サーバーのdirectionの番号からクライアントの番号に変更
int to_direction_client(int direction_num_server) {
	return direction_client[direction_num_server];
}


class Connect {
private:
	const URL url_base = U"http://localhost:3000/";
public:
	// 試合一覧取得
	JSON get_matches_list(void);
};

JSON Connect::get_matches_list(void) {
	const URL url = url_base + U"matches";
	const HashTable<String, String> headers{ {U"Content-Type", U"application/json"} };
	const FilePath saveFilePath = U"mathes_list.json";
	if (const auto response = SimpleHTTP::Get(url, headers, saveFilePath)) {
		Console << U"------";
		Console << response.getStatusLine().rtrimmed();
		Console << U"status code: " << FromEnum(response.getStatusCode());
		Console << U"------";
		Console << response.getHeader().rtrimmed();
		Console << U"------";
		if (response.isOK()){
			Print << TextReader{ saveFilePath }.readAll();
		}
	}
	else {
		Print << U"Failed.";
	}
	return JSON();
}



