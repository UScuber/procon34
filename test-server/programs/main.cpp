#include <time.h>
#include "base.hpp"


Field read_field(const int h, const int w, const int agents_num){
  Field field(h, w);
  // structures
  for(int i = 0; i < h; i++){
    for(int j = 0; j < w; j++){
      int val;
      std::cin >> val;
      if(val == 1) field.set_state(i, j, field.get_state(i, j) | State::Pond);
      if(val == 2) field.set_state(i, j, field.get_state(i, j) | State::Castle);
    }
  }
  // walls
  for(int i = 0; i < h; i++){
    for(int j = 0; j < w; j++){
      int val;
      std::cin >> val;
      if(val == 1) field.set_state(i, j, field.get_state(i, j) | State::WallAlly);
      if(val == 2) field.set_state(i, j, field.get_state(i, j) | State::WallEnemy);
    }
  }
  // region
  for(int i = 0; i < h; i++){
    for(int j = 0; j < w; j++){
      int val;
      std::cin >> val;
      if(val & 1) field.set_state(i, j, field.get_state(i, j) | State::AreaAlly);
      if(val & 2) field.set_state(i, j, field.get_state(i, j) | State::AreaEnemy);
    }
  }
  // agents
  Agents ally_agents(agents_num), enemy_agents(agents_num);
  for(int i = 0; i < h; i++){
    for(int j = 0; j < w; j++){
      int val;
      std::cin >> val;
      if(!val) continue;
      if(val > 0){
        ally_agents[val-1] = Point(i, j);
      }else{
        enemy_agents[-val-1] = Point(i, j);
      }
    }
  }
  for(const Point agent : ally_agents){
    field.set_state(agent, field.get_state(agent) | State::Ally);
  }
  for(const Point agent : enemy_agents){
    field.set_state(agent, field.get_state(agent) | State::Enemy);
  }
  field.ally_agents = ally_agents;
  field.enemy_agents = enemy_agents;
  return field;
}


int main(){
  srand(time(NULL));
  // start reading field
  int agents_num, current_turn;
  std::cin >> height >> width >> agents_num >> current_turn;
  Field field = read_field(height, width, agents_num);
  const auto &current_agents = field.get_now_turn_agents();
  const Actions actions = select_random_next_agents_acts(current_agents, field);
  assert(field.is_legal_action(actions));

  const std::vector<int> cmd_perm = { 0,3,2,1 };
  const std::vector<int> dir_perm = { 1,7,5,3,0,6,4,2 };

  std::cout << "{ \"turn\": " << current_turn << ", \"actions\": [";

  for(int i = 0; i < agents_num; i++){
    assert(i == actions[i].agent_idx);
    if(actions[i].command == Action::None){
      std::cout << "0 0\n";
      continue;
    }
    int dir = -1;
    for(int d = 0; d < 8; d++){
      if(current_agents[i] + dmove[d] == actions[i].pos){
        dir = d;
        break;
      }
    }
    assert(dir != -1);
    
    std::cout << "{\"type\": " << cmd_perm[actions[i].command] << ",\"dir\": " << dir_perm[dir] + 1 << "}";
    if(i != agents_num - 1) std::cout << ",";
  }
  std::cout << "]}\n";
}