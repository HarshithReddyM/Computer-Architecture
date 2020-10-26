/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
#define ENABLE_DEBUG_MESSAGES 1
int multiplyflag = 0;

/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
 int new_pc;
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES);
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);
  cpu->clock = 1;
  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 * 				implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if(stage->display_simulate == true)
  {	  
	  if (strcmp(stage->opcode, "STORE") == 0) {
		printf(
		  "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
	  }
	  if (strcmp(stage->opcode, "LOAD") == 0) {
		printf(
		  "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
	  }
	  if (strcmp(stage->opcode, "ADD") == 0) {
		printf(
		  "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
	  }
	  if (strcmp(stage->opcode, "SUB") == 0) {
		printf(
		  "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
	  }
	  if (strcmp(stage->opcode, "MUL") == 0) {
		printf(
		  "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
	  }
	  if (strcmp(stage->opcode, "AND") == 0) {
		printf(
		  "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
	  }
	  if (strcmp(stage->opcode, "OR") == 0) {
		printf(
		  "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
	  }
	  if (strcmp(stage->opcode, "EX-OR") == 0) {
		printf(
		  "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2);
	  }
	  if (strcmp(stage->opcode, "BZ") == 0) {
		printf(
		  "%s,#%d ", stage->opcode, stage->imm);
	  }
	  if (strcmp(stage->opcode, "BNZ") == 0) {
		printf(
		  "%s,#%d ", stage->opcode, stage->imm);
	  }
	  if (strcmp(stage->opcode, "JUMP") == 0) {
		printf(
		  "%s,#%d ", stage->opcode, stage->imm);
	  }
	  if (strcmp(stage->opcode, "HALT") == 0) {
		printf(
		  "%s,#%d ", stage->opcode, stage->imm);
	  }

	  if (strcmp(stage->opcode, "MOVC") == 0) {
		printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
	  }
  }
}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*void trans_bubble(CPU_Stage* stage){
	stage->pc = -1;
	stage->bubble = 1;
	strcpy(stage->opcode, "NOP");
}*/

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];
  if (!stage->busy && !stage->stalled) {  
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;
	stage->display_simulate = cpu->display_simulate;
    /* Update PC for next instruction */
    cpu->pc += 4;

    /* Copy data from fetch latch to decode latch*/
    //cpu->stage[DRF] = cpu->stage[F];
	
	if(cpu->stage[DRF].stalled == 0){
		cpu->stage[DRF] = cpu->stage[F];
	}
	else{
		cpu->stage[F].stalled = 1;
	}
  }
  else if(stage->stalled == 1 && cpu->stage[DRF].stalled == 0)
  {
		stage->stalled = 0;
		cpu->stage[DRF] = cpu->stage[F];
  }
  if(cpu->branch == 1)
	{
		cpu->stage[DRF] = cpu->stage[NOP];
		cpu->pc = cpu->branch_address;
		cpu->branch = 0;
	}
  
  
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch", stage);
    }
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];
  if (!stage->busy /* && !stage->stalled*/) {
    		
    /* Read data from register file for store */
	if (strcmp(stage->opcode, "MOVC") == 0) {
		cpu->regs_valid[stage->rd] = 0;
	}
    if (strcmp(stage->opcode, "STORE") == 0) {
		if((cpu->regs_valid[stage->rs1] == 1) && (cpu->regs_valid[stage->rs2] == 1))
		{
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        stage->stalled = 0;
		}   
		else{
		stage->stalled = 1;
		}
    }
	if (strcmp(stage->opcode, "LOAD") == 0) 
    {
        if(cpu->regs_valid[stage->rs1] == 1)
		{
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = stage->rs2;
        cpu->regs_valid[stage->rd] = 0;
		stage->stalled = 0;
		}   
		else{
		stage->stalled = 1;
		}
    }
	
	if (strcmp(stage->opcode, "MUL") == 0) 
    {
		if((cpu->regs_valid[stage->rs1] == 1) && (cpu->regs_valid[stage->rs2] == 1))
		{
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd] = 0;
		stage->stalled = 0;
		}   
		else{
		stage->stalled = 1;
		}
    }
	
	if (strcmp(stage->opcode, "EX-OR") == 0) 
    {
        if((cpu->regs_valid[stage->rs1] == 1) && (cpu->regs_valid[stage->rs2] == 1))
		{
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd] = 0;
		stage->stalled = 0;
		}   
		else{
		stage->stalled = 1;
		}
    }
	if (strcmp(stage->opcode, "OR") == 0) 
    {
        if((cpu->regs_valid[stage->rs1] == 1) && (cpu->regs_valid[stage->rs2] == 1))
		{
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd] = 0;
		stage->stalled = 0;
		}   
		else{
		stage->stalled = 1;
		}
    }
	if (strcmp(stage->opcode, "AND") == 0) 
    {
         if((cpu->regs_valid[stage->rs1] == 1) && (cpu->regs_valid[stage->rs2] == 1))
		{
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd] = 0;
		stage->stalled = 0;
		}   
		else{
		stage->stalled = 1;
		}
    }
	if (strcmp(stage->opcode, "SUB") == 0) 
    {
         if((cpu->regs_valid[stage->rs1] == 1) && (cpu->regs_valid[stage->rs2] == 1))
		{
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd] = 0;
		stage->stalled = 0;
		}   
		else{
		stage->stalled = 1;
		}
    }
    if (strcmp(stage->opcode, "ADD") == 0) 
    {
         if((cpu->regs_valid[stage->rs1] == 1) && (cpu->regs_valid[stage->rs2] == 1))
		{
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->regs_valid[stage->rd] = 0;
		stage->stalled = 0;
		}   
		else{
		stage->stalled = 1;
		}
    }
	if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ")==0){
		if(stage->cycles_in_execute == 0)
		{
			//printf("Opcode: %s\n" , cpu->stage[EX].opcode);
			if(strcmp(cpu->stage[EX].opcode, "ADD") == 0 || strcmp(cpu->stage[EX].opcode, "SUB") == 0)
			{
				stage->bubble = 1;
			}
			else if(strcmp(cpu->stage[EX].opcode, "MUL") == 0)
			{
				stage->bubble = 2;
			}
			stage->stalled = 1;
			stage->cycles_in_execute++;
		}
		else if(stage->bubble > 0 )
		{
			stage->cycles_in_execute++;
			stage->stalled = 1;
			stage->bubble--;
			//printf("here\n");
		}
		else
		{
			stage->stalled = 0;
		}
	}		 
	if (strcmp(stage->opcode, "JUMP") == 0) 
    {
       if(cpu->regs_valid[stage->rs1]==0)
	   {
		   stage->stalled = 1;
	   }
		   else{
			   stage->rs1_value = cpu->regs[stage->rs1];
		   }
	}
	
	if (strcmp(stage->opcode, "HALT") == 0) {
		cpu->stage[F].stalled = 1;
	}
	
	}
	
	if(cpu->stage[EX].stalled == 1)
	{
		stage->stalled = 1;
	}
	else if(stage->stalled == 1 && cpu->stage[EX].stalled == 0)
	{
		cpu->stage[EX] = cpu->stage[NOP];
	}
	else
	{
		cpu->stage[EX] = cpu->stage[DRF];
	}
	
	if(cpu->branch == 1)
	{
		cpu->stage[EX] = cpu->stage[NOP];
		if (strcmp(stage->opcode, "LOAD") == 0 || strcmp(stage->opcode, "AND") == 0 || strcmp(stage->opcode, "MOVC") == 0|| strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "EX-OR") == 0 || strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "MUL") == 0) {
		cpu->regs_valid[stage->rd] = 1;
		}
	}
	
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Decode/RF", stage);
    }

  
  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
