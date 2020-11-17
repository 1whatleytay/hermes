#include <hermes/error.h>

namespace hermes {
    LineDetails::LineDetails(const std::string &text, size_t index, bool backtrack) {
        size_t lineIndex = index;

        if (backtrack) {
            if (lineIndex > 0)
                lineIndex--;

            while (lineIndex == text.size() || (lineIndex > 0 && std::isspace(text[lineIndex])))
                lineIndex--;
        }

        // this is slow i think
        auto lineStart = text.rfind('\n', lineIndex);
        if (lineStart == std::string::npos) {
            lineStart = 0;
        } else {
            lineStart++;
        }

        auto lineEnd = text.find('\n', lineIndex);
        if (lineEnd == std::string::npos) {
            lineEnd = text.size();
        }

        line = text.substr(lineStart, lineEnd - lineStart);

        auto linePos = lineIndex - lineStart;

        std::stringstream markerStream;

        for (size_t a = 0; a < linePos; a++) {
            if (std::isspace(line[a]))
                markerStream << line[a];
            else
                markerStream << ' ';
        }
        markerStream << '^';

        marker = markerStream.str();

        lineNumber = std::count(text.begin(), text.begin() + lineStart, '\n') + 1;
    }

    const char *ParseError::what() const noexcept {
        return issue.c_str();
    }

    ParseError::ParseError(ParseError error, MatchLevel level) : ParseError(std::move(error)) {
        this->level = level;
    }

    ParseError::ParseError(const State &state, std::string message, MatchLevel level, size_t kind)
        : issue(std::move(message)), index(state.index), level(level), kind(kind) { }
}
