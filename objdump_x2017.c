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

void print_op(unsigned char data_type, unsigned char data, char * symbol_ls, int symbol_pt) {
    if (data_type == 0b00) {
        printf(" VAL %d", data);
    } else if (data_type == 0b01) {
        printf(" REG %d", data);
    } else if (data_type == 0b10) {
        printf(" STK ");
        for (int i = 0; i < symbol_pt; i++) {
            if (symbol_ls[i] == data) {
                printf("%c", get_symbol(symbol_pt - i - 1));
                break;
            }
        }
    } else if (data_type == 0b11) {
        printf(" PTR ");
        for (int i = 0; i < symbol_pt; i++) {
            if (symbol_ls[i] == data) {
                printf("%c", get_symbol(symbol_pt - i - 1));
                break;
            }
        }
    }
}

int parse_binary(FILE * fp, struct func ** func_ls, char * symbol_ls, int size) {
    unsigned char byte_buf;
    int stage = 1;
    int in_func = 0;
    int func_pt = 0;
    int bit_count = 0;  
    int func_length;
    enum opera_type {
        val, reg, stac, pt
    };
    enum opco_type {
        mov, cal, ret, ref, add, print, not, equ
    };
    enum opera_type curr_type;
    enum opco_type curr_opco_type;
    int symbol_pt = 0;
    struct func * this_func;
    struct operation * this_op;
    struct operation * this_op_ls;
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
                if (opcode == 0b000) {
                    curr_opco_type = mov;
                } else if (opcode == 0b001) {
                    curr_opco_type = cal;
                } else if (opcode == 0b010) {
                    curr_opco_type = ret;
                } else if (opcode == 0b011) {
                    curr_opco_type = ref;
                } else if (opcode == 0b100) {
                    curr_opco_type = add;
                } else if (opcode == 0b101) {
                    curr_opco_type = print;
                } else if (opcode == 0b110) {
                    curr_opco_type = not;
                } else if (opcode == 0b111) {
                    curr_opco_type = equ;
                }
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
            if (data_type == 0b00) {
                curr_type = val;
            } else if (data_type == 0b01) {
                curr_type = reg;
            } else if (data_type == 0b10) {
                curr_type = stac;
            } else if (data_type == 0b11) {
                curr_type = pt;
            }
            this_op -> type1 = data_type;
            bit_count += 2;
            stage = 3;
        } else if (stage == 3) {
            unsigned char data;
            if (curr_type == val) {
                data = get_bits(inbyte_dis, displacement, 8, fp, &byte_buf);
                bit_count += 8;
            } else if (curr_type == reg) {
                data = get_bits(inbyte_dis, displacement, 3, fp, &byte_buf);
                bit_count += 3;
            } else if (curr_type == stac) {
                data = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf);
                int exist = 0;
                for (int i = 0; i < symbol_pt; i++) {
                    if (data == symbol_ls[i]) {
                        for (int j = i; j < symbol_pt - 1; j++) {
                            symbol_ls[j] = symbol_ls[j+1];
                        }
                        symbol_ls[symbol_pt - 1] = data;
                        exist = 1;
                    }
                }
                if (!exist) {
                    symbol_ls[symbol_pt] = data;
                    symbol_pt ++;
                }
                bit_count += 5;
              
            } else if (curr_type == pt) {
                data = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf);
                for (int i = 0; i < symbol_pt; i++) {
                    if (data == symbol_ls[i]) {
                        for (int j = i; j < symbol_pt - 1; j++) {
                            symbol_ls[j] = symbol_ls[j+1];
                        }
                        symbol_ls[symbol_pt - 1] = data;
                    }
                }
                bit_count += 5;
            } 
            stage = 4;
            this_op -> opr1 = data;
            if (curr_opco_type == cal || curr_opco_type == print || 
            curr_opco_type == not || curr_opco_type == equ) {
                stage = 1;
                this_op_ls[func_length - func_pt] = *this_op;
                free(this_op);
            }

        } else if (stage == 4) {
            unsigned char data_type = get_bits(inbyte_dis, displacement, 2, fp, &byte_buf);
            if (data_type == 0b00) {
                curr_type = val;
            } else if (data_type == 0b01) {
                curr_type = reg;
            } else if (data_type == 0b10) {
                curr_type = stac;
            } else if (data_type == 0b11) {
                curr_type = pt;
            }
            this_op -> type2 = data_type;
            bit_count += 2;
            stage = 5;
        } else if (stage == 5) {
            unsigned char data;
            if (curr_type == val) {
                data = get_bits(inbyte_dis, displacement, 8, fp, &byte_buf);
                bit_count += 8;
            } else if (curr_type == reg) {
                data = get_bits(inbyte_dis, displacement, 3, fp, &byte_buf);
                bit_count += 3;
            } else if (curr_type == stac) {
                data = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf);
                int exist = 0;
                for (int i = 0; i < symbol_pt; i++) {
                    if (data == symbol_ls[i]) {
                        for (int j = i; j < symbol_pt - 1; j++) {
                            symbol_ls[j] = symbol_ls[j+1];
                        }
                        symbol_ls[symbol_pt - 1] = data;
                        exist = 1;
                    }
                }
                if (!exist) {
                    symbol_ls[symbol_pt] = data;
                    symbol_pt ++;
                }
                bit_count += 5;
            } else if (curr_type == pt) {
                data = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf);
                for (int i = 0; i < symbol_pt; i++) {
                    if (data == symbol_ls[i]) {
                        for (int j = i; j < symbol_pt - 1; j++) {
                            symbol_ls[j] = symbol_ls[j+1];
                        }
                        symbol_ls[symbol_pt - 1] = data;
                    }
                }
                bit_count += 5;
            }
            stage = 1;
            this_op -> opr2 = data;
            this_op_ls[func_length - func_pt] = *this_op;
            free(this_op);
        }
    }
    return symbol_pt;
}

void print_code(struct func * fpt, char * symbol_ls, int symbol_pt) {
    while (fpt) {
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
                print_op(op.type1, op.opr1, symbol_ls, symbol_pt);
                print_op(op.type2, op.opr2, symbol_ls, symbol_pt);
                printf("\n");
            } else if (op.opcode == 0b011) {
                printf("    REF");
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
    char symbol_ls[52];
    struct func * func_ls = NULL;
    int symbol_pt = parse_binary(fp, &func_ls, symbol_ls, size);
    struct func * fpt = func_ls;
    print_code(fpt, symbol_ls, symbol_pt);
    struct func * fpt2 = func_ls;
    while (fpt2) {
        struct func * next_pt = fpt2 -> next;
        free(fpt2 -> op_ls);
        free(fpt2);
        fpt2 = next_pt;
    }
}
