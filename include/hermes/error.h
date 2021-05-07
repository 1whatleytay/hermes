#pragma once

#include <hermes/state.h>

#include <exception>

namespace hermes {
    enum class MatchLevel {
        Priority,
        Strong,
        Light,
    };

    struct LineDetails {
        std::string line;
        std::string marker;

        size_t lineNumber = 0;

        LineDetails(const std::string &text, size_t index, bool backtrack = true);
    };

    class ParseError : public std::exception {
    public:
        std::string issue;
        size_t index = 0;
        size_t kind = ~0ull;
        MatchLevel level = MatchLevel::Light;

        [[nodiscard]] const char* what() const noexcept override;

        ParseError(ParseError error, MatchLevel level);
        ParseError(const State &state, std::string message, MatchLevel level, size_t kind);
    };
}
