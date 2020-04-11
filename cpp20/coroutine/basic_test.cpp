#include <gtest/gtest.h>
#include <iostream>
#include <experimental/generator>

struct BasicTest : public testing::Test
{
    const double cpu_freq = 2994370145;
};


std::experimental::generator<int> one_two_three()
{
    std::cout << "Going to yield 1" << std::endl;
    co_yield 1;
    std::cout << "Going to yield 2" << std::endl;
    co_yield 2;
    std::cout << "Going to yield 3" << std::endl;
    co_yield 3;
}
template <typename T>
std::experimental::generator<int> range(T first, T last)
{
    while (first != last)
    {
        std::cout << "Going to yield " << first << std::endl;
        co_yield first++;
    }
}

TEST_F(BasicTest, coyield_1)
{
    for (int i : range(0, 10))
    {
        printf("%d\n", i);
    }
    std::cout << "Hello World!\n";
}

TEST_F(BasicTest, coyield_2)
{
    for (int i : one_two_three())
    {
        printf("%d\n", i);
    }
}
