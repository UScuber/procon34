#include "base.hpp"

int main(){
  castles_coef = 100;
  area_coef = 10;
  wall_coef = 1;
  std::cin >> height >> width;
  Field field = create_random_field(height, width);
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width; j++){
      const State s = field.get_state(i, j);
      char c = '.';
      if(s & State::Pond) c = '#';
      else if(s & State::Ally) c = '@';
      else if(s & State::Enemy) c = '%';
      else if(s & State::Castle) c = '$';
      std::cout << c;
    }
    std::cout << "\n";
  }
}