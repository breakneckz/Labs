#include <iostream>

static int hello(int a)
{
    return (a + a);
};

int main() {
    int result = hello(5); 
    std::cout << "hello result: " << result << std::endl;
    return 0;
}
