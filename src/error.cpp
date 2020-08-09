#include <hermes/error.h>

namespace hermes {
    const char *ParseError::what() const noexcept {
        return issue.c_str();
    }

    ParseError::ParseError(const State &state, const std::string &message, bool matches) : matches(matches) {
        size_t index = state.index;

        if (index > 0)
            index--;

        while (index == state.text.size() || (index > 0 && std::isspace(state.text[index])))
            index--;

        // this is slow i think
        auto lineStart = state.text.rfind('\n', index);
        if (lineStart == std::string::npos) {
            lineStart = 0;
        } else {
            lineStart++;
        }

        auto lineEnd = state.text.find('\n', index);
        if (lineEnd == std::string::npos) {
            lineEnd = state.text.size();
        }

        std::string line = state.text.substr(lineStart, lineEnd - lineStart);

        auto linePos = index - lineStart;

        std::stringstream stream;
        stream << message << "\n" << line << "\n" << std::string(linePos, ' ') << "^";

        issue = stream.str();
    }
}
