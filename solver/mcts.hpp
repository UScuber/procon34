#include <cmath>
#include "base.hpp"

namespace Montecarlo {

constexpr double C = 1;
constexpr int max_expand = 16;
constexpr int max_playout_num = 6;

bool is_searching_ally = false;

double random_playout(Field field){
  const bool side = field.current_turn & 1;
  int cnt = 0;
  while(!field.is_finished() && cnt++ < max_playout_num){
    const auto acts = select_random_next_agents_acts(field.get_now_turn_agents(), field);
    field.update_turn(acts);
  }
  //int final_score = field.calc_final_score();
  const double final_score = Evaluate::evaluate_field(field);
  return final_score;
}

struct Node {
  std::vector<Node> child_nodes;
  int n;
  Node(const Field &field) : field(field), w(0), n(0){}

  double evaluate(){
    if(field.is_finished()){
      //const int score = field.calc_final_score();
      const double score = Evaluate::evaluate_field(field);
      w += score;
      n++;
      return score;
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
    double best_value = -1e18 * (is_searching_ally ? 1 : -1);
    int best_action_idx = -1;
    for(int i = 0; i < (int)child_nodes.size(); i++){
      const auto &child_node = child_nodes[i];
      const double ucb1_val = child_node.w / child_node.n + C * std::sqrt(2.0*std::log(t) / child_node.n);
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



std::vector<Action> montecarlo_tree_search(const Field &field, const int search_num){
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

using Montecarlo::montecarlo_tree_search;
