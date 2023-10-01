#pragma once

#include <set>
#include "field.hpp"

Actions enumerate_next_agent_acts(const Point &agent, const Field &field, const bool use_assert=true){
  const State ally = field.get_state(agent) & State::Human; // agentから見た味方
  const State enemy = ally ^ State::Human; // agentから見た敵
  if(use_assert) assert((ally == State::Enemy) == (field.current_turn & 1));
  if(use_assert) assert(ally == State::Ally || ally == State::Enemy);

  const State ally_wall = ally == State::Ally ? State::WallAlly : State::WallEnemy; // agentから見た味方のwall
  const State enemy_wall = ally_wall ^ State::Wall; // agentから見た敵のwall

  Actions actions;
  for(int dir = 0; dir < 8; dir++){
    const auto nxt = agent + dmove[dir];
    if(!is_valid(nxt)) continue;
    const State st = field.get_state(nxt);
    if(dir < 4){
      // break (自陣の壁の破壊は考慮しない)
      if(st & enemy_wall) actions.emplace_back(Action(nxt, Action::Break));
      // build
      if(!(st & (State::Wall | State::Castle | enemy))) actions.emplace_back(Action(nxt, Action::Build));
    }
    // can move to nxt
    if(!(st & (State::Pond | State::Human | enemy_wall))){
      actions.emplace_back(Action(nxt, Action::Move));
    }
  }
  return actions;
}


// 適当に選ぶ
Actions select_random_next_agents_acts(const std::vector<Point> &agents, const Field &field){
  Actions result;
  std::set<Action> cnt;
  for(const auto &agent : agents){
    assert(((field.get_state(agent) & State::Human) == State::Enemy) == (field.current_turn & 1));
    auto acts = enumerate_next_agent_acts(agent, field);
    const int acts_size = acts.size();
    // 建築を選びやすくする
    for(int i = 0; i < acts_size; i++) if(acts[i].command == Action::Build){
      for(int j = 0; j < 8; j++) acts.emplace_back(acts[i]);
    }
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
