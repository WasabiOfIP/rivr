#include "vm.h"
#include <stdio.h>

#define REG_VAR		0x00
#define REG_GLOB	0x01
#define REG_RARG	0x02
#define REG_RRET	0x03
#define REG_WARG	0x04
#define REG_WRET	0x05
#define REG_SPEC	0x06

int write_opcode(byte* prog, int pc, Operation op);
int write_register(byte* prog, int pc, int reg_id, int r_type);
int write_constant(byte* prog, int pc, Data data);
int write_constant_inline(byte* prog, int pc, long int n);
int write_byte(byte* prog, int pc, byte b);

void record_pc(byte* prog, int pc, int* refs, int nrefs);

int write_noops_halt(byte* prog);
int write_addition(byte* prog);
int write_input(byte* prog);
int write_memory(byte* prog);
int write_pow(byte* prog);
int write_branch(byte* prog);
int write_threading(byte* prog);
int write_functions(byte* prog);
int write_fibonacci(byte* prog);
int write_pushpop(byte* prog);

int (*progf)(byte*) = write_fibonacci;


int main(int argc, char** argv){
	int proglen = progf(NULL);
	byte* prog = calloc(proglen, sizeof(byte));
	int actual_proglen = progf(prog);
	
	printf("proglen initially counted as %d, was %d when writing\n", proglen, actual_proglen);
	
	FILE* fp;
	
	if (argc > 1){
		fp = fopen(argv[1], "w");
		if (!fp){
			printf("Error: unable to open %s\n", argv[1]);
			return 1;
		}
	}else{
		fp = fopen("rivr_prog.b", "w");
		if (!fp){
			printf("Error: unable to open rivr_prog.b\n");
		}
	}
	
	fwrite(prog, sizeof(byte), (size_t) proglen, fp);
	
	fclose(fp);
	
	return 0;
}

int write_noops_halt(byte* prog){
	Operation no_op = encode_operation(I_NOOP, SO_NUMBER);
	Operation halt = encode_operation(I_HALT, SO_NONE);
	
	int i;
	int pc = 0;
	int pc_nxt = 0;
	for (i = 0; i < 10; i++){
		pc = pc_nxt;
		pc_nxt = write_opcode(prog, pc, no_op);
	}
	
	pc = write_opcode(prog, pc, halt);
	
	return pc;
}

