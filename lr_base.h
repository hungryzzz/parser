#include <set>
#include <stack>
#include <queue>
#include <utility>
#include <algorithm>

#include "grammar.h"

using namespace std;

template<typename T>
class LR_Base : public Grammar {
  protected:
    string original_S;
    map<int, pair<string, string> > production_index;  // map of production to index
    
    set<T> states;  // 状态集合
    map<pair<int, string>, int> go_function;  // DFA 转换函数

    map<pair<int, string>, string> action_table;  // action 表
    map<pair<int, string>, int> goto_table; // goto 表

    // 拓广文法
    void extend_grammar() {
      string new_S = S + "'";
      nonterminals.insert(nonterminals.begin(), new_S);
      productions[new_S].push_back(S);
      original_S = S;
      S = new_S;
    }

    void add_production_index() {
      string A;
      vector<string> rights;
      int p_num = 0;
      for (int i = 0 ; i < nonterminals.size(); ++i) {
        A = nonterminals[i];
        rights = productions[A];
        for (int j = 0; j < rights.size(); ++j) {
          production_index[p_num++] = make_pair(A, rights[j]);
        }
      }
    }

    int find_production_by_string(string A, string beta) {
      for (map<int, pair<string, string> >::iterator it = production_index.begin(); it != production_index.end(); ++it) {
        if (it->second.first == A && it->second.second == beta) {
          return it->first;
        }
      }
      return -1;
    }

    int get_symbol_num(string beta) {
      int n = 0;
      for (int i = 0; i < beta.length(); ++i) {
        if (beta[i] == ' ') n++;
      }
      return n + 1;
    }

    T get_state_by_id(int id) {
      for (T s: states) {
        if (s->id == id) {
          return s;
        }
      }
      return NULL;
    }

    template<typename I>
    bool is_state_equal(T s1, T s2) {
      set<I> item_set1 = s1->items, item_set2 = s2->items;
      if (item_set1.size() != item_set2.size()) {
        return false;
      }
      for (auto it1 = item_set1.begin(), it2 = item_set2.begin(); 
           it1 != item_set1.end() && it2 != item_set2.end(); ++it1, ++it2) {
        I t1 = *it1, t2 = *it2;
        if (!(t1 == t2)) {
          return false;
        }
      }
      return true;
    }

    template<typename I>
    T is_state_exist(T s) {
      for (T state_item: states) {
        if (is_state_equal<I>(s, state_item)) {
          return state_item;
        }
      }
      return NULL;
    }

    template<typename I>
    bool is_item_exist(set<I> item_set, I item) {
      for (I t: item_set) {
        if (t == item) {
          return true;
        }
      }
      return false;
    }

  public:
    LR_Base(string filename) {
      read_grammar(filename);
      extend_grammar();
      add_production_index();
    }

    virtual void new_state(T& s) {}
    virtual void init_state(T& s0) {}
    virtual void get_item_closure(T& s) {}
    
    // 构建LR项目集规范族，生成DFA
    template<typename I>
    void build_DFA() {
      int state_num = 0;

      T s0, curr_s, find_s;
      // 初始化 s0
      new_state(s0);
      init_state(s0);

      // 新建的、未处理的状态序列 int 为上一级状态id string 为转移字符
      queue<pair<T, pair<int, string> > > state_queue;
      state_queue.push(make_pair(s0, make_pair(-1, "")));

      map<string, set<I> > item_cluster;
      string entry_symbol;
      int parent_s_id;

      while (!state_queue.empty()) {
        curr_s = state_queue.front().first;
        parent_s_id = state_queue.front().second.first;
        entry_symbol = state_queue.front().second.second;
        state_queue.pop();
        item_cluster.clear();

        // 求curr_s 的item 闭包
        get_item_closure(curr_s);

        if ((find_s = is_state_exist<I>(curr_s)) == NULL) {
          // 加入新状态
          curr_s->id = state_num++;
          states.insert(curr_s);

          // 遍历curr_s的项目集, 加入待处理队列中
          for (I t: curr_s->items) {
            item_cluster[t.get_after_dot_symbol()].insert(t);
          }
          for (auto it: item_cluster) {
            T s;
            new_state(s);
            for (I origin: it.second) {
              if (origin.get_after_dot_symbol().length() != 0) {
                I item(&origin);
                s->items.insert(item);
              }
            }
            if (s->items.size() != 0) {
              state_queue.push(make_pair(s, make_pair(curr_s->id, it.first)));
            }
          }
        } else {
          curr_s = find_s;
        }

        // 状态转移函数
        if (parent_s_id != -1) {
          go_function[make_pair(parent_s_id, entry_symbol)] = curr_s->id;
        }
      }
    }

