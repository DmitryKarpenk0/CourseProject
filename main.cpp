#include <cmath>
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>

#include "LSMTree.h"

using namespace std;

int main() {
  LSMTree<int, int> lsm;
  for (int i = 1; i <= 200000; i++) {
    lsm.add(i, i * 10);
  }
  cout << "ADDING DONE!" << endl;

  for (int i = 1; i <= 200000; i++) {
    int a = lsm.get(i);
    if (a == 0) {
      cout << "KEY " << i << " NOT FOUND!" << endl;
    }
  }

  cout << "GETTING DONE!" << endl;

  return 0;
}
