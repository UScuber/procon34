#include "base.hpp"


int calc_final_score(const Field &field){
  int ally_walls = 0, enemy_walls = 0;
  int ally_area = 0, enemy_area = 0;
  int allys_castle = 0, enemys_castle = 0;
  // 領域計算
  for(int i = 0; i < height; i++){
    for(int j = 0; j < width; j++){
      const State s = field.get_state(i, j);
      if(s & State::WallAlly) ally_walls++;
      if(s & State::WallEnemy) enemy_walls++;
      if(s & State::AreaAlly) ally_area++;
      if(s & State::AreaEnemy) enemy_area++;
      if(s & (State::Castle | State::AreaAlly)) allys_castle++;
      if(s & (State::Castle | State::AreaEnemy)) enemys_castle++;
    }
  }
  int score = (ally_walls-enemy_walls) + (ally_area-enemy_area)*10 + (allys_castle-enemys_castle)*100;
  return score;
}
