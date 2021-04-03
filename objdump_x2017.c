#include <stdlib.h>
#include <stdio.h>

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

void print_op (unsigned int line, int * internal_counter) {
    unsigned int opcode = line & 0b111;
    if (opcode == 0b000) {
        printf("    MOV ");
    } else if (opcode == 0b001) {
        printf("    CAL ");
    } else if (opcode == 0b010) {
        printf("    RET\n");
    } else if (opcode == 0b100) {
        printf("    ADD ");
    } else if (opcode == 0b101) {
        printf("    PRINT ");
    } else if (opcode == 0b110) {
        printf("    NOT ");
    } else if (opcode == 0b111) {
        printf("    EQU ");
    }
    *internal_counter += 3;
}

void print_first(unsigned int line, int * internal_counter, int symbol_pt, char * symbol_ls) {
    unsigned int data_type = (line >> 3) & 0b11;
    if (data_type == 0b00) {
        printf("VAL ");
        printf("%d", (line >> (5) & 0b11111111));
        *internal_counter += 10;
    } else if (data_type == 0b01) {
        printf("REG ");
        printf("%d", (line >> (5) & 0b111));
        *internal_counter += 5;
    } else if (data_type == 0b10) {
        printf("STK ");
        char data = line >> (5) & 0b11111;
        for (int i = 0; i < symbol_pt; i++) {
                if (data == symbol_ls[i]) {
                    printf("%c", get_symbol(i));
                }
        }
        *internal_counter += 7;
    } else if (data_type == 0b11) {
        printf("PT ");
        printf("%d", (line >> (5) & 0b11111));
        *internal_counter += 7;
    }
}

void print_second(unsigned int line, int * internal_counter, int symbol_pt, char * symbol_ls) {
    unsigned int data_type = (line >> (*internal_counter)) & 0b11;
    if (data_type == 0b00) {
        printf(" VAL ");
        //printf("%d", *internal_counter);
        printf("%d\n", ((line >> (*internal_counter + 2)) & 0b11111111));
        *internal_counter += 10;
    } else if (data_type == 0b01) {
        printf(" REG ");
        printf("%d\n", (line >> (*internal_counter + 2) & 0b111));
        *internal_counter += 5;
    } else if (data_type == 0b10) {
        printf(" STK ");
        char data = line >> (*internal_counter + 2) & 0b11111;
        for (int i = 0; i < symbol_pt; i++) {
                if (data == symbol_ls[i]) {
                    printf("%c\n", get_symbol(i));
                }
        }
        *internal_counter += 7;
    } else if (data_type == 0b11) {
        printf(" PT ");
        printf("%d\n", (line >> (*internal_counter + 2) & 0b11111));
        *internal_counter += 7;
    }
}

