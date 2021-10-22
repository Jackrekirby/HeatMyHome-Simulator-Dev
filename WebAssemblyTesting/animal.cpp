#include <iostream>
#include "animal.h"

Animal::Animal(int age)
    : age(age)
{
    std::cout << "age: " << age << "\n";
}