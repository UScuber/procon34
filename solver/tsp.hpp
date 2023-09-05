#pragma once

#include <tuple>
#include <map>
#include "base.hpp"
#include "timer.hpp"

// 指定された場所に職人全員で壁を立てる
namespace TSP {

constexpr int inf = 1024;

// 距離: 初手にたどり着くことができる職人を除いたグリッド上での移動距離
// 敵の壁がある場合は+1される
// 壁が目的地の場合、その時だけ入れるとする <- なくした
void calc_move_min_cost(const Point start, const Field &field, const State enemy_wall, std::vector<int> &dist, std::vector<int> &prev){
  static std::priority_queue<std::pair<int,Point>> que;

  dist.assign(height*width, inf);
  prev.assign(height*width, -1);
  
  // 0手目
  dist[to_idx(start)] = 0;

  // 1手目
  for(int i = 0; i < 8; i++){
    const Point nxt = start + dmove[i];
    if(!is_valid(nxt)) continue;
    const State st = field.get_state(nxt);
    if(st & (State::Pond | State::Human)) continue;
    int weight = 0;
    if(i < 4){
      if(st & enemy_wall) weight += 2;
      else weight++;
    }else{
      if(st & enemy_wall) continue;
      weight++;
    }
    dist[to_idx(nxt)] = weight;
    prev[to_idx(nxt)] = to_idx(start);
    que.emplace(weight, nxt);
  }
  
  // 2手目~
  while(!que.empty()){
    int cost; Point p;
    std::tie(cost, p) = que.top();
    que.pop();
    if(dist[to_idx(p)] < cost) continue;
    for(int k = 0; k < 8; k++){
      const Point nxt = p + dmove[k];
      if(!is_valid(nxt)) continue;
      const State st = field.get_state(nxt);

      if(st & State::Pond) continue;
      int weight = 0;
      if(k < 4){
        if(st & enemy_wall) weight += 2;
        else weight++;
      }else{
        if(st & enemy_wall) continue;
        weight++;
      }
      if(chmin(dist[to_idx(nxt)], cost + weight)){
        prev[to_idx(nxt)] = to_idx(p);
        que.emplace(cost+weight, nxt);
      }
      /*
      int weight = 0;
      if(k < 4){
        if(st & enemy_wall) weight += 2;
        else weight++;
      }else{
        if(st & (State::Pond | State::Human | enemy_wall)) continue;
        weight++;
      }
      if(chmin(dist[to_idx(nxt)], cost + weight)){
        prev[to_idx(nxt)] = to_idx(p);
        if(!(st & (State::Pond | State::Human))){
          que.emplace(cost+weight, nxt);
        }
      }
      */
    }
  }
}

// 距離: 職人を除いたグリッド上での移動距離
// 敵の壁がある場合は+1される
// 壁が目的地の場合、その時だけ上下左右から入れるとする
void calc_move_min_cost_except_human(const Point start, const Field &field, const State enemy_wall, std::vector<int> &dist, std::vector<int> &prev){
  static std::priority_queue<std::pair<int,Point>> que;

  dist.assign(height*width, inf);
  prev.assign(height*width, -1);
  
  dist[to_idx(start)] = 0;
  que.emplace(0, start);
  
  while(!que.empty()){
    int cost; Point p;
    std::tie(cost, p) = que.top();
    que.pop();
    if(dist[to_idx(p)] < cost) continue;
    for(int k = 0; k < 8; k++){
      const Point nxt = p + dmove[k];
      if(!is_valid(nxt)) continue;
      const State st = field.get_state(nxt);

      int weight = 0;
      if(k < 4){
        if(st & enemy_wall) weight += 2;
        else weight++;
      }else{
        if(st & (State::Pond | enemy_wall)) continue;
        weight++;
      }
      if(chmin(dist[to_idx(nxt)], cost + weight)){
        prev[to_idx(nxt)] = to_idx(p);
        if(!(st & State::Pond)){
          que.emplace(cost+weight, nxt);
        }
      }
    }
  }
}



// 2点間の移動距離
struct CostTable {
  CostTable(const Field &_field, const State _enemy_wall) :
    field(_field), enemy_wall(_enemy_wall), data(height*width), data2(height*width){}

