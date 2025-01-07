#include <iostream>
using namespace std;

int fib(int n) {
    int a = 0, b = 1, c, i; 
    if (n < 2) 
        return n; 
    
    for (i = 1; i < n; i++) {
        c = a + b;
        a = b;
        b = c;
    }
    
    return c; 
}
