#include <stdlib.h>
#include <stdio.h>
#include "vm.h"
#define PROGRAM_COUNTER 7
#define NUM_STK_COUNTER 5

// free all the malloc memory: the list of functions
void free_all(struct func * fpt) {
    while (fpt) {
        struct func * next_pt = fpt -> next;
        free(fpt -> op_ls);
        free(fpt);
        fpt = next_pt;
    }
}

// get index of a symbol within a specific stack frame in ram[256]
int get_index(int stk_index, int symbol_index, unsigned char * ram) {
    int index = 2;
    int frame_index = 0;
    while (frame_index < stk_index) {
        unsigned char frame_len = ram[index];
        index += (frame_len + 3);
        frame_index++;
    }
    return index + symbol_index + 1;
}

// get index of a symbol in the current stack frame in ram[256]
int get_symbol_index(unsigned char * reg_bank, char symbol_index, 
                     unsigned char * ram) {
    int stk_index = reg_bank[NUM_STK_COUNTER] - 1;
    return get_index(stk_index, symbol_index, ram);
}

// get stack pointer index of a specific stack frame
// stack pointer will register the return address to this stack
// frame when calling function from this stack frame.
int get_stk_pt_index(int stk_index, unsigned char * ram) {
    int index = 1;
    int frame_index = 0;
    while (frame_index < stk_index) {
        unsigned char frame_len = ram[index + 1];
        index += (frame_len + 3);
        frame_index++;
    } 
    return index;
}

// get the index of function label of a specfic stack frame
int get_stk_func_index(int stk_index, unsigned char * ram) {
    int index = 0;
    int frame_index = 0;
    while (frame_index < stk_index) {
        unsigned char frame_len = ram[index + 2];
        index += (frame_len + 3);
        frame_index++;
    }
    return index;
}

// get index of an element storing the length of a specific 
// stack frame in ram[256].
int get_stk_len_index(int stk_index, unsigned char * ram) {
    int index = 2;
    int frame_index = 0;
    while (frame_index < stk_index) {
        unsigned char frame_len = ram[index];
        index += (frame_len + 3);
        frame_index++;
    }
    return index;
}

// check if the entry point of the program exists
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

// get pointer to a function with a specific function label 
// from program memory.
struct func * get_func(struct func * func_ls, char func_label) {
    struct func * pt = func_ls;
    while (pt) {
        if (pt -> label == func_label) {
            return pt;
        }
        pt = pt -> next;
    }
    fprintf(stderr, "Function being called not defined\n");
    free_all(func_ls);
    exit(1);
}

// push a stack frame to ram[256]
void push_stack(unsigned char * ram, unsigned char * reg_bank, 
                unsigned char func_label, struct func * func_ls) {
    if (get_index(reg_bank[NUM_STK_COUNTER],
        (get_func(func_ls, func_label)) -> symbol_num - 1, ram) > 255) {
        fprintf(stderr, "Stack over flow\n");
        free_all(func_ls);
        exit(1);
    }
    ram[get_stk_func_index(reg_bank[NUM_STK_COUNTER], ram)] = func_label;
    ram[get_stk_pt_index(reg_bank[NUM_STK_COUNTER], ram)] = 0;
    ram[get_stk_len_index(reg_bank[NUM_STK_COUNTER], ram)] = 
    get_func(func_ls, func_label) -> symbol_num;
    reg_bank[NUM_STK_COUNTER] ++;
}

// get a global address of a symbol in the current stack frame
// composed of 3 bits representing the stack frame number,
// 5 bits representing the symbol.
char get_pointer(unsigned char * reg_bank, char symbol) {
    char stk_index = reg_bank[NUM_STK_COUNTER] - 1;
    return (stk_index << 5) + symbol;
}

// receive a symbol in current stack frame
// interpret the global address stored in the symbol
// return the index of the element the symbol points to in ram[256]
int indirect(unsigned char * reg_bank, char ptr_symbol, unsigned char * ram) {
    char ptr = ram[get_symbol_index(reg_bank, ptr_symbol, ram)];
    char ptr_stk_index = (ptr >> 5) & 0b00000111;
    char ptr_sym_index = (ptr & 0b00011111);
    return get_index(ptr_stk_index, ptr_sym_index, ram);
}

