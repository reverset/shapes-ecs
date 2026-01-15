#ifndef GAME_UTIL_H
#define GAME_UTIL_H

#include <vector>
#include <functional>
#include <iostream>
#include <utility>
#include <memory>
#include <string>
#include <stdexcept>

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


    // https://stackoverflow.com/questions/2342162/stdstring-formatting-like-sprintf
    template<typename ... Args>
    std::string string_format( const std::string& format, Args ... args ) {
        int size_s = std::snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
        if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
        auto size = static_cast<size_t>( size_s );
        std::unique_ptr<char[]> buf( new char[ size ] );
        std::snprintf( buf.get(), size, format.c_str(), args ... );
        return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
    }
}

#endif //GAME_UTIL_H