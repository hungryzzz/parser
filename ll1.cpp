#include <set>
#include <queue>
#include <stack>
#include <utility>
#include <algorithm>
#include "grammar.h"

using namespace std;

// AST node
struct node {
  string name;
  vector<node*> children;
};
typedef node* TreeNode;

class AST {    
  public:
    TreeNode root;

    void init(string name) {
      root = new node;
      root->name = name;
    }

    void insert_node(TreeNode parent, vector<string> symbols, stack<TreeNode>& ast_stack) {
      for (int i = symbols.size() - 1; i >= 0; --i) {
        TreeNode child = new node;
        child->name = symbols[i];
        parent->children.push_back(child);
        ast_stack.push(child);
      }
      rotate(parent->children.rbegin(), parent->children.rbegin() + 1, parent->children.rend());
    }

    void print_all_tree() {
      print_tree(root, 0);
    }

    void print_tree(TreeNode node, int deep) {
      cout << string(deep, '-') << node->name << endl;
      for (int i = 0; i < node->children.size(); ++i) {
        print_tree(node->children[i], deep+1);
      }
    }
};

string add_symbol(string prefix, string s) {
  return prefix + (prefix.length() == 0 ? "" : " ") + s; 
}

struct prefix_node {
  string s;
  vector<prefix_node*> children;
  int n;
};
typedef prefix_node* PrefixTreeNode;

class PrefixTree {
  private:
    PrefixTreeNode root;

    void init_root() {
      root = new prefix_node;
      root->s = "";
      root->children.clear();
      root->n = 0;
    }

    PrefixTreeNode insert_node(PrefixTreeNode node, string s) {
      for (int i = 0; i < node->children.size(); ++i) {
        if (node->children[i]->s == s) {
          ++node->children[i]->n;
          return node->children[i];
        }
      }
      PrefixTreeNode c = new prefix_node;
      c->n = 1;
      c->s = s;
      node->children.push_back(c);
      return c;
    }

    void build_prefix_tree(vector<string> rights) {
      string right, prefix, t = "";
      PrefixTreeNode curr_node;
      for (int i = 0; i < rights.size(); ++i) {
        ++root->n;
        curr_node = root;
        right = rights[i] + "#";
        for (int j = 0; j < right.length(); ++j) {
          if (right[j] == ' ' || right[j] == '#') {
            curr_node = insert_node(curr_node, t);
            t = "";
          } else {
            t += right[j];
          }
        }
        PrefixTreeNode node = new prefix_node;
        node->n = 0;
        node->s = right.substr(0, right.length()-1);
        curr_node->children.push_back(node);
      }
    }

  public:
    PrefixTree(vector<string> rights) {
      init_root();
      build_prefix_tree(rights);
    }

    void print_all_tree() {
      print_tree(root, 0);
    }

    void print_tree(PrefixTreeNode node, int deep) {
      cout << string(deep, '-') << node->s << ' ' << node->n << endl;
      for (int i = 0; i < node->children.size(); ++i) {
        print_tree(node->children[i], deep+1);
      }
    }

    void find_longest_prefix(vector<pair<PrefixTreeNode, string> >& prefix_pairs) {
      prefix_pairs.clear();
      if (root->n == root->children.size()) {
        return;
      }
      PrefixTreeNode child;
      pair<PrefixTreeNode, string> curr_node;
      queue<pair<PrefixTreeNode, string> > q;
      q.push(make_pair(root, ""));
      while (!q.empty()) {
        curr_node = q.front();
        q.pop();
        for (int i = 0; i < curr_node.first->children.size(); ++i) {
          child = curr_node.first->children[i];
          if (child->n != child->children.size()) {
            q.push(make_pair(child, add_symbol(curr_node.second, child->s)));
          } else if (child->n > 1) {
            prefix_pairs.push_back(make_pair(child, add_symbol(curr_node.second, child->s)));
          }
        }
      }
    }

    void find_post_symbols(PrefixTreeNode& node, vector<string>& post_symbols, vector<string>& removal_rights) {
      post_symbols.clear();
      removal_rights.clear();
      string t;
      PrefixTreeNode curr_node;
      for (int i = 0; i < node->children.size(); ++i) {
        t = "";
        curr_node = node->children[i];
        while(curr_node->n != 0) {
          t += " " + curr_node->s;
          curr_node = curr_node->children[0];
        }
        post_symbols.push_back(t.length() == 0 ? "@" : t.substr(1));
        removal_rights.push_back(curr_node->s);
      }
    }


};


