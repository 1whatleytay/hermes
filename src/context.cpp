#include <hermes/context.h>

#include <hermes/error.h>

namespace hermes {
    std::vector<char> Context::hard = {
        ':',
        ';',
        ',',
        '.',
        '{',
        '}',
        '+',
        '-',
        '=',
        '/',
        '\\',
        '@',
        '#',
        '$',
        '%',
        '^',
        '&',
        '|',
        '*',
        '(',
        ')',
        '!',
        '?',
        '<',
        '>',
        '~',
        '[',
        ']',
        '\"',
        '\''
    };

    bool Context::notSpace(const char *text, size_t) {
        return !std::isspace(*text);
    }

    bool Context::anyHard(const char *text, size_t) {
        return std::isspace(*text)
            || (std::find(hard.begin(), hard.end(), *text) != hard.end());
    }

    void Context::mark(MatchLevel level) {
        matchLevel = level;
    }

    void Context::error(const std::string &value) {
        throw ParseError(state, value,
            matchLevel == MatchLevel::Strong, matchLevel == MatchLevel::Light);
    }

    void Context::needs(const std::string &value, bool exclusive) {
        if ((!exclusive || state.complete(value.size(), stoppable)) && state.pull(value.size()) == value) {
            state.pop(value.size(), popStoppable);
        } else {
            std::stringstream stream;
            auto spaceStop = [](const char *text, size_t size) { return std::isspace(*text); };
            stream << "Expected \"" << value  << "\" but got \"" << state.pull(state.until(spaceStop)) << "\".";

            error(stream.str());
        }
    }

    bool Context::next(const std::string &value, bool exclusive) {
        bool works = (!exclusive || state.complete(value.size(), stoppable)) && state.pull(value.size()) == value;

        if (works) {
            state.pop(value.size(), popStoppable);
        }

        return works;
    }

    bool Context::peek(const std::string &value, bool exclusive) {
        return (!exclusive || state.complete(value.size(), stoppable)) && state.pull(value.size()) == value;
    }

    std::string Context::token() {
        std::string result = state.pull(state.until(stoppable));
        state.pop(result.size(), popStoppable);

        if (result.empty())
            error("Expected token but got stoppable character.");

        return result;
    }

    std::string Context::until(const std::vector<std::string> &values) {
        std::string result = state.pull(state.until([&values](const char *text, size_t size) {
            return std::any_of(values.begin(), values.end(), [text, size](const std::string &value) {
                return size >= value.size() && std::memcmp(text, value.c_str(), value.size()) == 0;
            });
        }));
        state.pop(result.size(), popStoppable);

        return result;
    }

    size_t Context::select(const std::vector<std::string> &values, bool optional) {
        for (size_t a = 0; a < values.size(); a++) {
            if (next(values[a])) {
                return a;
            }
        }

        constexpr size_t selectOptionsCount = 3;

        if (!optional) {
            std::stringstream stream;

            stream << "Expected one of ";

            for (size_t a = 0; a < std::min<size_t>(selectOptionsCount, values.size()); a++) {
                if (a != 0)
                    stream << " or ";

                stream << "\"" << values[a] << "\"";
            }

            if (values.size() > selectOptionsCount)
                stream << "...";

            auto spaceStop = [](const char *text, size_t size) { return std::isspace(*text); };
            stream << " but got " << state.pull(state.until(spaceStop)) << ".";

            error(stream.str());
        }

        return values.size();
    }

    std::unique_ptr<Context> Context::pick(const std::vector<Link> &links, bool optional) {
        size_t start = state.index;

        std::unique_ptr<ParseError> error;

        for (const Link &link : links) {
            try {
                return link(this);
            } catch (const ParseError &e) {
                if (e.matches)
                    throw;

                state.index = start;

                if (!e.light || !error)
                    error = std::make_unique<ParseError>(e);
            }
        }

        if (!optional && error) {
            throw ParseError(*error);
        }

        return nullptr;
    }

    bool Context::push(const std::vector<Link> &links, bool optional) {
        std::unique_ptr<Context> pointer = pick(links, optional);

        bool result = pointer.get();

        if (pointer) {
            children.push_back(std::move(pointer));
        }

        return result;
    }

    Context::Context(Context *parent, ssize_t kind)
        : state(parent->state), parent(parent), index(parent->state.index), kind(kind),
        stoppable(parent->stoppable), popStoppable(parent->popStoppable) { }
    Context::Context(State &state, ssize_t kind)
        : state(state), parent(nullptr), index(state.index), kind(kind) { }
}
