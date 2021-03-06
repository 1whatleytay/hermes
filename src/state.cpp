#include <hermes/state.h>

namespace hermes {
    void State::push(const Stoppable &stoppable) {
        while (index < text.size() && !stoppable(&text[index], text.size() - index)) {
            index++;
        }
    }

    std::string State::pull(size_t size) const {
        return text.substr(index, std::min(size, text.size() - index));
    }

    void State::pop(size_t size, const Stoppable &stoppable) {
        index += size;

        push(stoppable);
    }

    size_t State::until(const Stoppable &stoppable) {
        size_t size = 0;

        while ((index + size) < text.size()) {
            if (stoppable(&text[index + size], text.size() - index - size)) {
                break;
            }

            size++;
        }

        return size;
    }

    bool State::complete(size_t size, const Stoppable &stoppable) {
        return (index + size >= text.size()) || stoppable(&text[index + size], text.size() - index - size);
    }
    
    State::State(std::string text) : text(std::move(text)) { }
}
