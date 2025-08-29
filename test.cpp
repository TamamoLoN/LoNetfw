#include <iostream>
#include <vector>
template <typename T> static uint32_t countBytes(T val)
{
    uint32_t result = 0;
    for (; val; ++result)
    {
        val &= val - 1;
    }
    return result;
}
int main(int argc, char const *argv[])
{
    std::vector<int> v(1);
    std::cout << v.size() << std::endl;

    std::cout << countBytes(0b1111111111111111) << std::endl;
    return 0;
}
