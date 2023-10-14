#pragma once

#include <tuple>
#include <map>
#include <cmath>
#include <omp.h>
#include "base.hpp"
#include "timer.hpp"

// 指定された場所に職人全員で壁を立てる
namespace TSP {

constexpr int inf = 1024;

// 距離: 初手にたどり着くことができる職人を除いたグリッド上での移動距離
// 敵の壁がある場合は+1される
// 池には入れない
void calc_move_min_cost(const Point start, const Field &field, const State enemy_wall, std::vector<int> &dist, std::vector<int> &prev){
  std::priority_queue<std::pair<int,Point>> que;

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
    }
  }
}

// 距離: 職人を除いたグリッド上での移動距離
// 敵の壁がある場合は+1される
// 池が目的地の場合、その時だけ上下左右から入れるとする
void calc_move_min_cost_except_human(const Point start, const Field &field, const State enemy_wall, std::vector<int> &dist, std::vector<int> &prev){
  std::priority_queue<std::pair<int,Point>> que;

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

  void pre_calc_around_walls(const Walls &walls, const Agents &agents){
    std::vector<Point> points;
    for(const Wall wall : walls){
      for(int i = 0; i < 4; i++){
        const Point p = wall + dmove[i];
        if(!is_valid(p)) continue;
        points.emplace_back(p);
      }
    }
    for(const Agent agent : agents){
      points.emplace_back(agent);
    }
    std::sort(points.begin(), points.end());
    points.erase(std::unique(points.begin(), points.end()), points.end());
    const int points_num = points.size();

    #pragma omp parallel for
    for(int i = 0; i < points_num; i++){
      std::vector<int> prev;
      const int idx = to_idx(points[i]);
      calc_move_min_cost(points[i], field, enemy_wall, data[idx], prev);
      calc_move_min_cost_except_human(points[i], field, enemy_wall, data2[idx], prev);
    }
  }

private:
  std::vector<std::vector<int>> data, data2;
  const State enemy_wall;
  const Field &field;
};


// agentが建てるべき壁(walls)の建てる順番を決める
Walls calc_tsp_route(const Agent agent, Walls walls, CostTable &cost_table){
  const int walls_num = walls.size();
  std::vector<int> used(walls_num);
  Wall last_pos;
  Walls result;

  std::sort(walls.begin(), walls.end(), [&](const Point a, const Point b){
    return manche_dist(agent, a) < manche_dist(agent, b);
  });

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
  // std::sort(walls.begin(), walls.end(), [&](const Point a, const Point b){
  //   return cost_table.get_cost(agent, a) + manche_dist(agent, a) < cost_table.get_cost(agent, b) + manche_dist(agent, b);
  // });
  return result;
}

