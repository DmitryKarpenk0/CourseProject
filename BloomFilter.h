#pragma once

#include <cmath>
#include <functional>
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
  vector<bool> m_bits;

  size_t ith_hash(Key_T key, Key_T i);
};

template <typename Key_T, typename Hash>
BloomFilter<Key_T, Hash>::BloomFilter(size_t n) {
  this->m = round(-(double)n * log(1. / INT_MAX) / pow(log(2), 2));
  this->k = round((double)this->m / (double)n * log(2));
  m_bits.resize(m);
}

template <typename Key_T, typename Hash>
void BloomFilter<Key_T, Hash>::add(Key_T key) {
  for (size_t i = 0; i < this->k; i++) {
    this->m_bits[this->ith_hash(key, (Key_T)i)] = 1;
  }
}

template <typename Key_T, typename Hash>
bool BloomFilter<Key_T, Hash>::possibly_contains(Key_T key) {
  bool contains = 1;
  for (size_t i = 0; i < this->k; i++) {
    contains = contains & this->m_bits[Hash{}(key + (Key_T)i) % this->m];
  }
  return contains;
}

template <typename Key_T, typename Hash>
size_t BloomFilter<Key_T, Hash>::ith_hash(Key_T key, Key_T i) {
  return Hash{}(key + i) % this->m;
}
