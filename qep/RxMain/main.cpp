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

void test_1() {
	auto sc = rxsc::make_new_thread();
	auto so = synchronize_in_one_worker(sc);
	composite_subscription xyz = interval(std::chrono::seconds(1), so) | subscribe<long>(
		[](long val) {
			std::cout << "interval " << val << std::endl;
		});
	auto abc = range(0,10, so) | subscribe<int>( [](int val) {
			std::cout << "range " << val << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		});
	auto abc1 = range(0, 10) | subscribe<int>([](int val) {
		std::cout << "rangex " << val << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		});
	std::cout << "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX" << std::endl;
	std::this_thread::sleep_for(std::chrono::seconds(30));
}

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
	test_1();
	cout << "Hello CMake." << endl;

}
