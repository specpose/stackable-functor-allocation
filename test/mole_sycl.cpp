#include "stackable-functor-allocation/mole.hpp"
#include <sycl/sycl.hpp>
#include <vector>
#include <iostream>
using data_type = int;
using buffer_type = sycl::buffer<data_type>;
using params_type = std::tuple<data_type>;
static auto my_handler = [](sycl::exception_list e_list) { for (std::exception_ptr const& e : e_list) { try { std::rethrow_exception(e); } catch (std::exception const& e) { std::terminate(); } }};
template<> struct MOLE::Node<buffer_type> {
    using input_type = buffer_type;
    using output_type = input_type;
    Node(
        buffer_type& input,
        std::function<std::size_t(buffer_type&)> f = [](buffer_type& input) -> std::size_t { return input.size(); }
    ) : _input(input), _output(sycl::range<1>{f(input)}) { }
    buffer_type& _input;
    buffer_type _output;
};
template<> struct Adjacent_differences<buffer_type> : public MOLE::Node<buffer_type> {
    sycl::buffer<params_type> params{ sycl::range<1>{1} };
    std::vector<params_type> params_data{ {} };
    Adjacent_differences(sycl::queue& queue, buffer_type& input) : MOLE::Node<buffer_type>(input), _queue(queue) {
        params.set_final_data(params_data.data());
    }
    static void forward(buffer_type& input, buffer_type& output, decltype(params)& p) {}
    static void inverse(buffer_type& input, buffer_type& output, decltype(params)& p) {}
    sycl::queue& _queue;
};
template<> struct Amplify<buffer_type> : public MOLE::Node<buffer_type> {
    sycl::buffer<params_type> params{ sycl::range<1>{1} };
    std::vector<params_type> params_data{ {} };
    Amplify(sycl::queue& queue, buffer_type& input, int s=1) : MOLE::Node<buffer_type>(input), _queue(queue) {
        std::get<0>(params_data.front()) = s;
        params.set_final_data(params_data.data());
    }
    static void forward(buffer_type& input, buffer_type& output, decltype(params)& p) {}
    static void inverse(buffer_type& input, buffer_type& output, decltype(params)& p) {}
    sycl::queue& _queue;
};
int main(int, char**) {
    sycl::queue queue(sycl::cpu_selector_v,my_handler);
    std::vector<typename buffer_type::value_type> inputVector{ 1,0,1,0,-1,2,3,1,0,-1,-3,-5 };
    buffer_type inputData{ inputVector };
    Adjacent_differences<buffer_type> adj(queue, inputData);
    Amplify<buffer_type> ampl(queue, adj._output);
    std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges{ adj, ampl };
}