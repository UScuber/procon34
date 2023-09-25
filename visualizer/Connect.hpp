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

struct MatchDataBonus {
	int wall = 0;
	int territory = 0;
	int castle = 0;
	MatchDataBonus(){}
	MatchDataBonus(const JSON& bonus) {
		this->wall = bonus[U"wall"].get<int>();
		this->territory = bonus[U"territory"].get<int>();
		this->castle = bonus[U"castle"].get<int>();
	}
};
struct MatchDataBoard {
	int width;
	int height;
	int mason;
	Array<Array<int>> structures;
	Array<Array<int>> masons;
	MatchDataBoard() {}
	MatchDataBoard(const JSON& board) {
		this->width = board[U"width"].get<int>();
		this->height = board[U"height"].get<int>();
		this->mason = board[U"mason"].get<int>();
		this->structures = board[U"structures"].get<Array<Array<int>>>();
		this->masons = board[U"masons"].get<Array<Array<int>>>();
	}
};
struct MatchDataMatch {
	int id;
	int turns;
	int turnSeconds;
	MatchDataBonus bonus;
	MatchDataBoard board;
	String opponent;
	bool first;
	MatchDataMatch() {}
	MatchDataMatch(const JSON& matches) {
		this->id = matches[U"id"].get<int>();
		this->turns = matches[U"turns"].get<int>();
		this->turnSeconds = matches[U"turnSeconds"].get<int>();
		this->bonus = MatchDataBonus(matches[U"bonus"]);
		this->board = MatchDataBoard(matches[U"board"]);
		this->opponent = matches[U"opponent"].get<String>();
		this->first = matches[U"first"].get<bool>();
	}
};
struct MatchData {
	Array<MatchDataMatch> matches;
	MatchData() {}
	MatchData(const JSON& json) {
		for (const MatchDataMatch matchdatamatch : json[U"matches"].get<Array<MatchDataMatch>>()) {
			matches << matchdatamatch;
		}
		Console << matches;
	}
};

class MatchStatusBoard {
	int width;
	int height;
	int mason;
	Array<Array<int>> walls;
	Array<Array<int>> territories;
	Array<Array<int>> structures;
	Array<Array<int>> masons;
};
class MatchStatusLogAction {
	bool succeeded;
	int type;
	int dir;
};
class MatchStatusLog {
	int turn;
	MatchStatusLogAction actions;
};
class MatchStatus {
	int id;
	int turn;
	MatchStatusBoard board;
	MatchStatusLog logs;
};

class ActionPlanAction {
	int type;
	int dir;
};
class ActionPlan {
	int turn;
	Array<ActionPlanAction> actions;
};


class Connect {
private:
	const URL url_base = U"http://localhost:3000/";
	String token;
public:
	Connect(void);
	// 試合一覧取得
	Optional<MatchData> get_matches_list(void);
};


void output_console_response(const HTTPResponse& response) {
	Console << U"------";
	Console << response.getStatusLine().rtrimmed();
	Console << U"status code: " << FromEnum(response.getStatusCode());
	Console << U"------";
	Console << response.getHeader().rtrimmed();
	Console << U"------";
}
void output_console_fail(String str) {
	Console << U"------";
	Console << str << U" failed!!";
	Console << U"------";
}

Connect::Connect(void) {
	TextReader reader{ U"./.token" };
	if (not reader) {
		throw Error{ U"Failed to open '.token'" };
	}
	reader.readLine(token);
}

Optional<MatchData> Connect::get_matches_list(void) {
	const URL url = url_base + U"matches?token=" + U"aaa";
	//const URL url = url_base + U"matches" + U"?token=" + token;
	const HashTable<String, String> headers{ {U"Content-Type", U"application/json"} };
	const FilePath saveFilePath = U"./mathes_list.json";
	if (const auto response = SimpleHTTP::Get(url, headers, saveFilePath)) {
		output_console_response(response);
		if (response.isOK()){
			MatchData(JSON::Load(saveFilePath));
		}
	}else {
		output_console_fail(U"get_matches_list");
	}
	return none;
}