    // 分析过程
    bool parsing(ifstream& ifs) {
      string a = get_next_token(ifs), next_step;
      // 状态栈
      stack<T> state_stack;
      state_stack.push(get_state_by_id(0));
      T curr_s;
      pair<string, string> p;
      int beta_num;

      cout << "Token的" << "解析过程" << endl;

      while (true) {
        curr_s = state_stack.top();
        if (action_table.find(make_pair(curr_s->id, a)) == action_table.end()) {
          return false;
        }
        next_step = action_table[make_pair(curr_s->id, a)];
        cout << "状态栈栈顶 STATE_" << curr_s->id << "\t";
        cout << "当前字符 " << a << "\t";
        cout << "动作 ";

        if (next_step[0] == 's') {
          // 移进
          cout << "shift " << next_step.substr(2);
          state_stack.push(get_state_by_id(stoi(next_step.substr(2))));
          a = get_next_token(ifs);
        } else if (next_step[0] == 'r') {
          // 规约
          p = production_index[stoi(next_step.substr(2))];
          cout << "reduce " << p.first << " -> " << p.second;
          // 计算 beta 的元素个数，按照个数退栈
          beta_num = get_symbol_num(p.second);
          while (beta_num--) {
            state_stack.pop();
          }
          // 将GOTO[] 状态压栈
          state_stack.push(get_state_by_id(goto_table[make_pair(state_stack.top()->id, p.first)]));
        } else if (next_step == "acc") {
          // parse结束
          cout << "ACCEPT\n\n";
          return true;
        } else {
          return false;
        }
        cout << endl;
      }
      return true;
    }

    virtual bool get_table() {}

    void parser(string filename) {
      if (!get_table()) return;
      
      ifstream ifs(filename);
      if (ifs) {
        if (parsing(ifs)) {
          cout << "Parsing over!\n";
        } else {
          cout << "Error occurs in parsing!\n";
        }
      } else {
        cout << "Read " << filename << " Failed!\n";
      }
      ifs.close();
    }

    // utilities function
    virtual void print_state(T s) {}

    void print_states() {
      for (T s: states) {
        print_state(s);
      }
    }

    void print_go_function() {
      cout << "状态转换函数" << endl;
      for (map<pair<int, string>, int>::iterator it = go_function.begin(); it != go_function.end(); ++it) {
        cout << it->first.first << " -- " << it->first.second << " ==> " << it->second << endl;
      }
    }

    void print_table() {
      cout << "LR分析表" << endl;
      cout << " \t";
      for (string s: terminals) {
        cout << s << "\t";
      }
      cout << "#\t|\t";
      for (string s: nonterminals) {
        cout << s << "\t";
      }
      cout << endl;

      pair<int, string> key;
      for (T s: states) {
        cout << s->id << "\t";
        for (string terminal: terminals) {
          key = make_pair(s->id, terminal);
          if (action_table.find(key) != action_table.end()) {
            cout << action_table[key] << "\t";
          } else {
            cout << "   \t"; 
          }
        }
        key = make_pair(s->id, "#");
        if (action_table.find(key) != action_table.end()) {
          cout << action_table[key] << "\t";
        } else {
          cout << "   \t"; 
        }
        cout << "|\t";
        for (string nonterminal: nonterminals) {
          key = make_pair(s->id, nonterminal);
          if (goto_table.find(key) != goto_table.end()) {
            cout << goto_table[key] << "\t";
          } else {
            cout << "   \t"; 
          }
        }
        cout << endl;
      }
      cout << endl;
    }
};