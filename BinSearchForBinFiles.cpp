#include <chrono>
#include <fstream>
#include <iostream>
#include <unordered_map>

using namespace std;

void generate_data(size_t size) {
  fstream f("data.bin", ios::out | ios::binary);

  for (int i = 0; i < size; i++) {
    f.write((char*)&i, sizeof(int));
  }
  f.close();
}

int bin_search(int elem, size_t size) {
  fstream f("data.bin", ios::in | ios::binary);

  int l = 0, r = size, k;
  while (l < r) {
    k = (l + r) / 2;
    int tmp;
    f.seekg(k * sizeof(int));
    f.read((char*)&tmp, sizeof(int));
    if (tmp == elem) {
      f.close();
      return k;
    } else if (tmp < elem) {
      l = k + 1;
    } else {
      r = k;
    }
  }
  f.close();
  return -1;
}

int simple_search(int elem, size_t size) {
  fstream f("data.bin", ios::in | ios::binary);

  for (int i = 0; i < size; i++) {
    int tmp;
    f.seekg(i * sizeof(int));
    f.read((char*)&tmp, sizeof(int));
    if (tmp == elem) {
      f.close();
      return i;
    }
  }
  f.close();
  return -1;
}

/*
int main() {

  int SIZE = 10000;

  // for (int i = 1; i < 6; i++) {
  // SIZE *= 10;
  chrono::duration<double> time1, time2, time3;

  auto start_generate = chrono::steady_clock::now();
  generate_data(SIZE);
  auto end_generate = chrono::steady_clock::now();

  auto start_bin_search = chrono::steady_clock::now();
  for (int i = 0; i < SIZE; i++) {
    bin_search(i, SIZE);
  }
  auto end_bin_search = chrono::steady_clock::now();

  auto start_simple_search = chrono::steady_clock::now();
  for (int i = 0; i < SIZE; i++) {
    simple_search(i, SIZE);
  }
  auto end_simple_search = chrono::steady_clock::now();

  time1 = end_generate - start_generate;
  time2 = end_bin_search - start_bin_search;
  time3 = end_simple_search - start_simple_search;
  cout << "Generation: " << time1.count() << endl;
  cout << "BinSearch: " << time2.count() << endl;
  cout << "SimpleSearch: " << time3.count() << endl;
  //}
  return 0;
}
*/