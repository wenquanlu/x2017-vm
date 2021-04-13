#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function that writes bits to end of a file
// fw: pointer to file to be written to
// x: content for writing
// bit_num: number of bits to be written
// bit_ptr: tracking how many bits have been written
// atm: the last byte from the last write operation (can be incomplete)
void universal_write_func(FILE * fw, unsigned char x, int bit_num, int * bit_ptr, char * atm) {
    int multiple = *bit_ptr / 8;
    int remainder = *bit_ptr % 8;
    if (remainder == 0) {
        char content = x << (8 - bit_num);
        fwrite(&content, 1, 1, fw);
        *atm = content;
    } else {
        if (bit_num + remainder > 8) {
            char content = x >> ((bit_num + remainder) - 8);
            char result1 = *atm + content;
            fseek(fw, multiple, SEEK_SET);
            fwrite(&result1, 1, 1, fw);
            char result2 = x << (8 - ((bit_num + remainder) - 8));
            fwrite(&result2, 1, 1, fw);
            *atm = result2;
        } else {
            fseek(fw, multiple, SEEK_SET);
            char content  = x << (8 - bit_num - remainder);
            char result = *atm + content;
            fwrite(&result, 1, 1, fw);
            *atm = result;
        }
    }
    *bit_ptr += bit_num;
}

// Convert values to binary
unsigned char to_binary(char * num) {
    unsigned int inte = atoi(num);
    if (inte == 0 && *num != '0') {
        if (*num >= 65 && *num <= 90) {
            return *num - 65;
        } else {
            return *num - 71;
        }
    }
    return (unsigned char) inte;
}

// convert the string representation of operation type to binary
char bi_from_code(char * tok) {
    if (!strcmp(tok, "MOV")) {
        return 0b000;
    } else if (!strcmp(tok, "CAL")) {
        return 0b001;
    } else if (!strcmp(tok, "RET")) {
        return 0b010;
    } else if (!strcmp(tok, "REF")) {
        return 0b011;
    } else if (!strcmp(tok, "ADD")) {
        return 0b100;
    } else if (!strcmp(tok, "PRINT")) {
        return 0b101;
    } else if (!strcmp(tok, "NOT")) {
        return 0b110;
    } else if (!strcmp(tok, "EQU")) {
        return 0b111;
    } else {
        perror("unidentified instruction\n");
        exit(1);
    }
}

// get length (number of bits) of a certain type value
int get_type_len(char * tok) {
    if (!strcmp(tok, "VAL")) {
        return 8;
    } else if (!strcmp(tok, "REG")) {
        return 3;
    } else if (!strcmp(tok, "STK")) {
        return 5;
    } else if (!strcmp(tok, "PTR")) {
        return 5;
    } else {
        perror("unidentified type\n");
        exit(1);
    }
}

// convert the string representation of value type to binary
char bi_from_type(char * tok) {
    if (!strcmp(tok, "VAL")) {
        return 0b00;
    } else if (!strcmp(tok, "REG")) {
        return 0b01;
    } else if (!strcmp(tok, "STK")) {
        return 0b10;
    } else if (!strcmp(tok, "PTR")) {
        return 0b11;
    } else {
        perror("unidentified type\n");
        exit(1);
    }
}

