#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "types.h"

const type INT_TYPE = (type) {.d0 = 0, .d1 = 1, .d2 = 0, .list_indicies = {0}, .current_index = 0};
const type VOID_TYPE = (type) {.d0 = 1, .d1 = 0, .d2 = 0, .list_indicies = {0}, .current_index = 0};
const type CHAR_TYPE = (type) {.d0 = 1, .d1 = 1, .d2 = 0, .list_indicies = {0}, .current_index = 0};
const type EMPTY_TYPE = (type) {.d0 = 0, .d1 = 0, .d2 = 0, .list_indicies = {0}, .current_index = 0};

unsigned char alpha(char c){
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

unsigned char alphanumeric(char c){
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

unsigned char digit(char c){
	return c >= '0' && c <= '9';
}

void add_type_entry(type *t, type_entry entry){
	t->d0 <<= 1;
	if(entry&1){
		t->d0 |= 1;
	}
	t->d1 <<= 1;
	if(entry&2){
		t->d1 |= 1;
	}
	t->d2 <<= 1;
	if(entry&4){
		t->d2 |= 1;
	}
}

void poke_type_entry(type *t, type_entry entry, unsigned int position){
	if(entry&1){
		t->d0 |= 1ULL<<position;
	}
	if(entry&2){
		t->d1 |= 1ULL<<position;
	}
	if(entry&4){
		t->d2 |= 1ULL<<position;
	}
}

void append_type(type *t, type d){
	uint64_t d_mask;
	int shift_amount = 1;

	d_mask = d.d0 | d.d1 | d.d2;

	if(!d_mask){
		return;
	}

	if(d_mask&0xFFFFFFFF00000000ULL){
		shift_amount += 32;
		d_mask >>= 32;
	}
	if(d_mask&0xFFFF0000U){
		shift_amount += 16;
		d_mask >>= 16;
	}
	if(d_mask&0xFF00U){
		shift_amount += 8;
		d_mask >>= 8;
	}
	if(d_mask&0xF0U){
		shift_amount += 4;
		d_mask >>= 4;
	}
	if(d_mask&0xCU){
		shift_amount += 2;
		d_mask >>= 2;
	}
	if(d_mask&0x2){
		shift_amount++;
	}

	t->d0 = (t->d0<<shift_amount) | d.d0;
	t->d1 = (t->d1<<shift_amount) | d.d1;
	t->d2 = (t->d2<<shift_amount) | d.d2;
	while(d.current_index){
		d.current_index--;
		t->list_indicies[t->current_index] = d.list_indicies[d.current_index];
		t->current_index++;
	}
}

unsigned char parse_identifier(char **c, char *identifier_name, unsigned int identifier_length){
	if(!alpha(**c) || !strncmp(*c, "void", 4) || !strncmp(*c, "int", 3) || !strncmp(*c, "char", 4)){
		return 0;
	}
	
	if(identifier_length){
		identifier_length--;
	}

	while(alphanumeric(**c)){
		if(identifier_length && identifier_name){
			*identifier_name = **c;
			identifier_name++;
			identifier_length--;
		}
		++*c;
	}

	if(identifier_name){
		*identifier_name = '\0';
	}
	return 1;
}

unsigned char parse_datatype(type *t, char **c){
	if(!strncmp(*c, "void", 4)){
		if(t){
			add_type_entry(t, type_void);
		}
		*c += 4;
		return 1;
	} else if(!strncmp(*c, "int", 3)){
		if(t){
			add_type_entry(t, type_int);
		}
		*c += 3;
		return 1;
	} else if(!strncmp(*c, "char", 4)){
		if(t){
			add_type_entry(t, type_char);
		}
		*c += 4;
		return 1;
	} else {
		return 0;
	}
}

void skip_whitespace(char **c){
	while(**c == ' ' || **c == '\t' || **c == '\n'){
		++*c;
	}
}

int peek_type(type t){
	int output = 0;

	if(t.d0&1){
		output |= 1;
	}
	if(t.d1&1){
		output |= 2;
	}
	if(t.d2&1){
		output |= 4;
	}

	return output;
}

int pop_type(type *t){
	int output = 0;

	if(t->d0&1){
		output |= 1;
	}
	if(t->d1&1){
		output |= 2;
	}
	if(t->d2&1){
		output |= 4;
	}

	t->d0 >>= 1;
	t->d1 >>= 1;
	t->d2 >>= 1;

	return output;
}

unsigned char types_equal(type t0, type t1){
	return t0.d0 == t1.d0 && t0.d1 == t1.d1 && t0.d2 == t1.d2;
}

type get_argument_type(type *t){
	type_entry next_type_entry;
	type output;
	unsigned int function_count = 0;
	unsigned int current_entry_index = 0;

	output = EMPTY_TYPE;
	while((next_type_entry = pop_type(t)) >= 4 || function_count){
		poke_type_entry(&output, next_type_entry, current_entry_index);
		current_entry_index++;
		if(next_type_entry == type_function){
			function_count++;
		} else if(function_count && next_type_entry == type_returns){
			function_count--;
		}
	}
	poke_type_entry(&output, next_type_entry, current_entry_index);

	return output;
}

void print_type(type t){
	unsigned int type_entry_int;

	while(t.d0 | t.d1 | t.d2){
		type_entry_int = 0;
		if(t.d0&1){
			type_entry_int |= 1;
		}
		if(t.d1&1){
			type_entry_int |= 2;
		}
		if(t.d2&1){
			type_entry_int |= 4;
		}
		switch(type_entry_int){
			case type_void:
				printf("void, ");
				break;
			case type_int:
				printf("int, ");
				break;
			case type_char:
				printf("char, ");
				break;
			case type_pointer:
				printf("pointer to ");
				break;
			case type_function:
				printf("function ( ");
				break;
			case type_returns:
				printf(") returning ");
				break;
			case type_list:
				t.current_index--;
				printf("list of length %d of ", t.list_indicies[t.current_index]);
				break;
			default:
				printf("unknown ");
		}
		t.d0 >>= 1;
		t.d1 >>= 1;
		t.d2 >>= 1;
	}
}

static void parse_type_recursive(type *t, char **c, char *identifier_name, char *argument_names, unsigned int identifier_length, unsigned int num_arguments);

void parse_type(type *t, char **c, char *identifier_name, char *argument_names, unsigned int identifier_length, unsigned int num_arguments){
	skip_whitespace(c);
	if(!parse_datatype(t, c)){
		printf("Expected 'void', 'int', or 'char'\n");
		exit(1);
	}

	skip_whitespace(c);

	parse_type_recursive(t, c, identifier_name, argument_names, identifier_length, num_arguments);
}

void parse_type_arguments(type *t, char **c, char *argument_names, unsigned int identifier_length, unsigned int num_arguments){
	type temp_type = (type) {.d0 = 0, .d1 = 0, .d2 = 0};

	if(**c == ')'){
		return;
	}
	if(argument_names && !num_arguments){
		fprintf(stderr, "Too many arguments!\n");
		exit(1);
	}
	parse_type(&temp_type, c, argument_names, NULL, identifier_length, 0);
	skip_whitespace(c);
	if(**c == ','){
		++*c;
		skip_whitespace(c);
		if(argument_names && num_arguments){
			num_arguments--;
			argument_names += identifier_length;
		}
		parse_type_arguments(t, c, argument_names, identifier_length, num_arguments);
	} else if(**c != ')'){
		printf("Expected ',' or ')'\n");
	}

	append_type(t, temp_type);
}

static void parse_type_recursive(type *t, char **c, char *identifier_name, char *argument_names, unsigned int identifier_length, unsigned int num_arguments){
	type inner_type = EMPTY_TYPE;
	type list_type = EMPTY_TYPE;

	skip_whitespace(c);
	while(**c == '*'){
		add_type_entry(t, type_pointer);
		++*c;
		skip_whitespace(c);
	}

	if(!parse_identifier(c, identifier_name, identifier_length) && **c == '('){
		++*c;
		skip_whitespace(c);
		parse_type_recursive(&inner_type, c, identifier_name, argument_names, identifier_length, num_arguments);
		if(**c != ')'){
			printf("Expected ')'\n");
			exit(1);
		}
		++*c;
	}

	skip_whitespace(c);

	if(**c == '('){
		add_type_entry(t, type_returns);
		++*c;
		skip_whitespace(c);
		parse_type_arguments(t, c, argument_names, identifier_length, num_arguments);
		++*c;
		add_type_entry(t, type_function);
		if(**c == '(' || **c == '['){
			fprintf(stderr, "Invalid type\n");
			exit(1);
		}
	} else if(**c == '['){
		while(**c == '['){
			add_type_entry(&list_type, type_list);
			++*c;
			list_type.list_indicies[list_type.current_index] = strtol(*c, c, 10);
			list_type.current_index++;
			skip_whitespace(c);
			if(**c != ']'){
				fprintf(stderr, "Expected ']'\n");
				exit(1);
			}
			++*c;
		}
	}


	append_type(t, list_type);
	append_type(t, inner_type);
}

int main(int argc, char **argv){
	type t;
	char *type_string = "int (*(*(*test)(void))[5])[6][7]\n";
	char identifier_name[16];
	char argument_names[16*16];

	t = EMPTY_TYPE;
	parse_type(&t, &type_string, identifier_name, argument_names, 16, 16);
	print_type(t);
	return 0;
}