execute(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX];
  if (!stage->busy /*&& !stage->stalled*/) {
		  
	if (strcmp(stage->opcode, "JUMP") == 0) 
    {
        int new_prog_count = stage->imm + stage ->rs1_value;
		int num_instr_diff = ((new_prog_count - cpu->pc)/4);
		if(num_instr_diff > 0)
		{
			cpu->code_memory_size -=(num_instr_diff - 1);
		}
		else if(num_instr_diff < 0)
		{
			cpu->code_memory_size +=(num_instr_diff + 1);
		}
		cpu->pc = new_prog_count;
		
    }
	if (strcmp(stage->opcode, "LOAD") == 0) 
    {
        stage->buffer = stage->rs1_value + stage->rs2_value;
        if(stage->buffer == 0)
        {
            stage->flag_zero = true;
        }
    }
	if (strcmp(stage->opcode, "STORE") == 0) 
    {
        stage->buffer = stage->rs2_value + stage->imm;
    }
	if (strcmp(stage->opcode, "EX-OR") == 0) 
    {
        stage->buffer = stage->rs1_value ^ stage->rs2_value;
    }
	if (strcmp(stage->opcode, "OR") == 0) 
    {
        stage->buffer = stage->rs1_value | stage->rs2_value;
    }
	if (strcmp(stage->opcode, "AND") == 0) 
    {
        stage->buffer = stage->rs1_value & stage->rs2_value;
    }
	if (strcmp(stage->opcode, "MUL") == 0) 
    {
		stage->buffer = stage->rs1_value * stage->rs2_value;
		if(stage->cycles_in_execute == 0)
		{
			stage->cycles_in_execute = 1;
			stage->stalled = 1;
		}
		else
		{
			stage->stalled = 0;
			stage->cycles_in_execute = 0;
		}
		//printf("%d\n",stage->buffer);
	}
	
	if (strcmp(stage->opcode, "SUB") == 0) 
    {
        stage->buffer = stage->rs1_value - stage->rs2_value;
    }
    if (strcmp(stage->opcode, "ADD") == 0) 
    {
        stage->buffer = stage->rs1_value + stage->rs2_value;
    }
	if (strcmp(stage->opcode, "BZ") == 0) 
    {
       if(cpu->flag_zero == 1)
	   {
		   cpu->branch_address = stage->pc + stage->imm;
		   cpu->branch = 1;
	   }
    }
	if (strcmp(stage->opcode, "BNZ") == 0) 
    {
         if(cpu->flag_zero == 0)
	   {
		   cpu->branch_address = stage->pc + stage->imm;
		   cpu->branch = 1;
	   }      
    }
    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
		stage->buffer = stage->imm;
    }

    /* Copy data from Execute latch to Memory latch*/
    if(stage->stalled == 0)
	{
		cpu->stage[MEM] = cpu->stage[EX];
	}
	else
	{
		cpu->stage[MEM] = cpu->stage[NOP];
	}

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute", stage);
    }
  }
  return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
