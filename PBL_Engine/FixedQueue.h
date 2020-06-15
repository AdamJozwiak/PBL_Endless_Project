#pragma once
#include <queue>
template <typename T, int MaxLen, typename Container = std::deque<T>>
class FixedQueue : public std::queue<T, Container> {
  public:
    void push(const T& value) {
        if (this->size() == MaxLen) {
            return;
        }
        std::queue<T, Container>::push(value);
    }
    T pop() {
        auto x = std::queue<T, Container>::front();
        std::queue<T, Container>::pop();
        return x;
    }
};
