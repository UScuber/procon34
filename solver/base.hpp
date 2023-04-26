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
