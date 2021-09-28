#include "lr_base.h"
#include "lr1_item.h"

using namespace std;

// DFA 状态
struct state {
  int id; // 状态编号
  set<LR1_Item> items; // 项目集
};
typedef state* State;

class LR1 : public LR_Base<state*> {
  public:
    LR1(string filename) : LR_Base(filename) {}

    void new_state(State& s) { s = new state; }

    void init_state(State& s0) {
      LR1_Item item(find_production_by_string(S, original_S), S, original_S, end);
      s0->items.insert(item);
    }

    // 获取某个项目集闭包
    void get_item_closure(State& s) {
      set<LR1_Item> item_set = s->items;
      queue<pair<string, string> > symbols_queue;
      string after_dot_symbol, after_dot_left_symbols, B, beta_a;
      vector<string> rights;
      set<string> first_set;
      for (LR1_Item t: item_set) {
        after_dot_symbol = t.get_after_dot_symbol();
        if (is_nonterminal(after_dot_symbol)) {
          after_dot_left_symbols = t.get_after_dot_left_symbols();
          symbols_queue.push(
            make_pair(
              after_dot_symbol, 
              after_dot_left_symbols + (after_dot_left_symbols.length() == 0 ? "" : " ") + t.get_lookahead_s()));
        }
      }

      while (!symbols_queue.empty()) {
        B = symbols_queue.front().first;
        beta_a = symbols_queue.front().second;
        symbols_queue.pop();

        rights = productions[B];
        first_set.clear();
        get_alpha_first(beta_a, first_set);
        for (string r: rights) {
          for (string f: first_set) {
            LR1_Item item(find_production_by_string(B, r), B, r, f);
            // cout << "当前非终结符: " << f << endl;
            if (!is_item_exist<LR1_Item>(item_set, item)) {
              item_set.insert(item);
              after_dot_symbol = item.get_after_dot_symbol();
              if (is_nonterminal(after_dot_symbol)) {
                after_dot_left_symbols = item.get_after_dot_left_symbols();
                symbols_queue.push(
                  make_pair(
                    after_dot_symbol, 
                    after_dot_left_symbols + (after_dot_left_symbols.length() == 0 ? "" : " ") + item.get_lookahead_s()));
              }
            }
          }
        }
      }
      s->items = item_set;
    }

    // 根据DFA构建LR0分析表
    bool get_table() {
      get_FIRST();
      build_DFA<LR1_Item>();

      string after_dot_symbol, reduce_p, lookahead_s, shift_s;
      pair<int, string> t;
      for (State curr_s: states) {
        for (LR1_Item curr_item: curr_s->items) {
          after_dot_symbol = curr_item.get_after_dot_symbol();
          if (after_dot_symbol.length() == 0) {
            // A -> a.
            lookahead_s = curr_item.get_lookahead_s();
            t = make_pair(curr_s->id, lookahead_s);
            if (curr_item.get_nonterminal() == S && 
                curr_item.get_dot_left() == original_S && 
                curr_item.get_dot_right().length() == 0 &&
                lookahead_s == end) {
              // 接受项目，S -> E.
              action_table[t] = "acc";
            } else {
              // 规约项目，查看lookahead, [A -> a., b]
              if (action_table.find(t) != action_table.end()) {
                return false;
              }
              reduce_p = "r " + to_string(curr_item.get_p_index());
              action_table[t] = reduce_p;
            }
          } else if (is_terminal(after_dot_symbol) && go_function.find(t = make_pair(curr_s->id, after_dot_symbol)) != go_function.end()) {
            shift_s = "s " + to_string(go_function[t]);
            if (action_table.find(t) != action_table.end() && action_table[t] != shift_s) {
              return false;
            }
            // A -> .a  移进项目
            action_table[t] = shift_s;
          }
          if (go_function.find(t = make_pair(curr_s->id, curr_item.get_nonterminal())) != go_function.end()) {
            if (goto_table.find(t) != goto_table.end() && goto_table[t] != go_function[t]) {
              return false;
            }
            goto_table[t] = go_function[t];
          }
        }
      }
      return true;
    }

    // utilities function
    void print_state(State s) {
      cout << "STATE " << s->id << endl;
      for (LR1_Item item: s->items) {
        cout << item.get_p_index() << "\t" << item.get_nonterminal() << " -> " << item.get_dot_left() << " . " << item.get_dot_right() << " , " << item.get_lookahead_s() << endl;
      }
      cout << endl;
    }

    void print_items(set<LR1_Item> items) {
      for (LR1_Item item: items) {
        cout << item.get_p_index() << "\t" << item.get_nonterminal() << " -> " << item.get_dot_left() << " . " << item.get_dot_right() << " , " << item.get_lookahead_s() << endl;
      }
      cout << endl;
    }
};

int main() {
  string filename = "Grammars/lr1_grammar.txt";
  LR1 lr1(filename);
  lr1.parser("Grammars/lr1_tokens.txt");
  return 1;
}