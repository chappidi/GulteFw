#include "Sample_A.h"
#include "Sample_B.h"
#include "Sample_C.h"
#include <iostream>

// total loop count across threads
constexpr int LOOP_SIZE = 1000000;

int main()
{
	//std::cout << "SAMPLE_A" << std::endl << std::endl << std::endl;
	//SAMPLE_A<spin_lock>::test();
	std::cout << "SAMPLE_B" << std::endl << std::endl << std::endl;
	SAMPLE_B<no_lock>::test();
	//std::cout << "SAMPLE_C" << std::endl << std::endl << std::endl;
	//SAMPLE_C<no_lock>::test();
}
