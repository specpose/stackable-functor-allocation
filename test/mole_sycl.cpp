#include "stackable-functor-allocation/mole.hpp"
#include <sycl/sycl.hpp>
#include <vector>
#include <iostream>
using data_type = int;
using buffer_type = sycl::buffer<data_type>;
using params_type = std::tuple<data_type>;
auto async_handler_object = [](sycl::exception_list exceptions) {
    for (auto e : exceptions) {
        try {
            std::rethrow_exception(e);
        }
        catch (sycl::exception const& e) {
            std::cout << "Caught asynchronous SYCL exception:\n"<< e.what() << std::endl;
        }
    }
};
static sycl::queue queue = sycl::queue(sycl::cpu_selector_v, async_handler_object, { sycl::property::queue::in_order{} });;
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
    std::array<params_type,1> params_data;
    Adjacent_differences(buffer_type& input) : MOLE::Node<buffer_type>(input, [](buffer_type& input) -> std::size_t { return input.size() - 1; }),
                                                params_data{ params_type{ 0 } },
                                                params(params_data)
    {
        params.set_final_data(params_data.data());
    }
    void forward() {
        queue.submit([&](sycl::handler& cgh) {
            sycl::accessor in{ _input, cgh, sycl::read_only };
            sycl::accessor out{ _output, cgh, sycl::write_only };
            sycl::accessor p{ params, cgh, sycl::write_only };
            //std::get<0>(p[0]) = in[0];
            cgh.single_task<class k1>([=]() {
                std::get<0>(p[0]) = in[0];
                for (size_t i = 1; i < in.size(); i++) {
                    out[i-1] = in[i] - in[i-1];
                }
            });
        });
        queue.wait();
    }
    void inverse() {
        queue.submit([&](sycl::handler& cgh) {
            sycl::accessor in{ _input, cgh, sycl::write_only };
            sycl::accessor out{ _output, cgh, sycl::read_only };
            sycl::accessor p{ params, cgh, sycl::read_only };
            cgh.single_task<class k2>([=]() {
                in[0] = std::get<0>(p[0]);
                for (size_t i = 0; i < out.size(); i++) {
                    in[i+1] = in[i] + out[i];
                }
            });
        });
        queue.wait();
    }
};
template<> struct Amplify<buffer_type> : public MOLE::Node<buffer_type> {
    sycl::buffer<params_type> params;
    std::array<params_type,1> params_data;
    //sycl::accessor<params_type, 1, sycl::access_mode::read, sycl::target::device> p;
    //sycl::accessor<params_type, 1, sycl::access_mode::write, sycl::target::host_task> params_accessor;
    Amplify(buffer_type& input, int s=1) : MOLE::Node<buffer_type>(input),
                                            params_data{ params_type{ 1 } },
                                            params(params_data)
                                            //,p( params )
                                            //,params_accessor(params)
    {
        params.set_final_data(params_data.data());
    }
    void forward(){
        queue.submit([&](sycl::handler& cgh) {
            sycl::accessor in{ _input, cgh, sycl::read_only };
            sycl::accessor out{ _output, cgh, sycl::write_only };
            sycl::accessor p{ params, cgh, sycl::read_only };
            //cgh.require(p);
            cgh.parallel_for<class k3>(sycl::range{ out.size() }, [=](sycl::id<1> idx)
            {
                out[idx] = std::get<0>(p[0]) * in[idx];
            });
        });
        queue.wait();
    }
    void inverse(){
        queue.submit([&](sycl::handler& cgh) {
            sycl::accessor in{ _input, cgh, sycl::write_only };
            sycl::accessor out{ _output, cgh, sycl::read_only };
            sycl::accessor p{ params, cgh, sycl::read_only };
            //cgh.require(p);
            cgh.parallel_for<class k4>(sycl::range{ in.size() }, [=](sycl::id<1> idx)
            {
                in[idx] = out[idx] / std::get<0>(p[0]);
            });
        });
        queue.wait();
    }
    void setFactor(data_type s) {
        sycl::host_accessor<params_type>* factor = new sycl::host_accessor<params_type>(params);
        std::get<0>((*factor)[0])=s;
        delete factor;
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
    MOLE::get<1>(edges).setFactor(2);
    std::cout << "Stack size is " << std::tuple_size<std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>>>{} << std::endl;
    try {
    queue.wait();
    std::cout << "Sink: " << inputData << std::endl;
    MOLE::get<0>(edges).forward();
    std::cout << "Jammed1: " << MOLE::get<0>(edges)._output << std::endl;
    MOLE::get<1>(edges).forward();
    std::cout << "Source: " << MOLE::get<1>(edges)._output << std::endl;

    //sync to GPU
    MOLE::get<1>(edges).setFactor(1);

    MOLE::get<1>(edges).inverse();
    std::cout << "Jammed2: " << MOLE::get<0>(edges)._output << std::endl;
    MOLE::get<0>(edges).inverse();
    std::cout << "Sink: " << inputData << std::endl;
    queue.wait_and_throw();
    } catch (const sycl::exception& e) {
    std::cout << "Exception caught: " << e.what() << std::endl;
    }
}