int write_addition(byte* prog){
	
	Operation add_op = encode_operation(I_ADD, FORMAT1_SUBOP(SO_NUMBER, SO_REGISTER, SO_REGISTER, SO_NONE));
	Operation print_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_REGISTER, SO_NUMBER));
	Operation newline_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_CONSTANT, SO_STRING));
	Operation halt_op = encode_operation(I_HALT, SO_NONE);
	Operation increment_op = encode_operation(I_INCR, SO_NONE);
	
	Data newline;
	newline.bytes[0] = '\n';
	newline.bytes[1] = '\0';
	
	int pc = 0;
	// ADD $!1 $!1 > $0
	pc = write_opcode(prog, pc, add_op); // 2
	pc = write_register(prog, pc, 1, REG_SPEC); // 1
	pc = write_register(prog, pc, 1, REG_SPEC); // 1
	pc = write_register(prog, pc, 0, REG_VAR); // 1
	// PRINT $0
	pc = write_opcode(prog, pc, print_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_register(prog, pc, 0, REG_VAR); // 1
	// PRINT newline
	pc = write_opcode(prog, pc, newline_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_constant(prog, pc, newline); // _data
	// INCR $0 > $0
	pc = write_opcode(prog, pc, increment_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 0, REG_VAR);
	// PRINT $0
	pc = write_opcode(prog, pc, print_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_register(prog, pc, 0, REG_VAR); // 1
	// PRINT newline
	pc = write_opcode(prog, pc, newline_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_constant(prog, pc, newline); // _data
	// HALT
	pc = write_opcode(prog, pc, halt_op); // 2
	
	return pc;
}

int write_input(byte* prog){
	int proglen = 52 + (sizeof(Data)*6);
	int pc = 0;
	
	Operation num_input_op = encode_operation(I_INPUT, SO_NUMBER);
	Operation str_input_op = encode_operation(I_INPUT, SO_STRING);
	Operation num_print_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_REGISTER, SO_NUMBER));
	Operation str_print_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_REGISTER, SO_STRING));
	Operation nwl_print_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_CONSTANT, SO_STRING));
	Operation copy_op = encode_operation(I_MOVE, SO_REGISTER);
	Operation halt_op = encode_operation(I_HALT, SO_NONE);
	
	Data newline;
	newline.bytes[0] = '\n';
	newline.bytes[1] = '\0';
	
	// MOVE $!0 > $0
	pc = write_opcode(prog, pc, copy_op); // 2
	pc = write_register(prog, pc, 0, REG_SPEC); // 1
	pc = write_register(prog, pc, 0, REG_VAR); // 1
	// MOVE $!0 > $1
	pc = write_opcode(prog, pc, copy_op); // 2
	pc = write_register(prog, pc, 0, REG_SPEC); // 1
	pc = write_register(prog, pc, 1, REG_VAR); // 1
	// INPUT $!3 > $0
	pc = write_opcode(prog, pc, num_input_op); // 2
	pc = write_register(prog, pc,  SREG_STDIN, REG_SPEC); // 1
	pc = write_register(prog, pc, 0, REG_VAR); // 1
	// OUTPUT '/n'
	pc = write_opcode(prog, pc, nwl_print_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_constant(prog, pc, newline); // _data
	// OUTPUT $!2 $0
	pc = write_opcode(prog, pc, num_print_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_register(prog, pc, 0, REG_VAR); // 1
	// OUTPUT '/n'
	pc = write_opcode(prog, pc, nwl_print_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_constant(prog, pc, newline); // _data
	
	// INPUT $!3 > $1
	pc = write_opcode(prog, pc, str_input_op); // 2
	pc = write_register(prog, pc,  SREG_STDIN, REG_SPEC); // 1
	pc = write_register(prog, pc, 1, REG_VAR); // 1
	// OUTPUT '/n'
	pc = write_opcode(prog, pc, nwl_print_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_constant(prog, pc, newline); // _data
	// OUTPUT $!2 $1
	pc = write_opcode(prog, pc, str_print_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_register(prog, pc, 1, REG_VAR); // 1
	// OUTPUT '/n'
	pc = write_opcode(prog, pc, nwl_print_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_constant(prog, pc, newline); // _data
	
	// INPUT $!3 > $1
	pc = write_opcode(prog, pc, str_input_op); // 2
	pc = write_register(prog, pc,  SREG_STDIN, REG_SPEC); // 1
	pc = write_register(prog, pc, 1, REG_VAR); // 1
	// OUTPUT '/n'
	pc = write_opcode(prog, pc, nwl_print_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_constant(prog, pc, newline); // _data
	// OUTPUT $!2 $1
	pc = write_opcode(prog, pc, str_print_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_register(prog, pc, 1, REG_VAR); // 1
	// OUTPUT '/n'
	pc = write_opcode(prog, pc, nwl_print_op); // 2
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC); // 1
	pc = write_constant(prog, pc, newline); // _data
	
	// HALT
	pc = write_opcode(prog, pc, halt_op); // 2
	
	printf("pc=<%d>, proglen=<%d>\n", pc, proglen);
	
	return pc;
}

int write_memory(byte* prog){
	int pc = 0;
	Operation alloc_op = encode_operation(I_M_ALLOC, SO_CONSTANT);
	Operation store_op = encode_operation(I_M_STORE, FORMAT1_SUBOP(SO_NONE, SO_REGISTER, SO_CONSTANT, SO_NONE));
	Operation free_op = encode_operation(I_M_FREE, SO_NONE);
	Operation load_op = encode_operation(I_M_LOAD, SO_CONSTANT);
	Operation copy_op = encode_operation(I_MOVE, SO_REGISTER);
	Operation increment_op = encode_operation(I_INCR, SO_NONE);
	Operation num_print_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_REGISTER, SO_NUMBER));
	Operation obj_print_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_REGISTER, SO_OBJECT));
	Operation nwl_print_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_CONSTANT, SO_STRING));
	Operation halt_op = encode_operation(I_HALT, SO_NONE);
	
	Data newline;
	newline.bytes[0] = '\n';
	newline.bytes[1] = '\0';
	
	Data zero;
	zero.n = 0;
	Data one;
	one.n = 1;
	Data two;
	two.n = 2;
	Data three;
	three.n = 3;
	Data four;
	four.n = 4;
	Data five;
	five.n = 5;
	Data six;
	six.n = 6;
	
	// MOVE $!0 > $0
	pc = write_opcode(prog, pc, copy_op);
	pc = write_register(prog, pc, 0, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// MOVE $!1 > $1
	pc = write_opcode(prog, pc, copy_op);
	pc = write_register(prog, pc, 1, REG_SPEC);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	// INCR $1 > $2
	pc = write_opcode(prog, pc, increment_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// INCR $2 > $3
	pc = write_opcode(prog, pc, increment_op);
	pc = write_register(prog, pc, 2, REG_VAR);
	pc = write_register(prog, pc, 3, REG_VAR);
	
	// INCR $3 > $4
	pc = write_opcode(prog, pc, increment_op);
	pc = write_register(prog, pc, 3, REG_VAR);
	pc = write_register(prog, pc, 4, REG_VAR);
	
	// INCR $4 > $5
	pc = write_opcode(prog, pc, increment_op);
	pc = write_register(prog, pc, 4, REG_VAR);
	pc = write_register(prog, pc, 5, REG_VAR);
	
	// M_ALLOC 6 > $6
	pc = write_opcode(prog, pc, alloc_op);
	pc = write_constant(prog, pc, six);
	pc = write_register(prog, pc, 6, REG_VAR);
	
	// OUTPUT $!2 $6
	pc = write_opcode(prog, pc, obj_print_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_register(prog, pc, 6, REG_VAR);
	
	// OUTPUT $!2 '\n'
	pc = write_opcode(prog, pc, nwl_print_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_constant(prog, pc, newline);
	
	// M_STORE $6 0 $3
	pc = write_opcode(prog, pc, store_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	pc = write_register(prog, pc, 3, REG_VAR);
	pc = write_constant(prog, pc, zero);
	
	// M_STORE $6 1 $4
	pc = write_opcode(prog, pc, store_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	pc = write_register(prog, pc, 4, REG_VAR);
	pc = write_constant(prog, pc, one);
	
	// M_STORE $6 2 $5
	pc = write_opcode(prog, pc, store_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	pc = write_register(prog, pc, 5, REG_VAR);
	pc = write_constant(prog, pc, two);
	
	// M_STORE $6 3 $0
	pc = write_opcode(prog, pc, store_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_constant(prog, pc, three);
	
	// M_STORE $6 4 $1
	pc = write_opcode(prog, pc, store_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_constant(prog, pc, four);
	
	// M_STORE $6 5 $2
	pc = write_opcode(prog, pc, store_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	pc = write_register(prog, pc, 2, REG_VAR);
	pc = write_constant(prog, pc, five);
	
	// M_LOAD $6 0 > $0
	pc = write_opcode(prog, pc, load_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	pc = write_constant(prog, pc, zero);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// M_LOAD $6 1 > $1
	pc = write_opcode(prog, pc, load_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	pc = write_constant(prog, pc, one);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	// M_LOAD $6 2 > $2
	pc = write_opcode(prog, pc, load_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	pc = write_constant(prog, pc, two);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// M_LOAD $6 3 > $3
	pc = write_opcode(prog, pc, load_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	pc = write_constant(prog, pc, three);
	pc = write_register(prog, pc, 3, REG_VAR);
	
	// M_LOAD $6 4 > $4
	pc = write_opcode(prog, pc, load_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	pc = write_constant(prog, pc, four);
	pc = write_register(prog, pc, 4, REG_VAR);
	
	// M_LOAD $6 5 > $5
	pc = write_opcode(prog, pc, load_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	pc = write_constant(prog, pc, five);
	pc = write_register(prog, pc, 5, REG_VAR);
	
	// M_FREE $6
	pc = write_opcode(prog, pc, free_op);
	pc = write_register(prog, pc, 6, REG_VAR);
	
	// OUTPUT $!2 $5
	pc = write_opcode(prog, pc, num_print_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_register(prog, pc, 5, REG_VAR);
	
	// OUTPUT $!2 '\n'
	pc = write_opcode(prog, pc, nwl_print_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_constant(prog, pc, newline);
	
	// HALT
	pc = write_opcode(prog, pc, halt_op);
	
	
	return pc;
}

int write_pow(byte* prog){
	int pc = 0;
	
	Operation npow_op = encode_operation(I_POW, FORMAT1_SUBOP(SO_NUMBER, SO_REGISTER, SO_REGISTER, SO_NONE));
	Operation rpow_op = encode_operation(I_POW, FORMAT1_SUBOP(SO_RATIONAL, SO_REGISTER, SO_REGISTER, SO_NONE));
	Operation load_op = encode_operation(I_MOVE, SO_CONSTANT);
	Operation increment_op = encode_operation(I_INCR, SO_NONE);
	Operation num_print_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_REGISTER, SO_NUMBER));
	Operation rat_print_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_REGISTER, SO_RATIONAL));
	Operation nwl_print_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_CONSTANT, SO_STRING));
	Operation halt_op = encode_operation(I_HALT, SO_NONE);
	
	Data newline;
	newline.bytes[0] = '\n';
	newline.bytes[1] = '\0';
	
	Data done;
	done.bytes[0] = 'D';
	done.bytes[1] = 'o';
	done.bytes[2] = 'n';
	done.bytes[3] = 'e';
	done.bytes[4] = '\0';
	
	Data onef;
	onef.d = 1.0D;
	
	Data twof;
	twof.d = 2.0D;
	
	Data threef;
	threef.d = -3.0D;
	
	
	// INCR $!1 > $0
	pc = write_opcode(prog, pc, increment_op);
	pc = write_register(prog, pc, 1, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// INCR $0 > $1
	pc = write_opcode(prog, pc, increment_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	// POW $0 $1 > $2
	pc = write_opcode(prog, pc, npow_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// OUTPUT $!2 $2
	pc = write_opcode(prog, pc, num_print_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// OUTPUT $!2 '\n'
	pc = write_opcode(prog, pc, nwl_print_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_constant(prog, pc, newline);
	
	// MOVE twof > $0
	pc = write_opcode(prog, pc, load_op);
	pc = write_constant(prog, pc, twof);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// MOVE onef > $1
	pc = write_opcode(prog, pc, load_op);
	pc = write_constant(prog, pc, onef);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	// POW $0 $1 > $2
	pc = write_opcode(prog, pc, rpow_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// OUTPUT $!2 $2
	pc = write_opcode(prog, pc, rat_print_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// OUTPUT $!2 '\n'
	pc = write_opcode(prog, pc, nwl_print_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_constant(prog, pc, newline);
	
	// MOVE threef > $1
	pc = write_opcode(prog, pc, load_op);
	pc = write_constant(prog, pc, threef);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	// POW $0 $1 > $2
	pc = write_opcode(prog, pc, rpow_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// OUTPUT $!2 $2
	pc = write_opcode(prog, pc, rat_print_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// OUTPUT $!2 '\n'
	pc = write_opcode(prog, pc, nwl_print_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_constant(prog, pc, newline);
	
	// OUTPUT $!2 "Done"
	pc = write_opcode(prog, pc, nwl_print_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_constant(prog, pc, done);
	
	// HALT 
	pc = write_opcode(prog, pc, halt_op);
	
	return pc;
}

int write_branch(byte* prog){
	int pc = 0;
	Operation copy_op = encode_operation(I_MOVE, SO_REGISTER);
	Operation num_input_op = encode_operation(I_INPUT, SO_NUMBER);
	Operation gt_op = encode_operation(I_GT, FORMAT1_SUBOP(SO_NUMBER, SO_REGISTER, SO_REGISTER, SO_NONE));
	Operation branch_op = encode_operation(I_BRANCH, SO_NONE);
	Operation cons_str_output_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_CONSTANT, SO_STRING));
	Operation incr_op = encode_operation(I_INCR, SO_NONE);
	Operation jump_op = encode_operation(I_JUMP, FORMAT3_SUBOP(SO_CONSTANT, SO_ABSOLUTE));
	Operation num_output_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_REGISTER, SO_NUMBER));
	Operation halt_op = encode_operation(I_HALT, SO_NONE);
	
	Data str_greater_than_one;
	str_greater_than_one.bytes[0] = '>';
	str_greater_than_one.bytes[1] = '1';
	str_greater_than_one.bytes[2] = '\n';
	str_greater_than_one.bytes[3] = '\0';
	
	Data str_finished;
	str_finished.bytes[0] = 'f';
	str_finished.bytes[1] = 'i';
	str_finished.bytes[2] = 'n';
	str_finished.bytes[3] = '\n';
	str_finished.bytes[4] = '\0';
	
	Data str_newline;
	str_newline.bytes[0] = '\n';
	str_newline.bytes[1] = '\0';
	
	int label_input_loop_loc;
	int label_input_loop_ref;
	int label_else_loc;
	int label_else_ref;
	
	// MOVE $!0 > $0
	pc = write_opcode(prog, pc, copy_op);
	pc = write_register(prog, pc, 0, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// :INPUT_LOOP:
	label_input_loop_loc = pc;
	
	// INPUT $!3 > $1
	pc = write_opcode(prog, pc, num_input_op);
	pc = write_register(prog, pc,  SREG_STDIN, REG_SPEC);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	// GT $1 $!1 > $2
	pc = write_opcode(prog, pc, gt_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 1, REG_SPEC);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// BRANCH $2 :ELSE:
	pc = write_opcode(prog, pc, branch_op);
	pc = write_register(prog, pc, 2, REG_VAR);
	label_else_ref = pc;
	pc += sizeof(Data);
	
	// OUTPUT $!2 ">1\n"
	pc = write_opcode(prog, pc, cons_str_output_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_constant(prog, pc, str_greater_than_one);
	
	// INCR $0 > $0
	pc = write_opcode(prog, pc, incr_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// JUMP :INPUT_LOOP:
	pc = write_opcode(prog, pc, jump_op);
	label_input_loop_ref = pc;
	pc += sizeof(Data);
	
	// :ELSE:
	label_else_loc = pc;
	
	// OUTPUT $!2 "fin\n"
	pc = write_opcode(prog, pc, cons_str_output_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_constant(prog, pc, str_finished);
	
	// OUTPUT $!2 $0
	pc = write_opcode(prog, pc, num_output_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// OUTPUT $!2 "\n"
	pc = write_opcode(prog, pc, cons_str_output_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_constant(prog, pc, str_newline);
	
	// HALT
	pc = write_opcode(prog, pc, halt_op);
	
	record_pc(prog, label_else_loc, &label_else_ref, 1);
	record_pc(prog, label_input_loop_loc, &label_input_loop_ref, 1);
	
	return pc;
}

int write_threading(byte* prog){
	int pc = 0;
	
	Operation cons_str_output_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_CONSTANT, SO_STRING));
	Operation incr_op = encode_operation(I_INCR, SO_NONE);
	Operation halt_op = encode_operation(I_HALT, SO_NONE);
	Operation fork_op = encode_operation(I_TH_NEW, SO_CONSTANT);
	Operation jump_op = encode_operation(I_JUMP, FORMAT3_SUBOP(SO_CONSTANT, SO_ABSOLUTE));
	Operation kill_op = encode_operation(I_TH_KILL, SO_NONE);
	
	Data thread1;
	thread1.bytes[0] = 't';
	thread1.bytes[1] = 'h';
	thread1.bytes[2] = 'r';
	thread1.bytes[3] = '1';
	thread1.bytes[4] = '\n';
	thread1.bytes[5] = '\0';
	
	Data forked;
	forked.bytes[0] = 'f';
	forked.bytes[1] = 'o';
	forked.bytes[2] = 'r';
	forked.bytes[3] = 'k';
	forked.bytes[4] = 'e';
	forked.bytes[5] = 'd';
	forked.bytes[6] = '\n';
	forked.bytes[7] = '\0';
	
	Data killed;
	killed.bytes[0] = 'k';
	killed.bytes[1] = 'i';
	killed.bytes[2] = 'l';
	killed.bytes[3] = 'l';
	killed.bytes[4] = 'e';
	killed.bytes[5] = 'd';
	killed.bytes[6] = '\n';
	killed.bytes[7] = '\0';
	
	int label_loc;
	int label_ref[2];
	
	// OUTPUT $!2 'thr1\n'
	pc = write_opcode(prog, pc, cons_str_output_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_constant(prog, pc, thread1);
	
	// INCR $!0 > $0
	pc = write_opcode(prog, pc, incr_op);
	pc = write_register(prog, pc, 0, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// TH_NEW :Thread2: > $1
	pc = write_opcode(prog, pc, fork_op);
	label_ref[0] = pc;
	pc += sizeof(Data);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	// OUTPUT $!2 'forked\n'
	pc = write_opcode(prog, pc, cons_str_output_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_constant(prog, pc, forked);
	
	// TH_KILL $1
	pc = write_opcode(prog, pc, kill_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	// OUTPUT $!2 'killed\n'
	pc = write_opcode(prog, pc, cons_str_output_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_constant(prog, pc, killed); 
	
	// HALT
	pc = write_opcode(prog, pc, halt_op);
	
	// :Thread2:
	label_loc = pc;
	
	// INCR $!1 > $0
	pc = write_opcode(prog, pc, incr_op);
	pc = write_register(prog, pc, 1, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// JUMP :Thread2:
	pc = write_opcode(prog, pc, jump_op);
	label_ref[1] = pc;
	pc += sizeof(Data);
	
	// HALT
	pc = write_opcode(prog, pc, halt_op);
	
	record_pc(prog, label_loc, label_ref, 2);
	
	
	return pc;
}

int write_functions(byte* prog){
	int pc = 0;
	
	Operation incr_op = encode_operation(I_INCR, SO_NONE);
	Operation halt_op = encode_operation(I_HALT, SO_NONE);
	Operation reg_jump_op = encode_operation(I_JUMP, FORMAT3_SUBOP(SO_REGISTER, SO_ABSOLUTE));
	Operation rel_jump_op = encode_operation(I_JUMP, FORMAT3_SUBOP(SO_CONSTANT, SO_RELATIVE));
	Operation load_op = encode_operation(I_MOVE, SO_CONSTANT);
	Operation pop_op = encode_operation(I_POPFRAME, SO_NONE);
	Operation num_output_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_REGISTER, SO_NUMBER));
	Operation mul_op = encode_operation(I_MUL, FORMAT1_SUBOP(SO_NUMBER, SO_REGISTER, SO_REGISTER, SO_NONE));
	Operation func_create_op = encode_operation(I_F_CREATE, FORMAT3_SUBOP(SO_CLOSURE, SO_RELATIVE));
	Operation func_output_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_REGISTER, SO_FUNCTION));
	Operation func_call_op = encode_operation(I_F_CALL, SO_PUSHFIRST);
	
	int ret_label_ref;
	int ret_label_loc;
	
	// INCR $!1 > $1
	pc = write_opcode(prog, pc, incr_op);
	pc = write_register(prog, pc, 1, REG_SPEC);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	// F_CREATE +23 1 $1 > $0
	pc = write_opcode(prog, pc, func_create_op);	// 2
	pc = write_constant_inline(prog, pc, 23);		// 8
	pc = write_byte(prog, pc, 1);					// 1
	pc = write_register(prog, pc, 1, REG_VAR);		// 1
	pc = write_register(prog, pc, 0, REG_VAR);		// 1
	
	// JUMP +18
	pc = write_opcode(prog, pc, rel_jump_op);	// 2
	pc = write_constant_inline(prog, pc, 18);	// 8
	
	// MUL $a(1) $1 > $r(0)
	pc = write_opcode(prog, pc, mul_op);		// 2
	pc = write_register(prog, pc, 1, REG_RARG);	// 1
	pc = write_register(prog, pc, 1, REG_VAR);	// 1
	pc = write_register(prog, pc, 0, REG_WRET);	// 1
	
	// Every function ever should end with this
	// JUMP $a(0)
	pc = write_opcode(prog, pc, reg_jump_op);	// 2
	pc = write_register(prog, pc, 0, REG_RARG);	// 1
	
	// OUTPUT $!2 $0
	pc = write_opcode(prog, pc, func_output_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// INCR $!1 > $a(1)
	pc = write_opcode(prog, pc, incr_op);
	pc = write_register(prog, pc, 1, REG_SPEC);
	pc = write_register(prog, pc, 1, REG_WARG);
	
	// MOVE :RET_LABEL: > $a(0)
	pc = write_opcode(prog, pc, load_op);
	ret_label_ref = pc; pc += sizeof(Data);
	pc = write_register(prog, pc, 0, REG_WARG);
	
	// F_CALL $0
	pc = write_opcode(prog, pc, func_call_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// :RET_LABEL:
	ret_label_loc = pc;
	
	// POPFRAME
	pc = write_opcode(prog, pc, pop_op);
	
	// OUTPUT $!2 $r(0)
	pc = write_opcode(prog, pc, num_output_op);
	pc = write_register(prog, pc,  SREG_STDOUT, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_RRET);
	
	// HALT
	pc = write_opcode(prog, pc, halt_op);
	
	
	
	record_pc(prog, ret_label_loc, &ret_label_ref, 1);
	
	return pc;
}

int write_fibonacci(byte* prog){
	int pc = 0;
	
	Operation num_input_op = encode_operation(I_INPUT, SO_NUMBER);
	Operation lt_op = encode_operation(I_LT, FORMAT1_SUBOP(SO_NUMBER, SO_REGISTER, SO_CONSTANT, SO_NONE));
	Operation gt_op = encode_operation(I_LT, FORMAT1_SUBOP(SO_NUMBER, SO_REGISTER, SO_REGISTER, SO_NONE));
	Operation eq_op = encode_operation(I_EQ, FORMAT1_SUBOP(SO_NUMBER, SO_REGISTER, SO_REGISTER, SO_NONE));
	Operation or_op = encode_operation(I_EQ, FORMAT1_SUBOP(SO_NUMBER, SO_REGISTER, SO_REGISTER, SO_NONE));
	Operation add_op = encode_operation(I_ADD, FORMAT1_SUBOP(SO_RATIONAL, SO_REGISTER, SO_REGISTER, SO_NONE));
	Operation branch_op = encode_operation(I_BRANCH, SO_NONE);
	Operation copy_op = encode_operation(I_MOVE, SO_REGISTER);
	Operation alloc_op = encode_operation(I_M_ALLOC, SO_REGISTER);
	Operation store_op = encode_operation(I_M_STORE, FORMAT1_SUBOP(SO_NONE, SO_REGISTER, SO_REGISTER, SO_NONE));
	Operation decr_op = encode_operation(I_DECR, SO_NONE);
	Operation const_move_op = encode_operation(I_MOVE, SO_CONSTANT);
	Operation load_op = encode_operation(I_M_LOAD, SO_REGISTER);
	Operation incr_op = encode_operation(I_INCR, SO_NONE);
	Operation jump_op = encode_operation(I_JUMP, FORMAT3_SUBOP(SO_CONSTANT, SO_ABSOLUTE));
	Operation rat_output_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_REGISTER, SO_RATIONAL));
	Operation halt_op = encode_operation(I_HALT, SO_NONE);
	
	Data two;
	two.n = 2;
	
	// N = stdin.read(int)
	// INPUT(NUMBER) $!3 > $0
	pc = write_opcode(prog, pc, num_input_op);
	pc = write_register(prog, pc, 3, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// a = N < 2
	// LT $0 2 > $1
	pc = write_opcode(prog, pc, lt_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_constant(prog, pc, two);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	// if a (if N > 1) goto FIB_ARRAY_ALLOC
	// BRANCH $1 FIB_ARRAY_ALLOC
	pc = write_opcode(prog, pc, branch_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	int label_fib_array_alloc_ref = pc;
	pc += sizeof(Data);
	
	// ret = 1.0
	// MOVE $!6 > $32
	pc = write_opcode(prog, pc, copy_op);
	pc = write_register(prog, pc, 6, REG_SPEC);
	pc = write_register(prog, pc, 32, REG_VAR);
	
	// JUMP :RETURN
	pc = write_opcode(prog, pc, jump_op);
	int label_ret_ref = pc;
	pc += sizeof(Data);
	
	// :FIB_ARRAY_ALLOC
	int label_fib_array_alloc_loc = pc;
	
	// INCR $0 > $1
	pc = write_opcode(prog, pc, incr_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	// array = [N]
	// M_ALLOC $1 > $1
	pc = write_opcode(prog, pc, alloc_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	// M_STORE $1 $!6 $!0
	// array[0] = 1.0
	pc = write_opcode(prog, pc, store_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 6, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_SPEC);
	
	// M_STORE $1 $!6 $!1
	// array[1] = 1.0
	pc = write_opcode(prog, pc, store_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 6, REG_SPEC);
	pc = write_register(prog, pc, 1, REG_SPEC);
	
	// i = 2
	// MOVE 2 > $2
	pc = write_opcode(prog, pc, const_move_op);
	pc = write_constant(prog, pc, two);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// do
	// :FIB_LOOP
	int label_fib_loop_loc = pc;
	
	// x = i-1
	// DECR $2 > $3
	pc = write_opcode(prog, pc, decr_op);
	pc = write_register(prog, pc, 2, REG_VAR);
	pc = write_register(prog, pc, 3, REG_VAR);
	
	// y = x-1
	// DECR $3 > $4
	pc = write_opcode(prog, pc, decr_op);
	pc = write_register(prog, pc, 3, REG_VAR);
	pc = write_register(prog, pc, 4, REG_VAR);
	
	// x = array[x]
	// M_LOAD $1 $3 > $3
	pc = write_opcode(prog, pc, load_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 3, REG_VAR);
	pc = write_register(prog, pc, 3, REG_VAR);
	
	// y = array[y]
	// M_LOAD $1 $4 > $4
	pc = write_opcode(prog, pc, load_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 4, REG_VAR);
	pc = write_register(prog, pc, 4, REG_VAR);
	
	// x = x + y
	// ADD $3 $4 > $3
	pc = write_opcode(prog, pc, add_op);
	pc = write_register(prog, pc, 3, REG_VAR);
	pc = write_register(prog, pc, 4, REG_VAR);
	pc = write_register(prog, pc, 3, REG_VAR);
	
	// array[i] = x
	// M_STORE $1 $3 $2
	pc = write_opcode(prog, pc, store_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 3, REG_VAR);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// i++
	// INCR $2 > $2
	pc = write_opcode(prog, pc, incr_op);
	pc = write_register(prog, pc, 2, REG_VAR);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// while i < N
	// EQ $2 $0 > $3
	pc = write_opcode(prog, pc, eq_op);
	pc = write_register(prog, pc, 2, REG_VAR);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 3, REG_VAR);
	
	// GT $2 $0 > $4
	pc = write_opcode(prog, pc, gt_op);
	pc = write_register(prog, pc, 2, REG_VAR);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 4, REG_VAR);
	
	// OR $3 $4 > $3
	pc = write_opcode(prog, pc, or_op);
	pc = write_register(prog, pc, 3, REG_VAR);
	pc = write_register(prog, pc, 4, REG_VAR);
	pc = write_register(prog, pc, 3, REG_VAR);
	
	// BRANCH $3 FIB_LOOP
	pc = write_opcode(prog, pc, branch_op);
	pc = write_register(prog, pc, 3, REG_VAR);
	int label_fib_loop_ref = pc;
	pc += sizeof(Data);
	
	// DECR $2 > $2
	pc = write_opcode(prog, pc, decr_op);
	pc = write_register(prog, pc, 2, REG_VAR);
	pc = write_register(prog, pc, 2, REG_VAR);
	
	// ret = array[i-1]
	// M_LOAD $1 $2 > $32
	pc = write_opcode(prog, pc, load_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 2, REG_VAR);
	pc = write_register(prog, pc, 32, REG_VAR);
	
	// :RET
	int label_ret_loc = pc;
	
	// print ret
	// OUTPUT $!2 $32
	pc = write_opcode(prog, pc, rat_output_op);
	pc = write_register(prog, pc, 2, REG_SPEC);
	pc = write_register(prog, pc, 32, REG_VAR);
	
	// HALT
	pc = write_opcode(prog, pc, halt_op);
	
	record_pc(prog, label_ret_loc, &label_ret_ref, 1);
	record_pc(prog, label_fib_array_alloc_loc, &label_fib_array_alloc_ref, 1);
	record_pc(prog, label_fib_loop_loc, &label_fib_loop_ref, 1);
	
	return pc;
}

int write_pushpop(byte* prog){
	int pc = 0;
	
	Operation copy_op = encode_operation(I_MOVE, SO_REGISTER);
	Operation rel_jump_op = encode_operation(I_JUMP, FORMAT3_SUBOP(SO_CONSTANT, SO_RELATIVE));
	Operation num_input_op = encode_operation(I_INPUT, SO_NUMBER);
	Operation eq_op = encode_operation(I_EQ, FORMAT1_SUBOP(SO_NUMBER, SO_REGISTER, SO_REGISTER, SO_NONE));
	Operation not_op = encode_operation(I_NOT, SO_NONE);
	Operation branch_op = encode_operation(I_BRANCH, SO_NONE);
	Operation copy_cost_op = encode_operation(I_MOVE, SO_CONSTANT);
	Operation push_frame_op = encode_operation(I_PUSHFRAME, SO_NONE);
	Operation pop_frame_op = encode_operation(I_POPFRAME, SO_NONE);
	Operation abs_jump_op = encode_operation(I_JUMP, FORMAT3_SUBOP(SO_CONSTANT, SO_ABSOLUTE));
	Operation dyn_abs_jump_op = encode_operation(I_JUMP, FORMAT3_SUBOP(SO_REGISTER, SO_ABSOLUTE));
	Operation add_op = encode_operation(I_ADD, FORMAT1_SUBOP(SO_NUMBER, SO_REGISTER, SO_REGISTER, SO_NONE));
	Operation num_output_op = encode_operation(I_OUTPUT, FORMAT2_SUBOP(SO_REGISTER, SO_NUMBER));
	
	Data two;
	two.n = 2;
	
	int label_resume_refs[2];
	
	// arg-read[0] = RESUME; usually don't write to read-registers, but the first frame needs to find the popframe and exit
	pc = write_opcode(prog, pc, copy_cost_op);
	label_resume_refs[0] = pc;
	pc += sizeof(Data);
	pc = write_register(prog, pc, 0, REG_RARG);
	
	// :RECURSE_START
	int label_recurse_start_loc = pc;
	
	// n = stdin.read(int)
	pc = write_opcode(prog, pc, num_input_op);
	pc = write_register(prog, pc, 3, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// if n == 0 goto RETURN
	pc = write_opcode(prog, pc, eq_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 0, REG_SPEC);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	pc = write_opcode(prog, pc, not_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	pc = write_register(prog, pc, 1, REG_VAR);
	
	pc = write_opcode(prog, pc, branch_op);
	pc = write_register(prog, pc, 1, REG_VAR);
	int label_return_ref = pc;
	pc += sizeof(Data);
	
	// arg-write[0] = RESUME
	pc = write_opcode(prog, pc, copy_cost_op);
	label_resume_refs[1] = pc;
	pc += sizeof(Data);
	pc = write_register(prog, pc, 0, REG_WARG);
	
	// arg-write[1] = n
	pc = write_opcode(prog, pc, copy_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 1, REG_WARG);
	
	// PUSHFRAME
	pc = write_opcode(prog, pc, push_frame_op);
	
	// goto RECURSE_START
	pc = write_opcode(prog, pc, abs_jump_op);
	int label_recurse_start_ref = pc;
	pc += sizeof(Data);
	
	// :RESUME
	int label_resume_loc = pc;
	
	// POPFRAME
	pc = write_opcode(prog, pc, pop_frame_op);
	
	// print return-read[0]
	pc = write_opcode(prog, pc, num_output_op);
	pc = write_register(prog, pc, 2, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_RRET);
	
	// n = n + return-read[0]
	pc = write_opcode(prog, pc, add_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 0, REG_RRET);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// print n
	pc = write_opcode(prog, pc, num_output_op);
	pc = write_register(prog, pc, 2, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_VAR);
	
	// :RETURN
	int label_return_loc = pc;
	
	// return-write[0] = n
	pc = write_opcode(prog, pc, copy_op);
	pc = write_register(prog, pc, 0, REG_VAR);
	pc = write_register(prog, pc, 0, REG_WRET);
	
	// print return-write[0]
	pc = write_opcode(prog, pc, num_output_op);
	pc = write_register(prog, pc, 2, REG_SPEC);
	pc = write_register(prog, pc, 0, REG_WRET);
	
	// goto arg-read[0]
	pc = write_opcode(prog, pc, dyn_abs_jump_op);
	pc = write_register(prog, pc, 0, REG_RARG);
	
	
	record_pc(prog, label_recurse_start_loc, &label_recurse_start_ref, 1);
	record_pc(prog, label_return_loc, &label_return_ref, 1);
	record_pc(prog, label_resume_loc, label_resume_refs, 2);
	
	return pc;
}


int write_opcode(byte* prog, int pc, Operation op){
	
	if (!prog) return pc + 2;
	
	prog[pc] = op.opcode;
	prog[pc + 1] = op.subop;
	
	return pc + 2;
}

int write_register(byte* prog, int pc, int reg_id, int r_type){
	
	if (!prog) return pc + 1;
	
	reg_id = reg_id & 0xFF;
	byte reg = 0;
	
	switch(r_type){
		case REG_VAR:
			reg = (reg_id & 0x3F) | (0x00 << 5);
			break;
		case REG_GLOB:
			reg = (reg_id & 0x1F) | (0x06 << 5);
			break;
		case REG_RARG:
			reg = (reg_id & 0x1F) | (0x02 << 5);
			break;
		case REG_RRET:
			reg = (reg_id & 0x1F) | (0x03 << 5);
			break;
		case REG_WARG:
			reg = (reg_id & 0x1F) | (0x04 << 5);
			break;
		case REG_WRET:
			reg = (reg_id & 0x1F) | (0x05 << 5);
			break;
		case REG_SPEC:
			reg = (reg_id & 0x1F) | (0x07 << 5);
			break;
	}
	
	prog[pc] = reg;
	
	return pc + 1;
}

int write_constant(byte* prog, int pc, Data data){
	if (!prog) return pc + sizeof(Data);
	
	int i;
	
	for (i = 0; i < sizeof(Data); i++){
		prog[pc + i] = data.bytes[i];
	}
	
	return pc + sizeof(Data);
}

int write_constant_inline(byte* prog, int pc, long int n){
	Data data;
	data.n = n;
	
	return write_constant(prog, pc, data);
}

int write_byte(byte* prog, int pc, byte b){
	if (!prog) return pc + 1;
	
	prog[pc] = b;
	
	return pc + 1;
}

void record_pc(byte* prog, int pc, int* refs, int nrefs){
	Data location;
	location.addr = (PCType) pc;
	int i;
	for (i = 0; i < nrefs; i++){
		write_constant(prog, refs[i], location);
	}
}