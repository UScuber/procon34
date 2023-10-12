# pragma once
# include <Siv3D.hpp>
# include "Base.hpp"


//　↖　↑　↗　→　↘　↓　↙　←　に変更
const Array<int> direction_client = { 4, 0, 7, 3, 6, 2, 5, 1 };
// 　↑　←　↓　→　↖　↙　↘　↗　に変更
const Array<int> direction_server = { 1, 7, 5, 3, 0, 6, 4, 2 };

// クライアントのdirectionの番号からサーバーの番号に変更
int to_direction_server(const int direction_num_client){
	return direction_server[direction_num_client];
}
// サーバーのdirectionの番号からクライアントの番号に変更
int to_direction_client(const int direction_num_server){
	return direction_client[direction_num_server];
}

template <class T>
Array<T> get_1d_array(const JSON &json){
	Array<T> res;
	for(const auto &num : json.arrayView()){
		res << num;
	}
	return res;
}

Array<Array<int>> get_2d_array(const JSON &json){
	Array<Array<int>> res;
	for(const auto &object : json.arrayView()){
		Array<int> ary;
		for(const auto &num : object.arrayView()){
			ary << num.get<int>();
		}
		res << ary;
	}
	return res;
}


struct MatchDataBoard {
private:
	int width;
	int height;
	int mason;
	Array<Array<int>> structures;
	Array<Array<int>> masons;

public:
	MatchDataBoard(const JSON &board) :
		width(board[U"width"].get<int>()),
		height(board[U"height"].get<int>()),
		mason(board[U"mason"].get<int>()),
		structures(get_2d_array(board[U"structures"])),
		masons(get_2d_array(board[U"masons"])){}

	int get_height() const {
		return height;
	}
	int get_width() const {
		return width;
	}
	int get_mason_num() const {
		return mason;
	}
	const Array<Array<int>> &get_structures() const {
		return structures;
	}
	const Array<Array<int>> &get_masons() const {
		return masons;
	}
};

struct MatchDataMatch : MatchDataBoard {
private:
	int id;
	int turns;
	int turnSeconds;
	String opponent;
	bool first;

public:
	MatchDataMatch(const JSON &matches) :
		id(matches[U"id"].get<int>()),
		turns(matches[U"turns"].get<int>()),
		turnSeconds(matches[U"turnSeconds"].get<int>()),
		MatchDataBoard(matches[U"board"]),
		opponent(matches[U"opponent"].get<String>()),
		first(matches[U"first"].get<bool>()){}

	int get_id() const {
		return id;
	}
	int get_turn() const {
		return turns;
	}
	int get_turnSeconds() const {
		return turnSeconds;
	}
	bool get_first() const {
		return first;
	}
};


struct MatchStatusBoard {
private:
	int width;
	int height;
	int mason;
	Array<Array<int>> walls;
	Array<Array<int>> territories;
	Array<Array<int>> structures;
	Array<Array<int>> masons;

public:
	MatchStatusBoard(const JSON &board) :
		width(board[U"width"].get<int>()),
		height(board[U"height"].get<int>()),
		mason(board[U"mason"].get<int>()),
		walls(get_2d_array(board[U"walls"])),
		territories(get_2d_array(board[U"territories"])),
		structures(get_2d_array(board[U"structures"])),
		masons(get_2d_array(board[U"masons"])){}

	const Array<Array<int>> &get_walls() const {
		return walls;
	}
	const Array<Array<int>> &get_territories() const {
		return territories;
	}
	const Array<Array<int>> &get_structures() const {
		return structures;
	}
	const Array<Array<int>> &get_masons() const {
		return masons;
	}
};

struct MatchStatusLogAction {
	bool succeeded;
	int type;
	int dir;

	MatchStatusLogAction(const JSON &log_action) :
		succeeded(log_action[U"succeeded"].get<bool>()),
		type(log_action[U"type"].get<int>()),
		dir(log_action[U"dir"].get<int>()){}
};

struct MatchStatusLog {
private:
	int turn;
	Array<MatchStatusLogAction> actions;

public:
	MatchStatusLog(const JSON &log) :
		turn(log[U"turn"].get<int>()),
		actions(get_1d_array<MatchStatusLogAction>(log[U"actions"])){}

	const MatchStatusLogAction &get_action(const int i) const {
		assert(0 <= i && i < (int)actions.size());
		return actions[i];
	}
	int get_turn() const {
		return turn;
	}
};

struct MatchStatus {
private:
	int id;
	int turn;
	MatchStatusBoard board;
	Array<MatchStatusLog> logs;

public:
	MatchStatus(const JSON &json) :
		id(json[U"id"].get<int>()),
		turn(json[U"turn"].get<int>()),
		board(MatchStatusBoard(json[U"board"])),
		logs(get_1d_array<MatchStatusLog>(json[U"logs"])){}

