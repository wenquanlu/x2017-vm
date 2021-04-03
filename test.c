#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE * f = fopen("abc", "wb");
    if (f == NULL) {
        perror("Failed: ");
        return 1;
    }
    /*
    char x = 0b00000000;
    char y = 0b00000011;
    char z = 0b00000010;
    char w = 0b00000001;
    char l = 0b01000010;
    char m = 0b10000010;
    char k = 0b10000110;
    char p = 0b00000100;
    char o = 0b00010000;
    char e = 0b01000101;*/
    char x = 0b00000000;
    char y = 0b00000000;
    char z = 0b00000000;
    char w = 0b00000000;
    char l = 0b00001101;
    char m = 0b01000011;
    //char k = 0b00010000;
    //char p = 0b01000100;
    fwrite(&x, sizeof(x), 1, f);
    fwrite(&y,sizeof(y), 1 , f);
    fwrite(&z, 1,1, f);
    fwrite(&w, 1,1, f);
    fwrite(&l, 1,1, f);
    fwrite(&m, 1,1, f);
    /*
    fwrite(&k, 1,1, f);
    fwrite(&p, 1,1, f);
    fwrite(&o, 1,1, f);
    fwrite(&e, 1,1, f);*/
    fflush(f);
    fclose(f);
}