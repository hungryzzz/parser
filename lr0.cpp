#include "lr0_base.h"

class LR0 : public LR0_Base {
  public:
    LR0(string filename): LR0_Base(filename) {}

    // 判断是否为LR0文法
    bool judge_lr0() {
      map<string, set<Item> > item_cluster;
      bool shift_exist, reduce_exist;
      for (State s: states) {
        item_cluster.clear();
        shift_exist = false;
        reduce_exist = false;
        // 将有相同dot前缀的项目进行聚类
        for (Item item: s->items) {
          if (item.get_dot_left().length() != 0) {
            item_cluster[item.get_dot_left()].insert(item);
          }
        }
        // 判断每个状态的项目集是否存在冲突
        for (map<string, set<Item> >::iterator it = item_cluster.begin(); it != item_cluster.end(); ++it) {
          for (Item item: it->second) {
            if (item.get_after_dot_symbol().length() == 0) {
              // 如果当前项目为规约项目
              // 如果已经存在规约项目，则冲突
              if (reduce_exist) return false;
              else reduce_exist = true;
              // 如果已经存在移进项目，则冲突
              if (shift_exist) return false;
            } else if (is_terminal(item.get_after_dot_symbol())) {
              // 如果当前项目为移进项目
              if (reduce_exist) return false;
              shift_exist = true;
            }
          }
        }
      }
      return true;
    }

    // 根据DFA构建LR0分析表
    bool get_table() {
      build_DFA<Item>();
      if (!judge_lr0()) {
        cout << "该文法不是LR(0)文法\n";
        return false;
      }

      string after_dot_symbol, reduce_p;
      pair<int, string> t;
      for (State curr_s: states) {
        for (Item curr_item: curr_s->items) {
          after_dot_symbol = curr_item.get_after_dot_symbol();
          if (after_dot_symbol.length() == 0) {
            // A -> a.
            t = make_pair(curr_s->id, end);
            if (curr_item.get_nonterminal() == S && curr_item.get_dot_left() == original_S && curr_item.get_dot_right().length() == 0) {
              // 接受项目
              action_table[t] = "acc";
            } else {
              // 规约项目，所有的终结符
              reduce_p = "r " + to_string(curr_item.get_p_index());
              for (int k = 0; k < terminals.size(); ++k) {
                action_table[make_pair(curr_s->id, terminals[k])] = reduce_p;
              }
              action_table[t] = reduce_p;
            }
          } else if (is_terminal(after_dot_symbol) && go_function.find(t = make_pair(curr_s->id, after_dot_symbol)) != go_function.end()) {
            // A -> .a  移进项目
            action_table[t] = "s " + to_string(go_function[t]);
          }
          if (go_function.find(t = make_pair(curr_s->id, curr_item.get_nonterminal())) != go_function.end()) {
            goto_table[t] = go_function[t];
          }
        }
      }
      return true;
    }
};

int main() {
  string filename = "Grammars/lr0_grammar.txt";
  LR0 lr0(filename);
  lr0.parser("Grammars/lr0_tokens.txt");
  return 1;
}