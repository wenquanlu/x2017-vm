#include <stdio.h>
#include <stdlib.h>
int main() {
    FILE * fi = fopen("abc", "wb");
    if (fi == NULL) {
        perror("Failed: ");
        return 1;
    }
    char a = 0b00000000;
    char b = 0b01000000;
    char c = 0b00001000;
    char d = 0b00000011;
    char e = 0b10000001;
    char f = 0b10000000;
    char g = 0b00100000;
    char h = 0b10110000;
    char i = 0b01110000;
    char j = 0b10000110;
    char k = 0b00001011;
    char l = 0b00000111;
    char m = 0b00001000;
    char n = 0b00101010;
    char o = 0b00011010;
    char p = 0b10100100;
    char q = 0b11110000;
    char r = 0b10000010;
    char s = 0b00000000;
    char t = 0b11000010;
    char u = 0b00000000;
    char v = 0b10000010;
    char w = 0b10000010;
    char x = 0b10000110;
    char y = 0b00000100;
    char z = 0b00011000;
    char lwq = 0b01000110;
    fwrite(&a, 1, 1, fi);
    fwrite(&b, 1, 1, fi);
    fwrite(&c, 1, 1, fi);
    fwrite(&d, 1, 1, fi);
    fwrite(&e, 1, 1, fi);
    fwrite(&f, 1, 1, fi);
    fwrite(&g, 1, 1, fi);
    fwrite(&h, 1, 1, fi);
    fwrite(&i, 1, 1, fi);
    fwrite(&j, 1, 1, fi);
    fwrite(&k, 1, 1, fi);
    fwrite(&l, 1, 1, fi);
    fwrite(&m, 1, 1, fi);
    fwrite(&n, 1, 1, fi);
    fwrite(&o, 1, 1, fi);
    fwrite(&p, 1, 1, fi);
    fwrite(&q, 1, 1, fi);
    fwrite(&r, 1, 1, fi);
    fwrite(&s, 1, 1, fi);
    fwrite(&t, 1, 1, fi);
    fwrite(&u, 1, 1, fi);
    fwrite(&v, 1, 1, fi);
    fwrite(&w, 1, 1, fi);
    fwrite(&x, 1, 1, fi);
    fwrite(&y, 1, 1, fi);
    fwrite(&z, 1, 1, fi);
    fwrite(&lwq, 1, 1, fi);
    fflush(fi);
    fclose(fi);

}