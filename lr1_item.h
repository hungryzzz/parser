#include "lr0_item.h"
#include <iostream>
using namespace std;

class LR1_Item : public Item {
  private:
    string lookahead_s;
    string after_dot_left_symbols;

    void split_dot_right() {
      string first_symbol, left_symbols;
      split_pright(dot_right, first_symbol, left_symbols);
      after_dot_symbol = first_symbol;
      after_dot_left_symbols = left_symbols;
    }
  
  public:
    LR1_Item(int p_i, string A, string beta, string s): Item(p_i, A, beta) {
      lookahead_s = s;
      split_dot_right();
    }

    LR1_Item(LR1_Item *parent): Item(parent) {
      lookahead_s = parent->get_lookahead_s();
      split_dot_right();
    }

    string get_lookahead_s() const { return lookahead_s; }

    string get_after_dot_left_symbols() const { return after_dot_left_symbols; }

    bool operator ==(const LR1_Item &x) const {
      // cout << "lr1 item ==\n";
      return p_index == x.get_p_index() && 
             dot_left == x.get_dot_left() && 
             dot_right == x.get_dot_right() &&
             lookahead_s == x.get_lookahead_s();
    }

    bool operator <(const LR1_Item &rhs) const {
      if (p_index != rhs.get_p_index()) return p_index < rhs.get_p_index();
      if (lookahead_s != rhs.get_lookahead_s()) return lookahead_s < rhs.get_lookahead_s();
      return dot_right < rhs.get_dot_right();
    }
};