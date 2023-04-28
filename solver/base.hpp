#pragma once

#include <set>
#include "field.hpp"

std::vector<Action> enumerate_next_agent_acts(const Point &agent, const Field &field){
  const State ally = field.get_state(agent) & State::Human; // agentから見た味方
  assert((ally == State::Enemy) == (field.current_turn & 1));
  //const State enem = ally ^ State::Human; // agentから見た敵
  assert(ally == State::Ally || ally == State::Enemy);

  const State ally_wall = ally == State::Ally ? State::WallAlly : State::WallEnemy; // agentから見た味方のwall
  const State enemy_wall = ally_wall ^ State::Wall; // agentから見た敵のwall

  std::vector<Action> actions;
  for(int dir = 0; dir < 8; dir++){
    const Point nxt = agent + dmove[dir];
    if(!is_valid(nxt)) continue;
    const State st = field.get_state(nxt);
    if(st & State::Pond) continue;
    if(st & State::Human) continue;
    if(dir < 4){
      // break (自陣の壁の破壊は考慮しない)
      if(st & enemy_wall) actions.emplace_back(Action(nxt, Action::Break));
      // build
      if(!(st & (State::Wall | State::Castle))) actions.emplace_back(Action(nxt, Action::Build));
    }
    // can move to nxt
    if(!(st & enemy_wall)){
      actions.emplace_back(Action(nxt, Action::Move));
    }
  }
  return actions;
}


// 適当に選ぶ
std::vector<Action> select_random_next_agents_acts(const std::vector<Point> &agents, const Field &field){
  std::vector<Action> result;
  std::set<Action> cnt;
  for(const Point &agent : agents){
    assert(((field.get_state(agent) & State::Human) == State::Enemy) == (field.current_turn & 1));
    auto acts = enumerate_next_agent_acts(agent, field);
    if(acts.empty()) acts.emplace_back(Action(agent, Action::None));
    int num = 0;
    int idx = rnd(acts.size());
    while(num++ < 10 && cnt.count(acts[idx])) idx = rnd(acts.size());
    if(num >= 10){
      acts.emplace_back(Action(agent, Action::None));
      idx = (int)acts.size() - 1;
    }
    cnt.insert(acts[idx]);
    result.emplace_back(acts[idx]);
    result.back().agent_idx = (int)result.size() - 1;
  }
  return result;
}

#include <algorithm>
// 後でちゃんと書きます...
std::vector<std::vector<Action>> enumerate_next_all_agents_acts(const std::vector<Point> &agents, const Field &field){
  std::vector<std::vector<Action>> acts;
  for(int i = 0; i < 100; i++){
    acts.emplace_back(select_random_next_agents_acts(agents, field));
  }
  std::sort(acts.begin(), acts.end());
  acts.erase(std::unique(acts.begin(), acts.end()), acts.end());
  return acts;
}


namespace Evaluate {

int calc_agent_min_dist(const Field &field, const std::vector<Point> &ally_agents){
  int dc = 0;
  for(const auto &agent : ally_agents){
    int min_dist = 1000;
    for(const auto &castle : field.castles){
      const int d = manche_dist(agent, castle);
      if(min_dist > d) min_dist = d;
    }
    dc += min_dist * min_dist;
  }
  return dc;
}

int calc_wall_min_dist(const Field &field, const State wall){
  int dw = 0;
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width; j++) if(field.get_state(i, j) & wall){
      int min_dist = 1000;
      for(const auto &castle : field.castles){
        const int d = manche_dist(Point(i, j), castle);
        if(min_dist > d) min_dist = d;
      }
      dw += min_dist * min_dist;
    }
  }
  return dw;
}

std::vector<std::vector<int>> calc_castle_min_dist(const Field &field){
  std::vector<std::vector<int>> res(height, std::vector<int>(width));
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width; j++){
      int min_dist = 1000;
      for(const auto &castle : field.castles){
        const int d = manche_dist(Point(i, j), castle);
        if(min_dist > d) min_dist = d;
      }
      res[i][j] = min_dist;
    }
  }
  return res;
}

double calc_around_wall(const Field &field, const State wall, const int C){
  static std::vector<std::vector<int>> dist_table = calc_castle_min_dist(field);
  int wall_num = 0, mass = 0;
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width; j++) if(dist_table[i][j] <= C){
      if(field.get_state(i, j) & wall) wall_num++;
      mass++;
    }
  }
  return (double)wall_num / mass;
}

double calc_nearest_wall(const Field &field, const State wall){
  static std::vector<std::vector<int>> dist_table = calc_castle_min_dist(field);
  double res = 0;
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width; j++) if(field.get_state(i, j) & wall){
      res += 1.0 / dist_table[i][j];
    }
  }
  return res;
}

int calc_wall_by_enemy(const Field &field, const std::vector<Point> &enemy_agents, const State wall){
  int res = 0;
  for(const auto &agent : enemy_agents){
    for(int dir = 0; dir < 4; dir++){
      const auto nxt = agent + dmove[dir];
      if(!is_valid(nxt)) continue;
      if(field.get_state(nxt) & wall) res++;
    }
  }
  return res;
}

double evaluate_field(const Field &field){
  static constexpr int C = 5;
  // eval: 1
  const int dc = calc_agent_min_dist(field, field.ally_agents) - calc_agent_min_dist(field, field.enemy_agents);
  // eval: 2
  const int dw = calc_wall_min_dist(field, State::WallAlly) - calc_wall_min_dist(field, State::WallEnemy);
  // eval: 3
  const double pw = calc_around_wall(field, State::WallAlly, C) - calc_around_wall(field, State::WallEnemy, C);
  // eval: 4
  const int wd = calc_nearest_wall(field, State::WallAlly) - calc_nearest_wall(field, State::WallEnemy);
  // eval: 5

  // eval: 6
  const int n = field.calc_final_score();
  // eval: 7
  const int wn = calc_wall_by_enemy(field, field.enemy_agents, State::WallAlly) - calc_wall_by_enemy(field, field.ally_agents, State::WallEnemy);

  static constexpr double a = 0.006;
  static constexpr double b = 0.003;
  static constexpr double c = 0.9;
  static constexpr double d = 1.0;
  static constexpr double e = 0;
  static constexpr double f = 0.1;
  double res = 0;
  res -= dc * a;
  res -= dw * b;
  res += pw * c;
  res += wd * d;
  res += n * 0.2;
  res -= wn * f;
  return res * 1;
}

}
