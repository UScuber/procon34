#include <time.h>
#include "base.hpp"
//#include "thunder.hpp"
#include "simulated_annealing.hpp"


struct Game {
  Field field;
  Game(const Field &f) : field(f){}
  void run(){
    Assert(field.is_my_turn());
    const auto &current_agents = field.get_now_turn_agents();
    //Thunder::is_searching_ally = !(field.current_turn & 1);
    //const auto res = thunder_search(field, 1 << 9);
    SimulatedAnnealing::is_searching_ally = !(field.current_turn & 1);
    const auto res = SA(field);
    const int m = res.size();
    std::vector<int> dirs(m);
    std::vector<std::string> cmd(m, "none");
    for(int i = 0; i < m; i++){
      if(res[i].command == Action::Move) cmd[i] = "move";
      if(res[i].command == Action::Build) cmd[i] = "build";
      if(res[i].command == Action::Break) cmd[i] = "break";
      Assert(i == res[i].agent_idx);
      for(int d = 0; d < 8; d++){
        if(current_agents[i] + dmove[d] == res[i].pos){
          dirs[i] = d;
          break;
        }
      }
    }
    field.update_turn(res);
    field.debug();
    cerr << "my turn\n";
    for(int i = 0; i < m; i++){
      std::cout << dirs[i] << " " << cmd[i] << "\n";
      cerr << dirs[i] << " " << cmd[i] << "\n";
    }
  }

  void load(){
    Assert(!field.is_my_turn());
    const auto &current_agents = field.get_now_turn_agents();
    std::vector<Action> res;
    cerr << "enemy turn\n";
    for(int i = 0; i < (int)current_agents.size(); i++){
      int dir; std::string str;
      std::cin >> dir >> str;
      uchar cmd = Action::None;
      if(str == "move") cmd = Action::Move;
      if(str == "build") cmd = Action::Build;
      if(str == "break") cmd = Action::Break;
      res.emplace_back(Action(current_agents[i] + dmove[dir], cmd, i));
    }
    field.update_turn(res);
    field.debug();
  }
};



int main(){
  srand(time(NULL));
  std::cin >> height >> width;
  Field field = read_field(height, width);
  field.debug();

  Game game(field);
  while(!game.field.is_finished()){
    if(game.field.is_my_turn()){
      game.run();
    }else{
      game.load();
    }
  }
  cerr << "Finished\n";
}