memory(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
	  cpu->data_memory[stage->buffer] = stage->rs1_value;
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
	stage->buffer = cpu->data_memory[stage->buffer];
    }
	
	if (strcmp(stage->opcode, "LOAD") == 0 || strcmp(stage->opcode, "AND") == 0 || strcmp(stage->opcode, "MOVC") == 0|| strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "EX-OR") == 0 || strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "MUL") == 0) {
	cpu->regs_valid[stage->rd] = 0;
	}
    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM];

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory", stage);
    }
  }
  return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
writeback(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[WB];
  if (!stage->busy && !stage->stalled) {

    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
	  cpu->regs_valid[stage->rd] = 1;
	  cpu->ins_completed++;
    }
	if (strcmp(stage->opcode, "ADD") == 0) 
    {
      cpu->regs[stage->rd] = stage->buffer;
	  cpu->regs_valid[stage->rd] = 1;
	  cpu->ins_completed++;
	  if(stage->buffer == 0)
        {
            cpu->flag_zero = 1;
        }
		else
		{
			cpu->flag_zero = 0;
		}
	}
	if (strcmp(stage->opcode, "SUB") == 0) 
    {
      cpu->regs[stage->rd] = stage->buffer;
	  cpu->regs_valid[stage->rd] = 1;
	  cpu->ins_completed++;
	    if(stage->buffer == 0)
        {
            cpu->flag_zero = 1;
        }
		else
		{
			cpu->flag_zero = 0;
		}
	}
	if (strcmp(stage->opcode, "MUL") == 0) 
    {
      cpu->regs[stage->rd] = stage->buffer;
	  cpu->regs_valid[stage->rd] = 1;
	  cpu->ins_completed++;
	    if(stage->buffer == 0)
        {
            cpu->flag_zero = 1;
        }
		else
		{
			cpu->flag_zero = 0;
		}
	}
	if (strcmp(stage->opcode, "LOAD") == 0) 
    {
      cpu->regs[stage->rd] = stage->buffer;
	  cpu->regs_valid[stage->rd] = 1;
	  cpu->ins_completed++;
    }
	if (strcmp(stage->opcode, "AND") == 0) 
    {
      cpu->regs[stage->rd] = stage->buffer;
	  cpu->regs_valid[stage->rd] = 1;
	  cpu->ins_completed++;
	}
	if (strcmp(stage->opcode, "OR") == 0) 
    {
      cpu->regs[stage->rd] = stage->buffer;
	  cpu->regs_valid[stage->rd] = 1;
	  cpu->ins_completed++;
	}
	if (strcmp(stage->opcode, "EX-OR") == 0) 
    {
      cpu->regs[stage->rd] = stage->buffer;
	  cpu->regs_valid[stage->rd] = 1;
	  cpu->ins_completed++;
	}
	if (strcmp(stage->opcode, "STORE") == 0 || strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0) 
    {
		cpu->ins_completed++;
    }
	if (strcmp(stage->opcode, "HALT") == 0) 
    {
		cpu->ins_completed = cpu->code_memory_size;
		cpu->halt = 1;
    }

    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Writeback", stage);
    }
  return 0;
}
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 * 				 implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu)
{
  while (1) {

    /* All the instructions committed, so exit */
    if (cpu->halt == 1) {
      printf("(apex) >> Simulation Complete");
      break;
    }

    if (ENABLE_DEBUG_MESSAGES) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }

    writeback(cpu);
	memory(cpu);
	execute(cpu);
	decode(cpu);
    fetch(cpu);

    cpu->clock++;
  }
 printf("\n=================STATE OF ARCHITECTURAL FILE================\n");
 int valid = 0;
 for( int i=0;i<32;i++)
 {
	 valid = cpu->regs_valid[i];
	 printf("|REG[%d] | value=%d | Status=%s |",i,cpu->regs[i],(valid==1)? "VALID" : "INVALID");
	 printf("\n");
 }
 printf("===============STATE OF DATA MEMORY==================\n");
 for(int j=0;j<100;j++)
 {
 printf("|MEM[%d} | Data Value=%d \n",j,cpu->data_memory[j]);
 printf("\n");
}
  return 0;
}
