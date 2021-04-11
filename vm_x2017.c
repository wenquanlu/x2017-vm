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
    unsigned char len;
    unsigned char symbol_num;
    struct operation * op_ls;
    struct func *next;
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

int get_index(int stk_index, int symbol_index) {
    return (stk_index) * 34 + 2 + symbol_index;
}

int get_symbol_index(unsigned char * reg_bank, char symbol_index) {
    int stk_index = reg_bank[5] - 1;
    int int_symbol_index = symbol_index;
    return (stk_index) * 34 + 2 + int_symbol_index;
}

int get_stk_pt_index(int stk_index) {
    return (stk_index) * 34 + 1;
}

int get_stk_func_index(int stk_index) {
    return (stk_index) * 34;
}

void check_entry(struct func * func_ls) {
    struct func * pt = func_ls;
    while (pt) {
        if (pt -> label == 0b000) {
            return;
        }
        pt = pt -> next;
    }
    fprintf(stderr, "No entry point\n");
    free_all(func_ls);
    exit(1);
}

struct func * get_func(struct func * func_ls, char func_label) {
    struct func * pt = func_ls;
    while (pt) {
        if (pt -> label == func_label) {
            return pt;
        }
        pt = pt -> next;
    }
    fprintf(stderr, "function being called not defined\n");
    free_all(func_ls);
    exit(1);
}

void push_stack(unsigned char * ram, unsigned char * reg_bank, unsigned char func_label, struct func * func_ls) {
    if (get_index(reg_bank[5], 33) > 255) {
        fprintf(stderr, "stack over flow\n");
        free_all(func_ls);
        exit(1);
    }
    ram[get_stk_func_index(reg_bank[5])] = func_label;
    ram[get_stk_pt_index(reg_bank[5])] = 0;
    reg_bank[5] ++;
}

int is_cal(struct operation this_op) {
    return this_op.opcode == 0b001;
}

int is_ret(struct operation this_op) {
    return this_op.opcode == 0b010;
}

char get_func_label(struct operation this_op) {
    return this_op.opr1;
}

char get_pointer(unsigned char * reg_bank, char symbol) {
    char stk_index = reg_bank[5] - 1;
    return (stk_index << 5) + symbol;
}

int indirect(unsigned char * reg_bank, char ptr_symbol, unsigned char * ram) {
    //int curr_stk_index = reg_bank[5] - 1;
    char ptr = ram[get_symbol_index(reg_bank, ptr_symbol)];
    char ptr_stk_index = (ptr >> 5) & 0b00000111;
    char ptr_sym_index = (ptr & 0b00011111);
    return get_index(ptr_stk_index, ptr_sym_index);
}

