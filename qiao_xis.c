//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  qiao_xis is an assembler that converts Xia assembly into Xia bytecode.  //
//  Copyright (C) 2025 Tanika Claire Mellifont-Young                        //
//                                                                          //
//  This program is free software: you can redistribute it and/or modify    //
//  it under the terms of the GNU General Public License as published by    //
//  the Free Software Foundation, either version 3 of the License, or       //
//  (at your option) any later version.                                     //
//                                                                          //
//  This program is distributed in the hope that it will be useful,         //
//  but WITHOUT ANY WARRANTY; without even the implied warranty of          //
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           //
//  GNU General Public License for more details.                            //
//                                                                          //
//  You should have received a copy of the GNU General Public License       //
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include "libflare/strnchr.c"
#include "libflare/strncmp.c"
#include "libflare/strncnt.c"
#include "libflare/strncspn.c"
#include "libflare/xia_bytecode.h"
#include "libflare/dec2char.c"
#include "libflare/dec2short.c"
#include "libflare/dec2int.c"
#include "libflare/dec2long.c"
const char delimiters[] = {
    ' ', // space
    '\n', // newline
    '#', // comment
    '!', // instruction
    ':', // label declaration
    '&', // label reference
};
const unsigned long opcode_length = 5;
unsigned char get_opcode(char * name) {
    for (unsigned char i = 0; i < sizeof(xia_bytecode_str) / sizeof(char *); i++) {
        if (strncmp(name, xia_bytecode_str[i], 5) == 0) {
            return i;
        }
    };
    return 0;
};
struct label_t {
    char * label;
    unsigned long length;
    unsigned long destination;
};
int main(int argc, char * * argv) {
    char * input_buffer;
    unsigned long file_size;
    switch (argc) {
        case 2: {
        } break;
        default: {
            fprintf(stderr, "usage: qiao_s <file.xis>\n");
            //return 1;
        } break;
    };
    FILE * file = fopen("test.xis"/*argv[1]*/, "rb");
    if (!file) {
        fprintf(stderr, "fopen(argv[1], \"rb\") failed\n");
        return 1;
    };
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    input_buffer = (char *) malloc(file_size + 1);
    if (!input_buffer) {
        fprintf(stderr, "malloc(file_size + 1) failed\n");
        fclose(file);
        return 1;
    };
    unsigned long bytes_read = fread(input_buffer, 1, file_size, file);
    if (bytes_read != file_size) {
        fprintf(stderr, "fread(input_buffer, 1, file_size, file) failed\n");
        free(input_buffer);
        fclose(file);
        return 1;
    };
    input_buffer[file_size] = '\0';
    fclose(file);
    unsigned long approximate_label_count = strncnt(input_buffer, ':', file_size);
    unsigned long approximate_label_reference_count = strncnt(input_buffer, '&', file_size);
    unsigned long approximate_instruction_count = file_size / 5;
    struct label_t * labels = malloc(approximate_label_count * sizeof(struct label_t));
    if (!labels) {
        free(input_buffer);
        fprintf(stderr, "malloc(approximate_label_count * sizeof(struct label_t))");
    };
    unsigned long label_pointer = 0;
    struct label_t * label_references = malloc(approximate_label_reference_count * sizeof(struct label_t));
    if (!label_references) {
        free(labels);
        free(input_buffer);
        fprintf(stderr, "malloc(approximate_label_reference_count * sizeof(struct label_t)) failed.\n");
        return 1;
    };
    unsigned long label_reference_pointer = 0;
    unsigned char * instructions = malloc(approximate_instruction_count);
    if (!instructions) {
        free(labels);
        free(label_references);
        free(input_buffer);
        fprintf(stderr, "malloc(approximate_instruction_count) failed.\n");
        return 1;
    };
    unsigned long instruction_pointer = 0;
    for (unsigned long i = 0; i < file_size;) {
        unsigned long token_length = strncspn(input_buffer + sizeof(delimiters[0]) + i, file_size - i, delimiters, sizeof(delimiters) / sizeof(char));
        switch (input_buffer[i]) {
            case ' ': case '\n': {
                i += sizeof(char);
            } break;
            case '#': {
                i += strnchr(input_buffer + i, '\n', file_size - i) + sizeof(char);
            } break;
            case '!': {
                instructions[instruction_pointer] = get_opcode(input_buffer + sizeof(char) + i);
                i += sizeof(char) + opcode_length;
                instruction_pointer += 1;
            } break;
            case ':': {
                labels[label_pointer].label = input_buffer + sizeof(char) + i;
                labels[label_pointer].length = token_length;
                labels[label_pointer].destination = instruction_pointer + 1;
                i += sizeof(char) + token_length;
                label_pointer += 1;
            } break;
            case '&': {
                label_references[label_reference_pointer].label = input_buffer + sizeof(char) + i;
                label_references[label_reference_pointer].length = token_length;
                label_references[label_reference_pointer].destination = instruction_pointer;
                i += sizeof(char) + token_length;
                label_reference_pointer += 1;
                instruction_pointer += 8;
            } break;
            case 'c': {
                if (!dec2char(input_buffer + sizeof(char) + i, token_length, & instructions[instruction_pointer])) {
                    fprintf(stderr, "dec2char(input_buffer + i, token_length, & instructions[instruction_pointer]) failed at location:\n%.256s\n", input_buffer + i);
                    free(labels);
                    free(label_references);
                    free(instructions);
                    free(input_buffer);
                    return 1;
                };
                i += sizeof(char) + token_length;
                instruction_pointer += 1;
            } break;
            case 's': {
                if (!dec2short(input_buffer + sizeof(char) + i, token_length, (short *) & instructions[instruction_pointer])) {
                    fprintf(stderr, "dec2short(input_buffer + i, token_length, & instructions[instruction_pointer]) failed at location:\n%.256s\n", input_buffer + i);
                    free(labels);
                    free(label_references);
                    free(instructions);
                    free(input_buffer);
                    return 1;
                };
                i += sizeof(char) + token_length;
                instruction_pointer += 2;
            } break;
            case 'i': {
                if (!dec2int(input_buffer + sizeof(char) + i, token_length, (int *) & instructions[instruction_pointer])) {
                    fprintf(stderr, "dec2int(input_buffer + i, token_length, & instructions[instruction_pointer]) failed at location:\n%.256s\n", input_buffer + i);
                    free(labels);
                    free(label_references);
                    free(instructions);
                    free(input_buffer);
                    return 1;
                };
                i += sizeof(char) + token_length;
                instruction_pointer += 4;
            } break;
            case 'l': {
                if (!dec2long(input_buffer + sizeof(char) + i, token_length, (long *) & instructions[instruction_pointer])) {
                    fprintf(stderr, "dec2long(input_buffer + i, token_length, & instructions[instruction_pointer]) failed at location:\n%.256s\n", input_buffer + i);
                    free(labels);
                    free(label_references);
                    free(instructions);
                    free(input_buffer);
                    return 1;
                };
                i += sizeof(char) + token_length;
                instruction_pointer += 1;
            } break;
            default: {
                fprintf(stderr, "parsing failed, unknown token at location:\n%.256s\n", input_buffer + i);
                free(labels);
                free(label_references);
                free(instructions);
                free(input_buffer);
                return 1;
            };
        };
    };
    for (int i = 0; i < label_reference_pointer; i++) {
        int found = 0;
        for (int j = 0; j < label_pointer; j++) {
            if (strncmp(label_references[i].label, labels[j].label, label_references[i].length) == 0) {
                * (long *) (instructions + label_references[i].destination) = labels[j].destination;
                found = 1;
                break;
            };
        };
        if (!found) {
            fprintf(stderr, "parsing failed, unknown label:\n%.*s\n", label_references[i].length, label_references[i].label);
        };
    };
    fprintf(stdout, "%*.*s", instruction_pointer, instruction_pointer, instructions);
    free(labels);
    free(label_references);
    free(instructions);
    free(input_buffer);
    return 0;
};
