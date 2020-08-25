#pragma once

#include <hermes/error.h>
#include <hermes/state.h>

#include <vector>
#include <functional>
#include <unordered_map>

namespace hermes {
    class Context;

    using Link = std::function<std::unique_ptr<Context>(Context *parent)>;

    class Context {
    public:
        static std::vector<char> hard;
        static bool notSpace(const char *text, size_t size);
        static bool anyHard(const char *text, size_t size);

        size_t kind = 0;
        size_t index = 0;
        Context *parent = nullptr;
        Stoppable stoppable = anyHard;
        Stoppable popStoppable = notSpace;
        MatchLevel level = MatchLevel::Light;
        std::vector<std::unique_ptr<Context>> children;

    protected:
        State &state;

        void error(const std::string &value);

        void needs(const std::string &value, bool exclusive = false);
        bool next(const std::string &value, bool exclusive = false);
        bool peek(const std::string &value, bool exclusive = false);

        std::string token();
        std::string until(const std::vector<std::string> &values);
        size_t select(const std::vector<std::string> &values, bool optional = false);

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
        bool push(bool optional, A ... args) {
            return push({ link<T>(args ...) }, optional);
        }

        template <typename T>
        bool push() {
            return push({ link<T>() }, false);
        }

        template <typename T, typename ... A>
        Link link(A ... args) {
            return [args ...](Context *p) {
                return std::make_unique<T>(p, args ...);
            };
        }

    public:
        template <typename T>
        T *as() {
            return dynamic_cast<T *>(this);
        }

        template <typename T>
        T is() {
            return static_cast<T>(kind);
        }

        template <typename T>
        bool is(T value) {
            return is<T>() == value;
        }

        explicit Context(Context *parent);
        explicit Context(State &state);

        template <typename T>
        Context(Context *parent, T kind) : Context(parent) {
            this->kind = static_cast<size_t>(kind);
        }

        template <typename T>
        Context(State &state, T kind) : Context(state) {
            this->kind = static_cast<size_t>(kind);
        }

        virtual ~Context() = default;
    };
}
