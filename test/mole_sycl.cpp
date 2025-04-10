#include "stackable-functor-allocation/mole.hpp"
#include <sycl/sycl.hpp>
#include "stl-tuple/STLTuple.hpp"
#include <vector>
#include <iostream>
using data_type = int;
using buffer_type = sycl::buffer<data_type>;
using params_type = utility::tuple::Tuple<data_type>;
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
    sycl::buffer<params_type> params;
    std::vector<params_type> params_data{ { 0 } };
    Adjacent_differences(buffer_type& input) : MOLE::Node<buffer_type>(input, [](buffer_type& input) -> std::size_t { return input.size() - 1; }),
                                                params(params_data.data(), sycl::range<1>{1})
    {
    }
    static void forward(buffer_type& input, buffer_type& output, decltype(params)& params) {
        queue.submit([&](sycl::handler& cgh) {
            sycl::accessor in{ input, cgh, sycl::read_only };
            sycl::accessor out{ output, cgh, sycl::write_only };
            sycl::accessor p{ params, cgh, sycl::read_write };
            //std::get<0>(p[0]) = in[0];
            cgh.single_task<class k1>([=]() {
                utility::tuple::get<0>(p[0]) = in[0];
                for (size_t i = 1; i < in.size(); i++) {
                    out[i-1] = in[i] - in[i-1];
                }
            });
        });
    }
    static void inverse(buffer_type& input, buffer_type& output, decltype(params)& params) {
        queue.submit([&](sycl::handler& cgh) {
            sycl::accessor in{ input, cgh, sycl::write_only };
            sycl::accessor out{ output, cgh, sycl::read_only };
            sycl::accessor p{ params, cgh, sycl::read_only };
            cgh.single_task<class k2>([=]() {
                in[0] = utility::tuple::get<0>(p[0]);
                for (size_t i = 0; i < out.size(); i++) {
                    in[i+1] = in[i] + out[i];
                }
            });
        });
    }
};
template<> struct Amplify<buffer_type> : public MOLE::Node<buffer_type> {
    sycl::buffer<params_type> params;
    std::vector<params_type> params_data{ { 1 } };
    Amplify(buffer_type& input, int s=1) : MOLE::Node<buffer_type>(input),
                                            params(params_data.data(), sycl::range<1>{1})
    {
        sycl::host_accessor factor(params, sycl::write_only);
        utility::tuple::get<0>(factor[0]) = s;
    }
    static void forward(buffer_type& input, buffer_type& output, decltype(params)& params) {
        queue.submit([&](sycl::handler& cgh) {
            sycl::accessor in{ input, cgh, sycl::read_only };
            sycl::accessor out{ output, cgh, sycl::write_only };
            sycl::accessor p{ params, cgh, sycl::read_write };
            cgh.parallel_for<class k3>(sycl::range{ out.size() }, [=](sycl::id<1> idx)
            {
                out[idx] = utility::tuple::get<0>(p[0]) * in[idx];
            });
        });
    }
    static void inverse(buffer_type& input, buffer_type& output, decltype(params)& params) {
        queue.submit([&](sycl::handler& cgh) {
            sycl::accessor in{ input, cgh, sycl::write_only };
            sycl::accessor out{ output, cgh, sycl::read_only };
            sycl::accessor p{ params, cgh, sycl::read_only };
            cgh.parallel_for<class k4>(sycl::range{ in.size() }, [=](sycl::id<1> idx)
            {
                in[idx] = out[idx] / utility::tuple::get<0>(p[0]);
            });
        });
    }
};
std::ostream& operator<<(std::ostream& os, buffer_type& buf) {
    //sycl::accessor jammed1 = buf.get_access<sycl::access_mode::read>();
    //sycl::host_accessor source = buf.get_host_access();
    sycl::host_accessor vec(buf, sycl::read_only);
    std::for_each(vec.begin(), --vec.end(), [&](typename decltype(vec)::value_type e) {os << e << ","; });
    os << *(--vec.end());
    return os;
}
int main(int, char**) {
    std::vector<typename buffer_type::value_type> inputVector{ 1,0,1,0,-1,2,3,1,0,-1,-3,-5 };
    buffer_type inputData{ inputVector };
    MOLE::Stack<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges(inputData);
    std::cout << "Stack size is " << std::tuple_size<std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>>>{} << std::endl;
    try {
    queue = sycl::queue(sycl::cpu_selector_v, { sycl::property::queue::in_order{} });
    //sycl::host_accessor factor(MOLE::get<1>(edges).params, sycl::write_only);
    std::cout << "Sink: ";
    std::cout << inputData << std::endl;
    MOLE::get<0>(edges).forward(MOLE::get<0>(edges)._input, MOLE::get<0>(edges)._output, MOLE::get<0>(edges).params);
    std::cout << "Jammed1: ";
    std::cout << MOLE::get<0>(edges)._output << std::endl;
    MOLE::get<1>(edges).forward(MOLE::get<1>(edges)._input, MOLE::get<1>(edges)._output, MOLE::get<1>(edges).params);
    std::cout << "Source: ";
    std::cout << MOLE::get<1>(edges)._output << std::endl;

    //sync to GPU

    MOLE::get<1>(edges).inverse(MOLE::get<1>(edges)._input, MOLE::get<1>(edges)._output, MOLE::get<1>(edges).params);
    std::cout << "Jammed2: ";
    std::cout << MOLE::get<0>(edges)._output << std::endl;
    MOLE::get<0>(edges).inverse(MOLE::get<0>(edges)._input, MOLE::get<0>(edges)._output, MOLE::get<0>(edges).params);
    std::cout << "Sink: ";
    std::cout << inputData << std::endl;
    queue.throw_asynchronous();
    } catch (const sycl::exception& e) {
        std::cout << "Exception caught: " << e.what() << std::endl;
    }
}
