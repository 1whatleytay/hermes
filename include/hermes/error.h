#pragma once

#include <hermes/state.h>

#include <sstream>
#include <exception>

namespace hermes {
    class ParseError : std::exception {
        std::string issue;
    public:
        bool matches = false;

        const char* what() const noexcept override;

        ParseError(const State &state, const std::string &message, bool matches = false);
    };
}
