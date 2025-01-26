#include<tuple>
#include<algorithm>
//Use n nodes and n-1 bidirectional edges => Not SFA!
namespace MOLE {
    template<typename container_t> struct Invertible {
        Invertible() {}
        //std::tuple<> params{};
        //static decltype(params) forward(container_t& input, container_t& output, decltype(params)& p) = delete;//virtual
        //static decltype(params) inverse(container_t& input, container_t& output, decltype(params)& p) = delete;//virtual
        static std::size_t size(const container_t& input) = delete;//virtual
    };
}
template<typename container_t> struct Adjacent_differences : public MOLE::Invertible<container_t> {
    Adjacent_differences() : MOLE::Invertible<container_t>{} {}
    std::tuple<typename container_t::value_type> params{ 0 };
    static void forward(container_t& input, container_t& output, decltype(params)& p) {
        std::get<0>(p) = input.at(0);
        for (std::size_t i = 1; i < input.size(); i++)
            output.at(i - 1) = input.at(i) - input.at(i - 1);
    }
    static void inverse(container_t& input, container_t& output, decltype(params)& p) {
        input.at(0) = std::get<0>(p);
        for (std::size_t i = 0; i < output.size(); i++)
            input.at(i + 1) = input.at(i) + output.at(i);
    }
    static std::size_t size(const container_t& input) { return input.size() - 1; }
};
template<typename container_t> struct Amplify : public MOLE::Invertible<container_t> {
    Amplify() : MOLE::Invertible<container_t>{} {}
    Amplify(typename container_t::value_type factor) : params{ factor } {}
    std::tuple<typename container_t::value_type> params{ 0 };
    static void forward(container_t& input, container_t& output, decltype(params)& p) {
        std::transform(input.begin(), input.end(), output.begin(), [&](auto in) { return std::get<0>(p) * in; });
    }
    static void inverse(container_t& input, container_t& output, decltype(params)& p) {
        std::transform(output.begin(), output.end(), input.begin(), [&](auto out) { return out / std::get<0>(p); });
    }
    static std::size_t size(const container_t& input) { return input.size(); }
};
