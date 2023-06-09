#pragma once

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <unordered_map>
#include <vector>

#include "BloomFilter.h"

using namespace std;

template <typename Key_T, typename Value_T>
class LSMTree {
 public:
  LSMTree(int value_size = 1, bool is_filter = 1, bool is_bin_search = 1);
  ~LSMTree() = default;

  void add(Key_T key, Value_T *value);

  Value_T *get(Key_T key);

 private:
  // ������������ ������ ���������� ������ � ����������� ������ - 100 ��
  const int THRESHOLD = 100 * 1024 * 1024;

  bool is_bloom_filter;
  bool is_bin_search;
  int value_size;

  // ������ � ����������� ������, ���-�������
  unordered_map<Key_T, Value_T *> mem_table;

  // ������ ��� ����� � ����������� ������ � ������, ����� �� ���������� ������
  // �����
  vector<vector<Key_T>> keys;

  // ��������, ��������� �� ������ ���������� ������ � ����������� ������
  bool is_memory_limit();

  // ������ �����
  vector<BloomFilter<Key_T>> bloom_filters;
  void add_to_bloom_filter();

  // �������� ����� �� ����� � �������
  int search_bin(size_t file_number, Key_T key);

  // �������� ����� �� ����� � �������
  int search_linear(size_t file_number, Key_T key);

  // ���������� � ���������� ������ � �������� �������
  unordered_map<size_t, pair<Key_T, Key_T>> file_system_data;

  // ��������� ����� ����� �� ��� ������
  string get_filename(size_t file_number, bool is_value);
};

// ����������

template <typename Key_T, typename Value_T>
LSMTree<Key_T, Value_T>::LSMTree(int value_size, bool is_filter,
                                 bool is_bin_search) {
  this->value_size = value_size;
  is_bloom_filter = is_filter;
  this->is_bin_search = is_bin_search;
  mem_table = unordered_map<Key_T, Value_T *>();
  file_system_data = unordered_map<size_t, pair<Key_T, Key_T>>();
  if (!is_bloom_filter) {
    keys = vector<vector<Key_T>>();
  }
}

template <typename Key_T, typename Value_T>
void LSMTree<Key_T, Value_T>::add_to_bloom_filter() {
  for (auto &elem : mem_table) {
    bloom_filters.back().add(elem.first);
  }
}

template <typename Key_T, typename Value_T>
string LSMTree<Key_T, Value_T>::get_filename(size_t file_number,
                                             bool is_value) {
  string FILENAME_PATTERN = "data_";
  string FILE_ASSIGNEMNT;
  if (is_value) {
    FILE_ASSIGNEMNT = "value_";
  } else {
    FILE_ASSIGNEMNT = "key_";
  }
  string filename =
      FILENAME_PATTERN + FILE_ASSIGNEMNT + to_string(file_number) + ".bin";
  return filename;
}

template <typename Key_T, typename Value_T>
void LSMTree<Key_T, Value_T>::add(Key_T key, Value_T *value) {
  // ��������, ��� �� ������������� �������� ������ �� ������ ���� (���
  // ���������� ����������� ������ �������� ������ � ����������� ������)
  if (is_memory_limit()) {
    // ----
    // ��������� ��� ����� � ������ �����
    if (is_bloom_filter) {
      bloom_filters.push_back(BloomFilter<Key_T>(
          THRESHOLD / (sizeof(Key_T) + sizeof(Value_T) * value_size)));
      add_to_bloom_filter();
    }
    // ----
    // ��������� �������� key-value �� �����, ������� ���-�������
    vector<pair<Key_T, Value_T *>> elems(mem_table.begin(), mem_table.end());
    mem_table.clear();
    sort(elems.begin(), elems.end());

    // ----
    // �������� ������ �� ������ ����
    size_t curr_file_number = file_system_data.size();
    string filename_key = get_filename(curr_file_number, false);
    string filename_value = get_filename(curr_file_number, true);

    // ��������� ���������� � ����� �����, ��� ����� � �������� ������
    file_system_data.insert(
        {curr_file_number, make_pair(elems.front().first, elems.back().first)});

    fstream f_key;
    // ����� � ����
    if (is_bloom_filter) {
      f_key = fstream(filename_key, ios::out | ios::binary);
    }
    fstream f_value(filename_value, ios::out | ios::binary);
    if (!is_bloom_filter) {
      keys.push_back(vector<Key_T>());
    }
    for (auto &elem : elems) {
      if (is_bloom_filter) {
        f_key.write((char *)&elem.first, sizeof(Key_T));
      } else {
        keys[keys.size() - 1].push_back(elem.first);
      }
      f_value.write((char *)elem.second, sizeof(Value_T) * value_size);
      delete[] elem.second;
    }
    f_key.close();
    f_value.close();
    elems.clear();
  }
  mem_table.insert({key, value});
}

