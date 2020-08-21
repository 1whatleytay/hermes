#include <hermes/error.h>

namespace hermes {
    void ParseError::lineDetails(const std::string &text, size_t index, std::string &line, std::string &marker) {
        size_t lineIndex = index;

        if (lineIndex > 0)
            lineIndex--;

        while (lineIndex == text.size() || (lineIndex > 0 && std::isspace(text[lineIndex])))
            lineIndex--;

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
    }

    const char *ParseError::what() const noexcept {
        return issue.c_str();
    }

    ParseError::ParseError(const State &state, std::string message, bool matches, bool light)
        : matches(matches), light(light), issue(std::move(message)), index(state.index) { }
}
