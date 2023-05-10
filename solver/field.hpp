#pragma once

#include <queue>
#include "lib.hpp"


struct Action {
  static constexpr uchar None = 0;
  static constexpr uchar Break = 1;
  static constexpr uchar Build = 2;
  static constexpr uchar Move = 3;

  uchar command; int agent_idx;
  Point pos;

  // posは移動先を表す、構築/破壊はその対象の場所がposになる
  inline constexpr Action(const Point &_p, const uchar _cmd, const int _agent_idx=-1) : pos(_p), command(_cmd), agent_idx(_agent_idx){
    assert(0 <= command && command < 4);
  }
  inline constexpr bool operator<(const Action &act) const{
    if(command != act.command) return command < act.command;
    return pos < act.pos;
  }
  inline constexpr bool operator==(const Action &act) const{
    return command == act.command && agent_idx == act.agent_idx && pos == act.pos;
  }
};

using Actions = std::vector<Action>;
using Agents = std::vector<Point>;

struct Field {

  std::vector<std::vector<State>> field;
  Agents ally_agents, enemy_agents;
  std::vector<Point> castles;
  int side, current_turn, final_turn;

  Field(const int h, const int w,
        const std::vector<Point> &ponds,
        const std::vector<Point> &_castles,
        const Agents &_ally_agents,
        const Agents &_enemy_agents,
        const int _side, // 0 or 1
        const int _final_turn)
    : field(h, std::vector<State>(w, State::None)),
      ally_agents(_ally_agents),
      enemy_agents(_enemy_agents),
      castles(_castles),
      side(_side),
      current_turn(_side),
      final_turn(_final_turn){
    assert(ally_agents.size() == enemy_agents.size()); // check

    for(const auto &p : ponds) field[p.y][p.x] |= State::Pond;
    for(const auto &p : castles) field[p.y][p.x] |= State::Castle;
    for(const auto &p : ally_agents) field[p.y][p.x] |= State::Ally;
    for(const auto &p : enemy_agents) field[p.y][p.x] |= State::Enemy;
  }
  
  inline State get_state(const int y, const int x) const noexcept{
    assert(is_valid(y, x));
    return field[y][x];
  }
  inline State get_state(const Point &p) const noexcept{
    assert(is_valid(p.y, p.x));
    return get_state(p.y, p.x);
  }
  inline void set_state(const int y, const int x, const State state) noexcept{
    assert(is_valid(y, x));
    field[y][x] = state;
  }
  inline void set_state(const Point &p, const State state) noexcept{ set_state(p.y, p.x, state); }

  // スコア計算
  int calc_final_score() const{
    int ally_walls = 0, enemy_walls = 0;
    int ally_area = 0, enemy_area = 0;
    int allys_castle = 0, enemys_castle = 0;
    // 領域計算
    for(int i = 0; i < height; i++){
      for(int j = 0; j < width; j++){
        const State s = get_state(i, j);
        if(s & State::WallAlly) ally_walls++;
        if(s & State::WallEnemy) enemy_walls++;
        if(s & State::AreaAlly) ally_area++;
        if(s & State::AreaEnemy) enemy_area++;
        if(s & State::Castle){
          if(s & State::AreaAlly) allys_castle++;
          if(s & State::AreaEnemy) enemys_castle++;
        }
      }
    }
    const int score = (ally_walls-enemy_walls)*wall_coef + (ally_area-enemy_area)*area_coef + (allys_castle-enemys_castle)*castles_coef;
    return score;
  }