class LL1 : public Grammar {
  private:
    map<string, set<string> > productions_FIRST; // 产生式右部FIRST集
    map<pair<string, string>, string > Table; // LL(1)文法分析表

    void split_all(string p, vector<string>& symbols) {
      symbols.clear();
      string s(p), first_symbol, left_symbols;
      while (s.length() > 0) {
        split_pright(s, first_symbol, left_symbols);
        symbols.push_back(first_symbol);
        s = left_symbols;
      }
    }

    // X ^ Y == EMPTY ? 
    bool is_intersection_empty(set<string> X, set<string> Y) {
      set<string> result;
      set_intersection(X.begin(), X.end(), Y.begin(), Y.end(), inserter(result, result.begin()));
      return result.empty();
    }

  public:
    AST ast;

    LL1(string filename) { read_grammar(filename); }

    // 消除文法左递归
    void eliminate_left_recursion() {
      for (int i = 0; i < nonterminals.size(); ++i) {
        string Ai = nonterminals[i], A, gamma, p;
        vector<string> new_productions = productions[Ai], old_productions, alpha_symbols, beta_symbols;
        // 产生式替换
        for (int j = 0; j < i; ++j) {
          old_productions = new_productions;
          new_productions.clear();
          string Aj = nonterminals[j];
          for (int k = 0; k < old_productions.size(); ++k) {
            p = old_productions[k];
            split_pright(p, A, gamma);
            if (A == Aj) {
              // 替换
              vector<string> Aj_productions = productions[Aj];
              for (int t = 0; t < Aj_productions.size(); ++t) {
                new_productions.push_back(Aj_productions[t] + " " + gamma);
              }
            } else {
              // 直接添加
              new_productions.push_back(p);
            }
          }
        }
        // 消除Ai产生式的立即左递归
        for (int j = 0; j < new_productions.size(); ++j) {
          p = new_productions[j];
          split_pright(p, A, gamma);
          if (A == Ai) {
            alpha_symbols.push_back(gamma);
          } else {
            beta_symbols.push_back(p);
          }
        }
        if (alpha_symbols.empty()) {
          // 没有左递归
          productions[Ai] = beta_symbols;
        } else {
          string Ai_ = Ai + "'";
          nonterminals.push_back(Ai_);
          productions[Ai].clear();
          for (int k = 0; k < beta_symbols.size(); ++k) {
            productions[Ai].push_back(beta_symbols[k] + " " + Ai_);
          }
          for (int k = 0; k < alpha_symbols.size(); ++k) {
            productions[Ai_].push_back(alpha_symbols[k] + " " + Ai_);
          }
          productions[Ai_].push_back("@");
        }
      }
    }

    // 提取左因子
    void left_factoring() {
      string A, A_;
      vector<string> rights, post_symbols, removal_rights;
      vector<pair<PrefixTreeNode, string> > prefix_pairs; 
      // 对于每一个非终结符的所有产生式，提取左因子
      for (int i = 0; i < nonterminals.size(); ++i) {
        A = A_ = nonterminals[i];
        rights = productions[A];
        while (true) {
          PrefixTree prefix_tree(rights);
          prefix_tree.find_longest_prefix(prefix_pairs);
          // 直到没有最长前缀，则退出
          if (prefix_pairs.empty()) break;
          // 处理前缀
          for (int j = 0; j < prefix_pairs.size(); ++j) {
            // 每一个前缀都产生一个新的非终结符
            A_ += '\'';
            // 改写当前终结符产生式
            rights.push_back(add_symbol(prefix_pairs[j].second, A_));

            // 查找后缀，构建新的非终结符的产生式，更新当前A的产生式
            prefix_tree.find_post_symbols(prefix_pairs[j].first, post_symbols, removal_rights);
            productions[A_] = post_symbols;
            nonterminals.push_back(A_);
            for (int k = 0; k < removal_rights.size(); ++k) {
              rights.erase(remove(rights.begin(), rights.end(), removal_rights[k]), rights.end());
            }
          }
        }
        productions[A] = rights;
      }
    }

