#include "stackable-functor-allocation/mole.hpp"
#include <sycl/sycl.hpp>
#include <vector>
#include <iostream>
using data_type = int;
using buffer_type = sycl::buffer<data_type>;
using params_type = std::tuple<data_type>;
//static auto my_handler = [](sycl::exception_list e_list) { for (std::exception_ptr const& e : e_list) { try { std::rethrow_exception(e); } catch (std::exception const& e) { std::terminate(); } }};
static sycl::queue queue;
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
    Adjacent_differences(buffer_type& input) : MOLE::Node<buffer_type>(input, [](buffer_type& input) -> std::size_t { return input.size() - 1; }) {
        params.set_final_data(params_data.data());
    }
    static void forward(buffer_type& input, buffer_type& output) {
        queue.submit([&](sycl::handler& cgh) {
            sycl::accessor in{ input, cgh, sycl::read_only };
            sycl::accessor out{ output, cgh, sycl::write_only };
            cgh.single_task<class k1>([=]() {
                for (size_t i = 1; i < in.size(); i++) {
                    out[i-1] = in[i] - in[i-1];
                }
            });
        });
    }
    static void inverse(buffer_type& input, buffer_type& output) {
        queue.submit([&](sycl::handler& cgh) {
            sycl::accessor in{ input, cgh, sycl::write_only };
            sycl::accessor out{ output, cgh, sycl::read_only };
            cgh.single_task<class k2>([=]() {
                for (size_t i = 0; i < out.size(); i++) {
                    in[i+1] = in[i] + out[i];
                }
            });
        });
    }
};
template<> struct Amplify<buffer_type> : public MOLE::Node<buffer_type> {
    sycl::buffer<params_type> params{ sycl::range<1>{1} };
    std::vector<params_type> params_data{ {} };
    Amplify(buffer_type& input, int s=1) : MOLE::Node<buffer_type>(input) {
        std::get<0>(params_data.front()) = s;
        params.set_final_data(params_data.data());
    }
    static void forward(buffer_type& input, buffer_type& output) {
        queue.submit([&](sycl::handler& cgh) {
            sycl::accessor in{ input, cgh, sycl::read_only };
            sycl::accessor out{ output, cgh, sycl::write_only };
            cgh.parallel_for<class k3>(sycl::range{ out.size() }, [=](sycl::id<1> idx)
            {
                out[idx] = 2 * in[idx];
            });
        });
    }
    static void inverse(buffer_type& input, buffer_type& output) {
        queue.submit([&](sycl::handler& cgh) {
            sycl::accessor in{ input, cgh, sycl::write_only };
            sycl::accessor out{ output, cgh, sycl::read_only };
            cgh.parallel_for<class k4>(sycl::range{ in.size() }, [=](sycl::id<1> idx)
            {
                in[idx] = out[idx] / 1;
            });
        });
    }
};
int main(int, char**) {
    std::vector<typename buffer_type::value_type> inputVector{ 1,0,1,0,-1,2,3,1,0,-1,-3,-5 };
    buffer_type inputData{ inputVector };
    MOLE::Stack<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges(inputData);
    std::cout << "Stack size is " << std::tuple_size<std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>>>{} << std::endl;
    try {
    queue = sycl::queue(sycl::cpu_selector_v, { sycl::property::queue::in_order{} });
    sycl::host_accessor sink1(inputData, sycl::read_only);
    std::cout << "\nSink: ";
    for (int i = 0; i < sink1.size(); i++) { printf("%d,", sink1[i]); }
    MOLE::get<0>(edges).forward(MOLE::get<0>(edges)._input, MOLE::get<0>(edges)._output);
    sycl::accessor jammed1 = MOLE::get<0>(edges)._output.get_access<sycl::access_mode::read>();
    std::cout << "\nJammed1: ";
    for (int i = 0; i < jammed1.size(); i++) { printf("%d,", jammed1[i]); }
    MOLE::get<1>(edges).forward(MOLE::get<1>(edges)._input, MOLE::get<1>(edges)._output);
    sycl::host_accessor source = MOLE::get<1>(edges)._output.get_host_access();
    std::cout << "\nSource: ";
    for (int i = 0; i < source.size(); i++) { printf("%d,",source[i]); }

    //sync to GPU

    MOLE::get<1>(edges).inverse(MOLE::get<1>(edges)._input, MOLE::get<1>(edges)._output);
    //sycl::host_accessor jammed2 = MOLE::get<0>(edges)._output.get_host_access();
    //std::cout << "\nJammed2: ";
    //for (int i = 0; i < jammed2.size(); i++) { printf("%d,", jammed2[i]); }
    MOLE::get<0>(edges).inverse(MOLE::get<0>(edges)._input, MOLE::get<0>(edges)._output);
    //sycl::host_accessor sink2 = inputData.get_host_access();
    //std::cout << "\nSink: ";
    //for (int i = 0; i < sink2.size(); i++) { printf("%d,", sink2[i]); }
    queue.throw_asynchronous();
    } catch (const sycl::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }
}
