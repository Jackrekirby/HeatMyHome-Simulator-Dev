#include <iostream>
#include <sstream>
#include <cmath>
extern "C"
{

    class Animal
    {
    public:
        int age;

        Animal(int age)
            : age(age)
        {
            std::cout << "age: " << age << "\n";
        }
    }

    int_sqrt(int x)
    {
        std::cout << "called int_sqrt"
                  << "\n";

        Animal dear(78);
        return std::sqrt(x);
    }

    // int main(int argc, char **argv)
    // {
    //     std::cout << "You have entered " << argc
    //               << " arguments:"
    //               << "\n";

    //     for (int i = 0; i < argc; ++i)
    //     {
    //         std::cout << argv[i] << "\n";
    //     }

    //     std::istringstream ss(argv[1]);
    //     int n;
    //     if (ss >> n)
    //     {
    //         int total = 0;
    //         for (int i = 0; i < n; ++i)
    //         {
    //             total += i;
    //             std::cout << "This is i: " << i << ", " << total << '\n';
    //         }
    //         return total;
    //     }

    //     return 0;
    // }
}