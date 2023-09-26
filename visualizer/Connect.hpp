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

Array<Array<int>> get_2d_array(const JSON& json) {
	Array<Array<int>> res;
	for (const auto& object : json.arrayView()){
		Array<int> ary;
		for (const auto& num : object.arrayView()) {
			ary << num.get<int>();
		}
		res << ary;
	}
	return res;
}


struct MatchDataBoard {
	int width = 0;
	int height = 0;
	int mason = 0;
	Array<Array<int>> structures = Array<Array<int>>(0);
	Array<Array<int>> masons = Array<Array<int>>(0);
	MatchDataBoard() {}
	MatchDataBoard(const JSON& board) {
		this->width = board[U"width"].get<int>();
		this->height = board[U"height"].get<int>();
		this->mason = board[U"mason"].get<int>();
		this->structures = get_2d_array(board[U"structures"]);
		this->masons = get_2d_array(board[U"masons"]);
	}
};
struct MatchDataMatch {
	int id = 0;
	int turns = 0;
	int turnSeconds = 0;
	MatchDataBoard board = MatchDataBoard();
	String opponent = U"";
	bool first = false;
	MatchDataMatch() {}
	MatchDataMatch(const JSON& matches) {
		this->id = matches[U"id"].get<int>();
		this->turns = matches[U"turns"].get<int>();
		this->turnSeconds = matches[U"turnSeconds"].get<int>();
		this->board = MatchDataBoard(matches[U"board"]);
		this->opponent = matches[U"opponent"].get<String>();
		this->first = matches[U"first"].get<bool>();
	}
};
struct MatchData {
	Array<MatchDataMatch> matches = Array<MatchDataMatch>(0);
	MatchData() {}
	MatchData(const JSON& json) {
		for (const MatchDataMatch matchdatamatch : json[U"matches"].arrayView()) {
			matches << matchdatamatch;
		}
	}
};

struct MatchStatusBoard {
	int width = 0;
	int height = 0;
	int mason = 0;
	Array<Array<int>> walls = Array<Array<int>>(0);
	Array<Array<int>> territories = Array<Array<int>>(0);
	Array<Array<int>> structures = Array<Array<int>>(0);
	Array<Array<int>> masons = Array<Array<int>>(0);
	MatchStatusBoard() {}
	MatchStatusBoard(const JSON& board) {
		this->width = board[U"width"].get<int>();
		this->height = board[U"height"].get<int>();
		this->mason = board[U"mason"].get<int>();
		this->walls = get_2d_array(board[U"walls"]);
		this->territories = get_2d_array(board[U"territories"]);
		this->structures = get_2d_array(board[U"structures"]);
		this->masons = get_2d_array(board[U"masons"]);
	}
};
struct MatchStatusLogAction {
	bool succeeded = false;
	int type = 0;
	int dir = 0;
	MatchStatusLogAction() {}
	MatchStatusLogAction(const JSON& log_action) {
		this->succeeded = log_action[U"succeeded"].get<bool>();
		this->type = log_action[U"type"].get<int>();
		this->dir = log_action[U"dir"].get<int>();
	}
};
struct MatchStatusLog {
	int turn = 0;
	MatchStatusLogAction actions = MatchStatusLogAction();
	MatchStatusLog() {}
	MatchStatusLog(const JSON& log) {
		this->turn = log.get<int>();
		this->actions = MatchStatusLogAction(log[U"actions"]);
	}
};
class MatchStatus {
	int id = 0;
	int turn = 0;
	MatchStatusBoard board = MatchStatusBoard();
	Array<MatchStatusLog> logs = Array<MatchStatusLog>();
	MatchStatus() {}
	MatchStatus(const JSON& json) {
		this->id = json[U"id"].get<int>();
		this->turn = json[U"turn"].get<int>();
		this->board = MatchStatusBoard(json[U"board"]);
		for (const MatchStatusLog matchstatuslog : json[U"logs"].arrayView()) {
			logs << matchstatuslog;
		}
	}
};

struct ActionPlanAction {
	int type = 0;
	int dir = 0;
	ActionPlanAction() {}
	ActionPlanAction(const JSON& action) {
		this->type = action[U"type"].get<int>();
		this->dir = action[ U"dir"].get<int>();
	}
};
struct  ActionPlan {
	int turn = 0;
	Array<ActionPlanAction> actions = Array<ActionPlanAction>(0);
	ActionPlan() {}
	ActionPlan(const JSON& json) {
		this->turn = json[U"turn"].get<int>();
		for (const ActionPlanAction actionplanaction : json[U"actions"].arrayView()) {
			actions << actionplanaction;
		}
	}
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
void output_console_fail(const String& str) {
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






