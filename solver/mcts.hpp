#include <cmath>
#include "base.hpp"

namespace Montecarlo {

constexpr double C = 1;
constexpr int max_expand = 20;


double random_playout(Field field){
  const bool side = field.current_turn & 1;
  while(!field.is_finished()){
    const auto acts = select_random_next_agents_acts(field.get_now_turn_agents(), field);
    field.update_turn(acts);
  }
  int final_score = field.calc_final_score();
  if(!side) final_score = -final_score;
  if(final_score > 0) return 1.0;
  if(final_score < 0) return 0.0;
  return 0.5;
}

struct Node {
  std::vector<Node> child_nodes;
  int n;
  Node(const Field &field) : field(field), w(0), n(0){}

  double evaluate(){
    if(field.is_finished()){
      double value = 0.5;
      const int score = field.calc_final_score();
      if(score > 0) value = 1.0;
      else if(score < 0) value = 0.0;
      w += value;
      n++;
      return value;
    }
    if(child_nodes.empty()){
      const double value = random_playout(field);
      w += value;
      n++;
      if(n >= max_expand) expand();
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
    double best_value = -1e18;
    int best_action_idx = -1;
    for(int i = 0; i < (int)child_nodes.size(); i++){
      const auto &child_node = child_nodes[i];
      const double ucb1_val = 1.0 - child_node.w / child_node.n + C * std::sqrt(2.0*std::log(t) / child_node.n);
      if(ucb1_val > best_value){
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



std::vector<Action> montecarlo_tree_search(const Field &field, const int search_num){
  Node root_node = Node(field);
  const auto legal_actions = root_node.expand();
  for(int i = 0; i < search_num; i++){
    root_node.evaluate();
  }
  //const auto legal_actions = enumerate_next_all_agents_acts(field.get_now_turn_agents(), field);
  int best_act_searched_idx = -1;
  int best_act_idx = -1;
  assert(legal_actions.size() == root_node.child_nodes.size());
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

using Montecarlo::montecarlo_tree_search;
