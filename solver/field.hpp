#pragma once

#include <queue>
#include "lib.hpp"


struct Action {
  static constexpr int None = 0;
  static constexpr int Break = 1;
  static constexpr int Build = 2;
  static constexpr int Move = 3;

  int command, agent_idx;
  Point pos;

  // posは移動先を表す、構築/破壊はその対象の場所がposになる
  Action(const Point &_p, const int _cmd, const int _agent_idx=-1) : pos(_p), command(_cmd), agent_idx(_agent_idx){
    assert(0 <= command && command < 4);
  }
};


struct Field {

  std::vector<std::vector<State>> field;
  std::vector<Point> ally_agents, enemy_agents;
  std::vector<Point> castles;

  Field(const int h, const int w,
        const std::vector<Point> &ponds,
        const std::vector<Point> &_castles,
        const std::vector<Point> &_ally_agents,
        const std::vector<Point> &_enemy_agents)
    : field(h, std::vector<State>(w, State::None)),
      ally_agents(_ally_agents),
      enemy_agents(_enemy_agents),
      castles(_castles){
    assert(ally_agents.size() == enemy_agents.size()); // check

    for(const Point &p : ponds) field[p.y][p.x] |= State::Pond;
    for(const Point &p : castles) field[p.y][p.x] |= State::Castle;
    for(const Point &p : ally_agents) field[p.y][p.x] |= State::Ally;
    for(const Point &p : enemy_agents) field[p.y][p.x] |= State::Enemy;
  }
  
  inline State get_state(const int y, const int x) const noexcept{
    assert(is_valid(y, x));
    return field[y][x];
  }
  inline State get_state(const Point &p) const noexcept{ return get_state(p.y, p.x); }
  inline void set_state(const int y, const int x, const State state) noexcept{
    assert(is_valid(y, x));
    field[y][x] = state;
  }
  inline void set_state(const Point &p, const State state) noexcept{ set_state(p.y, p.x, state); }

  // 領地の更新
  void update_region(){
    static constexpr uchar NotSeen = 0;
    static constexpr uchar Area = 1;
    static constexpr uchar Neutral = 2;

    auto calc_region = [&](const State my_wall) -> std::vector<std::vector<uchar>> {
      std::vector<std::vector<uchar>> used(height, std::vector<uchar>(width, NotSeen));
      std::queue<Point> que;

      // pを始点にstateで移動する
      auto fill_region = [&](const Point &p, const uchar value){
        assert(que.empty());
        que.push(p);
        used[p.y][p.x] = value;
        while(!que.empty()){
          const Point pos = que.front();
          que.pop();
          for(int dir = 0; dir < 4; dir++){
            const Point nxt = pos + dmove[dir];
            if(!is_valid(nxt)) continue;
            if(used[nxt.y][nxt.x] == NotSeen && !(get_state(nxt) & my_wall)){
              used[nxt.y][nxt.x] = value;
              que.push(nxt);
            }
          }
        }
      };
      // fill Neutral
      for(int i = 0; i < height; i++){
        if(!(get_state(i, 0) & my_wall)){
          fill_region(Point(i, 0), Neutral);
        }
        if(!(get_state(i, width-1) & my_wall)){
          fill_region(Point(i, width-1), Neutral);
        }
      }
      for(int j = 0; j < width; j++){
        if(!(get_state(0, j) & my_wall)){
          fill_region(Point(0, j), Neutral);
        }
        if(!(get_state(height-1, j) & my_wall)){
          fill_region(Point(height-1, j), Neutral);
        }
      }
      // fill ally or enemy 's area
      while(!que.empty()){
        const Point pos = que.front();
        que.pop();
        for(int dir = 0; dir < 4; dir++){
          const Point nxt = pos + dmove[dir];
          if(!is_valid(nxt)) continue;
          if(used[nxt.y][nxt.x] == NotSeen && !(get_state(nxt) & my_wall)){
            used[nxt.y][nxt.x] = used[pos.y][pos.x];
            que.push(nxt);
          }
        }
      }
      for(int i = 1; i < height-1; i++){
        for(int j = 1; j < width-1; j++){
          if(used[i][j] == NotSeen && !(get_state(i, j) & my_wall)){
            fill_region(Point(i, j), Area);
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
          set_state(i, j, st | State::AreaAlly | State::AreaEnemy);
        }else if(ally_reg[i][j] == Area){
          set_state(i, j, (st | State::AreaAlly) & ~State::AreaEnemy);
        }else if(enemy_reg[i][j] == Area){
          set_state(i, j, (st | State::AreaEnemy) & ~State::AreaAlly);
        }
      }
    }
  }

  // side: 味方:0, 敵:1
  void update_field(const std::vector<Action> &acts, const bool side=false){
    assert(acts.size() == ally_agents.size());
    std::vector<Action> act_list[4];
    for(const auto &act : acts){
      act_list[act.command].emplace_back(act);
    }
    if(!side){
      // break
      for(const auto &act : act_list[Action::Break]){
        const State st = get_state(act.pos);
        assert(st & State::Wall);
        set_state(act.pos, st & ~State::Wall);
      }
      // build
      for(const auto &act : act_list[Action::Build]){
        const State st = get_state(act.pos);
        assert(!(st & (State::Wall | State::Enemy)));
        set_state(act.pos, st | State::WallAlly);
      }
      // move
      for(const auto &act : act_list[Action::Move]){
        const State st = get_state(act.pos);
        assert(!(st & (State::Human | State::Pond | State::WallEnemy)));
        const Point from = ally_agents[act.agent_idx];
        set_state(from, get_state(from) ^ State::Ally);
        set_state(act.pos, st | State::Ally);
        ally_agents[act.agent_idx] = act.pos;
      }
    }else{

    }
    update_region();
  }
};

Field create_random_field(const int h, const int w, int agents_num=-1, int castles_num=-1){
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
  return Field(
    h, w,
    gen_rnd_poses(ponds_num),
    gen_rnd_poses(castles_num),
    gen_rnd_poses(agents_num),
    gen_rnd_poses(agents_num)
  );
}
