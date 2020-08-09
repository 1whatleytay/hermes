#pragma once

#include <hermes/state.h>

#include <vector>
#include <functional>
#include <unordered_map>

namespace hermes {
    class Context;

    using Link = std::function<std::unique_ptr<Context>(Context *parent)>;

    class Context {
        bool matches = false;

    public:
        static std::vector<char> hard;
        static bool defaultStoppable(const char *text, size_t size);

        ssize_t kind = 0;
        size_t index = 0;
        Context *parent = nullptr;
        Stoppable stoppable = defaultStoppable;
        std::vector<std::unique_ptr<Context>> children;

    protected:
        State &state;

        void match(bool value = true);
        void error(const std::string &value);

        void needs(const std::string &value, bool exclusive = false);
        bool next(const std::string &value, bool exclusive = false);
        bool peek(const std::string &value, bool exclusive = false);

        std::string token();
        std::string until(const std::vector<std::string> &values);
        std::string select(const std::vector<std::string> &values);

        template <typename T>
        T select(const std::unordered_map<std::string, T> &values) {
            std::vector<std::string> keys;
            keys.reserve(values.size());

            for (const auto &pair : values)
                keys.push_back(pair.first);

            return values.at(select(keys));
        }

        std::unique_ptr<Context> pick(const std::vector<Link> &links, bool optional = false);
        bool push(const std::vector<Link> &links, bool optional = false);

        template <typename T, typename ... A>
        std::unique_ptr<T> pick(bool optional, A ... args) {
            return std::unique_ptr<T>(dynamic_cast<T *>(pick({ link<T>(args ...) }, optional).release()));
        }

        template <typename T>
        std::unique_ptr<T> pick() {
            return std::unique_ptr<T>(dynamic_cast<T *>(pick({ link<T>() }, false).release()));
        }

        template <typename T, typename ... A>
        Link link(A ... args) {
            return [args ...](Context *p) {
                return std::make_unique<T>(p, args ...);
            };
        }

    public:
        explicit Context(Context *parent, ssize_t kind = -1);
        explicit Context(State &state, ssize_t kind = -1);

        virtual ~Context() = default;
    };
}
