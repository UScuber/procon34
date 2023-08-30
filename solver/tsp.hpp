#pragma once

#include <tuple>
#include "base.hpp"
#include "timer.hpp"

// 指定された場所に職人全員で壁を立てる
namespace TSP {


// 壁は最後の1度だけ入れるとする
std::vector<std::vector<int>> calc_move_cost(const Field &field, const State enemy_wall){
  std::vector<std::vector<int>> dist(height*width);

  std::priority_queue<std::pair<int,Point>> que;

  Timer timer;
  
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width; j++){
      std::vector<int> d(height*width, 1024);
      que.push({ 0, Point(i, j) });
      d[i*width + j] = 0; 
      
      while(!que.empty()){
        int cost; Point p;
        std::tie(cost, p) = que.top();
        que.pop();
        if(d[to_idx(p)] < cost) continue;
        for(int k = 0; k < 8; k++){
          const Point nxt = p + dmove[k];
          if(!is_valid(nxt)) continue;
          const State st = field.get_state(nxt);
          int weight = 0;
          if(k < 4){
            if(st & enemy_wall) weight += 2;
            else weight++;
          }else{
            if(st & (State::Pond | State::Human | enemy_wall)) continue;
            weight++;
          }
          if(chmin(d[to_idx(nxt)], cost + weight)){
            if(!(st & (State::Pond | State::Human))){
              que.emplace(cost+weight, nxt);
            }
          }
        }
      }
      dist[i*width + j] = d;
    }
  }
  cerr << "calc time: " << timer.result() << "\n";
  return dist;
}


// agentが建てるべき壁(walls)の建てる順番を決める
std::vector<Point> calc_tsp_route(const Point agent, std::vector<Point> walls, const std::vector<std::vector<int>> &cost_table){
  std::sort(walls.begin(), walls.end());
  return walls;
}

int calc_agent_move_cost(const Point agent, const std::vector<Point> &walls, const Field &field, const std::vector<std::vector<int>> &cost_table){
  if(walls.empty()) return 0;
  const int walls_num = walls.size();
  std::vector<std::vector<int>> dp(walls_num, std::vector<int>(4, 1024));
  for(int i = 0; i < 4; i++){
    const Point p = walls[0] + dmove[i];
    if(!is_valid(p)) continue;
    if(field.get_state(p) & State::Pond) continue;
    dp[0][i] = cost_table[to_idx(agent)][to_idx(p)] + 1;
  }
  for(int i = 1; i < walls_num; i++){
    for(int j = 0; j < 4; j++){
      const Point p = walls[i] + dmove[j];
      if(!is_valid(p)) continue;
      if(field.get_state(p) & State::Pond) continue;
      for(int k = 0; k < 4; k++){
        if(dp[i-1][k] >= 1024) continue;
        chmin(dp[i][j], dp[i-1][k] + cost_table[to_idx(walls[i-1]+dmove[k])][to_idx(p)] + 1);
      }
    }
  }
  int min_cost = 1024;
  for(int i = 0; i < 4; i++){
    chmin(min_cost, dp[walls_num-1][i]);
  }
  return min_cost;
}

// 初めに建てるべき壁の建てる方向
int find_agent_build_wall_dir(const Point agent, const std::vector<Point> &walls, const Field &field, const std::vector<std::vector<int>> &cost_table){
  assert(!walls.empty());

  const int walls_num = walls.size();
  std::vector<std::vector<int>> dp(walls_num, std::vector<int>(4, 1024)), prev(walls_num, std::vector<int>(4, -1));

  for(int i = 0; i < 4; i++){
    const Point p = walls[0] + dmove[i];
    if(!is_valid(p)) continue;
    if(field.get_state(p) & State::Pond) continue;
    dp[0][i] = cost_table[to_idx(agent)][to_idx(p)] + 1;
  }
  for(int i = 1; i < walls_num; i++){
    for(int j = 0; j < 4; j++){
      const Point p = walls[i] + dmove[j];
      if(!is_valid(p)) continue;
      if(field.get_state(p) & State::Pond) continue;
      for(int k = 0; k < 4; k++){
        if(dp[i-1][k] >= 1024) continue;
        const int c = dp[i-1][k] + cost_table[to_idx(walls[i-1]+dmove[k])][to_idx(p)] + 1;
        if(chmin(dp[i][j], c)){
          prev[i][j] = k;
        }else if(dp[i][j] == c){
          if(dp[i-1][prev[i][j]] > dp[i-1][k]) prev[i][j] = k;
        }
      }
    }
  }

  int min_cost = 1024;
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

  // build or break
  if(agent == target){
    if(field.get_state(first_wall) & enemy_wall){
      return Action(first_wall, Action::Break);
    }else{
      assert(!(field.get_state(first_wall) & State::Wall));
      return Action(first_wall, Action::Build);
    }
  }

  std::priority_queue<std::pair<int,Point>> que;
  std::vector<int> dist(height*width, 1024), prev(height*width, -1);
  que.push({ 0, agent });
  dist[to_idx(agent)] = 0;
  
  while(!que.empty()){
    int cost; Point p;
    std::tie(cost, p) = que.top();
    que.pop();
    if(dist[to_idx(p)] < cost) continue;
    for(int k = 0; k < 8; k++){
      const Point nxt = p + dmove[k];
      if(!is_valid(nxt)) continue;
      const State st = field.get_state(nxt);
      if(st & (State::Pond | State::Human)) continue;
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
    }
  }
  assert(dist[to_idx(target)] < 1024);

  int nxt_pos = to_idx(target);
  while(prev[nxt_pos] != to_idx(agent)){
    nxt_pos = prev[nxt_pos];
  }
  const Point nxt(nxt_pos/width, nxt_pos%width);
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

  // すでに置いた壁をなくす
  std::vector<Point> walls;
  for(const Point p : build_walls){
    if(!(field.get_state(p) & ally_wall)) walls.emplace_back(p);
  }
  const int walls_num = walls.size();

  const auto cost_table = calc_move_cost(field, enemy_wall);
  std::vector<std::vector<Point>> wall_part(agents_num);
  for(const Point wall : walls){
    int min_cost = 1024;
    int best_idx = -1;
    for(int i = 0; i < agents_num; i++){
      if(chmin(min_cost, cost_table[to_idx(agents[i])][to_idx(wall)])){
        best_idx = i;
      }
    }
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
