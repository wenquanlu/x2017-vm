#include <stdlib.h>
#include <stdio.h>
struct operation {
    unsigned char opcode;
    unsigned char type1;
    unsigned char opr1;
    unsigned char type2;
    unsigned char opr2;
};

struct func {
    unsigned char label;
    struct operation * op_ls;
    struct func *next;
    unsigned char len;
};

void free_all(struct func * fpt) {
    while (fpt) {
        struct func * next_pt = fpt -> next;
        free(fpt -> op_ls);
        free(fpt);
        fpt = next_pt;
    }
}

char get_bits(int bit_shift, int displacement, int length, FILE *fp, unsigned char * byte_buf) {
    if (bit_shift > 8 - length) {
        fseek(fp, -displacement, SEEK_END);
        fread(byte_buf, sizeof(*byte_buf), 1, fp);
        unsigned char second = *byte_buf;
        fseek(fp, -(displacement)-1, SEEK_END);
        fread(byte_buf, sizeof(*byte_buf), 1, fp);
        unsigned char first = *byte_buf;
        unsigned short concat = ((short) first << 8) | ((0x00ff & second));
        unsigned char result = (concat >> bit_shift) & ((1 << length) - 1);    
        return result;
    } else {
        fseek(fp, -displacement, SEEK_END);
        fread(byte_buf, sizeof(*byte_buf), 1, fp);
        char result = (*byte_buf >> bit_shift) & ((1 << length) - 1);
        return result;
    }
}

char get_symbol(int index) {
    if (index <= 25) {
        return 'A' + index;
    } else {
        return 'a' + index;
    }
}

void print_op(unsigned char data_type, unsigned char data, char * symbol_ls, int symbol_pt, struct func * func_ls) {
    if (data_type == 0b00) {
        printf(" VAL %d", data);
    } else if (data_type == 0b01) {
        printf(" REG %d", data);
    } else if (data_type == 0b10) {
        printf(" STK ");
        int exist = 0;
        for (int i = 0; i < symbol_pt; i++) {
            if (symbol_ls[i] == data) {
                printf("%c", get_symbol(i));
                exist = 1;
                break;
            }
        }
        if (!exist) {
            fprintf(stderr, "symbol refered before init\n");
            free_all(func_ls);
            exit(1);
        }
    } else if (data_type == 0b11) {
        printf(" PTR ");
        int exist = 0;
        for (int i = 0; i < symbol_pt; i++) {
            if (symbol_ls[i] == data) {
                printf("%c", get_symbol(i));
                exist = 1;
                break;
            }
        }
        if (!exist) {
            fprintf(stderr, "symbol refered before init\n");
            free_all(func_ls);
            exit(1);
        }
    }
}

