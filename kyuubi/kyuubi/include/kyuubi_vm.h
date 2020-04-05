#pragma once
#include "kyuubi_vm_types.h"
#include "kyuubi_dumper.h"
#include "kyuubi_filter_manager.h"

namespace kyuubi
{
	class VM
	{
	public:
		// 默认情况下，用一个定长数组初始化的 VM，不会拷贝传入的所有指令。
		template<size_t size>
		VM(VmInstruction(&instructions)[size], bool needRelease = false) {
			new(this)VM(instructions, size, needRelease);
		}

		VM(const VmInstruction* instructions, size_t instructionCount, bool needRelease = true);
		virtual ~VM();

	public:
		void dump(xiyue::JsonObject* data, Dumper* dumper);

		void setFilterManager(FilterManager* filterManager) { m_filterManager = filterManager; }

		const VmInstruction* getInstructions() const { return m_instructions; }
		size_t getInstructionCount() const { return m_instructionCount; }

	protected:
		void run();

	private:
		VmInstruction* m_instructions;
		size_t m_instructionCount;
		bool m_needRelease;
		xiyue::JsonObject* m_data;
		Dumper* m_dumper;
		FilterManager* m_filterManager;
	};
}
