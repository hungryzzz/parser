#include "lr0_base.h"

class SLR1 : public LR0_Base {
  public:
    SLR1(string filename): LR0_Base(filename) {}
    
    // 判断是否为SLR1文法
    bool judge_slr1() {
      map<string, set<Item> > item_cluster;
      string after_dot_symbol;
      set<string> follow_symbols, duplicate_symbols;
      for (State s: states) {
        item_cluster.clear();
        // 将有相同dot前缀的项目进行聚类
        for (Item item: s->items) {
          if (item.get_dot_left().length() != 0) {
            item_cluster[item.get_dot_left()].insert(item);
          }
        }
        // 判断每个状态的项目集是否存在冲突
        for (map<string, set<Item> >::iterator it = item_cluster.begin(); it != item_cluster.end(); ++it) {
          duplicate_symbols.clear();
          for (Item item: it->second) {
            after_dot_symbol = item.get_after_dot_symbol();
            if (after_dot_symbol.length() && is_terminal(after_dot_symbol)) {
              // 如果当前项目为移进项目，将点后字符加入集合中
              duplicate_symbols.insert(after_dot_symbol);
            } 
          }
          for (Item item: it->second) {
            if (item.get_after_dot_symbol().length() == 0) {
              // 如果当前项目为规约项目，检查FOLLOW集中是否有已经出现的非终结符
              follow_symbols = FOLLOW[item.get_nonterminal()];
              for (string symbol: follow_symbols) {
                if (duplicate_symbols.find(symbol) != duplicate_symbols.end()) {
                  return false;
                }
                duplicate_symbols.insert(symbol);
              }
            }
          }
        }
      }
      return true;
    }

    // 根据DFA构建SLR1分析表
    bool get_table() {
      get_FIRST();
      get_FOLLOW();
      build_DFA<Item>();
      if (!judge_slr1()) {
        cout << "该文法不是SLR(1)文法\n";
        return false;
      }

      string after_dot_symbol, reduce_p;
      pair<int, string> t;
      set<string> follow_symbols;
      for (State curr_s: states) {
        for (Item curr_item: curr_s->items) {
          after_dot_symbol = curr_item.get_after_dot_symbol();
          if (after_dot_symbol.length() == 0) {
            // A -> a.
            t = make_pair(curr_s->id, end);
            if (curr_item.get_nonterminal() == S && curr_item.get_dot_left() == original_S && curr_item.get_dot_right().length() == 0) {
              // 接受项目，S -> E.
              action_table[t] = "acc";
            } else {
              // 规约项目，FOLLOW(A)
              reduce_p = "r " + to_string(curr_item.get_p_index());
              follow_symbols = FOLLOW[curr_item.get_nonterminal()];
              for (string s: follow_symbols) {
                action_table[make_pair(curr_s->id, s)] = reduce_p;
              }
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
  string filename = "Grammars/slr1_grammar.txt";
  SLR1 slr1(filename);
  slr1.parser("Grammars/slr1_tokens.txt");
  return 1;
}