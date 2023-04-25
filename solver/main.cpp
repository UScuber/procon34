#include "base.hpp"
#include "mcts.hpp"

void debug_field(const Field &field){
  std::vector<std::string> board(height), wall(height), region(height);
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width; j++){
      const State s = field.get_state(i, j);
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
      wall[i] += c;
      // region
      c = '.';
      if((s & State::AreaAlly) && (s & State::AreaEnemy)) c = '$';
      else if(s & State::AreaAlly) c = '@';
      else if(s & State::AreaEnemy) c = '%';
      region[i] += c;
    }
  }
  std::cout << "board" << std::string(width-4, ' ') << ": walls" << std::string(width-4, ' ') << ": region\n";
  for(int i = 0; i < height; i++){
    std::cout << board[i] << " : " << wall[i] << " : " << region[i] << "\n";
  }
  std::cout << "\n";
}

int main(){
  castles_coef = 100;
  area_coef = 10;
  wall_coef = 1;
  //std::cin >> height >> width;
  height = width = 6;
  Field field = create_random_field(height, width, 2, 1, 10);
  debug_field(field);
  const auto res = montecarlo_tree_search(field, 2);
  field.update_turn(res);
  debug_field(field);
}