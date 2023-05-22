#include <climits>
#include <cmath>
#include <ctime>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>
#include <algorithm>

#include "LSMTree.h"

using namespace std;

#define KEY_TYPE unsigned int
#define VALUE_TYPE unsigned int

int main() {
  
  const KEY_TYPE KEYS_ADD = 100000000;
  const KEY_TYPE KEYS_MAX = 1000000000;
  const VALUE_TYPE VALUE_MAX = numeric_limits<VALUE_TYPE>::max();
  const int VALUE_ARRAY_LENGTH = 15;
  const bool BLOOM_FILTER = true;
  const bool BIN_SEARCH = true;
  random_device rd;
  default_random_engine generator(rd());
  std::uniform_int_distribution<VALUE_TYPE> value_distribution(1, VALUE_MAX);
  std::uniform_int_distribution<KEY_TYPE> key_distribution(1, KEYS_MAX);

  // Psuedo random number generator
  std::mt19937 prng(rd());

  vector<KEY_TYPE> keys;
  for (KEY_TYPE i = 1; i < KEYS_MAX; i++) {
    keys.push_back(i);
  }

  std::shuffle(keys.begin(), keys.end(), prng);

  LSMTree<KEY_TYPE, VALUE_TYPE> lsm(VALUE_ARRAY_LENGTH, BLOOM_FILTER,
  BIN_SEARCH);

  auto add_start = clock();
  for (VALUE_TYPE i = 1; i <= KEYS_ADD; i++) {
    VALUE_TYPE *tmp = new VALUE_TYPE[VALUE_ARRAY_LENGTH];
    for (int i = 0; i < VALUE_ARRAY_LENGTH; i++) {
      tmp[i] = value_distribution(generator);
    }
    lsm.add(keys[i], tmp);
  }
  auto add_end = clock();
  cout << "Adding time (s): " << (double)(add_end - add_start) / CLOCKS_PER_SEC
       << endl;

  keys.clear();

  auto get_start = clock();
  for (int i = 1; i <= 100000; i++) {
    VALUE_TYPE* a = lsm.get(key_distribution(generator));
  }

  auto get_end = clock();
  cout << "Getting elements time (s): "
       << (double)(get_end - get_start) / CLOCKS_PER_SEC << endl;




  
  /*
  LSMTree<KEY_TYPE, VALUE_TYPE> lsm(1, false, true);
  for (KEY_TYPE i = 0; i < 3000000; i++) {
    unsigned int *a = new unsigned int(i + 1);
    lsm.add(i, a);
  }

  cout << "ADDING DONE" << endl;

  for (KEY_TYPE i = 0; i < 3000000; i++) {
    unsigned int *a = lsm.get(i);
    if (a[0] != i + 1) {
      cout << "WRONG VALUE ON INDX " << i << endl;
    }
  }

  cout << "GETTING DONE" << endl;
  */

  /*int a[2] = {1, 2};

fstream fout("test.bin", ios::out | ios::binary);
cout << sizeof(a) << endl;
fout.write((char *)a, 2 * sizeof(int));
int b[2];
for (int i = 0; i < 2; i++) {
  cout << b[i] << endl;
}
fout.close();
fstream fin("test.bin", ios::in | ios::binary);
fin.seekg(0);
fin.read((char *)b, sizeof(b));
fin.close();
for (int i = 0; i < 2; i++) {
  cout << b[i] << endl;
}*/

  return 0;
}
