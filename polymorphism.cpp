#include <iostream>
#include <memory>

// Classic polymorphism
struct ISomeStruct
{
public:
    virtual ~ISomeStruct() = default;
    virtual void doSomething() = 0;
};

struct SomeStruct : public ISomeStruct
{
public:
    void doSomething() override
    {
        std::cout << "Hello from SomeStruct!" << std::endl;
    }
};

void check_classic_polymorphism(ISomeStruct &entry)
{
    entry.doSomething();
}

// template polymorphism
template <typename SomeType>
void check_template_polymorphism(SomeType &entry)
{
    entry.doSomething();
}

struct SomeOtherStruct
{
public:
    void doSomething()
    {
        std::cout << "Hello from SomeOtherStruct!" << std::endl;
    }
};

int main()
{
    std::unique_ptr<ISomeStruct> ptr = std::make_unique<SomeStruct>();
    check_classic_polymorphism(*ptr);

    SomeStruct value;
    check_template_polymorphism(value);

    SomeOtherStruct other;
    // Compile error
    // check_classic_polymorphism(other);
    check_template_polymorphism(other);

    return 0;
}