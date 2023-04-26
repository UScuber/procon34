#include "base.hpp"
#include "mcts.hpp"

int main(){
  castles_coef = 100;
  area_coef = 10;
  wall_coef = 1;
  //std::cin >> height >> width;
  height = width = 8;
  Field field = create_random_field(height, width, 2, 1, 20*2);
  debug_field(field);
  while(!field.is_finished()){
    if(field.current_turn & 1){
      std::cout << "agent move\n";
      int idx = 0;
      for(const auto &p : field.enemy_agents){
        std::cout << "idx: " << idx << " " << p << "\n";
        idx++;
      }
      std::cout << "listen... (dir, command)\n";
      std::vector<Action> res;
      for(int i = 0; i < (int)field.enemy_agents.size(); i++){
        std::cout << "idx: " << i << " ";
        int dir; std::string str;
        std::cin >> dir >> str;
        uchar cmd = Action::None;
        if(str == "move") cmd = Action::Move;
        if(str == "build") cmd = Action::Build;
        if(str == "break") cmd = Action::Break;
        res.emplace_back(Action(field.enemy_agents[i] + dmove[dir], cmd, i));
      }
      field.update_turn(res);
    }else{
      const auto res = montecarlo_tree_search(field, 4096);
      field.update_turn(res);
    }
    debug_field(field);
  }
  std::cout << "Finished\n";
}