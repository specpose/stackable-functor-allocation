#include "stackable-functor-allocation/mole.hpp"
#include <sycl/sycl.hpp>
#include <vector>
#include <iostream>
using data_type = int;
using container_type = std::vector<data_type>;
using buffer_type = sycl::buffer<data_type>;
using params_type = std::tuple<data_type>;
template<> struct Adjacent_differences<buffer_type> {
    sycl::buffer<params_type> params{ sycl::range<1>{1} };
    std::vector<params_type> params_data{ {} };
    Adjacent_differences(sycl::queue& queue) : _queue(queue) {
        params.set_final_data(params_data.data());
    }
    static void forward(buffer_type& input, buffer_type& output, decltype(params)& p) {}
    static void inverse(buffer_type& input, buffer_type& output, decltype(params)& p) {}
    sycl::queue& _queue;
};
template<> struct Amplify<buffer_type> {
    sycl::buffer<params_type> params{ sycl::range<1>{1} };
    std::vector<params_type> params_data{ {} };
    Amplify(sycl::queue& queue, int s) : _queue(queue) {
        std::get<0>(params_data.front()) = s;
        params.set_final_data(params_data.data());
    }
    static void forward(buffer_type& input, buffer_type& output, decltype(params)& p) {}
    static void inverse(buffer_type& input, buffer_type& output, decltype(params)& p) {}
    sycl::queue& _queue;
};
static auto my_handler = [](sycl::exception_list e_list) { for (std::exception_ptr const& e : e_list) {try { std::rethrow_exception(e);} catch (std::exception const& e) {std::terminate();}}};
/*std::ostream& operator<<(std::ostream& os, const buffer_type& vec) {
    std::for_each(vec.begin(), --vec.end(), [&](typename buffer_type::value_type e) {os << e << ","; });
    os << *(--vec.end());
    return os;
}*/
int main() {
    sycl::queue queue(sycl::cpu_selector_v,my_handler);
    std::tuple<buffer_type, buffer_type, buffer_type> nodes{ buffer_type{sycl::range{}},buffer_type{sycl::range{}},buffer_type{sycl::range{}} };
    std::tuple<Adjacent_differences<buffer_type>, Amplify<buffer_type>> edges{ Adjacent_differences<buffer_type>(queue), Amplify<buffer_type>(queue,2) };
    /*std::get<0>(nodes) = buffer_type{ 1,0,1,0,-1,2,3,1,0,-1,-3,-5 };
    std::cout << "Sink: " << std::get<0>(nodes) << std::endl;
    std::get<1>(nodes) = buffer_type(Adjacent_differences<buffer_type>::size(std::get<0>(nodes)));
    std::get<2>(nodes) = buffer_type(Amplify<buffer_type>::size(std::get<1>(nodes)));
    std::get<0>(edges).forward(std::get<0>(nodes), std::get<1>(nodes), std::get<0>(edges).params);
    std::cout << "Jammed1: " << std::get<1>(nodes) << std::endl;
    std::fill(std::get<0>(nodes).begin(), std::get<0>(nodes).end(), 0);
    std::get<1>(edges).forward(std::get<1>(nodes), std::get<2>(nodes), std::get<1>(edges).params);
    std::cout << "Source: " << std::get<2>(nodes) << std::endl;
    std::get<0>(std::get<1>(edges).params) = 1;
    //sync to GPU
    std::get<1>(edges).inverse(std::get<1>(nodes), std::get<2>(nodes), std::get<1>(edges).params);
    std::cout << "Jammed2: " << std::get<1>(nodes) << std::endl;
    std::get<0>(edges).inverse(std::get<0>(nodes), std::get<1>(nodes), std::get<0>(edges).params);
    std::cout << "Sink: " << std::get<0>(nodes) << std::endl;*/
}