	const Array<Array<int>> &get_walls() const {
		return board.get_walls();
	}
	const Array<Array<int>> &get_territories() const {
		return board.get_territories();
	}
	const Array<Array<int>> &get_structures() const {
		return board.get_structures();
	}
	const Array<Array<int>> &get_masons() const {
		return board.get_masons();
	}
	const MatchStatusLog &get_log(const int t) const {
		for(const MatchStatusLog &log : logs){
			if(log.get_turn() == t) return log;
		}
		return logs.back();
	}
	int get_turn() const {
		return turn;
	}
};

struct ActionPlanAction {
private:
	int type;
	int dir;

public:
	ActionPlanAction(const int type, const int dir) : type(type), dir(dir){}

	JSON output_json(void) const {
		JSON json;
		json[U"type"] = this->type;
		json[U"dir"] = this->dir;
		if(this->type == (int)ACT::NOTHING){
			json[U"dir"] = 0;
		}
		return json;
	}
};

struct ActionPlan {
private:
	int turn;
	Array<ActionPlanAction> actions;

public:
	ActionPlan(const int turn) : turn(turn){}

	void push_back_action(const int type, const int dir){
		actions << ActionPlanAction(type, dir);
	}

	JSON output_json(void) const {
		JSON json;
		json[U"turn"] = this->turn;
		for(const ActionPlanAction &actionplanaction : actions){
			json[U"actions"].push_back(actionplanaction.output_json());
		}
		return json;
	}
};


class Connect {
private:
	URL url_base;
	const HashTable<String, String> headers{ { U"Content-Type", U"application/json" } };
	int match_id;
	String token;

public:
	Connect(void);
	// 試合一覧取得
	Optional<MatchDataMatch> get_matches_list(void) const;
	// 試合状況取得
	Optional<MatchStatus> get_match_status(void) const;
	// 行動計画更新
	Optional<int> post_action_plan(const ActionPlan &action) const;
};


void output_console_response(const HTTPResponse &response){
	Console << U"------";
	Console << U"status code: " << FromEnum(response.getStatusCode());
}

void output_console_json(const JSON &json){
	Console << U"------";
	Console << json.format();
}

void output_console_fail(const String &str){
	Console << U"------";
	Console << str << U" failed!!";
	Console << U"------";
}

Connect::Connect(void){
	// 接続先のURLをローカルファイルから取得
	TextReader reader_url{ U"./url.env" };
	if(not reader_url){
		throw Error{ U"Failed to open 'url.env'" };
	}
	reader_url.readLine(url_base);
	// solver.exeのチームトークンをローカルファイルから取得
	TextReader reader_token{ U"./token.env" };
	if(not reader_token){
		throw Error{ U"Failed to open 'token.env'" };
	}
	reader_token.readLine(token);
	// 行う試合のidをローカルファイルから取得
	TextReader reader_id{ U"./id.env" };
	if(not reader_id){
		throw Error{ U"Failed to open 'id.env'" };
	}
	String tmp_id;
	reader_id.readLine(tmp_id);
	match_id = Parse<int>(tmp_id);
}

Optional<MatchDataMatch> Connect::get_matches_list(void) const {
	const URL url = url_base + U"matches" + U"?token=" + token;
	const FilePath saveFilePath = U"./mathes_list.json";
	if(const auto response = SimpleHTTP::Get(url, headers, saveFilePath)){
		output_console_response(response);
		if(response.isOK()){
			const JSON json = JSON::Load(saveFilePath);
			output_console_json(json);
			const Array<MatchDataMatch> matchdata = get_1d_array<MatchDataMatch>(json[U"matches"]);
			for(const MatchDataMatch &matchdatamatch : matchdata){
				if(matchdatamatch.get_id() == this->match_id){
					return matchdatamatch;
				}
			}
			output_console_fail(U"get same match id");
		}
	}else{
		output_console_fail(U"get_matches_list");
	}
	return none;
}

Optional<MatchStatus> Connect::get_match_status(void) const {
	const URL url = url_base + U"matches/" + Format(match_id) + U"?token=" + token;
	const FilePath saveFilePath = U"./match_status.json";
	if(const auto response = SimpleHTTP::Get(url, headers, saveFilePath)){
		//output_console_response(response);
		if(response.isOK()){
			//output_console_json(JSON::Load(saveFilePath));
			return MatchStatus(JSON::Load(saveFilePath));
		}
	}else{
		output_console_fail(U"get_match_status");
	}
	return none;
}

Optional<int> Connect::post_action_plan(const ActionPlan &actionplan) const {
	const URL url = url_base + U"matches/" + Format(match_id) + U"?token=" + token;
	const FilePath saveFilePath = U"./tmp.json";
	const std::string data = actionplan.output_json().formatUTF8();
	if(const auto response = SimpleHTTP::Post(url, headers, data.data(), data.size(), saveFilePath)){
		output_console_response(response);
		if(response.isOK()){
			output_console_json(JSON::Load(saveFilePath));
			return (JSON::Load(saveFilePath))[U"accepted_at"].get<int>();
		}else{
			output_console_fail(U"post_action_plan");
		}
	}else{
		Print << U"Failed";
	}
	return none;
}
