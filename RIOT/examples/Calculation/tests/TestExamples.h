/*
 * TestExamples.h
 *
 *  Created on: 23.03.2017
 *      Author: mbesuden
 */

#ifndef TESTS_TESTEXAMPLES_H_
#define TESTS_TESTEXAMPLES_H_

inline void test_examples_while_loop(void)
{

	Memory::instance().clear();
	VM vm(&Memory::instance(), PID::instances());
	uint8_t program[] = {VM_INSTRUCTION_LOAD, VM_OPERAND_TYPE_UINT8 | VM_LITERAL, 0x20, 0x00, 0x00,//Load uint8_t 0 an 0x0020
			VM_INSTRUCTION_ADD, VM_OPERAND_TYPE_UINT8 | VM_LITERAL, 0x20, 0x00, 0x01,//add 1 to 0x0020
			VM_INSTRUCTION_COMPARE, VM_OPERAND_TYPE_UINT8 | VM_LITERAL, 0x20, 0x00, 0x0a, 0x05, 0x00, 0x05, 0x00, 0x00, 0x00//compare 0x0020 with uint8_t 10, jump to add while 0x0020 <= 10, else run program again
	};
	vm.setProgram(program, 21);
	//program will never halt
	for(uint8_t i = 0; i < 100; i++)
	{
		vm.executeStep();
		ASSERT(vm.getProgramcounter() == 5, "programcounter wrong");
		ASSERT(Memory::instance().load(0x0020) == 0, "Load wrong");
		ASSERT(!vm.halted(), "VM should never halt");

		vm.executeStep();
		ASSERT(vm.getProgramcounter() == 10, "programcounter wrong");
		ASSERT(Memory::instance().load(0x0020) == 1, "Load wrong");
		ASSERT(!vm.halted(), "VM should never halt");

		for(uint8_t j = 0; j < 21; j++)//ten times add, eleven compares
		{
			vm.executeStep();
		}
		ASSERT(vm.getProgramcounter() == 0, "programcounter wrong");
		ASSERT(Memory::instance().load(0x0020) == 11, "Load wrong");
		ASSERT(!vm.halted(), "VM should never halt");
	}
	ASSERT(!vm.halted(), "VM should never halt");
}

inline void  test_examples_after_x_ms()
{
	Memory::instance().clear();
	VM vm(&Memory::instance(), PID::instances());

	uint8_t program[] = {
			VM_INSTRUCTION_TIME, 0x10, 0x00,//save CPU time in 0x0010
			VM_INSTRUCTION_COMPARETIME, 0x10, 0x00, 0x88, 0x13, 0x00, 0x00, 0x0e, 0x00, 0x03, 0x00,//compare time in 0x0010 plus timeout (5s) with now
			VM_INSTRUCTION_HALT
	};
	vm.setProgram(program, 15);
	uint32_t before = xtimer_now()/1000;
	while(!vm.halted())
	{
		vm.executeStep();
	}
	uint32_t after = xtimer_now()/1000;
	ASSERT(vm.getProgramcounter() == 14, "programcounter wrong");
	ASSERT(after - before >= 5000, "Program does not wait long enough");
	ASSERT(after - before < 6500, "Not in 0.5s tolerance");
}