  inline int get_cost(const Point from, const Point to, const bool not_in_hum=false){
    const int idx = to_idx(from);
    std::vector<int> prev;
    if(!not_in_hum){
      if(data[idx].empty()){
        calc_move_min_cost(from, field, enemy_wall, data[idx], prev);
      }
      return data[idx][to_idx(to)];
    }else{
      if(data2[idx].empty()){
        calc_move_min_cost_except_human(from, field, enemy_wall, data2[idx], prev);
      }
      return data2[idx][to_idx(to)];
    }
  }

private:
  std::vector<std::vector<int>> data, data2;
  const State enemy_wall;
  const Field &field;
};


// agentが建てるべき壁(walls)の建てる順番を決める
std::vector<Point> calc_tsp_route(const Point agent, std::vector<Point> walls, CostTable &cost_table){
  /*
  const int walls_num = walls.size();
  std::vector<int> used(walls_num);
  Point last_pos;
  std::vector<Point> result;

  {
    int min_cost = inf, best_idx = -1;
    for(int i = 0; i < walls_num; i++){
      if(chmin(min_cost, cost_table.get_cost(agent, walls[i], true))){
        best_idx = i;
      }
    }
    assert(best_idx != -1);
    used[best_idx] = 1;
    last_pos = walls[best_idx];
    result.emplace_back(last_pos);
  }

  for(int i = 0; i < walls_num-1; i++){
    double min_cost = inf;
    int best_idx = -1;
    for(int j = 0; j < walls_num; j++) if(!used[j]){
      if(chmin(min_cost, cost_table.get_cost(last_pos, walls[j], true))){
        best_idx = j;
      }
    }
    assert(best_idx != -1);
    used[best_idx] = 1;
    last_pos = walls[best_idx];
    result.emplace_back(last_pos);
  }
  std::sort(walls.begin(), walls.end());
  */
  return walls;
}

// 壁を順に建てる時の最小コストを計算
int calc_agent_move_cost(const Point agent, const std::vector<Point> &walls, const Field &field, CostTable &cost_table){
  if(walls.empty()) return 0;
  const int walls_num = walls.size();
  std::vector<int> dp(4, inf);
  for(int i = 0; i < 4; i++){
    const Point p = walls[0] + dmove[i];
    if(!is_valid(p)) continue;
    if(field.get_state(p) & State::Pond) continue;
    dp[i] = cost_table.get_cost(agent, p) + 1;
  }
  for(int i = 1; i < walls_num; i++){
    std::vector<int> nxt(4, inf);
    for(int j = 0; j < 4; j++){
      const Point p = walls[i] + dmove[j];
      if(!is_valid(p)) continue;
      if(field.get_state(p) & State::Pond) continue;
      for(int k = 0; k < 4; k++){
        if(dp[k] >= inf) continue;
        chmin(nxt[j], dp[k] + cost_table.get_cost(walls[i-1]+dmove[k], p) + 1);
      }
    }
    std::swap(nxt, dp);
  }
  int min_cost = inf;
  for(int i = 0; i < 4; i++){
    chmin(min_cost, dp[i]);
  }
  return min_cost;
}

// 初めに建てるべき壁の建てる方向
int find_agent_build_wall_dir(const Point agent, const std::vector<Point> &walls, const Field &field, CostTable &cost_table){
  assert(!walls.empty());

  const int walls_num = walls.size();
  std::vector<std::vector<int>> dp(walls_num, std::vector<int>(4, inf)), prev(walls_num, std::vector<int>(4, -1));

  for(int i = 0; i < 4; i++){
    const Point p = walls[0] + dmove[i];
    if(!is_valid(p)) continue;
    if(field.get_state(p) & State::Pond) continue;
    dp[0][i] = cost_table.get_cost(agent, p) + 1;
  }
  for(int i = 1; i < walls_num; i++){
    for(int j = 0; j < 4; j++){
      const Point p = walls[i] + dmove[j];
      if(!is_valid(p)) continue;
      if(field.get_state(p) & State::Pond) continue;
      for(int k = 0; k < 4; k++){
        if(dp[i-1][k] >= inf) continue;
        const int c = dp[i-1][k] + cost_table.get_cost(walls[i-1]+dmove[k], p) + 1;
        if(chmin(dp[i][j], c)){
          prev[i][j] = k;
        }else if(dp[i][j] == c){
          if(dp[i-1][prev[i][j]] > dp[i-1][k]) prev[i][j] = k;
        }
      }
    }
  }

  int min_cost = inf;
  int dir = -1;
  for(int i = 0; i < 4; i++){
    if(chmin(min_cost, dp[walls_num-1][i])){
      dir = i;
    }
  }
  for(int i = walls_num-1; i > 0; i--){
    dir = prev[i][dir];
  }
  assert(dir != -1);
  return dir;
}

// 初めの壁と建てる向きから経路を出す
Action get_first_action(const Point agent, const Point first_wall, const int dir, const Field &field, const State enemy_wall){
  const Point target = first_wall + dmove[dir];
  assert(is_valid(target));
  assert(!(field.get_state(target) & State::Pond));

  // build or break
  if(agent == target){
    if(field.get_state(first_wall) & enemy_wall){
      return Action(first_wall, Action::Break);
    }else{
      assert(!(field.get_state(first_wall) & State::Wall));
      return Action(first_wall, Action::Build);
    }
  }

  std::vector<int> dist, prev;
  
  calc_move_min_cost(agent, field, enemy_wall, dist, prev);

  assert(dist[to_idx(target)] < inf);

  int nxt_pos = to_idx(target);
  while(prev[nxt_pos] != to_idx(agent)){
    nxt_pos = prev[nxt_pos];
  }
  const Point nxt = to_point(nxt_pos);
  assert(is_around(agent, nxt));
  return Action(nxt, Action::Move);
}


Actions calculate_build_route(const std::vector<Point> &build_walls, const Field &field){
  const auto &agents = field.get_now_turn_agents();
  const int agents_num = agents.size();
  const State ally = field.get_state(agents[0]) & State::Human; // agentから見た味方
  const State enemy = ally ^ State::Human; // agentから見た敵
  assert((ally == State::Enemy) == (field.current_turn & 1));
  assert(ally == State::Ally || ally == State::Enemy);

  const State ally_wall = ally == State::Ally ? State::WallAlly : State::WallEnemy; // agentから見た味方のwall
  const State enemy_wall = ally_wall ^ State::Wall; // agentから見た敵のwall

  CostTable cost_table(field, enemy_wall);

  // すでに置いた壁をなくす
  std::vector<Point> walls;
  for(const Point p : build_walls){
    if(!(field.get_state(p) & ally_wall)) walls.emplace_back(p);
  }
  const int walls_num = walls.size();

  std::vector<std::vector<Point>> wall_part(agents_num);
  const int max_parts_num = (walls_num + agents_num-1) / agents_num;
  for(const Point wall : walls){
    int min_cost = inf;
    int best_idx = -1;
    for(int i = 0; i < agents_num; i++){
      if((int)wall_part[i].size() > max_parts_num) continue;
      for(int j = 0; j < 4; j++){
        const Point p = wall + dmove[j];
        if(!is_valid(p)) continue;
        // 池の場合infになるのでそのままでOK
        if(chmin(min_cost, cost_table.get_cost(agents[i], p))){
          best_idx = i;
        }
      }
    }
    cerr << "wall: " << wall << "\n";
    assert(best_idx != -1);
    wall_part[best_idx].emplace_back(wall);
  }
  
  for(int i = 0; i < agents_num; i++){
    if(!wall_part[i].empty()){
      wall_part[i] = calc_tsp_route(agents[i], wall_part[i], cost_table);
    }
  }

  Actions result;
  for(int i = 0; i < agents_num; i++){
    if(!wall_part[i].empty()){
      const int dir = find_agent_build_wall_dir(agents[i], wall_part[i], field, cost_table);
      result.emplace_back(get_first_action(agents[i], wall_part[i][0], dir, field, enemy_wall));
    }else{
      result.emplace_back(Action(Point(), Action::None));
    }
  }
  for(int i = 0; i < agents_num; i++){
    result[i].agent_idx = i;
  }
  return result;
}

};

using TSP::calculate_build_route;