int main(int argc, char **argv) {
    //char mem[256];
    //char * data_mem = (char *) malloc(45*sizeof(char));
    //int capacity = 45;
    FILE *fp = fopen(argv[1], "rb");
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
    char line_ls[30];
    int line_counter = 0;
    char line_length = 0;
    while (1) {
        int displacement = bit_count / 8 + 1;
        int inbyte_dis;
        inbyte_dis = bit_count%8;
        if (stage == 1) {
            if (!in_func) {
                if (size*8 - bit_count < 8) {
                    line_ls[line_counter] = size*8 - bit_count;
                    line_counter++;
                    break;
                }
                func_length = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf);
                in_func = 1;
                bit_count += 5;
                stage = 1;
            }else if (func_length == func_pt) {
                //char func = get_bits(inbyte_dis, displacement, 3, fp, &byte_buf); //
                //printf("FUNC LABEL %d\n", func); //
                bit_count += 3;
                in_func = 0;
                func_length = 0;
                func_pt = 0;
                stage = 1;  
                /////////////////////
                line_ls[line_counter] = 3;
                line_counter ++;
                line_length = 0;
                ////////////////////

            } else  {
                char opcode = get_bits(inbyte_dis, displacement, 3, fp, &byte_buf);
                if (opcode == 0b000) {
                    //printf("    MOV "); //
                    curr_opco_type = mov;
                } else if (opcode == 0b001) {
                    //printf("    CAL "); //
                    curr_opco_type = cal;
                } else if (opcode == 0b010) {
                    //printf("    RET\n"); //
                    curr_opco_type = ret;
                } else if (opcode == 0b100) {
                    //printf("    ADD "); //
                    curr_opco_type = add;
                } else if (opcode == 0b101) {
                    //printf("    PRINT "); //
                    curr_opco_type = print;
                } else if (opcode == 0b110) {
                    //printf("    NOT "); //
                    curr_opco_type = not;
                } else if (opcode == 0b111) {
                    //printf("    EQU "); //
                    curr_opco_type = equ;
                }
                bit_count += 3;
                stage = 2;
                if (opcode == 0b010) {
                    stage = 1;
                    ///////////////
                    line_ls[line_counter] = 5;
                    line_counter ++;
                    ///////////////
                    ///////////////
                    line_ls[line_counter] = 3;
                    line_counter ++;
                    line_length = 0;
                    ///////////////
                }
                func_pt ++;
                //////////////
                line_length = 3;
                //////////////
            }
        } else if (stage == 2) {
            char data_type = get_bits(inbyte_dis, displacement, 2, fp, &byte_buf);
            if (data_type == 0b00) {
                curr_type = val;
                //printf("VAL ");
            } else if (data_type == 0b01) {
                curr_type = reg;
                //printf("REG ");
            } else if (data_type == 0b10) {
                curr_type = stac;
                //printf("STK ");
            } else if (data_type == 0b11) {
                curr_type = pt;
                //printf("PT ");
            }
            ////////////////
            line_length += 2;
            ////////////////
            bit_count += 2;
            stage = 3;
        } else if (stage == 3) {
            if (curr_type == val) {
                //char data = get_bits(inbyte_dis, displacement, 8, fp, &byte_buf); //
                //printf("%d", data); //
                bit_count += 8;
                ////////////////
                line_length += 8;
                ////////////////
            } else if (curr_type == reg) {
                //char data = get_bits(inbyte_dis, displacement, 3, fp, &byte_buf); //
                //printf("%d", data); //
                bit_count += 3;
                ////////////////
                line_length += 3;
                ////////////////
            } else if (curr_type == stac) {
                char data = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf);
                int exist = 0;
                for (int i = 0; i < symbol_pt; i++) {
                    if (data == symbol_ls[i]) {
                        //printf("%c", get_symbol(i)); //
                        exist = 1;
                    }
                }
                if (!exist) {
                    symbol_ls[symbol_pt] = data;
                    //printf("%c", get_symbol(symbol_pt)); //
                    symbol_pt ++;
                }
                bit_count += 5;
                ////////////////
                line_length += 5;
                ////////////////
              
            } else if (curr_type == pt) {
                //char data = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf); //
                //printf("%d", data); //
                bit_count += 5;
                ////////////////
                line_length += 5;
                ////////////////
            } 
            stage = 4;
            if (curr_opco_type == cal || curr_opco_type == print || 
            curr_opco_type == not || curr_opco_type == equ) {
                stage = 1;
                //printf("\n"); //
                /////////////////
                line_ls[line_counter] = line_length;
                line_counter ++;
                line_length = 0;
                /////////////////
            }
        
            //don't forget to set instruction length in function here for unary operation
        } else if (stage == 4) {
            char data_type = get_bits(inbyte_dis, displacement, 2, fp, &byte_buf);
            if (data_type == 0b00) {
                curr_type = val;
                //printf(" VAL "); //
            } else if (data_type == 0b01) {
                curr_type = reg;
                //printf(" REG "); //
            } else if (data_type == 0b10) {
                curr_type = stac;
                //printf(" STK "); //
            } else if (data_type == 0b11) {
                curr_type = pt;
                //printf(" PT "); //
            }
            bit_count += 2;
            stage = 5;
            ////////////////
            line_length += 2;
            ////////////////
        } else if (stage == 5) {
            if (curr_type == val) {
                //char data = get_bits(inbyte_dis, displacement, 8, fp, &byte_buf); //
                //printf("%d\n", data); //
                bit_count += 8;
                ////////////////
                line_length += 8;
                ////////////////
            } else if (curr_type == reg) {
                //char data = get_bits(inbyte_dis, displacement, 3, fp, &byte_buf); //
                //printf("%d\n", data); //
                bit_count += 3;
                ////////////////
                line_length += 3;
                ////////////////
            } else if (curr_type == stac) {
                char data = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf);
                int exist = 0;
                for (int i = 0; i < symbol_pt; i++) {
                    if (data == symbol_ls[i]) {
                        //printf("%c\n", get_symbol(i)); //
                        exist = 1;
                    }
                }
                if (!exist) {
                    symbol_ls[symbol_pt] = data;
                    //printf("%c\n", get_symbol(symbol_pt)); //
                    symbol_pt ++;
                }
                bit_count += 5;
                ////////////////
                line_length += 5;
                ////////////////
            } else if (curr_type == pt) {
                //char data = get_bits(inbyte_dis, displacement, 5, fp, &byte_buf); //
                //printf("%d\n", data); //
                bit_count += 5;
                ////////////////
                line_length += 5;
                ////////////////
            }
            stage = 1;
            /////////////////
            line_ls[line_counter] = line_length;
            line_counter ++;
            line_length = 0;
            /////////////////
            //don't forget to set instruction length in function here 
        }
    }
    //printf("########################\n");
    fseek(fp, 0, SEEK_SET);
    unsigned char chunk;
    int po_dis = line_ls[line_counter - 1];
    for (int i = line_counter-2; i >= 0; i--) {
        int bit_diff = po_dis%8;
        int byte_diff = po_dis/8;
        po_dis += line_ls[i];
        int line_len = line_ls[i];
        fseek(fp, byte_diff, SEEK_SET);
        fread(&chunk, sizeof(chunk), 1, fp);
        if (line_len == 3) {
            if (line_ls[i - 1] == 5) {
                printf("    RET\n");
            } else {
                //printf("%x\n", (chunk >> bit_diff) & 0b111);
                if (bit_diff + 3 <= 8) {
                    //printf("chunk: %x, bit_diff: %x\n", chunk, bit_diff);
                    //printf("last_bug: %x", (chunk >> 2));
                    printf("FUNC LABEL %d\n", (chunk >> (8-bit_diff - 3)) & 0b111);
                } else {
                    fseek(fp, byte_diff, SEEK_SET);
                    fread(&chunk, sizeof(chunk), 1, fp);
                    unsigned char first = chunk;
                    fseek(fp, byte_diff + 1, SEEK_SET);
                    fread(&chunk, sizeof(chunk), 1, fp);
                    unsigned char second = chunk;
                    unsigned short tuple = (first << 8) | (0x00ff & second);
                    //printf("tuple: %x\n", tuple);
                    printf("FUNC LABEL %d\n", ((tuple >> (16 - bit_diff - line_len))) & 0b111);
                }
            }
        } else if (line_len != 5) {
            if (bit_diff + line_len <= 8) {
                int internal_counter = 0;
                fseek(fp, byte_diff, SEEK_SET);
                fread(&chunk, sizeof(chunk), 1, fp);
                unsigned char line = (chunk >> (8 - bit_diff - line_len)) & ((1 << line_len) - 1);
                //printf("line: %x\n", line);
                print_op(line, &internal_counter);
                print_first(line, &internal_counter,symbol_pt, symbol_ls);
                if (internal_counter < line_len) {
                    print_second(line, &internal_counter,symbol_pt, symbol_ls);
                } else {
                    printf("\n");
                }
            } else if (bit_diff + line_len > 8 && bit_diff + line_len <= 16) {
                fseek(fp, byte_diff, SEEK_SET);
                fread(&chunk, sizeof(chunk), 1, fp);
                unsigned char first = chunk;
                fseek(fp, byte_diff + 1, SEEK_SET);
                fread(&chunk, sizeof(chunk), 1, fp);
                unsigned char second = chunk;
                unsigned short tuple = (first << 8) | (0x00ff & second);
                //printf("tuple: %x",tuple);
                unsigned short line = ((tuple >> (16 - bit_diff - line_len))) & ((1 << line_len) - 1);
                int internal_counter = 0;
                //printf("line: %x\n", line);
                print_op(line, &internal_counter);
                print_first(line, &internal_counter,symbol_pt, symbol_ls);
                if (internal_counter < line_len) {
                    print_second(line, &internal_counter,symbol_pt, symbol_ls);
                } else {
                    printf("\n");
                }


            } else if (bit_diff + line_len > 16 && bit_diff + line_len <= 24) {
                //printf("bit_diff: %d, line_len: %d\n",bit_diff, line_len);
                fseek(fp, byte_diff, SEEK_SET);
                fread(&chunk, sizeof(chunk), 1, fp);
                unsigned char first = chunk;
                //printf("first: %x\n", first);
                fseek(fp, byte_diff + 1, SEEK_SET);
                fread(&chunk, sizeof(chunk), 1, fp);
                unsigned char second = chunk;
                //printf("second: %x\n", second);
                fseek(fp, byte_diff + 2, SEEK_SET);
                fread(&chunk, sizeof(chunk), 1, fp);
                unsigned char third = chunk;
                //printf("third: %x\n", third);
                unsigned int triple = (first << 16) | (second << 8) | (0x00ff & third);
                //printf("triple: %x\n", triple);
                //printf("triple shift: %x\n", triple >> (24 - bit_diff - line_len));
                unsigned int line = (triple >> (24 - bit_diff - line_len)) & ((1 << line_len) - 1);
                //printf("line: %x\n", line);
                int internal_counter = 0;
                print_op(line, &internal_counter);
                print_first(line, &internal_counter,symbol_pt, symbol_ls);
                if (internal_counter < line_len) {
                    print_second(line, &internal_counter,symbol_pt, symbol_ls);
                } else {
                    printf("\n");
                }


            } else if (bit_diff + line_len > 24 && bit_diff + line_len <= 36) {
                fseek(fp, byte_diff, SEEK_SET);
                fread(&chunk, sizeof(chunk), 1, fp);
                unsigned char first = chunk;
                //printf("first chunk: %x\n", first);
                fseek(fp, byte_diff + 1, SEEK_SET);
                fread(&chunk, sizeof(chunk), 1, fp);
                unsigned char second = chunk;
                //printf("second chunk: %x\n", second);
                fseek(fp, byte_diff + 2, SEEK_SET);
                fread(&chunk, sizeof(chunk), 1, fp);
                unsigned char third = chunk;
                //printf("third chunk: %x\n", third);
                fseek(fp, byte_diff + 3, SEEK_SET);
                fread(&chunk, sizeof(chunk), 1, fp);
                unsigned char fourth = chunk;
                //printf("fourth chunk: %x\n", fourth);
                unsigned int quad = (first << 24) | (second << 16) | (third << 8) | (0x00ff & fourth);
                //printf("quad: %x\n", quad);
                //printf("byte_diff: %d", byte_diff);
                //printf("bit_diff: %x", bit_diff);
                unsigned int line = (quad >> (32 - bit_diff - line_len)) & ((1 << line_len) - 1);
                int internal_counter = 0;
                //printf("line: %x\n", line);
                print_op(line, &internal_counter);
                print_first(line, &internal_counter,symbol_pt, symbol_ls);
                if (internal_counter < line_len) {
                    print_second(line, &internal_counter,symbol_pt, symbol_ls);
                } else {
                    printf("\n");
                }
            }
        }

    }

}