void parse_binary(FILE * fp, struct func ** func_ls, int size) {
    unsigned char byte_buf;
    int stage = 1;
    int in_func = 0;
    int func_pt = 0;
    int bit_count = 0;  
    int func_length = 0;
    unsigned char curr_type = 0;
    unsigned char curr_opco_type = 0;
    struct func * this_func = NULL;
    struct operation * this_op;
    struct operation * this_op_ls = NULL;
    while (1) {
        int displacement = bit_count / 8 + 1;
        int inbyte_dis;
        inbyte_dis = bit_count%8;
        if (stage == 1) {
            if (!in_func) {
                if (size*8 - bit_count < 8) {
                    break;
                }
                this_func = (struct func *) malloc(sizeof(struct func));
                func_length = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf);
                this_op_ls = (struct operation *) malloc(sizeof(struct operation) * func_length);
                this_func -> len = func_length;
                in_func = 1;
                bit_count += 5;
                stage = 1;
            }else if (func_length == func_pt) {
                unsigned char func = get_bits(inbyte_dis, displacement, 3, fp, &byte_buf);
                this_func -> op_ls = this_op_ls;
                this_func -> label = func;
                this_func -> next = *func_ls;
                *func_ls = this_func;
                bit_count += 3;
                in_func = 0;
                func_length = 0;
                func_pt = 0;
                stage = 1;  

            } else  {
                this_op = (struct operation *) malloc(sizeof(struct operation));
                unsigned char opcode = get_bits(inbyte_dis, displacement, 3, fp, &byte_buf);
                curr_opco_type = opcode;
                bit_count += 3;
                stage = 2;
                if (opcode == 0b010) {
                    this_op -> opcode = 0b010;
                    this_op_ls[func_length - func_pt - 1] = *this_op;
                    free(this_op);
                    stage = 1;
                } else {
                    this_op -> opcode = opcode;
                }
                func_pt ++;
            }
        } else if (stage == 2) {
            unsigned char data_type = get_bits(inbyte_dis, displacement, 2, fp, &byte_buf);
            curr_type = data_type;
            this_op -> type1 = data_type;
            bit_count += 2;
            stage = 3;
        } else if (stage == 3) {
            unsigned char data  = 0;
            if (curr_type == 0b00) {
                data = get_bits(inbyte_dis, displacement, 8, fp, &byte_buf);
                bit_count += 8;
            } else if (curr_type == 0b01) {
                data = get_bits(inbyte_dis, displacement, 3, fp, &byte_buf);
                bit_count += 3;
            } else if (curr_type == 0b10) {
                data = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf);
                bit_count += 5;
              
            } else if (curr_type == 0b11) {
                data = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf);
                bit_count += 5;
            } 
            stage = 4;
            this_op -> opr1 = data;
            if (curr_opco_type == 0b001 || curr_opco_type == 0b101 || 
            curr_opco_type == 0b110 || curr_opco_type == 0b111) {
                stage = 1;
                this_op_ls[func_length - func_pt] = *this_op;
                free(this_op);
            }

        } else if (stage == 4) {
            unsigned char data_type = get_bits(inbyte_dis, displacement, 2, fp, &byte_buf);
            curr_type = data_type;
            this_op -> type2 = data_type;
            bit_count += 2;
            stage = 5;
        } else if (stage == 5) {
            unsigned char data = 0;
            if (curr_type == 0b00) {
                data = get_bits(inbyte_dis, displacement, 8, fp, &byte_buf);
                bit_count += 8;
            } else if (curr_type == 0b01) {
                data = get_bits(inbyte_dis, displacement, 3, fp, &byte_buf);
                bit_count += 3;
            } else if (curr_type == 0b10) {
                data = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf);
                bit_count += 5;
            } else if (curr_type == 0b11) {
                data = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf);
                bit_count += 5;
            }
            stage = 1;
            this_op -> opr2 = data;
            this_op_ls[func_length - func_pt] = *this_op;
            free(this_op);
        }
    }
}

void print_code(struct func * fpt) {
    struct func * func_ls = fpt;
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
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt, func_ls);
                printf("\n");
            } else if (op.opcode == 0b101) {
                printf("    PRINT");
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt, func_ls);
                printf("\n");
            } else if (op.opcode == 0b110) {
                printf("    NOT");
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt, func_ls);
                printf("\n");
            } else if (op.opcode == 0b111) {
                printf("    EQU");
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt, func_ls);
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
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt, func_ls);
                print_op(op.type2, op.opr2, symbol_ls, symbol_pt, func_ls);
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
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt, func_ls);
                print_op(op.type2, op.opr2, symbol_ls, symbol_pt, func_ls);
                printf("\n");
            } else if (op.opcode == 0b100) {
                printf("    ADD");
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt, func_ls);
                print_op(op.type2, op.opr2, symbol_ls, symbol_pt, func_ls);
                printf("\n");
            }
        }
        fpt = fpt -> next;
    }
}

int main(int argc, char **argv) {
    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        fprintf(stderr, "open file error\n");
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
    free_all(fpt2);
}
