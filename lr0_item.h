#include <string>
#include <iostream>

using namespace std;

class Item {
  protected:
    int p_index;

    string nonterminal;
    string dot_left;
    string dot_right;

    string after_dot_symbol;

    string get_first_symbol(string beta) {
      string s = "";
      beta += "#";
      for (int i = 0; i < beta.length(); ++i) {
        if (beta[i] == ' ' || beta[i] == '#') {
          return s;
        } else {
          s += beta[i];
        }
      }
      return s;
    }

    void split_pright(string p, string& first_symbol, string& left_symbols) {
      first_symbol = "";
      left_symbols = "";
      int i = 0;
      for (; i < p.length() && p[i] != ' '; ++i) {
        first_symbol += p[i];
      }
      if (i < p.length()) {
        left_symbols = p.substr(i + 1);
      }
    }

  public:
    Item () {};

    Item(int p_i, string A, string beta) {
      p_index = p_i;
      nonterminal = A;
      dot_left = "";
      dot_right = beta;
      after_dot_symbol = get_first_symbol(beta);
    }

    Item(Item *parent) {
      p_index = parent->get_p_index();
      nonterminal = parent->get_nonterminal();

      string next_first_symbol, next_left_symbols;
      split_pright(parent->get_dot_right(), next_first_symbol, next_left_symbols);
      dot_left = parent->get_dot_left() + (parent->get_dot_left().length() == 0 ? "" : " ") + next_first_symbol;
      dot_right = next_left_symbols;
      after_dot_symbol = get_first_symbol(next_left_symbols);
    }


    int get_p_index() const { return p_index; }

    string get_nonterminal() const { return nonterminal; }

    string get_dot_left() const { return dot_left; }

    string get_dot_right() const { return dot_right; };

    string get_after_dot_symbol() const { return after_dot_symbol; }

    bool operator ==(const Item &x) const {
      cout << "lr0 item ==\n";
      return p_index == x.get_p_index() && dot_left == x.get_dot_left() && dot_right == x.get_dot_right();
    }

    bool operator <(const Item &rhs) const {
      if (p_index != rhs.get_p_index()) return p_index < rhs.get_p_index();
      return dot_right < rhs.get_dot_right();
    }

};