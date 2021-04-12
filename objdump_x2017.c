#include <stdlib.h>
#include <stdio.h>
#include "vm.h"

char get_symbol(int index) {
    if (index <= 25) {
        return 'A' + index;
    } else {
        return 'a' + index;
    }
}

void print_op(unsigned char data_type, unsigned char data, 
              char * symbol_ls, int symbol_pt) {
    if (data_type == 0b00) {
        printf(" VAL %d", data);
    } else if (data_type == 0b01) {
        printf(" REG %d", data);
    } else if (data_type == 0b10) {
        printf(" STK ");
        for (int i = 0; i < symbol_pt; i++) {
            if (symbol_ls[i] == data) {
                printf("%c", get_symbol(i));
                break;
            }
        }
    } else if (data_type == 0b11) {
        printf(" PTR ");
        for (int i = 0; i < symbol_pt; i++) {
            if (symbol_ls[i] == data) {
                printf("%c", get_symbol(i));
                break;
            }
        }
    }
}

void print_code(struct func * fpt) {
    while (fpt) {
        char symbol_ls[32];
        int symbol_pt = 0;
        printf("FUNC LABEL %d\n", fpt -> label);
        for (int i = 0; i < fpt -> len; i++) {
            struct operation op = fpt -> op_ls[i];
            if (op.opcode == 0b010) {
                printf("    RET\n");
            } else if (op.opcode == 0b001) {
                printf("    CAL");
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt);
                printf("\n");
            } else if (op.opcode == 0b101) {
                printf("    PRINT");
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt);
                printf("\n");
            } else if (op.opcode == 0b110) {
                printf("    NOT");
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt);
                printf("\n");
            } else if (op.opcode == 0b111) {
                printf("    EQU");
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt);
                printf("\n");
            } else if (op.opcode == 0b000) {
                printf("    MOV");
                if (op.type1 == 0b10) {
                    int exist = 0;
                    for (int i = 0; i < symbol_pt; i++) {
                        if (op.opr1 == symbol_ls[i]) {
                            exist = 1;
                        }
                    }
                    if (!exist) {
                        symbol_ls[symbol_pt] = op.opr1;
                        symbol_pt++;
                    }
                }
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt);
                print_op(op.type2, op.opr2, symbol_ls, symbol_pt);
                printf("\n");
            } else if (op.opcode == 0b011) {
                printf("    REF");
                if (op.type1 == 0b10) {
                    int exist = 0;
                    for (int i = 0; i < symbol_pt; i++) {
                        if (op.opr1 == symbol_ls[i]) {
                            exist = 1;
                        }
                    }
                    if (!exist) {
                        symbol_ls[symbol_pt] = op.opr1;
                        symbol_pt++;
                    }
                }
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt);
                print_op(op.type2, op.opr2, symbol_ls, symbol_pt);
                printf("\n");
            } else if (op.opcode == 0b100) {
                printf("    ADD");
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt);
                print_op(op.type2, op.opr2, symbol_ls, symbol_pt);
                printf("\n");
            }
        }
        fpt = fpt -> next;
    }
}

int main(int argc, char **argv) {
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        exit(1);
    }
    int size = 0;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, -1, SEEK_END);
    struct func * func_ls = NULL;
    parse_binary(fp, &func_ls, size);
    struct func * fpt = func_ls;
    print_code(fpt);
    struct func * fpt2 = func_ls;
    while (fpt2) {
        struct func * next_pt = fpt2 -> next;
        free(fpt2 -> op_ls);
        free(fpt2);
        fpt2 = next_pt;
    }
}