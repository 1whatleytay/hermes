#pragma once

#include <hermes/error.h>
#include <hermes/state.h>

#include <memory>
#include <vector>
#include <optional>
#include <functional>
#include <unordered_map>

namespace hermes {
    template <typename T>
    using SelectMap = std::vector<std::pair<std::string, T>>;

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

        [[nodiscard]] const State &getState() const;

        State &state;

        [[nodiscard]] bool end() const;
        void match();
        void error(const std::string &value);

        bool peek(const std::string &value, bool exclusive = false);
        bool next(const std::string &value, bool exclusive = false);
        void needs(const std::string &value, bool exclusive = false);
        void match(const std::string &value, bool exclusive = false);

        std::string token();
        std::string until(const std::vector<std::string> &values);
        size_t attempt(const std::vector<std::string> &values, bool exclusive = false, bool optional = false);

        template <typename T>
        T select(const SelectMap<T> &values, bool exclusive = false) {
            std::vector<std::string> options(values.size());
            std::transform(values.begin(), values.end(), options.begin(), [](const auto &pair) { return pair.first; });

            return values[attempt(options, exclusive)].second;
        }

        template <typename T>
        std::optional<T> maybe(const SelectMap<T> &values, bool exclusive = false) {
            std::vector<std::string> options(values.size());
            std::transform(values.begin(), values.end(), options.begin(), [](const auto &pair) { return pair.first; });

            size_t result = attempt(options, exclusive, true);

            return (result == options.size()) ? std::nullopt : std::optional<T>(values[result].second);
        }

        template <typename T>
        T decide(const SelectMap<T> &values, T defaultValue, bool exclusive = false) {
            auto v = maybe(values, exclusive);

            return v ? *v : defaultValue;
        }

        std::unique_ptr<Node> pick(const std::vector<Link> &links, bool optional = false);
        bool push(const std::vector<Link> &links, bool optional = false);

        template <typename T, typename ... Args>
        std::unique_ptr<T> pick(bool optional, Args &&... args) {
            return std::unique_ptr<T>(dynamic_cast<T *>(pick({
                link<T>(std::forward<Args>(args) ...)
            }, optional).release()));
        }

        template <typename T>
        std::unique_ptr<T> pick() {
            return std::unique_ptr<T>(dynamic_cast<T *>(pick({ link<T>() }, false).release()));
        }

        template <typename T, typename N, typename ...Args>
        std::unique_ptr<Node> pick(bool optional = false) {
            return pick(links<T, N, Args...>(), optional);
        }

        template <typename T, typename ...Args>
        bool push(bool optional, Args &&...args) {
            return push({ link<T>(std::forward<Args>(args) ...) }, optional);
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
        Link link(Args... args) {
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

        template <typename T>
        T *as() {
            return dynamic_cast<T *>(this);
        }

        template <typename T>
        const T *as() const {
            return dynamic_cast<const T *>(this);
        }

        template <typename T>
        T is() const {
            return static_cast<T>(kind);
        }

        template <typename T>
        bool is(T value) const {
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
