#pragma once

#include <algorithm>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "BloomFilter.h"

using namespace std;

template <typename Key_T, typename Value_T>
class LSMTree {
 public:
  LSMTree();
  ~LSMTree() = default;

  void add(Key_T key, Value_T value);

  Value_T get(Key_T key);

 private:
  // ������������ ������ ���������� ������ � ����������� ������ - 1 ��
  const int THRESHOLD = 1 * 1024 * 1024;

  // ������ � ����������� ������, ���-�������
  unordered_map<Key_T, Value_T> mem_table;

  // ��������, ��������� �� ������ ���������� ������ � ����������� ������
  bool is_memory_limit();

  // ������ �����
  vector<BloomFilter<Key_T>> bloom_filters;
  void add_to_bloom_filter();

  // ����� �� �����
  Value_T search(size_t file_number, Key_T key);

  // ���������� � ���������� ������ � �������� �������
  unordered_map<size_t, pair<Key_T, Key_T>> file_system_data;

  // ��������� ����� ����� �� ��� ������
  string get_filename(size_t file_number);
};

// ����������

// �� �� ����� � �������� ���������?
template <typename Key_T, typename Value_T>
LSMTree<Key_T, Value_T>::LSMTree() {
  mem_table = unordered_map<Key_T, Value_T>();
  file_system_data = unordered_map<size_t, pair<Key_T, Key_T>>();
}

template <typename Key_T, typename Value_T>
void LSMTree<Key_T, Value_T>::add_to_bloom_filter() {
  for (auto &elem : mem_table) {
    bloom_filters.back().add(elem.first);
  }
}

template <typename Key_T, typename Value_T>
string LSMTree<Key_T, Value_T>::get_filename(size_t file_number) {
  string FILENAME_PATTERN = "data_";
  string filename = FILENAME_PATTERN + to_string(file_number) + ".bin";
  return filename;
}

template <typename Key_T, typename Value_T>
void LSMTree<Key_T, Value_T>::add(Key_T key, Value_T value) {
  // ��������, ��� �� ������������� �������� ������ �� ������ ���� (���
  // ���������� ����������� ������ �������� ������ � ����������� ������)
  if (is_memory_limit()) {
    // ----
    // ��������� ��� ����� � ������ �����
    bloom_filters.push_back(
        BloomFilter<Key_T>(THRESHOLD / (sizeof(Key_T) + sizeof(Value_T))));
    add_to_bloom_filter();

    // ----
    // ��������� �������� key-value �� �����, ������� ���-�������
    vector<pair<Key_T, Value_T>> elems(mem_table.begin(), mem_table.end());
    mem_table.clear();
    sort(elems.begin(), elems.end());

    // ----
    // �������� ������ �� ������ ����
    size_t curr_file_number = file_system_data.size();
    string filename = get_filename(curr_file_number);

    // ��������� ���������� � ����� �����, ��� ����� � �������� ������
    file_system_data.insert(
        {curr_file_number, make_pair(elems.front().first, elems.back().first)});

    // ����� � ����
    fstream f(filename, ios::out | ios::binary);
    for (auto &elem : elems) {
      f.write((char *)&elem.first, sizeof(Key_T));
      f.write((char *)&elem.second, sizeof(Value_T));
    }
    f.close();
    elems.clear();
  }
  mem_table.insert({key, value});
}

template <typename Key_T, typename Value_T>
Value_T LSMTree<Key_T, Value_T>::search(size_t file_number, Key_T key) {
  // ----
  // ����� �� �����, �������� ��������� ������
  string filename = get_filename(file_number);
  fstream f(filename, ios::in | ios::binary);

  f.seekg(0, ios::end);
  size_t l = 0, r = f.tellg() / (sizeof(Key_T) + sizeof(Value_T)), k;
  while (l < r) {
    k = (l + r) / 2;
    Key_T tmp;
    f.seekg(k * (sizeof(Key_T) + sizeof(Value_T)));
    f.read((char *)&tmp, sizeof(Key_T));
    if (tmp == key) {
      Value_T value;
      f.read((char *)&value, sizeof(Value_T));
      f.close();
      return value;
    } else if (tmp < key) {
      l = k + 1;
    } else {
      r = k;
    }
  }
  f.close();

  // �� ��� �� ������, ��� ����������, ���� ������ ��� � �����???
  return NULL;
}

template <typename Key_T, typename Value_T>
Value_T LSMTree<Key_T, Value_T>::get(Key_T key) {
  auto elem = mem_table.find(key);
  if (elem == mem_table.end()) {
    // ----
    // �������� ��� � ����������� ������. ��������� ������ �����
    if (!bloom_filters.empty()) {
      for (size_t i = 0; i < bloom_filters.size(); i++) {
        // ----
        // ���� ���� �� ���� �� ��������� ������� �����, ��������������� �������
        // ����� ����� ����, �� ����� �������� ��� ����� ������
        if (bloom_filters[i].possibly_contains(key)) {
          // ----
          // ���� �������� � ������� ����� ��������, ����������� ����� ��
          // ������ � ������� �����

          // ��������, ��������� �� ������� ���� � �������� ������ �����
          auto borders = file_system_data[i];
          if (key >= borders.first && key <= borders.second) {
            Value_T value = search(i, key);
            if (value != NULL) {
              return value;
            } else
              continue;
          } else
            continue;
        } else
          continue;
      }
      return NULL;
    } else {
      // ��� ���������� ��� ���������� ��������?????
      return NULL;
    }
  } else {
    return elem->second;
  }
  return Value_T();
}

template <typename Key_T, typename Value_T>
bool LSMTree<Key_T, Value_T>::is_memory_limit() {
  return (mem_table.size() + 1) * (sizeof(Key_T) + sizeof(Value_T)) >=
         THRESHOLD;
}
