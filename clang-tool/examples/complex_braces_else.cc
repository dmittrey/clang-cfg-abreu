int fib(int n) {
    int a = 0, b = 1, c, i, k;
    if (n < 2) {
		return n;
	}
    for (i = 1; i < n; i++) {
        c = a + b;
		if (c == 0) {
            k = 5;
            if (c == 0) {
                break;
            }
            break;
		}   
        else {
            k = 4;
            for (i = 1; i < n; i++) {
                k = 4;
                break;
            }
            // continue;
        }
        b = c;
    }
    return c;
}