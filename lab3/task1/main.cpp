
#include <iostream>

int hello (int a)
{
    int res = a + a;
    return res; 
}

int main()
{
    int result = hello(5);

     std::cout << "Result: " << result << std::endl;

    return 0;
}