int main(int argc, char ** argv) {
    FILE * f = fopen(argv[1], "r");
    if (f == NULL) {
        return 1;
    }
    char line[256];
    int bit_count = 0;
    char * tok1;
    char * tok2;
    char * tok3;
    char * func = "FUNC";
    char * cal = "CAL";
    char * ret = "RET";
    char * print = "PRINT";
    char * not = "NOT";
    char * equ = "EQU";
    char * val = "VAL";
    char * reg = "REG";
    char * stk = "STK";
    char * ptr = "PTR";
    while (fgets(line, 256, f)) {
        int len = strlen(line);
        line[len - 1] = '\0';
        tok1 = strtok(line, " ");
        if (!strcmp(tok1, func)) {
            tok2 = strtok(NULL, " ");
            bit_count+= 3;
        } else if (!strcmp(tok1, ret)) {
            bit_count += 8;
        } else {
            bit_count += 3;
            bit_count += 2;
            tok2 = strtok(NULL, " ");
            if (!strcmp(tok2, val)) {
                bit_count += 8;
            } else if (!strcmp(tok2, reg)) {
                bit_count += 3;
            } else if (!strcmp(tok2, stk)) {
                bit_count += 5;
            } else if (!strcmp(tok2, ptr)) {
                bit_count += 5;
            }
            if (!strcmp(tok1, cal) || !strcmp(tok1, ret) || !strcmp(tok1, print) || !strcmp(tok1, not)|| !strcmp(tok1, equ)) {
                continue;
            }
            strtok(NULL, " ");
            tok3 = strtok(NULL, " ");
            bit_count += 2;
            if (!strcmp(tok3, val)) {
                bit_count += 8;
            } else if (!strcmp(tok3, reg)) {
                bit_count += 3;
            } else if (!strcmp(tok3, stk)) {
                bit_count += 5;
            } else if (!strcmp(tok3, ptr)) {
                bit_count += 5;
            }
        }
    }
    int multiple = bit_count / 8;
    int remain = bit_count % 8;
    if (remain != 0) {
        multiple ++;
    }
    int total_bits = multiple * 8;
    int padding = total_bits - bit_count;
    fseek(f, 0, SEEK_SET);
    FILE * fw = fopen(argv[2], "wb");
    int bit_ptr = 0;
    char pad = 0b0;
    if (padding != 0) {
        fwrite(&pad, 1, 1, fw);
    }
    char atm = 0;
    bit_ptr = padding;
    char * kot1;
    char * kot2;
    char * kot3;
    char * kot4;
    char * kot5;
    int func_count = 0;
    while (fgets(line, 256, f)) {
        int len = strlen(line);
        line[len - 1] = '\0';
        kot1 = strtok(line, " ");
        if (!strcmp(kot1, func)) {
            kot2 = strtok(NULL, " ");
            kot3 = strtok(NULL, " ");
            unsigned char content = atoi(kot3);
            universal_write_func(fw,content, 3, &bit_ptr, &atm);
            func_count = 0;
        } else if (!strcmp(kot1, ret)) {
            char content = 0b010;
            universal_write_func(fw,content, 3, &bit_ptr, &atm);
            func_count ++;
            char count = func_count;
            universal_write_func(fw, count, 5, &bit_ptr, &atm);
        } else {
            kot2 = strtok(NULL, " ");
            kot3 = strtok(NULL, " ");
            if (!strcmp(kot1, cal) || !strcmp(kot1, ret) || !strcmp(kot1, print) || !strcmp(kot1, not)|| !strcmp(kot1, equ)) {
                unsigned char content = to_binary(kot3);
                universal_write_func(fw, content, get_type_len(kot2), &bit_ptr, &atm);
                universal_write_func(fw, bi_from_type(kot2), 2, &bit_ptr, &atm);
                universal_write_func(fw, bi_from_code(kot1), 3, &bit_ptr, &atm);
                func_count++;
                continue;
            }
            kot4 = strtok(NULL, " ");
            kot5 = strtok(NULL, " ");
            unsigned char content2 = to_binary(kot3);
            unsigned char content1 = to_binary(kot5);
            universal_write_func(fw, content1, get_type_len(kot4), &bit_ptr, &atm);
            universal_write_func(fw, bi_from_type(kot4), 2, &bit_ptr,&atm);
            universal_write_func(fw, content2, get_type_len(kot2), &bit_ptr, &atm);
            universal_write_func(fw, bi_from_type(kot2), 2, &bit_ptr, &atm);
            universal_write_func(fw, bi_from_code(kot1), 3, &bit_ptr, &atm);
            func_count++;
        }
    }
    fflush(fw);
    
}