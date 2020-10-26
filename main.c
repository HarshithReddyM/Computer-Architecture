/*
 *  main.c
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	
#include "cpu.h"

int
main(int argc, char const* argv[])
{
  if (argc != 4) {
    fprintf(stderr, "APEX_Help : Usage %s <input_file>\n", argv[0]);
    exit(1);
  }

  APEX_CPU* cpu = APEX_cpu_init(argv[1]);
  if (!cpu) {
    fprintf(stderr, "APEX_Error : Unable to initialize CPU\n");
    exit(1);
  }
  
  bool is_display_or_Simulate = false;
  
  if (strcmp(argv[2], "display") == 0 ||
		strcmp(argv[2], "Display") == 0 ||
		strcmp(argv[2], "DISPLAY") == 0)
	  is_display_or_Simulate = true;
  else 
	  is_display_or_Simulate = false;
  
  cpu->display_simulate = is_display_or_Simulate;
  
  APEX_cpu_run(cpu);
  APEX_cpu_stop(cpu);
  return 0;
}
