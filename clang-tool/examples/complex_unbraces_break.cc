int fib(int n) {
    int a = 0, b = 1, c, i, k;
    if (n < 2) {
		return n;
	}
    for (i = 1; i < n; i++) {
        c = a + b;
		// if (c == 0)
			// break;
        b = c;
        for (k = 0; k < n; k++) {
            // b = c;
            if (i == 1) {
                break;
            } else {
                for (int m = 0; m < n; m++) {
                    b = c;
                    if (i == 1) {
                        break;
                    } else {
                        continue;
                    }
                }
            }
        }
        // c = b;
    }
    return c;
}