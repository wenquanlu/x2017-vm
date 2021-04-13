#include <stdio.h>
#ifndef VM_H
#define VM_H

// structure of a single line operation
// opcode: type of operation
// type1, opr1: first type and first value
// type2, opr2: second type and second value
struct operation {
        unsigned char opcode;
        unsigned char type1;
        unsigned char opr1;
        unsigned char type2;
        unsigned char opr2;
};

// stucture of a function
// label: 3 bits function label
// len: size of operations in the function
// symbol_num: number of symbols appear in the function
// op_ls: pointer to a list of operations
// func: pointer to next parsed function
struct func {
    unsigned char label;
    unsigned char len;
    unsigned char symbol_num;
    struct operation * op_ls;
    struct func *next;
};

char get_bits(int, int, int, FILE *, unsigned char *);

void parse_binary(FILE *, struct func **, int);

void free_all(struct func *);

int get_index(int, int, unsigned char *);

int get_symbol_index(unsigned char *, char, unsigned char * ram);

int get_stk_pt_index(int, unsigned char *);

int get_stk_func_index(int, unsigned char *);

int get_stk_len_index(int, unsigned char *);

void check_entry(struct func *);

struct func * get_func(struct func *, char);

void push_stack(unsigned char *, unsigned char *, unsigned char, struct func *);

char get_pointer(unsigned char *, char);

void execute(struct operation, unsigned char *, unsigned char *, struct func *);

int indirect(unsigned char *, char, unsigned char *);

void check_update_symbol(struct func *, unsigned char, unsigned char *, char *, int);

void check_update(struct func *);

char get_symbol(int);

void print_op(unsigned char, unsigned char, char *, int);

void print_code(struct func *);
#endif