#include <iostream>
#include <rxcpp/rx.hpp>

namespace Rx {
	using namespace rxcpp;
	using namespace rxcpp::sources;
	using namespace rxcpp::operators;
	using namespace rxcpp::util;
}
using namespace Rx;

using namespace std;

void test_2() {
	auto ints = rxcpp::observable<>::create<int>(
		[](rxcpp::subscriber<int> s) {
			for (int itr = 1; itr < 100; ++itr) {
				s.on_next(itr);
				std::this_thread::sleep_for(std::chrono::milliseconds(70));
			}
			s.on_completed();
		});
	auto mnz = ints | last();
	auto xyz = mnz | subscribe<int>(
		[](int  val) {
			std::cout << "create" << val << std::endl;
		});
}
int main()
{
	test_2();
	cout << "Hello CMake." << endl;

}
