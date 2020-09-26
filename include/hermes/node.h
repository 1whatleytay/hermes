#pragma once

#include <hermes/error.h>
#include <hermes/state.h>

#include <vector>
#include <functional>
#include <unordered_map>

namespace hermes {
    class Node {
    public:
        using Link = std::function<std::unique_ptr<Node>(Node *parent)>;

        static std::vector<char> hard;
        static bool notSpace(const char *text, size_t size);
        static bool anyHard(const char *text, size_t size);

        size_t kind = 0;
        size_t index = 0;
        Node *parent = nullptr;
        MatchLevel level = MatchLevel::Priority;
        std::vector<std::unique_ptr<Node>> children;

        Stoppable tokenStoppable = anyHard;
        Stoppable spaceStoppable = notSpace;

    protected:
        State &state;

        bool end() const;
        void match();
        void error(const std::string &value);

        bool peek(const std::string &value, bool exclusive = false);
        bool next(const std::string &value, bool exclusive = false);
        void needs(const std::string &value, bool exclusive = false);
        void match(const std::string &value, bool exclusive = false);

        std::string token();
        std::string until(const std::vector<std::string> &values);
        size_t select(const std::vector<std::string> &values, bool exclusive = false, bool optional = false);

        template <typename T>
        T select(const std::vector<std::string> &values, bool exclusive = false, bool optional = false) {
            return static_cast<T>(select(values, exclusive, optional));
        }

        std::unique_ptr<Node> pick(const std::vector<Link> &links, bool optional = false);
        bool push(const std::vector<Link> &links, bool optional = false);

        template <typename T, typename ... A>
        std::unique_ptr<T> pick(bool optional, A ... args) {
            return std::unique_ptr<T>(dynamic_cast<T *>(pick({ link<T>(args ...) }, optional).release()));
        }

        template <typename T>
        std::unique_ptr<T> pick() {
            return std::unique_ptr<T>(dynamic_cast<T *>(pick({ link<T>() }, false).release()));
        }

        template <typename T, typename N, typename ...Args>
        std::unique_ptr<Node> pick(bool optional = false) {
            return pick(links<Args...>(), optional);
        }

        template <typename T, typename ...Args>
        bool push(bool optional, Args ...args) {
            return push({ link<T>(args ...) }, optional);
        }

        template <typename T>
        bool push() {
            return push({ link<T>() }, false);
        }

        template <typename T, typename N, typename ...Args>
        bool push(bool optional = false) {
            return push(links<T, N, Args...>(), optional);
        }

        template <typename T, typename ...Args>
        Link link(Args ...args) {
            return [args...](Node *p) {
                return std::unique_ptr<T>(new T(p, args...));
            };
        }

        template <typename T>
        std::vector<Link> links() {
            return { link<T>() };
        }

        template <typename T, typename N, typename ...Args>
        std::vector<Link> links() {
            std::vector<Link> result = links<T>();
            std::vector<Link> concat = links<N, Args...>();

            result.insert(result.end(), concat.begin(), concat.end());

            return result;
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

        explicit Node(Node *parent);
        explicit Node(State &state);

        template <typename T>
        Node(Node *parent, T kind) : Node(parent) {
            this->kind = static_cast<size_t>(kind);
        }

        template <typename T>
        Node(State &state, T kind) : Node(state) {
            this->kind = static_cast<size_t>(kind);
        }

        virtual ~Node() = default;
    };
}