void execute(struct operation this_op, unsigned char * ram, unsigned char * reg_bank, struct func * func_ls) {
    if (this_op.opcode == 0b000) {
        if (this_op.type1 == 0b01 && this_op.type2 == 0b00) {
            int reg = this_op.opr1;
            reg_bank[reg] = this_op.opr2;
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b01) {
            int reg = this_op.opr2;
            ram[get_symbol_index(reg_bank, this_op.opr1)] = reg_bank[reg];
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b11) {
            int reg = this_op.opr1;
            reg_bank[reg] = ram[indirect(reg_bank, this_op.opr2, ram)];
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b01) {
            int reg = this_op.opr2;
            ram[indirect(reg_bank, this_op.opr1, ram)] = reg_bank[reg];
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b00) {
            ram[get_symbol_index(reg_bank, this_op.opr1)] = this_op.opr2;
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b00) {
            ram[indirect(reg_bank, this_op.opr1, ram)] = this_op.opr2;
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b10) {
            int reg = this_op.opr1;
            reg_bank[reg] = ram[get_symbol_index(reg_bank, this_op.opr2)];
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b11) {
            ram[indirect(reg_bank, this_op.opr1, ram)] =
            ram[indirect(reg_bank, this_op.opr2, ram)];
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b10) {
            ram[indirect(reg_bank, this_op.opr1, ram)] = 
            ram[get_symbol_index(reg_bank, this_op.opr2)];
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b11) {
            ram[get_symbol_index(reg_bank, this_op.opr1)] = 
            ram[indirect(reg_bank, this_op.opr2, ram)];
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b10) {
            ram[get_symbol_index(reg_bank, this_op.opr1)] = ram[get_symbol_index(reg_bank, this_op.opr2)];
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b00) {
            int reg = this_op.opr1;
            reg_bank[reg] = this_op.opr2;
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b01) {
            int reg_dest = this_op.opr1;
            int reg_src = this_op.opr2;
            reg_bank[reg_dest] = reg_bank[reg_src];
        } else {
            fprintf(stderr, "invalid assembly format\n");
            free_all(func_ls);
            exit(1);
        }

    } else if (this_op.opcode == 0b011) {
        if (this_op.type1 == 0b01 && this_op.type2 == 0b10) {
            char address = get_pointer(reg_bank, this_op.opr2);
            int reg = this_op.opr1;
            reg_bank[reg] = address;
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b10) {
            char address = get_pointer(reg_bank, this_op.opr2);
            ram[get_symbol_index(reg_bank, this_op.opr1)] = address;
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b10) {
            char address = get_pointer(reg_bank, this_op.opr2);
            ram[indirect(reg_bank,this_op.opr1, ram)] = address;
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b11) {
            int reg = this_op.opr1;
            reg_bank[reg] = ram[get_symbol_index(reg_bank, this_op.opr2)];
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b11) {
            ram[get_symbol_index(reg_bank, this_op.opr1)] =
            ram[get_symbol_index(reg_bank, this_op.opr2)];
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b11) {
            ram[indirect(reg_bank,this_op.opr1, ram)] = 
            ram[get_symbol_index(reg_bank, this_op.opr2)];
        } else {
            fprintf(stderr, "invalid assembly format\n");
            free_all(func_ls);
            exit(1);
        }
    } else if (this_op.opcode == 0b100) {
        if (this_op.type1 == 0b01 && this_op.type2 == 0b01) {
            int reg1 = this_op.opr1;
            int reg2 = this_op.opr2;
            reg_bank[reg1] = reg_bank[reg1] + reg_bank[reg2];
        } else {
            fprintf(stderr, "invalid assembly format\n");
            free_all(func_ls);
            exit(1);
        }
    } else if (this_op.opcode == 0b101) {
        if (this_op.type1 == 0b00) {
            unsigned int content = this_op.opr1;
            printf("%u\n", content);
        } else if (this_op.type1 == 0b01) {
            int reg = this_op.opr1;
            unsigned int content = reg_bank[reg];
            printf("%u\n", content);
        } else if (this_op.type1 == 0b10) {
            unsigned int content = ram[get_symbol_index(reg_bank, this_op.opr1)];
            printf("%u\n", content);
        } else if (this_op.type1 == 0b11) {
            unsigned int content = ram[indirect(reg_bank, this_op.opr1, ram)];
            printf("%u\n", content);
        } else {
            fprintf(stderr, "invalid assembly format\n");
            free_all(func_ls);
            exit(1);
        }
    } else if (this_op.opcode == 0b110) {
        int reg = this_op.opr1;
        reg_bank[reg] = ~reg_bank[reg];
    } else if (this_op.opcode == 0b111) {
        int reg = this_op.opr1;
        if (reg_bank[reg] == 0) {
            reg_bank[reg] = 1;
        } else {
            reg_bank[reg] = 0;
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
void check_symbol_exist(struct func * func_ls, unsigned char data_type, 
                        unsigned char * data, char * symbol_ls, int symbol_pt) {
    if (data_type == 0b10 || data_type == 0b11) {
        int exist = 0;
        for (int i = 0; i < symbol_pt; i++) {
            if (symbol_ls[i] == *data) {
                *data = i;
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
void check_validity(struct func * fpt) {
    struct func * func_ls = fpt;
    while (fpt) {
        char symbol_ls[32];
        int symbol_pt = 0;
        for (int i = 0; i < fpt -> len; i++) {
            struct operation * op = &(fpt -> op_ls[i]);
            if (op->opcode == 0b000) {
                if (op->type1 == 0b10) {
                    int exist = 0;
                    for (int i = 0; i < symbol_pt; i++) {
                        if (op->opr1 == symbol_ls[i]) {
                            op->opr1 = i;
                            exist = 1;
                        }
                    }
                    if (!exist) {
                        symbol_ls[symbol_pt] = op->opr1;
                        op->opr1 = symbol_pt;
                        symbol_pt++;
                    }
                }
                check_symbol_exist(func_ls, op->type1, &(op->opr1),symbol_ls, symbol_pt);
                check_symbol_exist(func_ls, op->type2, &(op->opr2),symbol_ls, symbol_pt);
            } else if (op->opcode == 0b011) {
                if (op->type1 == 0b10) {
                    int exist = 0;
                    for (int i = 0; i < symbol_pt; i++) {
                        if (op->opr1 == symbol_ls[i]) {
                            op->opr1 = i;
                            exist = 1;
                        }
                    }
                    if (!exist) {
                        symbol_ls[symbol_pt] = op->opr1;
                        op->opr1 = symbol_pt;
                        symbol_pt++;
                    }
                }
                check_symbol_exist(func_ls, op->type1, &(op->opr1),symbol_ls, symbol_pt);
                check_symbol_exist(func_ls, op->type2, &(op->opr2),symbol_ls, symbol_pt);
            } else if (op-> opcode == 0b101) {
                check_symbol_exist(func_ls, op->type1, &(op->opr1),symbol_ls, symbol_pt);
            }
        }
        fpt -> symbol_num = symbol_pt;
        fpt = fpt -> next;
    }
}

int main(int argc, char **argv) {
    if (argc == 1) {
        fprintf(stderr, "No argument\n");
        exit(1);
    }
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
    check_validity(func_ls);
    unsigned char ram[256] = {};
    unsigned char reg_bank[8] = {}; //reg_bank[5] stores the total size of stack frames
    check_entry(func_ls);
    push_stack(ram, reg_bank, 0, func_ls);
    while (1) {
        int stk_index = reg_bank[5] - 1;
        int pc = reg_bank[7];
        struct operation this_op 
        = get_func(func_ls, ram[get_stk_func_index(stk_index)]) -> op_ls[pc];
        ram[get_stk_pt_index(stk_index)] ++;
        reg_bank[7] ++;
        if (is_cal(this_op)) {          
            push_stack(ram, reg_bank, get_func_label(this_op), func_ls);
            reg_bank[7] = 0;
            continue;
        }

        if (is_ret(this_op)) {
            reg_bank[5] --;
            if (reg_bank[5] == 0) {
                break;
            }
            reg_bank[7] = ram[get_stk_pt_index(reg_bank[5] - 1)];
            continue;
        }
        execute(this_op, ram, reg_bank, func_ls);
    }

    struct func * fpt2 = func_ls;
    free_all(fpt2);
}
