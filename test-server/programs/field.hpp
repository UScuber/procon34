#pragma once

#include <queue>
#include <map>
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

  Field(const int h, const int w, const bool s)
    : field(h, std::vector<State>(w, State::None)),
      current_turn(0),
      side(s),
      final_turn(1){}
  
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

  // s: 味方:0, 敵:1
  bool is_legal_action(const Actions &acts, int s = -1) const{
    if(s == -1) s = side;
    assert(acts.size() == ally_agents.size());
    Actions act_list[4];
    std::map<Point, int> agent_poses;

    for(const auto &act : acts){
      act_list[act.command].emplace_back(act);
      assert(0 <= act.agent_idx && act.agent_idx < (int)acts.size());
      if(act.command == Action::Move) agent_poses[act.pos]++;
    }
    
    // ally turn
    if(!side){
      for(const Point agent : ally_agents) agent_poses[agent]++;
      // break
      for(const auto &act : act_list[Action::Break]){
        const State st = get_state(act.pos);
        if(!(st & State::Wall)) return false;
      }
      // build
      for(const auto &act : act_list[Action::Build]){
        const State st = get_state(act.pos);
        if(st & (State::WallEnemy | State::Enemy | State::Castle)) return false;
        if(st & State::WallAlly){ // someone already built
          continue;
        }
      }
      // move
      for(const auto &act : act_list[Action::Move]){
        const State st = get_state(act.pos);
        if(st & (State::Human | State::Pond | State::WallEnemy)) return false;
        if(agent_poses[act.pos] >= 2) return false;
      }
    }
    // enemy turn
    else{
      for(const Point agent : enemy_agents) agent_poses[agent]++;
      // break
      for(const auto &act : act_list[Action::Break]){
        const State st = get_state(act.pos);
        if(!(st & State::Wall)) return false;
      }
      // build
      for(const auto &act : act_list[Action::Build]){
        const State st = get_state(act.pos);
        if(st & (State::WallAlly | State::Ally | State::Castle)) return false;
        if(st & State::WallEnemy){ // someone already built
          continue;
        }
      }
      // move
      for(const auto &act : act_list[Action::Move]){
        const State st = get_state(act.pos);
        if(st & (State::Human | State::Pond | State::WallAlly)) return false;
        if(agent_poses[act.pos] >= 2) return false;
      }
    }
    return true;
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
    cerr << "board" << std::string(width-4, ' ') << ": walls" << std::string(width-4, ' ') << ": region\n";
    for(int i = 0; i < height; i++){
      cerr << board[i] << " : " << wall[i] << " : " << region[i] << "\n";
    }
    cerr << "\n";
  }
};
