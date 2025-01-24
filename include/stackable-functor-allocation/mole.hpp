//Use n nodes and n-1 bidirectional edges => Not SFA!
namespace MOLE {
    template<typename container_t> struct Invertible {
        Invertible() {}
        virtual void forward(container_t& input, container_t& output) = 0;
        virtual void inverse(container_t& input, container_t& output) = 0;
        static std::size_t size(const container_t& input) = delete;
    };
}
template<typename container_t> struct Adjacent_differences : public MOLE::Invertible<container_t> {
    Adjacent_differences() : MOLE::Invertible<container_t>{} {}
    virtual void forward(container_t& input, container_t& output) {
        first_value = input.at(0);
        for (std::size_t i = 1; i < input.size(); i++)
            output.at(i - 1) = input.at(i) - input.at(i - 1);
    }
    virtual void inverse(container_t& input, container_t& output) {
        input.at(0) = first_value;
        for (std::size_t i = 0; i < output.size(); i++)
            input.at(i + 1) = input.at(i) + output.at(i);
    }
    static std::size_t size(const container_t& input) { return input.size() - 1; }
    typename container_t::value_type first_value = 0;
};
template<typename container_t> struct Amplify : public MOLE::Invertible<container_t> {
    Amplify() : MOLE::Invertible<container_t>{} {}
    Amplify(typename container_t::value_type factor) : factor(factor) {}
    virtual void forward(container_t& input, container_t& output) {
        std::transform(input.begin(), input.end(), output.begin(), [&](auto in) { return factor * in; });
    }
    virtual void inverse(container_t& input, container_t& output) {
        std::transform(output.begin(), output.end(), input.begin(), [&](auto out) { return out / factor; });
    }
    static std::size_t size(const container_t& input) { return input.size(); }
    typename container_t::value_type factor = 1;
};