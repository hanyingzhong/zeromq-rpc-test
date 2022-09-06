#include <string>

using std::string;

template <typename... Args> void g (Args... args)
{
    printf ("%d\r\n", sizeof...(Args));
//    std::cout << sizeof...(Args) << std::endl; //���Ͳ�������Ŀ
//    std::cout << sizeof...(args) << std::endl; //������������Ŀ
}

int main333333 ()
{
    int i = 0;
    double d = 3.14;
    string s = "how now brown cow";

    g (i, s, 42, d);
    g (s, 42, "hi");
    g (d, s);
    g ("hi");

    return 0;
}
