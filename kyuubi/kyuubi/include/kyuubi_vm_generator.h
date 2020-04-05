#pragma once
#include "xiyue_const_string.h"
#include "kyuubi_vm_types.h"
#include "kyuubi_vm.h"

namespace kyuubi
{
	class VMGenerator
	{
	public:
		VMGenerator();
		virtual ~VMGenerator() = default;

	public:
		VM* makeVM();

		uint32_t generateInstruction(VmDirective directive, int arg1 = 0, xiyue::ConstString arg2 = xiyue::ConstString());
		VmInstruction& getInstruction(uint32_t addr);
		uint32_t getCurrentAddress() const;
		void popInstruction();
		void swapInstructions(std::vector<VmInstruction>* tempInstructions);
		void appendInstructions(std::vector<VmInstruction>* instructions);

	protected:
		std::vector<VmInstruction> m_instructionsBuffer;
		std::vector<VmInstruction>* m_instructions;
	};
}
