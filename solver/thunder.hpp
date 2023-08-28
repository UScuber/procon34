#include "base.hpp"

namespace Thunder {

bool is_searching_ally = false;

struct Node {
  std::vector<Node> child_nodes;
  int n;
  Node(const Field &field) : field(field), w(0), n(0){}

  double evaluate(){
    if(field.is_finished()){
      const bool win = field.calc_final_score() > 0;
      const double score = win ? 10 : -10;
      w += score;
      n++;
      return score;
    }
    if(child_nodes.empty()){
      const double value = Evaluate::evaluate_field(field);
      w += value;
      n++;
      expand();
      return value;
    }else{
      const double value = nextChildNode().evaluate();
      w += value;
      n++;
      return value;
    }
  }

  std::vector<std::vector<Action>> expand(){
    child_nodes.clear();
    const auto legal_actions = enumerate_next_all_agents_acts(field.get_now_turn_agents(), field);
    for(const auto &acts : legal_actions){
      child_nodes.emplace_back(field);
      child_nodes.back().field.update_turn(acts);
    }
    return legal_actions;
  }
  Node &nextChildNode(){
    double t = 0;
    for(auto &child_node : child_nodes){
      if(!child_node.n) return child_node;
      t += child_node.n;
    }
    double best_value = -1e18 * (is_searching_ally ? 1 : -1);
    int best_action_idx = -1;
    for(int i = 0; i < (int)child_nodes.size(); i++){
      const auto &child_node = child_nodes[i];
      const double ucb1_val = child_node.w / child_node.n;
      if((ucb1_val > best_value) ^ !is_searching_ally){
        best_value = ucb1_val;
        best_action_idx = i;
      }
    }
    return child_nodes[best_action_idx];
  }
private:
  double w;
  Field field;
};



std::vector<Action> thunder_search(const Field &field, const int search_num){
  Node root_node = Node(field);
  const auto legal_actions = root_node.expand();
  for(int i = 0; i < search_num; i++){
    root_node.evaluate();
  }
  int best_act_searched_idx = -1;
  int best_act_idx = -1;
  Assert(legal_actions.size() == root_node.child_nodes.size());
  for(int i = 0; i < (int)legal_actions.size(); i++){
    const int n = root_node.child_nodes[i].n;
    if(n > best_act_searched_idx){
      best_act_searched_idx = n;
      best_act_idx = i;
    }
  }
  return legal_actions[best_act_idx];
}

}

using Thunder::thunder_search;