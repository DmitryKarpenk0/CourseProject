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
  LSMTree(bool is_filter = 1, bool is_bin_search = 1);
  ~LSMTree() = default;

  void add(Key_T key, Value_T value);

  Value_T get(Key_T key);

 private:
  // Максимальный размер хранящихся данных в оперативной памяти - 100 Мб
  const int THRESHOLD = 100 * 1024 * 1024;

  bool is_bloom_filter;
  bool is_bin_search;

  // Данные в оперативной памяти, хеш-таблица
  unordered_map<Key_T, Value_T> mem_table;

  // Проверка, достигнут ли предел хранящихся данных в оперативной памяти
  bool is_memory_limit();

  // Фильтр Блюма
  vector<BloomFilter<Key_T>> bloom_filters;
  void add_to_bloom_filter();

  // Бинарный поиск по файлу с ключами
  size_t search_bin(size_t file_number, Key_T key);

  // Линейный поиск по файлу с ключами
  size_t search_linear(size_t file_number, Key_T key);

  // Метаданные о диапазонах ключей и файловой системе
  unordered_map<size_t, pair<Key_T, Key_T>> file_system_data;

  // Получение имени файла по его номеру
  string get_filename(size_t file_number, bool is_value);
};

// Реализация

// мб не нужен и оставить дефолтным?
template <typename Key_T, typename Value_T>
LSMTree<Key_T, Value_T>::LSMTree(bool is_filter, bool is_bin_search) {
  is_bloom_filter = is_filter;
  this->is_bin_search = is_bin_search;
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
void LSMTree<Key_T, Value_T>::add(Key_T key, Value_T value) {
  // Проверка, нет ли необходимости сгрузить данные на жёсткий диск (при
  // превышении допустимого объёма хранения данных в оперативной памяти)
  if (is_memory_limit()) {
    // ----
    // Добавляем все ключи в фильтр Блюма
    if (is_bloom_filter) {
      bloom_filters.push_back(
          BloomFilter<Key_T>(THRESHOLD / (sizeof(Key_T) + sizeof(Value_T))));
      add_to_bloom_filter();
    }
    // ----
    // Сортируем элементы key-value по ключу, удаляем хеш-таблицу
    vector<pair<Key_T, Value_T>> elems(mem_table.begin(), mem_table.end());
    mem_table.clear();
    sort(elems.begin(), elems.end());

    // ----
    // Сгружаем данные на жёсткий диск
    size_t curr_file_number = file_system_data.size();
    string filename_key = get_filename(curr_file_number, false);
    string filename_value = get_filename(curr_file_number, true);

    // Добавляем метаданные о новом файле, его номер и диапазон ключей
    file_system_data.insert(
        {curr_file_number, make_pair(elems.front().first, elems.back().first)});

    // Пишем в файл
    fstream f_key(filename_key, ios::out | ios::binary);
    fstream f_value(filename_value, ios::out | ios::binary);
    for (auto &elem : elems) {
      f_key.write((char *)&elem.first, sizeof(Key_T));
      f_value.write((char *)&elem.second, sizeof(Value_T));
    }
    f_key.close();
    f_value.close();
    elems.clear();
  }
  mem_table.insert({key, value});
}

template <typename Key_T, typename Value_T>
size_t LSMTree<Key_T, Value_T>::search_bin(size_t file_number, Key_T key) {
  // ----
  // Поиск по файлу с ключами, алгоритм бинарного поиска
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

  // TODO: вернуть exception
  return NULL;
}

template <typename Key_T, typename Value_T>
size_t LSMTree<Key_T, Value_T>::search_linear(size_t file_number, Key_T key) {
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
  return NULL;
}

template <typename Key_T, typename Value_T>
Value_T LSMTree<Key_T, Value_T>::get(Key_T key) {
  auto elem = mem_table.find(key);
  if (elem == mem_table.end()) {
    // ----
    // Элемента нет в оперативной памяти. Проверяем фильтр Блюма
    for (size_t i = 0; i < file_system_data.size(); i++) {
      bool flag = true;
      // ----
      // Если хотя бы один из элементов фильтра Блюма, соответствующий данному
      // ключу равен нулю, то этого элемента нет среди данных
      if (is_bloom_filter) {
        flag = bloom_filters[i].possibly_contains(key);
      }
      if (flag) {
        // ----
        // Если проверка в фильтре Блюма пройдена, осуществить поиск по
        // данным с жётского диска

        // Проверим, подпадает ли искомый ключ в диапазон ключей файла
        auto borders = file_system_data[i];
        if (key >= borders.first && key <= borders.second) {
          size_t value_idx;
          if (is_bin_search) {
            value_idx = search_bin(i, key);
          } else {
            value_idx = search_linear(i, key);
          }
          string filename = get_filename(i, true);
          fstream f(filename, ios::in | ios::binary);
          f.seekg(value_idx * sizeof(Value_T));
          Value_T value;
          f.read((char *)&value, sizeof(Value_T));
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
    return elem->second;
  }
}

template <typename Key_T, typename Value_T>
bool LSMTree<Key_T, Value_T>::is_memory_limit() {
  return (mem_table.size() + 1) * (sizeof(Key_T) + sizeof(Value_T)) >=
         THRESHOLD;
}
