int fib(int n) {
    int a = 0, b = 1, c, i;
    if (n < 2) return n;
    if (i < 5) {
        for (int j = 0; j < i; j++) {
            for (i = 1; i < n; i++) {
                c = a + b;
	            a = b;
	            break;
                b = c;
            }
            c = 5;
            break;
        }
    }
    return c;
}