// execute the current operation
void execute(struct operation this_op, unsigned char * ram, 
             unsigned char * reg_bank, struct func * func_ls) {
    if (this_op.opcode == 0b000) {
        if (this_op.type1 == 0b01 && this_op.type2 == 0b00) {
            int reg = this_op.opr1;
            reg_bank[reg] = this_op.opr2;
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b01) {
            int reg = this_op.opr2;
            ram[get_symbol_index(reg_bank, this_op.opr1, ram)] = reg_bank[reg];
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b11) {
            int reg = this_op.opr1;
            reg_bank[reg] = ram[indirect(reg_bank, this_op.opr2, ram)];
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b01) {
            int reg = this_op.opr2;
            ram[indirect(reg_bank, this_op.opr1, ram)] = reg_bank[reg];
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b00) {
            ram[get_symbol_index(reg_bank, this_op.opr1, ram)] = this_op.opr2;
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b00) {
            ram[indirect(reg_bank, this_op.opr1, ram)] = this_op.opr2;
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b10) {
            int reg = this_op.opr1;
            reg_bank[reg] = ram[get_symbol_index(reg_bank, this_op.opr2, ram)];
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b11) {
            ram[indirect(reg_bank, this_op.opr1, ram)] =
            ram[indirect(reg_bank, this_op.opr2, ram)];
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b10) {
            ram[indirect(reg_bank, this_op.opr1, ram)] = 
            ram[get_symbol_index(reg_bank, this_op.opr2, ram)];
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b11) {
            ram[get_symbol_index(reg_bank, this_op.opr1, ram)] = 
            ram[indirect(reg_bank, this_op.opr2, ram)];
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b10) {
            ram[get_symbol_index(reg_bank, this_op.opr1, ram)] = 
            ram[get_symbol_index(reg_bank, this_op.opr2, ram)];
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b00) {
            int reg = this_op.opr1;
            reg_bank[reg] = this_op.opr2;
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b01) {
            int reg_dest = this_op.opr1;
            int reg_src = this_op.opr2;
            reg_bank[reg_dest] = reg_bank[reg_src];
        } else {
            fprintf(stderr, "Invalid assembly format\n");
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
            ram[get_symbol_index(reg_bank, this_op.opr1, ram)] = address;
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b10) {
            char address = get_pointer(reg_bank, this_op.opr2);
            ram[indirect(reg_bank,this_op.opr1, ram)] = address;
        } else if (this_op.type1 == 0b01 && this_op.type2 == 0b11) {
            int reg = this_op.opr1;
            reg_bank[reg] = ram[get_symbol_index(reg_bank, this_op.opr2, ram)];
        } else if (this_op.type1 == 0b10 && this_op.type2 == 0b11) {
            ram[get_symbol_index(reg_bank, this_op.opr1, ram)] =
            ram[get_symbol_index(reg_bank, this_op.opr2, ram)];
        } else if (this_op.type1 == 0b11 && this_op.type2 == 0b11) {
            ram[indirect(reg_bank,this_op.opr1, ram)] = 
            ram[get_symbol_index(reg_bank, this_op.opr2, ram)];
        } else {
            fprintf(stderr, "Invalid assembly format\n");
            free_all(func_ls);
            exit(1);
        }
    } else if (this_op.opcode == 0b100) {
        if (this_op.type1 == 0b01 && this_op.type2 == 0b01) {
            int reg1 = this_op.opr1;
            int reg2 = this_op.opr2;
            reg_bank[reg1] = reg_bank[reg1] + reg_bank[reg2];
        } else {
            fprintf(stderr, "Invalid assembly format\n");
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
            unsigned int content = 
            ram[get_symbol_index(reg_bank, this_op.opr1, ram)];
            printf("%u\n", content);
        } else if (this_op.type1 == 0b11) {
            unsigned int content = ram[indirect(reg_bank, this_op.opr1, ram)];
            printf("%u\n", content);
        } else {
            fprintf(stderr, "Invalid assembly format\n");
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

// check the validity of the parsed binary
// map symbol A, B, C... to 0, 1, 2... and update them
void check_update_symbol(struct func * func_ls, unsigned char data_type, 
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
            fprintf(stderr, "Symbol refered before init\n");
            free_all(func_ls);
            exit(1);
        }
    } 
}

// check the validity of the parsed binary and update symbol
void check_update(struct func * fpt) {
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
                            exist = 1;
                        }
                    }
                    if (!exist) {
                        symbol_ls[symbol_pt] = op->opr1;
                        symbol_pt++;
                    }
                }
                check_update_symbol(func_ls, op->type1, 
                                   &(op->opr1),symbol_ls, symbol_pt);
                check_update_symbol(func_ls, op->type2, 
                                   &(op->opr2),symbol_ls, symbol_pt);
            } else if (op->opcode == 0b011) {
                if (op->type1 == 0b10) {
                    int exist = 0;
                    for (int i = 0; i < symbol_pt; i++) {
                        if (op->opr1 == symbol_ls[i]) {
                            exist = 1;
                        }
                    }
                    if (!exist) {
                        symbol_ls[symbol_pt] = op->opr1;
                        symbol_pt++;
                    }
                }
                check_update_symbol(func_ls, op->type1, 
                                   &(op->opr1),symbol_ls, symbol_pt);
                check_update_symbol(func_ls, op->type2, 
                                   &(op->opr2),symbol_ls, symbol_pt);
            } else if (op-> opcode == 0b101) {
                check_update_symbol(func_ls, op->type1, 
                                   &(op->opr1),symbol_ls, symbol_pt);
            }
            if (op -> type1 == 0b01) {
                if (op -> opr1 < 7 && op -> opr1 > 3) {
                    fprintf(stderr, "Access REG denialed\n");
                    free_all(func_ls);
                    exit(1);
                }
            }
            if (op -> type2 == 0b01) {
                if (op -> opr2 < 7 && op -> opr2 > 3) {
                    fprintf(stderr, "Access REG denialed\n");
                    free_all(func_ls);
                    exit(1);
                }
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
        fprintf(stderr, "Open file error\n");
        exit(1);
    }
    int size = 0;
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, -1, SEEK_END);
    struct func * func_ls = NULL;
    parse_binary(fp, &func_ls, size);
    fclose(fp);
    check_update(func_ls);
    unsigned char ram[256] = {};
    unsigned char reg_bank[8] = {};
    check_entry(func_ls);
    push_stack(ram, reg_bank, 0, func_ls);
    //reg_bank[5] stores the total size of stack frames
    reg_bank[NUM_STK_COUNTER] = 1;
    while (1) {
        int stk_index = reg_bank[NUM_STK_COUNTER] - 1;
        int pc = reg_bank[PROGRAM_COUNTER];
        struct operation this_op 
        = get_func(func_ls, ram[get_stk_func_index(stk_index, ram)]) -> op_ls[pc];
        reg_bank[PROGRAM_COUNTER] ++;

        if (this_op.opcode == 0b001) {   
            ram[get_stk_pt_index(stk_index, ram)] = reg_bank[PROGRAM_COUNTER];      
            push_stack(ram, reg_bank, this_op.opr1, func_ls);
            reg_bank[PROGRAM_COUNTER] = 0;
            continue;
        }
        if (this_op.opcode == 0b010) {
            reg_bank[NUM_STK_COUNTER] --;
            if (reg_bank[NUM_STK_COUNTER] == 0) {
                break;
            }
            reg_bank[PROGRAM_COUNTER] = 
            ram[get_stk_pt_index(reg_bank[NUM_STK_COUNTER] - 1, ram)];
            continue;
        }

        execute(this_op, ram, reg_bank, func_ls);
        if (reg_bank[PROGRAM_COUNTER] >= 
            get_func(func_ls, ram[get_stk_func_index(stk_index, ram)]) -> len) {
            fprintf(stderr, "Out of frame error\n");
            free_all(func_ls);
            exit(1);
        }
    }
    free_all(func_ls);
}
