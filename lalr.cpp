#include "lr1.cpp"

using namespace std;

// DFA 状态
struct state {
  int id; // 状态编号
  set<LR1_Item> items; // 项目集
};
typedef state* State;

class LALR1 : public LR1 {

  public:
    LALR1(string filename) : LR1(filename) {}

    void judge_LALR1() {

    }
};


int main() {

  return 1;
}