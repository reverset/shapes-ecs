#ifndef GAME_UTIL_H
#define GAME_UTIL_H

#include <vector>
#include <functional>
#include <iostream>
#include <utility>

namespace GameUtil {
    // TODO broken rn
    template <typename T>
    void insertSorted(std::vector<T>* list, T&& item, const std::function<int(const T&, const T&)>& comparator) {
        for (size_t i = 0; i < list->size(); i++) {
            if (comparator(list->at(i), item) >= 0) {
                list->insert(list->begin() + i, std::move(item));
                return;
            }
        }
        list->push_back(std::move(item));
    }
}

#endif //GAME_UTIL_H