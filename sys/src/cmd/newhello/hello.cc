#include <stdio.h>

class TestCPP
{
public:
    TestCPP()
    {
        printf("Hello C++\r\n");
    }

    ~TestCPP()
    {
        printf("Byt C++\r\n");
    }
};

TestCPP t;

extern "C"
int main(void)
{
    printf("Hello World\n");

    return 0;
}