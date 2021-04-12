#include <stdio.h>
#ifndef VM_H
#define VM_H
struct operation {
        unsigned char opcode;
        unsigned char type1;
        unsigned char opr1;
        unsigned char type2;
        unsigned char opr2;
};

struct func {
    unsigned char label;
    unsigned char len;
    unsigned char symbol_num;
    struct operation * op_ls;
    struct func *next;
};
char get_bits(int, int, int, FILE *, unsigned char *);

void parse_binary(FILE *, struct func **, int);
#endif