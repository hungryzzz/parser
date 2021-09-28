#include "lr_base.h"
#include "lr0_item.h"

using namespace std;

// DFA 状态
struct state {
  int id; // 状态编号
  set<Item> items; // 项目集
};
typedef state* State;

class LR0_Base : public LR_Base<state*> {
  public:
    LR0_Base(string filename) : LR_Base(filename) {}

    void new_state(State& s) { s = new state; }

    void init_state(State& s0) {
      Item item(find_production_by_string(S, original_S), S, original_S);
      s0->items.insert(item);
    }

    // 获取某个项目集闭包
    void get_item_closure(State& s) {
      set<Item> item_set = s->items;
      queue<string> symbols_queue;
      string after_dot_symbol, symbol;
      vector<string> rights;
      for (Item t: item_set) {
        after_dot_symbol = t.get_after_dot_symbol();
        if (is_nonterminal(after_dot_symbol)) {
          symbols_queue.push(after_dot_symbol);
        }
      }
      while (!symbols_queue.empty()) {
        symbol = symbols_queue.front();
        symbols_queue.pop();
        
        rights = productions[symbol];
        for (string r: rights) {
          Item item(find_production_by_string(symbol, r), symbol, r);
          if (!is_item_exist<Item>(item_set, item)) {
            item_set.insert(item);
            after_dot_symbol = item.get_after_dot_symbol();
            if (is_nonterminal(after_dot_symbol)) {
              symbols_queue.push(after_dot_symbol);
            }
          }
        }
      }
      s->items = item_set;
    }

    // utilities function
    void print_state(State s) {
      cout << "STATE " << s->id << endl;
      for (Item item: s->items) {
        cout << item.get_p_index() << "\t" << item.get_nonterminal() << " -> " << item.get_dot_left() << " . " << item.get_dot_right() << endl;
      }
      cout << endl;
    }
};