  // 領地の更新
  void update_region(){
    static constexpr uchar NotSeen = 0;
    static constexpr uchar Area = 1;
    static constexpr uchar Neutral = 2;

    static std::queue<Point> que;

    auto calc_region = [&](const State my_wall) -> std::vector<std::vector<uchar>> {
      std::vector<std::vector<uchar>> used(height, std::vector<uchar>(width, NotSeen));

      // fill Neutral
      for(int i = 0; i < height; i++){
        if(!(get_state(i, 0) & my_wall)){
          que.push(Point(i, 0));
          used[i][0] = Neutral;
        }
        if(!(get_state(i, width-1) & my_wall)){
          que.push(Point(i, width-1));
          used[i][width-1] = Neutral;
        }
      }
      for(int j = 0; j < width; j++){
        if(!(get_state(0, j) & my_wall)){
          que.push(Point(0, j));
          used[0][j] = Neutral;
        }
        if(!(get_state(height-1, j) & my_wall)){
          que.push(Point(height-1, j));
          used[height-1][j] = Neutral;
        }
      }
      while(!que.empty()){
        const Point pos = que.front();
        que.pop();
        for(int dir = 0; dir < 4; dir++){
          const Point nxt = pos + dmove[dir];
          if(!is_valid(nxt)) continue;
          if(used[nxt.y][nxt.x] == NotSeen && !(get_state(nxt) & my_wall)){
            used[nxt.y][nxt.x] = Neutral;
            que.push(nxt);
          }
        }
      }
      // fill ally or enemy 's area
      for(int i = 1; i < height-1; i++){
        for(int j = 1; j < width-1; j++){
          if(used[i][j] == NotSeen && !(get_state(i, j) & my_wall)){
            used[i][j] = Area;
          }
        }
      }
      return used;
    };

    const auto ally_reg = calc_region(State::WallAlly);
    const auto enemy_reg = calc_region(State::WallEnemy);
    
    // 片方囲われている -> 外された <-- そのまま
    // 片方囲われている -> 片方に   <-- その片方に
    // 両方囲われている -> 外された <-- そのまま(まだ不明)
    // 両方囲われている -> 片方に   <-- その片方に
    //           |
    //           v
    // 外されている -> そのまま
    // 片方になる   -> その片方に
    for(int i = 0; i < height; i++){
      for(int j = 0; j < width; j++){
        const State st = get_state(i, j);
        if(ally_reg[i][j] == Area && enemy_reg[i][j] == Area){
          set_state(i, j, st | State::Area);
        }else if(ally_reg[i][j] == Area){
          set_state(i, j, (st | State::AreaAlly) & ~State::AreaEnemy);
        }else if(enemy_reg[i][j] == Area){
          set_state(i, j, (st | State::AreaEnemy) & ~State::AreaAlly);
        }
        if(st & State::WallAlly) set_state(i, j, st & ~State::AreaAlly);
        if(st & State::WallEnemy) set_state(i, j, st & ~State::AreaEnemy);
      }
    }
  }

  // bug: 移動に問題あり、職人がいた場所に職人が移動できてしまう
  // side: 味方:0, 敵:1
  void update_field(const Actions &acts){
    assert(acts.size() == ally_agents.size());
    Actions act_list[4];
    for(const auto &act : acts){
      act_list[act.command].emplace_back(act);
      assert(0 <= act.agent_idx && act.agent_idx < (int)acts.size());
    }
    if(!(current_turn & 1)){
      // break
      for(const auto &act : act_list[Action::Break]){
        const State st = get_state(act.pos);
        assert(st & State::Wall);
        set_state(act.pos, st & ~State::Wall);
      }
      // build
      for(const auto &act : act_list[Action::Build]){
        const State st = get_state(act.pos);
        assert(!(st & (State::WallEnemy | State::Enemy | State::Castle)));
        if(st & State::WallAlly){ // someone already built
          continue;
        }
        set_state(act.pos, st | State::WallAlly);
      }
      // move
      for(const auto &act : act_list[Action::Move]){
        const State st = get_state(act.pos);
        assert(!(st & (State::Human | State::Pond | State::WallEnemy)));
        const auto from = ally_agents[act.agent_idx];
        set_state(from, get_state(from) ^ State::Ally);
        set_state(act.pos, st | State::Ally);
        ally_agents[act.agent_idx] = act.pos;
      }
    }else{
      // break
      for(const auto &act : act_list[Action::Break]){
        const State st = get_state(act.pos);
        assert(st & State::Wall);
        set_state(act.pos, st & ~State::Wall);
      }
      // build
      for(const auto &act : act_list[Action::Build]){
        const State st = get_state(act.pos);
        assert(!(st & (State::WallAlly | State::Ally | State::Castle)));
        if(st & State::WallEnemy){ // someone already built
          continue;
        }
        set_state(act.pos, st | State::WallEnemy);
      }
      // move
      for(const auto &act : act_list[Action::Move]){
        const State st = get_state(act.pos);
        assert(!(st & (State::Human | State::Pond | State::WallAlly)));
        const auto from = enemy_agents[act.agent_idx];
        set_state(from, get_state(from) ^ State::Enemy);
        set_state(act.pos, st | State::Enemy);
        enemy_agents[act.agent_idx] = act.pos;
      }
    }
    update_region();
  }