// 壁を順に建てる時の最小コストを計算
int calc_agent_move_cost(const Agent agent, const Walls &walls, const Field &field, CostTable &cost_table){
  if(walls.empty()) return 0;
  const int walls_num = walls.size();
  std::vector<int> dp(4, inf);
  for(int i = 0; i < 4; i++){
    const Point p = walls[0] + dmove[i];
    if(!is_valid(p)) continue;
    if(field.get_state(p) & State::Pond) continue;
    dp[i] = cost_table.get_cost(agent, p) + 1;
    dp[i] += std::min(cost_table.get_cost(agent, p), 5);
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

// 全体の移動回数のコスト計算
int calc_all_agent_move_cost(const Agents &agents, const std::vector<Walls> &wall_part, const Field &field, CostTable &cost_table){
  int max_cost = 0;
  for(int i = 0; i < (int)agents.size(); i++){
    const int cost = calc_agent_move_cost(agents[i], wall_part[i], field, cost_table);
    chmax(max_cost, cost);
  }
  return max_cost;
}


// 初めに建てるべき壁の建てる方向
int find_agent_build_wall_dir(const Agent agent, const Walls &walls, const Field &field, CostTable &cost_table){
  assert(!walls.empty());

  const int walls_num = walls.size();
  std::vector<std::vector<int>> dp(walls_num, std::vector<int>(4, inf)), prev(walls_num, std::vector<int>(4, -1));

  for(int i = 0; i < 4; i++){
    const Point p = walls[0] + dmove[i];
    if(!is_valid(p)) continue;
    if(field.get_state(p) & State::Pond) continue;
    dp[0][i] = cost_table.get_cost(agent, p) + 1;
    dp[0][i] += std::min(cost_table.get_cost(agent, p), 5);
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
Action get_first_action(const Agent agent, const Wall first_wall, const int dir, const Field &field, const State enemy_wall){
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
  if(field.get_state(nxt) & enemy_wall){
    return Action(nxt, Action::Break);
  }else{
    return Action(nxt, Action::Move);
  }
}


void find_none_act_agents(Actions actions, int pos, const Agents &agents, const State enemy_wall, const State enemy, std::vector<Actions> &actions_list, const Field &field){
  while(pos < (int)actions.size() && !actions[pos].is_none()) pos++;
  if(pos >= (int)actions.size()){
    actions_list.emplace_back(actions);
    return;
  }
  for(int i = 0; i < 4; i++){
    const Point p = agents[pos] + dmove[i];
    if(!is_valid(p)) continue;
    const State st = field.get_state(p);
    if(st & (State::Castle | enemy)) continue;
    if(st & enemy_wall){
      actions[pos] = Action(p, Action::Break, pos);
    }else if(!(st & State::Wall)){
      actions[pos] = Action(p, Action::Build, pos);
    }else continue;
    find_none_act_agents(actions, pos+1, agents, enemy_wall, enemy, actions_list, field);
  }
  // none
  find_none_act_agents(actions, pos+1, agents, enemy_wall, enemy, actions_list, field);
}

// actがnoneだったら何か行動する
Actions act_none_act_agents(const Actions &actions, const Agents &agents, const State enemy_wall, const State enemy, const Field &field){
  std::vector<Actions> actions_list;
  find_none_act_agents(actions, 0, agents, enemy_wall, enemy, actions_list, field);

  const int size = actions_list.size();
  std::vector<int> scores(size);
  #pragma omp parallel for
  for(int i = 0; i < size; i++){
    Field f = field;
    f.update_field(actions_list[i]);
    scores[i] = f.calc_final_score();
  }

  int max_val = -1, max_idx = -1;
  for(int i = 0; i < size; i++){
    if(chmax(max_val, scores[i])) max_idx = i;
  }
  assert(max_idx != -1);

  return actions_list[max_idx];
}

Actions calculate_build_route(const Walls &build_walls, const Field &field){
  //const int TL = field.TL;
  const int TL = 2200;
  const auto &agents = field.get_now_turn_agents();
  const int agents_num = agents.size();
  const State ally = field.get_state(agents[0]) & State::Human; // agentから見た味方
  const State enemy = ally ^ State::Human; // agentから見た敵
  assert((ally == State::Enemy) == ((field.current_turn & 1) ^ field.side));
  assert(ally == State::Ally || ally == State::Enemy);

  const State ally_wall = ally == State::Ally ? State::WallAlly : State::WallEnemy; // agentから見た味方のwall
  const State enemy_wall = ally_wall ^ State::Wall; // agentから見た敵のwall

  StopWatch sw;

  CostTable cost_table(field, enemy_wall);

  // すでに置いた壁をなくす
  Walls walls;
  for(const Wall p : build_walls){
    if(!(field.get_state(p) & ally_wall)) walls.emplace_back(p);
  }
  const int walls_num = walls.size();

  StopWatch timer;
  cost_table.pre_calc_around_walls(walls, agents);
  cerr << "Calc Time: " << timer.get_ms() << "[ms]\n";

  std::vector<Walls> wall_part(agents_num);
  const int max_parts_num = (walls_num + agents_num-1) / agents_num;
  for(const Wall wall : walls){
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

  const double T0 = 5;
  const double T1 = 1;
  double temp = T0;
  double spend_time = 0;
  std::vector<int> costs(agents_num);
  int best_score = 0;
  for(int i = 0; i < agents_num; i++){
    costs[i] = calc_agent_move_cost(agents[i], wall_part[i], field, cost_table);
    chmax(best_score, costs[i]);
  }
  auto awesome_wall_part = wall_part;
  int awesome_score = best_score;

  cerr << "Start SA(TSP)\n";
  cerr << "First Score: " << awesome_score << "\n";
  int steps = 0, updated_num = 0;
  if(walls_num) for(; ; steps++){
    if(!(steps & 127)){
      spend_time = sw.get_ms();
      const double p = spend_time / TL;
      if(p >= 1.0) break;
      temp = (T1 - T0) * p + T0;
    }

    auto &wp = wall_part;
    int a = -1, b = -1, ai = -1, bi = -1;
    bool is_swap = false;
    // swap
    if(rnd(10) < 8 && (int)walls.size() >= 2){
      a = rnd(agents_num), b = rnd(agents_num);
      while(wp[a].empty() || wp[b].empty()){
        a = rnd(agents_num);
        b = rnd(agents_num);
      }
      if(a == b && (int)wp[a].size() <= 1){
        steps--;
        continue;
      }
      ai = rnd(wp[a].size());
      bi = rnd(wp[b].size());
      if(a == b){
        while(ai == bi){
          ai = rnd(wp[a].size());
          bi = rnd(wp[b].size());
        }
      }
      std::swap(wp[a][ai], wp[b][bi]);
      is_swap = true;
    }
    // move a <- b
    else{
      a = rnd(agents_num), b = rnd(agents_num);
      while(a == b || wp[b].empty()){
        a = rnd(agents_num);
        b = rnd(agents_num);
      }
      ai = rnd(wp[a].size()+1);
      bi = rnd(wp[b].size());
      wp[a].insert(wp[a].begin() + ai, wp[b][bi]);
      wp[b].erase(wp[b].begin() + bi);
    }

    int score = 0;
    for(int i = 0; i < agents_num; i++){
      if(i == a || i == b){
        const int cost = calc_agent_move_cost(agents[i], wp[i], field, cost_table);
        chmax(score, cost);
      }else{
        chmax(score, costs[i]);
      }
    }

    if(awesome_score > score){
      awesome_score = score;
      best_score = score;
      awesome_wall_part = wp;
      costs[a] = calc_agent_move_cost(agents[a], wp[a], field, cost_table);
      if(a != b) costs[b] = calc_agent_move_cost(agents[b], wp[b], field, cost_table);
      updated_num++;
    }else if(exp((double)(best_score - score) / temp) > rnd(2048)/2048.0){
      best_score = score;
      costs[a] = calc_agent_move_cost(agents[a], wp[a], field, cost_table);
      if(a != b) costs[b] = calc_agent_move_cost(agents[b], wp[b], field, cost_table);
      updated_num++;
    }else{
      if(is_swap){
        std::swap(wp[a][ai], wp[b][bi]);
      }else{
        wp[b].insert(wp[b].begin() + bi, wp[a][ai]);
        wp[a].erase(wp[a].begin() + ai);
      }
    }
  }
  cerr << "Steps: " << steps << "\n";
  cerr << "Updated: " << updated_num << "\n";
  cerr << "Final Score: " << awesome_score << "\n";

  wall_part = awesome_wall_part;

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
    result[i].set_idx(i);
  }

  StopWatch timer2;
  // conflictしてる職人の数を減らす
  Actions pre_result = result;
  field.fix_actions(result);

  std::vector<int> cant_move_idx;
  for(int i = 0; i < agents_num; i++){
    if(!pre_result[i].is_none() && result[i].is_none()){
      cant_move_idx.emplace_back(i);
    }
  }
  const int cant_move_num = cant_move_idx.size();
  Actions best_act = result;
  int max_num = 0;
  for(int i = 0; i < (1 << cant_move_num); i++){
    Actions acts = result;
    int num = 0;
    for(int j = 0; j < cant_move_num; j++) if(i >> j & 1){
      acts[cant_move_idx[j]] = pre_result[cant_move_idx[j]];
      num++;
    }
    if(field.is_legal_action(acts)){
      if(chmax(max_num, num)) best_act = acts;
    }
  }
  assert(field.is_legal_action(best_act));

  best_act = act_none_act_agents(best_act, agents, enemy_wall, enemy, field);
  cerr << "None Time: " << timer2.get_ms() << "[ms]\n";

  return best_act;
}

};

using TSP::calculate_build_route;
