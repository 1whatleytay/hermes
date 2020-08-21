#pragma once

#include <string>

namespace hermes {
    using Stoppable = std::function<bool(const char *, size_t)>;

    class State {
    public:
        std::string text;
        size_t index = 0;

        void push(const Stoppable &stoppable);

        std::string pull(size_t size) const;
        void pop(size_t size, const Stoppable &stoppable);
        size_t until(const Stoppable &stoppable);
        bool complete(size_t size, const Stoppable &stoppable);

        explicit State(std::string text);
    };
}
