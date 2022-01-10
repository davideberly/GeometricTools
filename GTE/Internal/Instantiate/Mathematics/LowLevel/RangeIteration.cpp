#include <Mathematics/RangeIteration.h>
#include <iostream>
#include <vector>

namespace gte
{
    void TestRangeIteration()
    {
        std::vector<int> numbers(4);
        int i = 0;
        for (auto& number : numbers)
        {
            number = i++;
            std::cout << number << ' ';
        }
        
        for (auto& number : gte::reverse(numbers))
        {
            std::cout << number << ' ';
        }
    }
}
