#include <cmath>
#include <ctime>
#include <functional>
#include <iostream>
#include <random>
#include <string>
#include <unordered_map>

#include "LSMTree.h"

using namespace std;

int main() {
   const int KEYS_ADD = 900000;
   const int KEYS_MAX = 1000000;
   const bool BLOOM_FILTER = false;

   random_device rd;
   default_random_engine generator(rd());
   std::uniform_int_distribution<int> distribution(1, KEYS_MAX);

   LSMTree<int, int> lsm(BLOOM_FILTER);

   auto add_start = clock();
   for (int i = 1; i <= KEYS_ADD; i++) {
     lsm.add(distribution(generator), i);
   }
   auto add_end = clock();
   cout << "Adding time (s): " << (double)(add_end - add_start) / CLOCKS_PER_SEC
        << endl;

   auto get_start = clock();
   for (int i = 1; i <= KEYS_MAX; i++) {
     int a = lsm.get(i);
   }
   auto get_end = clock();
   cout << "Getting elements time (s): "
        << (double)(get_end - get_start) / CLOCKS_PER_SEC << endl;
        

  return 0;
}
