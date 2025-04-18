#include<tuple>
#include<algorithm>
#include<functional>
//Use n nodes and n-1 bidirectional edges => Not SFA!
namespace MOLE {
    template<typename container_t> struct Node {
        using input_type = container_t;
        using output_type = input_type;
        Node(
            container_t& input,
            std::function<std::size_t(container_t&)> f = [](container_t& input) -> std::size_t { return input.size(); }
        ) : _input(input), _output(f(input)) {}
        container_t& _input;
        container_t _output;
    };
    template<typename... Empty> struct Stack {
        Stack() {}
    };
    template<typename functor_t > struct Stack<functor_t> {
        using functor_type = functor_t;
        Stack(typename functor_t::input_type& input, std::size_t index = 0) : _myself(input), _index(index) {}
        functor_t _myself;
        std::size_t _index;
    };
    template<typename functor_t, typename... other_functors> struct Stack<functor_t, other_functors...> {
        using functor_type = functor_t;
        Stack(typename functor_t::input_type& input, std::size_t index=0) : _myself(input), _index(index), _others(_myself._output,++_index) {}
        functor_t _myself;
        std::size_t _index;
        Stack<other_functors...> _others;
    };
    //template<class T> struct Stack_size;
    //template<class... Types> struct Stack_size<Stack<Types...>> : std::integral_constant<std::size_t, sizeof...(Types)> {};

    template<std::size_t I, class T> struct Stack_element;
    template<std::size_t I, class Head, class... Tail> struct Stack_element<I, Stack<Head, Tail...>> : Stack_element<I - 1, Stack<Tail...>>{};
    template<class Head, class... Tail> struct Stack_element<0, Stack<Head, Tail...>> { using type = Head;};

    template<std::size_t I, class myType>
    myType& get(Stack<myType>& t) noexcept {
        return t;
    };
    template<std::size_t I, class myType, class... OtherTypes>
    typename Stack_element<I, Stack<myType, OtherTypes...>>::type& get(Stack<myType, OtherTypes...>& t) noexcept {
        void* element = &t;
        for (int i = I; i > 0; i--) {
            element = reinterpret_cast<void*>(&(t._others));
        }
        auto ref = reinterpret_cast<typename Stack_element<I, Stack<myType, OtherTypes...>>::type*>(element);
        return *ref;
    };
}
template<typename container_t> struct Adjacent_differences : public MOLE::Node<container_t> {
    Adjacent_differences(container_t& input) : MOLE::Node<container_t>(input, [](container_t& input) -> std::size_t { return input.size() - 1; }) {}
    std::tuple<typename container_t::value_type> params{ 0 };
    void forward() {
        std::get<0>(params) = this->_input.at(0);
        for (std::size_t i = 1; i < this->_input.size(); i++)
            this->_output.at(i - 1) = this->_input.at(i) - this->_input.at(i - 1);
    }
    void inverse() {
        this->_input.at(0) = std::get<0>(params);
        for (std::size_t i = 0; i < this->_output.size(); i++)
            this->_input.at(i + 1) = this->_input.at(i) + this->_output.at(i);
    }
};
template<typename container_t> struct Amplify : public MOLE::Node<container_t> {
    Amplify(container_t& input, typename container_t::value_type factor=1) : MOLE::Node<container_t>(input), params{ factor } {}
    std::tuple<typename container_t::value_type> params{ 0 };
    void forward() {
        std::transform(this->_input.begin(), this->_input.end(), this->_output.begin(), [&](auto in) { return std::get<0>(params) * in; });
    }
    void inverse() {
        std::transform(this->_output.begin(), this->_output.end(), this->_input.begin(), [&](auto out) { return out / std::get<0>(params); });
    }
};
