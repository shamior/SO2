void
*memset(void *dst, int c, int len) {
    char *d = (char *) dst;
    for(int i = 0; i < len; i++) {
        d[i] = c;
    }
    return d;
}