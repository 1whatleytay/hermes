#include <hermes/node.h>

#include <hermes/error.h>

#include <algorithm>

namespace hermes {
    std::vector<char> Node::hard = {
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

    bool Node::notSpace(const char *text, size_t) {
        return !std::isspace(*text);
    }

    bool Node::anyHard(const char *text, size_t) {
        return std::isspace(*text)
            || (std::find(hard.begin(), hard.end(), *text) != hard.end());
    }

    const State &Node::getState() const {
        return state;
    }

    bool Node::end() const {
        return state.index == state.text.size();
    }

    void Node::match() {
        level = MatchLevel::Strong;
    }

    void Node::error(const std::string &value) {
        throw ParseError(state, value, level, kind);
    }

    bool Node::peek(const std::string &value, bool exclusive) {
        return (!exclusive || state.complete(value.size(), tokenStoppable)) && state.pull(value.size()) == value;
    }

    bool Node::next(const std::string &value, bool exclusive) {
        bool works = (!exclusive || state.complete(value.size(), tokenStoppable)) && state.pull(value.size()) == value;

        if (works)
            state.pop(value.size(), spaceStoppable);

        return works;
    }

    void Node::needs(const std::string &value, bool exclusive) {
        if ((!exclusive || state.complete(value.size(), tokenStoppable)) && state.pull(value.size()) == value) {
            state.pop(value.size(), spaceStoppable);
        } else {
            std::stringstream stream;
            auto spaceStop = [](const char *text, size_t size) { return std::isspace(*text); };
            stream << "Expected \"" << value  << "\" but got \"" << state.pull(state.until(spaceStop)) << "\".";

            error(stream.str());
        }
    }

    void Node::match(const std::string &value, bool exclusive) {
        needs(value, exclusive);
        match();
    }

    std::string Node::token() {
        std::string result = state.pull(state.until(tokenStoppable));
        state.pop(result.size(), spaceStoppable);

        if (result.empty())
            error("Expected token but got stoppable character.");

        return result;
    }

    std::string Node::until(const std::vector<std::string> &values) {
        std::string result = state.pull(state.until([&values](const char *text, size_t size) {
            return std::any_of(values.begin(), values.end(), [text, size](const std::string &value) {
                return size >= value.size() && std::memcmp(text, value.c_str(), value.size()) == 0;
            });
        }));
        state.pop(result.size(), spaceStoppable);

        return result;
    }

    size_t Node::select(const std::vector<std::string> &values, bool exclusive, bool optional) {
        for (size_t a = 0; a < values.size(); a++) {
            if (next(values[a], exclusive)) {
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

    std::unique_ptr<Node> Node::pick(const std::vector<Link> &links, bool optional) {
        size_t start = state.index;

        std::unique_ptr<ParseError> error;

        for (const Link &link : links) {
            try {
                return link(this);
            } catch (const ParseError &e) {
                if (e.level == MatchLevel::Strong)
                    throw;

                state.index = start;

                if (e.level != MatchLevel::Light || !error)
                    error = std::unique_ptr<ParseError>(new ParseError(e));
            }
        }

        if (!optional && error) {
            if (level == MatchLevel::Strong)
                throw ParseError(*error, level);
            else
                throw ParseError(*error);
        }

        return nullptr;
    }

    bool Node::push(const std::vector<Link> &links, bool optional) {
        std::unique_ptr<Node> pointer = pick(links, optional);

        bool result = pointer.get();

        if (pointer) {
            children.push_back(std::move(pointer));
        }

        return result;
    }

    Node::Node(Node *parent)
        : state(parent->state), parent(parent), index(parent->state.index),
        tokenStoppable(parent->tokenStoppable), spaceStoppable(parent->spaceStoppable) { }
    Node::Node(State &state)
        : state(state), parent(nullptr), index(state.index) { }
}