template <typename Key_T, typename Value_T>
int LSMTree<Key_T, Value_T>::search_bin(size_t file_number, Key_T key) {
  if (is_bloom_filter) {
    // ----
    // ����� �� ����� � �������, �������� ��������� ������
    string filename = get_filename(file_number, false);
    fstream f(filename, ios::in | ios::binary);

    f.seekg(0, ios::end);
    size_t l = 0, r = f.tellg() / sizeof(Key_T), k;
    while (l < r) {
      k = (l + r) / 2;
      Key_T tmp;
      f.seekg(k * (sizeof(Key_T)));
      f.read((char *)&tmp, sizeof(Key_T));
      if (tmp == key) {
        size_t idx = f.tellg() / sizeof(Key_T) - 1;
        f.close();
        return idx;
      } else if (tmp < key) {
        l = k + 1;
      } else {
        r = k;
      }
    }
    f.close();
  } else {
    size_t l = 0, r = keys[file_number].size(), k;
    while (l < r) {
      k = (l + r) / 2;
      if (keys[file_number][k] == key) {
        return k;
      } else if (keys[file_number][k] < key) {
        l = k + 1;
      } else {
        r = k;
      }
    }
  }

  return -1;
}

template <typename Key_T, typename Value_T>
int LSMTree<Key_T, Value_T>::search_linear(size_t file_number, Key_T key) {
  if (is_bloom_filter) {
    string filename = get_filename(file_number, false);
    fstream f(filename, ios::in | ios::binary);
    Key_T tmp;
    f.seekg(0, SEEK_END);
    long length = f.tellg() / sizeof(Key_T);
    f.seekg(0);
    for (long i = 0; i < length; i++) {
      f.read((char *)&tmp, sizeof(Key_T));
      if (tmp == key) {
        f.close();
        return i;
      }
    }
    f.close();
  } else {
    for (int i = 0; i < keys[file_number].size(); i++) {
      if (keys[file_number][i] == key) return i;
    }
  }
  return -1;
}

template <typename Key_T, typename Value_T>
Value_T *LSMTree<Key_T, Value_T>::get(Key_T key) {
  auto elem = mem_table.find(key);
  if (elem == mem_table.end()) {
    // ----
    // �������� ��� � ����������� ������. ��������� ������ �����
    for (size_t i = 0; i < file_system_data.size(); i++) {
      bool flag = true;
      // ----
      // ���� ���� �� ���� �� ��������� ������� �����, ��������������� �������
      // ����� ����� ����, �� ����� �������� ��� ����� ������

      if (is_bloom_filter) {
        flag = bloom_filters[i].possibly_contains(key);
      }

      if (flag) {
        // ----
        // ���� �������� � ������� ����� ��������, ����������� ����� ��
        // ������ � ������� �����

        // ��������, ��������� �� ������� ���� � �������� ������ �����
        auto borders = file_system_data[i];
        if (key >= borders.first && key <= borders.second) {
          int value_idx;
          if (is_bin_search) {
            value_idx = search_bin(i, key);
          } else {
            value_idx = search_linear(i, key);
          }
          if (value_idx == -1) continue;

          string filename = get_filename(i, true);
          fstream f(filename, ios::in | ios::binary);
          f.seekg(value_idx * sizeof(Value_T));
          Value_T *value = new Value_T[value_size];
          f.read((char *)value, sizeof(Value_T) * value_size);
          for (int i = 0; i < value_size; i++) {
            if (value[i] != NULL) {
              return value;
            }
          }
          continue;
        } else
          continue;
      } else
        continue;
    }
    return nullptr;

  } else {
    return elem->second;
  }
}

template <typename Key_T, typename Value_T>
bool LSMTree<Key_T, Value_T>::is_memory_limit() {
  return (mem_table.size() + 1) *
             (sizeof(Key_T) + sizeof(Value_T) * value_size) >=
         THRESHOLD;
}
