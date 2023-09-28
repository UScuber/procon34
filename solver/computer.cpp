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
      {14, 5},
      {15, 4},
      {16, 3},
      {17, 2},
      {18, 2},
      {19, 3},
      {20, 3},
      {21, 3},
      {22, 3},
      {23, 4},
      {24, 5},
      {23, 6},
      {22, 7},
      {21, 8},
      {22, 9},
      {22, 10},
      {22, 11},
      {21, 12},
      {21, 13},
      {22, 14},
      {22, 15},
      {21, 16},
      {20, 17},
      {19, 18},
      {18, 19},
      {17, 20},
      {16, 21},
      {15, 22},
      {14, 23},
      {13, 24},
      {12, 23},
      {11, 22},
      {10, 23},
      {9, 24},
      {8, 23},
      {7, 22},
      {6, 21},
      {5, 21},
      {4, 21},
      {3, 21},
      {3, 20},
      {2, 19},
      {1, 18},
      {0, 17},
      {1, 16},
      {2, 15},
      {1, 14},
      {2, 13},
      {1, 12},
      {2, 11},
      {3, 10},
      {2, 9},
      {3, 8},
      {4, 7},
      {5, 6},
      {6, 5},
      {7, 4},
      {8, 3},
      {9, 2},
      {10, 1},
      {11, 2},
      {12, 3},
      {13, 4},
      {14, 5},
      {7, 4},
      {6, 5},
      {5, 6},
      {4, 7},
      {3, 8},
      {2, 9},
    };
    Actions res = calculate_build_route(build_walls, field);
    const int m = res.size();
    std::vector<int> dirs(m);
    std::vector<std::string> cmd(m, "none");
    for(int i = 0; i < m; i++){
      assert(i == res[i].agent_idx);
      for(int d = 0; d < 8; d++){
        if(current_agents[i] + dmove[d] == res[i].pos){
          dirs[i] = d;
          break;
        }
      }
    }
    field.update_turn_and_fix_actions(res);
    for(int i = 0; i < m; i++){
      if(res[i].command == Action::Move) cmd[i] = "move";
      if(res[i].command == Action::Build) cmd[i] = "build";
      if(res[i].command == Action::Break) cmd[i] = "break";
    }
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
      Timer timer;
      game.run();
      cerr << "Elapsed Time: " << timer.result() << "[ms]\n";
    }else{
      game.load();
    }
  }
  cerr << "Finished\n";
}