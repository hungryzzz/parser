#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

using namespace std;

class Grammar {
  protected:
    string end;

    vector<string> terminals;  // 终结符
    vector<string> nonterminals;  // 非终结符
    string S;  // 开始符号

    map<string, vector<string> > productions;  // 产生式
    map<string, set<string> > FIRST;  // FIRST集
    map<string, set<string> > FOLLOW;  // FOLLOW集

    bool is_nonterminal(string s) {
      return find(nonterminals.begin(), nonterminals.end(), s) != nonterminals.end();
    }

    bool is_terminal(string s) {
      return find(terminals.begin(), terminals.end(), s) != terminals.end();
    }

    void add_productions(string left, string right) {
      string p = "";
      for (int i = 0; i < right.size(); ++i) {
        if (right[i] == '|') {
          productions[left].push_back(p);
          p = "";
        } else {
          p += right[i];
        }
      }
      productions[left].push_back(p);
    }

    void add_terminals() {
      for (map<string, vector<string> >::iterator i = productions.begin(); 
          i != productions.end(); ++i) {
        vector<string> rights = i->second;
        for (int j = 0; j < rights.size(); ++j) {
          string r = rights[j] + "#";
          string t = "";
          for (int k = 0; k < r.length(); ++k) {
            if (r[k] == ' ' || r[k] == '#') {
              if (find(nonterminals.begin(), nonterminals.end(), t) == nonterminals.end() &&
                  find(terminals.begin(), terminals.end(), t) == terminals.end() &&
                  t != "@") {
                terminals.push_back(t);
              }
              t = "";
            } else {
              t += r[k];
            }
          }
        }
      }
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

    // FIRST(X) = FIRST(X) U (FIRST(Y) \ {EMPTY})
    void add_set_except_empty(set<string>& FIRST_X, set<string>& FIRST_Y) {
      for (set<string>::iterator it = FIRST_Y.begin(); it != FIRST_Y.end(); ++it) {
        if (*it != "@") {
          FIRST_X.insert(*it);
        }
      }
    }

    bool exist_empty_in_first(set<string> s) {
      return s.find("@") != s.end();
    }

    void print_set(set<string> s) {
      for (string a: s) {
        cout << a << ' ';
      }
      cout << endl;
    }

    void get_alpha_first_with_recursion(string X, string alpha, map<string, bool>& first_over) {
      if (alpha.length() == 0) return;
      string s = alpha, first_symbol, left_symbols;
      while (s.length() > 0) {
        split_pright(s, first_symbol, left_symbols);
        if (first_over.find(first_symbol) == first_over.end() || !first_over[first_symbol]) {
          get_nonterminals_first(first_symbol, first_over);
        }
        // 将FIRST[Y1]中所有非空元素 加入 FIRST[X]
        add_set_except_empty(FIRST[X], FIRST[first_symbol]);
        // FIRST[Y1]中存在空元素，则继续循环
        if (exist_empty_in_first(FIRST[first_symbol])) {
          s = left_symbols;
        } else {
          break;
        }
      }
      if (s.length() == 0) {
        FIRST[X].insert("@");
      }
    }

    // 无法处理存在递归的文法
    void get_nonterminals_first(string X, map<string, bool>& first_over) {
      string Y, first_symbol, left_symbols;
      vector<string> rights = productions[X], waiting_handle_rights;
      for (string Y: rights) {
        split_pright(Y, first_symbol, left_symbols);
        if (first_symbol == X) {
          waiting_handle_rights.push_back(Y);
          continue;
        }
        get_alpha_first_with_recursion(X, Y, first_over);
      }
      if (exist_empty_in_first(FIRST[X]) && waiting_handle_rights.size() > 0) {
        for (string s: waiting_handle_rights) {
          split_pright(s, first_symbol, left_symbols);
          get_alpha_first_with_recursion(X, left_symbols, first_over);
        }
      }
      first_over[X] = true;
    }

    // 获取alpha的FIRST集
    void get_alpha_first(string alpha, set<string>& alpha_first) {
      if (alpha.length() == 0) return;
      string s = alpha, first_symbol, left_symbols;
      while (s.length() > 0) {
        split_pright(s, first_symbol, left_symbols);
        add_set_except_empty(alpha_first, FIRST[first_symbol]);
        if (exist_empty_in_first(FIRST[first_symbol])) {
          s = left_symbols;
        } else {
          break;
        }
      }
      if (s.length() == 0) {
        alpha_first.insert("@");
      }
    }

    // Y <= X ?
    bool is_contain_sets(set<string> X, set<string> Y) {
      set<string> result;
      set_difference(Y.begin(), Y.end(), X.begin(), X.end(), inserter(result, result.begin()));
      return result.empty() || (result.size() == 1 && result.find("@") != result.end());
    }

  public:
    Grammar() {
      end = "#";
    }

    // 读入文法产生式
    void read_grammar(string filename) { 
      ifstream ifs(filename);
      if (ifs) {
        string line;
        int delimiter_pos;
        while (getline(ifs, line)) {
          delimiter_pos = line.find("->");
          string left = line.substr(0, delimiter_pos);
          string right = line.substr(delimiter_pos+2, line.size()-delimiter_pos);
          nonterminals.push_back(left);
          add_productions(left, right);
        }
        add_terminals();
        S = nonterminals[0];
      } else {
        cout << "Read " << filename << " Failed!\n";
      }
      ifs.close();

    }

    // get token
    string get_next_token(ifstream& ifs) {
      char c;
      string token = "";
      while (ifs.peek() != EOF) {
        c = ifs.peek();
        if (c == '\n' || c == ' ') {
          ifs.get();
          return token;
        } else {
          token += c;
          ifs.get();
        }
      }
      if (ifs.peek() == EOF && token.length() == 0) {
        ifs.get();
        return end;
      }
      return token;
    }

    // 获取FIRST集合 
    void get_FIRST() {
      FIRST.clear();
      map<string, bool> first_over;
      string A, alpha, first_symbol, left_symbols;
      vector<string> rights;

      FIRST["@"].insert("@");
      first_over["@"] = true;

      FIRST[end].insert(end);
      first_over[end] = true;

      // 终结符
      for (int i = 0; i < terminals.size(); ++i) {
        FIRST[terminals[i]].insert(terminals[i]);
        first_over[terminals[i]] = true;
      }

      // 非终结符
      for (int i = 0; i < nonterminals.size(); ++i) {
        get_nonterminals_first(nonterminals[i], first_over);
      }
    }

    // 获取FOLLOW集
    void get_FOLLOW() {
      FOLLOW.clear();
      // 将结束符号加入开始符号的FOLLOW集中
      FOLLOW[S].insert("#");

      string A, right, B, beta;
      vector<string> rights;
      set<string> beta_first;
      bool stop;

      while (true) {
        stop = true;
        for (int i = 0; i < nonterminals.size(); ++i) {
          A = nonterminals[i];
          rights = productions[A];
          for (int j = 0; j < rights.size(); ++j) {
            right = rights[j];
            while (right.length() > 0) {
              split_pright(right, B, beta);
              if (is_nonterminal(B)) {
                beta_first.clear();
                if (beta.length() > 0) {
                  // 求beta的FIRST集，加入到B的FOLLOW集中
                  get_alpha_first(beta, beta_first);
                  if (!is_contain_sets(FOLLOW[B], beta_first)) {
                    add_set_except_empty(FOLLOW[B], beta_first);
                    stop = false;
                  }
                }
                if (exist_empty_in_first(beta_first) || beta.length() == 0) {
                  if (!is_contain_sets(FOLLOW[B], FOLLOW[A])) {
                    add_set_except_empty(FOLLOW[B], FOLLOW[A]);
                    stop = false;
                  }
                }
              }
              right = beta;
            }
          }
        }
        if (stop) break;
      }
      // 删除没有被用到的非终结符
      vector<string> del_nonterminals;
      string s;
      for (int i = 0; i < nonterminals.size(); ++i) {
        s = nonterminals[i];
        if (FOLLOW.find(s) == FOLLOW.end() || FOLLOW[s].size() == 0) {
          del_nonterminals.push_back(s);
        }
      }
      for (int i = 0; i < del_nonterminals.size(); ++i) {
        s = del_nonterminals[i];
        nonterminals.erase(remove(nonterminals.begin(), nonterminals.end(), s), nonterminals.end());
        productions.erase(productions.find(s));
        FIRST.erase(FIRST.find(s));
      }
    }

    // utilities function
    void print_non_and_terminals() {
      cout << "非终结符：" << endl;
      for (int i = 0; i < nonterminals.size(); ++i) {
        cout << nonterminals[i] << ' ';
      }
      cout << endl << "终结符：" << endl;
      for (int i = 0; i < terminals.size(); ++i) {
        cout << terminals[i] << ' ';
      }
      cout << endl << endl;
    }

    void print_productions() {
      cout << "文法产生式：" << endl;
      for (int i = 0; i < nonterminals.size(); ++i) {
        cout << nonterminals[i] << " -> ";
        vector<string> right = productions[nonterminals[i]];
        for (int j = 0; j < right.size(); ++j) {
          if (j != 0) {
            cout << " | ";
          }
          cout << right[j];
        }
        cout << endl;
      }
      cout << endl;
    }

    void print_FIRST() {
      cout << "FIRST集：" << endl;
      for (int i = 0; i < nonterminals.size(); ++i) {
        cout << nonterminals[i] << "\t\t";
        for (set<string>::iterator it = FIRST[nonterminals[i]].begin(); it != FIRST[nonterminals[i]].end(); ++it) {
          cout << *it << ' ';
        }
        cout << endl;
      }
      cout << endl;
    }

    void print_FOLLOW() {
      cout << "FOLLOW集：" << endl;
      for (int i = 0; i < nonterminals.size(); ++i) {
        cout << nonterminals[i] << "\t\t";
        for (set<string>::iterator it = FOLLOW[nonterminals[i]].begin(); it != FOLLOW[nonterminals[i]].end(); ++it) {
          cout << *it << ' ';
        }
        cout << endl;
      }
      cout << endl;
    }
};