#include <time.h>
#include "base.hpp"
#include "mcts.hpp"
#include "thunder.hpp"

int main(){
  srand(time(NULL));
  castles_coef = 80;
  area_coef = 3;
  wall_coef = 2;
  //std::cin >> height >> width;
  height = width = 16;
  int side, vs_same;
  std::cout << "side(0/1): ";
  std::cin >> side;
  std::cout << "auto ? ";
  std::cin >> vs_same;
  Field field = create_random_field(height, width, 2+1, 1+3, 30*2);
  field.debug();
  while(!field.is_finished()){
    if((field.current_turn & 1) != side && !vs_same){
      std::cout << "agent move\n";
      int idx = 0;
      for(const auto &p : field.enemy_agents){
        std::cout << "idx: " << idx << " " << p << "\n";
        idx++;
      }
      std::cout << "listen... (dir, command)\n";
      Actions res;
      for(int i = 0; i < (int)field.enemy_agents.size(); i++){
        std::cout << "idx: " << i << ": ";
        int dir; std::string str;
        std::cin >> dir >> str;
        auto cmd = Action::None;
        if(str == "move") cmd = Action::Move;
        if(str == "build") cmd = Action::Build;
        if(str == "break") cmd = Action::Break;
        res.emplace_back(Action(field.get_now_turn_agents()[i] + dmove[dir], cmd, i));
      }
      field.update_turn(res);
      field.debug();
    }else{
      Montecarlo::is_searching_ally = !(field.current_turn & 1);
      Thunder::is_searching_ally = !(field.current_turn & 1);
      //const auto res = montecarlo_tree_search(field, 1 << 12+1);
      const auto res = (field.current_turn & 1) ? thunder_search(field, 1 << 9) : montecarlo_tree_search(field, 1 << 13);
      const int m = res.size();
      std::vector<int> dirs(m);
      std::vector<std::string> cmd(m, "none");
      for(int i = 0; i < m; i++){
        if(res[i].command == Action::Move) cmd[i] = "move";
        if(res[i].command == Action::Build) cmd[i] = "build";
        if(res[i].command == Action::Break) cmd[i] = "break";
        assert(i == res[i].agent_idx);
        for(int d = 0; d < 8; d++){
          if(field.get_now_turn_agents()[i] + dmove[d] == res[i].pos){
            dirs[i] = d;
            break;
          }
        }
      }
      field.update_turn(res);
      field.debug();
      for(int i = 0; i < m; i++){
        std::cout << "idx: " << i << ": " << dirs[i] << " " << cmd[i] << "\n";
      }
    }
  }
  std::cout << "Finished\n";
  std::cout << "Ally's Score: " << field.calc_final_score() << "\n";
}