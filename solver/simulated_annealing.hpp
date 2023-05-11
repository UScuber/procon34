#include "base.hpp"
#include "timer.hpp"
#include <cmath>

namespace SimulatedAnnealing {

constexpr int max_depth = 3;
constexpr double limit_time = 1.0;

using Score = double;

bool is_searching_ally = false;


std::vector<Actions> enumerate_agent_moves(const Point &agent, const int agent_idx, const Field &field){
  std::vector<Actions> result;
  Actions current_acts;
  auto dfs = [&](auto &&self, const Point &agent, int dep){
    if(!current_acts.empty()) result.emplace_back(current_acts);
    if(!dep) return;
    for(const auto &act : enumerate_next_agent_acts(agent, field)){
      current_acts.emplace_back(act);
      current_acts.back().agent_idx = agent_idx;
      if(act.command == Action::Build || act.command == Action::Break){
        self(self, agent, dep - 1);
      }else{
        self(self, act.pos, dep - 1);
      }
      current_acts.pop_back();
    }
  };
  dfs(dfs, agent, max_depth);
  return result;
}

// side: 味方:0, 敵:1
// 動けないような移動が与えられた場合はNoneの行動になり、1が返る(建設や破壊は一応問題ない)
// 次に動かす敵の行動はすべてNone
std::vector<int> advance_field(const Actions &acts, Field &field){
  assert(acts.size() == field.ally_agents.size());
  std::vector<int> cannot_move(acts.size());
  Agents last_move_ally_agents, last_move_enemy_agents;
  Actions act_list[4];
  for(const auto &act : acts){
    act_list[act.command].emplace_back(act);
    assert(0 <= act.agent_idx && act.agent_idx < (int)acts.size());
  }
  if(!(field.current_turn & 1)){
    // break
    for(const auto &act : act_list[Action::Break]){
      const State st = field.get_state(act.pos);
      assert(!(st & State::Castle));
      if(!(st & State::Wall)){
        continue;
      }
      field.set_state(act.pos, st & ~State::Wall);
    }
    // build
    for(const auto &act : act_list[Action::Build]){
      const State st = field.get_state(act.pos);
      assert(!(st & State::Castle));
      // この操作だけ行動できないだけ
      if(st & (State::Wall | State::Enemy)){
        continue;
      }
      field.set_state(act.pos, st | State::WallAlly);
    }
    // move
    for(const auto &act : act_list[Action::Move]){
      const State st = field.get_state(act.pos);
      if(st & (State::Human | State::Pond | State::WallEnemy)){
        cannot_move[act.agent_idx]++;
        continue;
      }
      const auto from = field.ally_agents[act.agent_idx];
      last_move_ally_agents.emplace_back(from);
      //field.set_state(from, field.get_state(from) ^ State::Ally);
      field.set_state(act.pos, st | State::Ally);
      field.ally_agents[act.agent_idx] = act.pos;
    }
  }else{
    // break
    for(const auto &act : act_list[Action::Break]){
      const State st = field.get_state(act.pos);
      assert(!(st & State::Castle));
      if(!(st & State::Wall)){
        continue;
      }
      field.set_state(act.pos, st & ~State::Wall);
    }
    // build
    for(const auto &act : act_list[Action::Build]){
      const State st = field.get_state(act.pos);
      assert(!(st & State::Castle));
      // この操作だけ行動できないだけ
      if(st & (State::Wall | State::Ally)){
        continue;
      }
      field.set_state(act.pos, st | State::WallEnemy);
    }
    // move
    for(const auto &act : act_list[Action::Move]){
      const State st = field.get_state(act.pos);
      if(st & (State::Human | State::Pond | State::WallAlly)){
        cannot_move[act.agent_idx]++;
        continue;
      }
      const auto from = field.enemy_agents[act.agent_idx];
      last_move_enemy_agents.emplace_back(from);
      //field.set_state(from, field.get_state(from) ^ State::Enemy);
      field.set_state(act.pos, st | State::Enemy);
      field.enemy_agents[act.agent_idx] = act.pos;
    }
  }
  for(const auto &p : last_move_ally_agents){
    field.set_state(p, field.get_state(p) ^ State::Ally);
  }
  for(const auto &p : last_move_enemy_agents){
    field.set_state(p, field.get_state(p) ^ State::Enemy);
  }
  field.update_region();
  field.current_turn++;
  // enemy's act
  Actions enem_acts;
  for(int i = 0; i < (int)field.enemy_agents.size(); i++){
    enem_acts.emplace_back(Action(Point(), Action::None, i));
  }
  field.update_turn(enem_acts);

  return cannot_move;
}

Score calc_score(const std::vector<Actions> &agents_acts, Field field){
  const auto &agents = field.get_now_turn_agents();
  const int agents_num = agents.size();
  std::vector<int> cannot_move(agents_num);

  for(int i = 0; i < max_depth; i++){
    Actions current_acts;
    int idx = 0;
    for(const auto &acts : agents_acts){
      if(!cannot_move[idx] && (int)acts.size() > i){
        current_acts.emplace_back(acts[i]);
      }else{
        current_acts.emplace_back(Action(Point(), Action::None, idx));
      }
      idx++;
    }
    const auto cannot_state = advance_field(current_acts, field);
    for(int i = 0; i < agents_num; i++){
      cannot_move[i] |= cannot_state[i];
    }
  }
  return Evaluate::evaluate_field(field);
}


Actions SA(const Field &field){
  const auto &agents = field.get_now_turn_agents();
  std::vector<std::vector<Actions>> acts_cand;
  int agent_idx = 0;
  for(const auto &agent : agents){
    acts_cand.emplace_back(enumerate_agent_moves(agent, agent_idx, field));
    agent_idx++;
  }
  const int agents_num = agents.size();
  auto choice = [&](const std::vector<int> &indices) -> std::vector<Actions> {
    std::vector<Actions> result;
    int idx = 0;
    for(const int x : indices){
      result.emplace_back(acts_cand[idx][x]);
      idx++;
    }
    return result;
  };


  std::vector<int> current_indices(agents_num);
  auto good_indices = current_indices, best_indices = current_indices;

  Score current_score = calc_score(choice(current_indices), field);
  Score good_score = current_score;
  Score best_score = current_score;

  Timer timer;
  int trials = 0, updated_num = 0;
  static constexpr double t0 = 5;
  static constexpr double t1 = 0.0005;
  double temp = t0;

  std::cerr << "Start SA\n";
  std::cerr << "first score: " << current_score << "\n";
  for(; ; trials++){
    static constexpr int mask = (1 << 4) - 1;
    if(!(trials & mask)){
      const double p = timer.result() * 0.001 / limit_time;
      if(p > 1.0) break;
      temp = std::pow(t0, 1.0-p) * std::pow(t1, p);
    }

    // random change
    const int pos = rnd(agents_num);
    const int last_idx_val = current_indices[pos];
    current_indices[pos] = rnd((int)acts_cand[pos].size());
    const Score score = calc_score(choice(current_indices), field);
    if((score > best_score) ^ !is_searching_ally){
      best_score = score;
      good_score = score;
      best_indices = current_indices;
      good_indices = current_indices;
      updated_num++;
    }else if(std::exp((double)(score - good_score) * (is_searching_ally ? -1 : 1) / temp) > rnd(2048)/2048.0){
      //std::cerr << score - good_score << " ";
      good_score = score;
      good_indices = current_indices;
      updated_num++;
    }else{
      current_indices[pos] = last_idx_val;
    }
  }
  std::cerr << "trials : " << trials << "\n";
  std::cerr << "updated: " << updated_num << "\n";
  std::cerr << "score  : " << best_score << "\n";

  Actions result;
  int idx = 0;
  for(const int x : best_indices){
    if((int)acts_cand[idx].size() > x){
      result.emplace_back(acts_cand[idx][x].front());
    }else{
      result.emplace_back(Action(Point(), Action::None, idx));
    }
    idx++;
  }
  return result;
}


}

using SimulatedAnnealing::SA;