inline void test_print_heater_simulation()
{
	uint8_t heater_simulation_short[] = {
			VM_INSTRUCTION_URLMAP, 0x00 | VM_OPERAND_TYPE_DEC, VM_MAP_OPTION_RESOURCE_LITERAL | VM_MAP_OPTION_URL_LITERAL | VM_MAP_OPTION_DIRECTION_CLIENT | VM_MAP_OPTION_METHOD_GET | VM_MAP_OPTION_LIFETIME_EVER ,0x70 , 0x00, 0x00, 0x00, 'a', 'f', 'f', 'e', ':', ':', '1', 0x00, '/', 's', 'e', 'n', 's', 'o', 'r', 0x00,
			VM_INSTRUCTION_URLMAP, 0x10 | VM_OPERAND_TYPE_DEC, VM_MAP_OPTION_RESOURCE_LITERAL | VM_MAP_OPTION_URL_ADDRESS | VM_MAP_OPTION_DIRECTION_CLIENT | VM_MAP_OPTION_METHOD_GET | VM_MAP_OPTION_LIFETIME_EVER ,0x74 , 0x00, 0x00, 0x00, 0x07, 0x00, '/', 's', 'e', 't', 'p', 'o', 'i', 'n', 't', 0x00,
			VM_INSTRUCTION_URLMAP, 0x20 | VM_OPERAND_TYPE_DEC, VM_MAP_OPTION_RESOURCE_LITERAL | VM_MAP_OPTION_URL_ADDRESS | VM_MAP_OPTION_DIRECTION_CLIENT | VM_MAP_OPTION_METHOD_POST | VM_MAP_OPTION_LIFETIME_EVER ,0x78 , 0x00, 0x00, 0x00, 0x07, 0x00, '/', 'h', 'e', 'a', 't', 'e', 'r', 0x00,
			VM_INSTRUCTION_PIDINIT, 0x00, 0x70, 0x00, 0x78, 0x00, 0x74, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x64, 0x00, 0x00, 0x00,
			//						id	  input		  output	  setpoint	  kp					  ki					  kd					  sampletime (uint32)	  lowerlimit			  upperlimit	  		  dir
			VM_INSTRUCTION_PIDRUN, 0x00,
			VM_INSTRUCTION_HALT
	};
	printf("\nHeater Simulation Bytecode Program:\n");
	for(uint8_t i = 0; i < sizeof(heater_simulation_short); i++)
	{
//		Memory::instance().store(i, heater_simulation_short[i]);
		printf("%02x", heater_simulation_short[i]);
	}
	printf("\n");
}

inline void test_print_ggt()
{
	uint8_t ggt_program[] = {
			VM_INSTRUCTION_LOAD, 0x01, 0x70, 0x00, 0x5F, 0xE5, 0x73, 0x6D, //7 (07)
			VM_INSTRUCTION_LOAD, 0x01, 0x74, 0x00, 0x82, 0x3F, 0xA5, 0x43, //15 (0F)
			VM_INSTRUCTION_TIME, 0x80, 0x00, //18 (12)
			VM_INSTRUCTION_COMPARE, 0x01, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2B, 0x00, 0x21, 0x00, 0x2B, 0x00, //32 (20)
			VM_INSTRUCTION_LOAD, 0x00, 0x70, 0x00, 0x74, 0x00, //38 (26)
			VM_INSTRUCTION_JUMP, 0x01, 0x59, 0x00, //42 (2A)
			VM_INSTRUCTION_COMPARE, 0x01, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x39, 0x00, 0x59, 0x00, 0x39, 0x00, //56 (38)
			VM_INSTRUCTION_COMPARE, 0x00, 0x70, 0x00, 0x74, 0x00, 0x4F, 0x00, 0x4F, 0x00, 0x45, 0x00, //68 (44)
			VM_INSTRUCTION_SUB, 0x00, 0x70, 0x00, 0x74, 0x00, //74 (4A)
			VM_INSTRUCTION_JUMP, 0x01, 0x2B, 0x00, //78 (4E)
			VM_INSTRUCTION_SUB, 0x00, 0x74, 0x00, 0x70, 0x00, //84 (54)
			VM_INSTRUCTION_JUMP, 0x01, 0x2B, 0x00, //88 (58)
			VM_INSTRUCTION_TIME, 0x84, 0x00, //91 (5B)
			VM_INSTRUCTION_SUB, 0x00, 0x84, 0x00, 0x80, 0x00, //97 (61)
			VM_INSTRUCTION_HALT //98 (62)
	};

	Memory::instance().clear();
	VM vm(&Memory::instance(), PID::instances());
	printf("\nGCD Bytecode Program:\n");
	for(uint16_t i = 0; i < sizeof(ggt_program); i++)
	{
		Memory::instance().store(i, ggt_program[i]);
		printf("%02x", ggt_program[i]);
	}
	printf("\n");
	while(!vm.halted())
	{
		vm.executeStep();
	}
	//printf("Status: 0x%02x\n", vm.getStatuscode());
	//printf("ggt: %d\n", Memory::instance().loadunsigned(0x0070));
	//printf("Time: %d\n", Memory::instance().loadunsigned(0x0084));
	ASSERT(vm.getProgramcounter() == 98, "programcounter wrong");
	ASSERT(Memory::instance().loadunsigned(0x0070) == 1, "gcd wrong should be 1");
}

inline void test_examples(void)
{
#ifndef TEST_EXAMPLES_OFF
	test_examples_while_loop();
	test_examples_after_x_ms();
	test_print_heater_simulation();
	test_print_ggt();
#else
	TESTINFO("Test Examples off");
#endif
}

#endif /* TESTS_TESTEXAMPLES_H_ */
