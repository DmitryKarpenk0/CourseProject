#pragma once

#include <cmath>
#include <ctime>
#include <functional>
#include <random>
#include <vector>

using namespace std;

template <typename Key_T, typename Hash = std::hash<Key_T>>
class BloomFilter {
 public:
  BloomFilter(size_t n);

  void add(Key_T key);
  bool possibly_contains(Key_T key);

 private:
  size_t m;
  size_t k;
  vector<size_t> a;
  vector<size_t> b;
  vector<bool> m_bits;

  size_t ith_hash(Key_T key, Key_T i);
};

template <typename Key_T, typename Hash>
BloomFilter<Key_T, Hash>::BloomFilter(size_t n) {
  m = round(-(double)n * log(1. / INT_MAX) / pow(log(2), 2));
  k = round((double)m / (double)n * log(2));
  m_bits.resize(m);
  srand(time(NULL));
  for (int i = 0; i < k; i++) {
    a.push_back(rand());
    b.push_back(rand());
  }
}

template <typename Key_T, typename Hash>
void BloomFilter<Key_T, Hash>::add(Key_T key) {
  for (size_t i = 0; i < k; i++) {
    m_bits[ith_hash(key, (Key_T)i)] = 1;
  }
}

template <typename Key_T, typename Hash>

bool BloomFilter<Key_T, Hash>::possibly_contains(Key_T key) {
  bool contains = 1;
  for (size_t i = 0; i < k; i++) {
    size_t hash = Hash{}(key);
    size_t x = a[i] * hash + b[i];
    contains = contains && m_bits[x % m];
  }
  return contains;
}

template <typename Key_T, typename Hash>
size_t BloomFilter<Key_T, Hash>::ith_hash(Key_T key, Key_T i) {
  size_t hash = Hash{}(key);
  size_t x = a[i] * hash + b[i];
  return x % m;
}
