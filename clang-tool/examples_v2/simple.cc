#include <iostream>

struct Base {
public:
    void example_method () {
        std::cout << "Hello_Base" << std::endl;
    }

public:
    virtual void example_method_virt () {}

private:
    void example_method_private () {}

public:
    int example_field;

private:
    int example_field_private;
};

struct Derived : Base {
private:
    void example_method_private_v2 () {}

public:
    void example_method() {
        
    }

public:
    int example_field_v2;
};  
