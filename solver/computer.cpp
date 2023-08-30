#include <time.h>
#include "base.hpp"
//#include "thunder.hpp"
//#include "simulated_annealing.hpp"
#include "tsp.hpp"

struct Game {
  Field field;
  Game(const Field &f) : field(f){}
  void run(){
    assert(field.is_my_turn());
    const auto &current_agents = field.get_now_turn_agents();
    //Thunder::is_searching_ally = !(field.current_turn & 1);
    //const auto res = thunder_search(field, 1 << 9);
    //SimulatedAnnealing::is_searching_ally = !(field.current_turn & 1);
    //const auto res = SA(field);
    cerr << "run\n";
    // ** board 17 only
    const std::vector<Point> build_walls = {
      {11, 19},
      {12, 18},
      {13, 17},
      {14, 16},
      {15, 15},
      {16, 14},
      {17, 13},
      {18, 12},
      {19, 11},
      {20, 12},
      {21, 13},
      {22, 14},
      {21, 15},
      {20, 16},
      {19, 17},
      {18, 18},
      {17, 19},
      {16, 20},
      {15, 21},
      {14, 22},
      {13, 21},
      {12, 20},
    };
    const auto res = calculate_build_route(build_walls, field);
    const int m = res.size();
    std::vector<int> dirs(m);
    std::vector<std::string> cmd(m, "none");
    for(int i = 0; i < m; i++){
      if(res[i].command == Action::Move) cmd[i] = "move";
      if(res[i].command == Action::Build) cmd[i] = "build";
      if(res[i].command == Action::Break) cmd[i] = "break";
      assert(i == res[i].agent_idx);
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
    assert(!field.is_my_turn());
    const auto &current_agents = field.get_now_turn_agents();
    std::vector<Action> res;
    cerr << "enemy turn\n";
    for(int i = 0; i < (int)current_agents.size(); i++){
      int dir; std::string str;
      std::cin >> dir >> str;
      cerr << dir << " " << str << "\n";
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