    // 获取每个符号和产生式的FIRST集
    void get_symbols_productions_FIRST() { 
      FIRST.clear();

      get_FIRST();

      // 获取每一个产生式的FIRST集
      string A, alpha;
      vector<string> rights;

      for (int i = 0; i < nonterminals.size(); ++i) {
        A = nonterminals[i];
        rights = productions[A];
        for (int j = 0; j < rights.size(); ++j) {
          alpha = rights[j];
          get_alpha_first(alpha, productions_FIRST[alpha]);
        }
      }
    }

    bool judge_LL1() {
      string A, alpha, beta;
      vector<string> rights;
      for (int i = 0; i < nonterminals.size(); ++i) {
        A = nonterminals[i];
        rights = productions[A];
        for (int j = 0; j < rights.size() - 1; ++j) {
          alpha = rights[j];
          for (int k = j+1; k < rights.size(); ++k) {
            beta = rights[k];
            if (!is_intersection_empty(productions_FIRST[alpha], productions_FIRST[beta]) || 
                (exist_empty_in_first(productions_FIRST[alpha]) && !is_intersection_empty(FOLLOW[A], productions_FIRST[beta])) ||
                (exist_empty_in_first(productions_FIRST[beta]) && !is_intersection_empty(FOLLOW[A], productions_FIRST[alpha]))) {
              return false;
            }
          }
        }
      }
      return true;
    }

    bool get_table() {
      eliminate_left_recursion();
      left_factoring();
      get_symbols_productions_FIRST();
      get_FOLLOW();
      if (!judge_LL1()) {
        return false;
      }
      
      // 构建分析表
      string A, alpha, first_symbol, left_symbols;
      vector<string> rights;
      set<string> alpha_first;
      for (int i = 0; i < nonterminals.size(); ++i) {
        A = nonterminals[i];
        rights = productions[A];
        for (int j = 0; j < rights.size(); ++j) {
          // 对于每一个产生式
          alpha = rights[j];
          // 把当前产生式 加入M[A, a]中，a为alpha的FIRST集
          for (set<string>::iterator it = productions_FIRST[alpha].begin(); it != productions_FIRST[alpha].end(); ++it) {
            if (*it != "@") {
              Table[make_pair(A, *it)] = alpha;
            } else {
              for (set<string>::iterator it_ = FOLLOW[A].begin(); it_ != FOLLOW[A].end(); ++it_) {
                Table[make_pair(A, *it_)] = alpha;
              }
            }
          }
        }
      }
      return true;
    }

    bool parsing(ifstream& ifs) {
      // 预测分析
      string a = get_next_token(ifs), p;
      vector<string> symbols;

      // AST构建
      stack<TreeNode> ast_stack;
      TreeNode sharp_symbol_node = new node;
      sharp_symbol_node->name = "#";
      ast_stack.push(sharp_symbol_node);
      ast.init(S);
      ast_stack.push(ast.root);
      TreeNode ast_node, X_node = ast_stack.top();

      while (X_node->name != "#") {
        // 如果X等于当前token，弹出栈顶符号，get next token
        if (X_node->name == a) {
          ast_stack.pop();
          a = get_next_token(ifs);          
        } else if (is_terminal(X_node->name)) {
          // error
          return false;
        } else if (Table.find(make_pair(X_node->name, a)) == Table.end()) {
          // error
          return false;
        } else {
          p = Table[make_pair(X_node->name, a)];

          ast_node = ast_stack.top();
          ast_stack.pop();
          // 将p划分成Y1...Yn，反向压栈
          if (p != "@") {
            split_all(p, symbols);
            // 创建AST节点，压栈
            ast.insert_node(ast_node, symbols, ast_stack);
          }
        }
        X_node = ast_stack.top();
      }
      return true;
    }

    void parser(string filename) {
      if (!get_table()) {
        cout << "该文法不是LL(1)文法\n";
        return;
      }
      ifstream ifs(filename);
      if (ifs) {
        if (parsing(ifs)) {
          cout << endl << "AST构建如下：\n";
          ast.print_all_tree();
          cout << endl;
        } else {
          cout << "Error occurs in parsing!\n";
        }
      } else {
        cout << "Read " << filename << " Failed!\n";
      }
      ifs.close();
    }

};

int main() {
  string filename = "Grammars/ll1_grammar.txt";
  LL1 ll1(filename);
  ll1.parser("Grammars/ll1_tokens.txt");
}