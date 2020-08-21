#pragma once

#include <hermes/state.h>

#include <sstream>
#include <exception>

namespace hermes {
    class ParseError : std::exception {
    public:
        std::string issue;
        size_t index = 0;

        bool matches = false;
        bool light = false;

        static void lineDetails(const std::string &text, size_t index, std::string &line, std::string &marker);

        [[nodiscard]] const char* what() const noexcept override;

        ParseError(const State &state, std::string message, bool matches = false, bool light = false);
    };
}
