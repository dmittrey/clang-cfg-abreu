int fib(int n) {
    int a = 0, b = 1, c, i, k;
    if (n < 2) {
		return n;
	}
    for (i = 1; i < n; i++) {
        c = a + b;
		if (c == 0)
			continue;
        b = c;
    }
    return c;
}