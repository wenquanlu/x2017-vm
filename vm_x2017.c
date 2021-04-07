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
    unsigned char len; ///consider the ordering of the struct
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

int get_index(int stk_index, int symbol_index) {
    return (stk_index) * 34 + 2 + symbol_index;
}

int get_stk_pt_index(int stk_index) {
    return (stk_index) * 34 + 1;
}

int get_stk_func_index(int stk_index) {
    return (stk_index) * 34;
}

struct func * get_entry(struct func * func_ls) {
    struct func * pt = func_ls;
    while (pt) {
        if (pt -> label == 0b000) {
            return pt;
        }
        pt = pt -> next;
    }
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
    exit(1);
}

void push_stack(char * ram, char * reg_bank, struct func * this_func) {
    if (get_index(reg_bank[5], 31) > 255) {
        exit(1);
    }
    ram[get_stk_func_index(reg_bank[5])] = this_func -> label;
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

char get_pointer(char * reg_bank, char symbol) {
    char stk_index = reg_bank[5] - 1;
    return (stk_index << 5) + symbol;
}

void execute(struct operation this_op, char * ram, char * reg_bank) {
    //to be implemented
    if (this_op.opcode == 0b000) {
        if (this_op.type1 == 0b01 && this_op.type2 == 0b00) {
            int reg = this_op.opr1;
            reg_bank[reg] = this_op.opr2;
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b01) {
            int symbol = this_op.opr1;
            int stk_index = reg_bank[5] - 1;
            int symbol_index = get_index(stk_index, symbol);
            int reg = this_op.opr2;
            ram[symbol_index] = reg_bank[reg];
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b11) {
            int curr_stk_index = reg_bank[5] - 1;
            char ptr = ram[get_index(curr_stk_index, this_op.opr2)];
            char ptr_stk_index = (ptr >> 5) & 0b00000111;
            char ptr_sym_index = (ptr & 0b00011111);
            int reg = this_op.opr1;
            reg_bank[reg] = ram[get_index(ptr_stk_index, ptr_sym_index)];
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b01) {
            int reg = this_op.opr2;
            int curr_stk_index = reg_bank[5] - 1;
            char ptr = ram[get_index(curr_stk_index, this_op.opr1)];
            char ptr_stk_index = (ptr >> 5) & 0b00000111;
            char ptr_sym_index = (ptr & 0b00011111);
            ram[get_index(ptr_stk_index, ptr_sym_index)] = reg_bank[reg];
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b00) {
            int curr_stk_index = reg_bank[5] - 1;
            ram[get_index(curr_stk_index, this_op.opr1)] = this_op.opr2;
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b00) {
            int curr_stk_index = reg_bank[5] - 1;
            char ptr = ram[get_index(curr_stk_index, this_op.opr1)];
            char ptr_stk_index = (ptr >> 5) & 0b00000111;
            char ptr_sym_index = (ptr & 0b00011111);
            ram[get_index(ptr_stk_index, ptr_sym_index)] = this_op.opr2;
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b10) {
            int symbol = this_op.opr2;
            int stk_index = reg_bank[5] - 1;
            int symbol_index = get_index(stk_index, symbol);
            int reg = this_op.opr1;
            reg_bank[reg] = ram[symbol_index];
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b11) {
            int curr_stk_index = reg_bank[5] - 1;
            int dest_ptr = ram[get_index(curr_stk_index, this_op.opr1)];
            int src_ptr = ram[get_index(curr_stk_index, this_op.opr2)];
            int dest_ptr_stk_index = (dest_ptr >> 5) & 0b00000111;
            int dest_ptr_sym_index = (dest_ptr & 0b00011111);
            int src_ptr_stk_index = (src_ptr >> 5) & 0b00000111;
            int src_ptr_sym_index = (src_ptr & 0b00011111);
            ram[get_index(dest_ptr_stk_index, dest_ptr_sym_index)] =
            ram[get_index(src_ptr_stk_index, src_ptr_sym_index)];
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b10) {
            int curr_stk_index = reg_bank[5] - 1;
            char ptr = ram[get_index(curr_stk_index, this_op.opr1)];
            char ptr_stk_index = (ptr >> 5) & 0b00000111;
            char ptr_sym_index = (ptr & 0b00011111);
            ram[get_index(ptr_stk_index, ptr_sym_index)] = 
            ram[get_index(curr_stk_index, this_op.opr2)];
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b11) {
            int curr_stk_index = reg_bank[5] - 1;
            char ptr = ram[get_index(curr_stk_index, this_op.opr2)];
            char ptr_stk_index = (ptr >> 5) & 0b00000111;
            char ptr_sym_index = (ptr & 0b00011111);
            ram[get_index(curr_stk_index, this_op.opr1)] = 
            ram[get_index(ptr_stk_index, ptr_sym_index)];
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b10) {
            int stk_index = reg_bank[5] - 1;
            ram[get_index(stk_index, this_op.opr1)] = ram[get_index(stk_index, this_op.opr2)];
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b00) {
            int reg = this_op.opr1;
            reg_bank[reg] = this_op.opr2;
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b01) {
            int reg_dest = this_op.opr1;
            int reg_src = this_op.opr2;
            reg_bank[reg_dest] = reg_bank[reg_src];
        }

    } else if (this_op.opcode == 0b011) {
        if (this_op.type1 == 0b01 && this_op.type2 == 0b10) {
            char address = get_pointer(reg_bank, this_op.opr2);
            int reg = this_op.opr1;
            reg_bank[reg] = address;
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b10) {
            char address = get_pointer(reg_bank, this_op.opr2);
            int stk_index = reg_bank[5] - 1;
            ram[get_index(stk_index, this_op.opr1)] = address;
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b10) {
            char address = get_pointer(reg_bank, this_op.opr2);
            int curr_stk_index = reg_bank[5] - 1;
            char ptr = ram[get_index(curr_stk_index, this_op.opr1)];
            char ptr_stk_index = (ptr >> 5) & 0b00000111;
            char ptr_sym_index = (ptr & 0b00011111);
            ram[get_index(ptr_stk_index, ptr_sym_index)] = address;
        } else {
            exit(1);
        }
    } else if (this_op.opcode == 0b100) {
        if (this_op.type1 == 0b01 && this_op.type2 == 0b01) {
            int reg1 = this_op.opr1;
            int reg2 = this_op.opr2;
            reg_bank[reg1] = reg_bank[reg1] + reg_bank[reg2];
        } else {
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
            int stk_index = reg_bank[5] - 1;
            unsigned int content = ram[get_index(stk_index, this_op.opr1)];
            printf("%u\n", content);
        } else if (this_op.type1 == 0b11) {
            int curr_stk_index = reg_bank[5] - 1;
            char ptr = ram[get_index(curr_stk_index, this_op.opr1)];
            char ptr_stk_index = (ptr >> 5) & 0b00000111;
            char ptr_sym_index = (ptr & 0b00011111);
            unsigned int content = ram[get_index(ptr_stk_index, ptr_sym_index)];
            printf("%u\n", content);
        } else {
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

int main(int argc, char **argv) {

    FILE *fp = fopen(argv[1], "rb");
    if (fp == NULL) {
        exit(1);
    }
    int size = 0;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, -1, SEEK_END);
    unsigned char byte_buf;
    fread(&byte_buf, sizeof(byte_buf), 1, fp);
    int stage = 1;
    int in_func = 0;
    int func_pt = 0;
    int bit_count = 0;  
    int func_length;
    char symbol_ls[52];
    enum opera_type {
        val, reg, stac, pt
    };
    enum opco_type {
        mov, cal, ret, ref, add, print, not, equ
    };
    enum opera_type curr_type;
    enum opco_type curr_opco_type;
    int symbol_pt = 0;
    struct func * func_ls = NULL;
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
                this_func -> next = func_ls;
                func_ls = this_func;
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
                bit_count += 5;
            }
            stage = 1;
            this_op -> opr2 = data;
            this_op_ls[func_length - func_pt] = *this_op;
            free(this_op);
        }
    }

    char ram[256] = {};
    char reg_bank[8] = {}; //reg_bank[5] stores the total size of stack frames
    struct func * entry = get_entry(func_ls);
    push_stack(ram, reg_bank, entry);
    while (1) {
        int stk_index = reg_bank[5] - 1;
        //struct func * fn_pt = get_func(func_ls, ram[get_stk_func_index(stk_index)]);
        int pc = reg_bank[7];
        struct operation this_op 
        = get_func(func_ls, ram[get_stk_func_index(stk_index)]) -> op_ls[pc];
        ram[get_stk_pt_index(stk_index)] ++;
        reg_bank[7] ++;
        if (is_cal(this_op)) {          
            push_stack(ram, reg_bank, get_func(func_ls, get_func_label(this_op)));
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
        execute(this_op, ram, reg_bank);
    }

    struct func * fpt2 = func_ls;
    while (fpt2) {
        struct func * next_pt = fpt2 -> next;
        free(fpt2 -> op_ls);
        free(fpt2);
        fpt2 = next_pt;
    }
}