  void update_turn(const Actions &acts){
    update_field(acts);
    current_turn++;
  }
  Agents &get_now_turn_agents(){
    if(current_turn & 1) return enemy_agents;
    return ally_agents;
  }
  const Agents &get_now_turn_agents() const{
    if(current_turn & 1) return enemy_agents;
    return ally_agents;
  }
  bool is_finished() const{ return current_turn == final_turn; }
  bool is_my_turn() const{ return (current_turn & 1) == side; }
  void debug() const{
    std::vector<std::string> board(height), wall(height), region(height);
    for(int i = 0; i < height; i++){
      for(int j = 0; j < width; j++){
        const State s = get_state(i, j);
        //  board
        char c = '.';
        if(s & State::Pond) c = '#';
        else if(s & State::Ally) c = '@';
        else if(s & State::Enemy) c = '%';
        else if(s & State::Castle) c = '$';
        board[i] += c;
        // wall
        c = '.';
        if((s & State::WallAlly) && (s & State::WallEnemy)) c = '$';
        else if(s & State::WallAlly) c = '@';
        else if(s & State::WallEnemy) c = '%';
        else if(s & State::Castle) c = '$';
        wall[i] += c;
        // region
        c = '.';
        if((s & State::AreaAlly) && (s & State::AreaEnemy)) c = '$';
        else if(s & State::AreaAlly) c = '@';
        else if(s & State::AreaEnemy) c = '%';
        else if(s & State::Castle) c = '$';
        region[i] += c;
      }
    }
    std::cerr << "board" << std::string(width-4, ' ') << ": walls" << std::string(width-4, ' ') << ": region\n";
    for(int i = 0; i < height; i++){
      std::cerr << board[i] << " : " << wall[i] << " : " << region[i] << "\n";
    }
    std::cerr << "\n";
  }
};


// side: 先攻後攻
Field create_random_field(const int h, const int w, int agents_num=-1, int castles_num=-1, int final_turn=-1, int side=0){
  std::vector<std::vector<int>> used(h, std::vector<int>(w));
  auto gen_rnd_poses = [&](const int num) -> std::vector<Point> {
    std::vector<Point> res(num);
    for(int i = 0; i < num; i++){
      int posy = rnd(h), posx = rnd(w);
      while(used[posy][posx]){
        posy = rnd(h);
        posx = rnd(w);
      }
      res[i] = Point(posy, posx);
      used[posy][posx] = 1;
    }
    return res;
  };
  const int area = h * w;
  const int ponds_num = rnd(0, area/40+1);
  if(castles_num == -1) castles_num = rnd(1, area/125+2);
  if(agents_num == -1) agents_num = rnd(2, 7);
  if(final_turn == -1) final_turn = rnd(15, 100) * 2;
  std::vector<Point> castles(castles_num);
  for(int i = 0; i < castles_num; i++){
    int posy = rnd(1, h-1), posx = rnd(1, w-1);
    while(used[posy][posx]){
      posy = rnd(1, h-1);
      posx = rnd(1, w-1);
    }
    castles[i] = Point(posy, posx);
    used[posy][posx] = 1;
  }
  return Field(
    h, w,
    gen_rnd_poses(ponds_num),
    castles,
    gen_rnd_poses(agents_num),
    gen_rnd_poses(agents_num),
    side,
    final_turn
  );
}

Field read_field(const int h, const int w){
  auto get_points = []() -> std::vector<Point> {
    int num;
    std::cin >> num;
    assert(0 <= num);
    std::vector<Point> res(num);
    for(int i = 0; i < num; i++) std::cin >> res[i];
    return res;
  };

  int side; // 先行:0,後攻:1
  int final_turn;
  std::cin >> side >> final_turn;
  assert(side == 0 || side == 1);
  const auto ponds = get_points();
  const auto castles = get_points();
  const auto ally_agents = get_points();
  const auto enemy_agents = get_points();
  return Field(
    h, w,
    ponds,
    castles,
    ally_agents,
    enemy_agents,
    side,
    final_turn
  );
}