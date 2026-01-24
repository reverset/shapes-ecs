#ifndef GAME_ITER_H
#define GAME_ITER_H

#include <optional>
#include <vector>
#include <functional>
#include <memory>

template <typename T>
class Iter {
public:
    virtual ~Iter() = default;

    virtual std::optional<T> next() = 0;

    std::unique_ptr<Iter> take(std::size_t max) {
        class TakeIter : public Iter {
            std::size_t max = max;
            std::size_t taken = 0;
            Iter* outer;

            std::optional<T> next() override {
                if (taken >= max) return std::nullopt;

                if (auto val = outer->next(); val.has_value()) {
                    taken++;
                    return val;
                }
                return std::nullopt;
            }

        public:
            TakeIter(std::size_t max, Iter* outer) {
                this->max = max;
                this->outer = outer;
            }
        };

        return std::make_unique<TakeIter>(max, this);
    }

    template <typename K>
    std::unique_ptr<Iter<K>> map(std::function<K(T)> mapper) {
        class MapIter : public Iter<K> {
            std::function<K(T)> mapper;
            Iter* outer;

            std::optional<K> next() override {
                if (auto value = outer->next(); value.has_value()) {
                    return mapper(value.value());
                }
                return std::nullopt;
            }

        public:
            MapIter(std::function<K(T)> mapper, Iter* outer) {
                this->mapper = mapper;
                this->outer = outer;
            }
        };

        return std::make_unique<MapIter>(mapper, this); // FIXME!!!!, all iters are behind unique pointers which means this pointer could be invalid by the time it is used!!!!!
    }

    void forEach(std::function<void(T)> func) {
        auto val = next();
        for (; val.has_value(); val = next()) {
            func(val.value());
        }
    }

    std::vector<T> collectToVector() {
        std::vector<T> res;

        std::optional<T> val = next();
        for (; val.has_value(); val = next()) {
            res.push_back(val.value());
        }

        return res;
    }
};

namespace Iterators {
    template <typename T>
    std::unique_ptr<Iter<T>> ofFunctional(std::function<std::optional<T>()>& sup) {
        class FunctionalIter : public Iter<T> {
            std::function<std::optional<T>()> sup;

            std::optional<T> next() override {
                return sup();
            }
        public:
            explicit FunctionalIter(std::function<std::optional<T>()>& sup) {
                this->sup = sup;
            }
        };

        return std::make_unique<FunctionalIter>(sup);
    }

    template <typename T>
    std::unique_ptr<Iter<T>> ofVec(const std::vector<T>* vec) {
        class VecIter : public Iter<T> {
            const std::vector<T>* vec;
            std::size_t i = 0;

            std::optional<T> next() override {
                if (vec == nullptr) return std::nullopt;

                if (i < vec->size()) {
                    return vec->at(i++);
                }
                return std::nullopt;
            }
        public:
            explicit VecIter(const std::vector<T>* vec) {
                this->vec = vec;
            }
        };

        return std::make_unique<VecIter>(vec);
    }

    template <typename T>
    std::unique_ptr<Iter<T>> ofRange(T startInc, T endEx) {
        class RangeIter : public Iter<T> {
            T endEx;
            T val;

            std::optional<T> next() override {
                if (val < endEx) {
                    return val++;
                }
                return std::nullopt;
            }

        public:
            RangeIter(T startInc, T endInc) {
                this->val = startInc;
                this->endEx = endInc;
            }
        };

        return std::make_unique<RangeIter>(startInc, endEx);
    }
}

#endif //GAME_ITER_H