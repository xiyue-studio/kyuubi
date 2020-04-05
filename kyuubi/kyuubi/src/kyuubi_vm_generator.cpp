#include "stdafx.h"
#include "kyuubi_vm_generator.h"

using namespace std;
using namespace xiyue;
using namespace kyuubi;

VMGenerator::VMGenerator()
{
	m_instructions = &m_instructionsBuffer;
}

uint32_t VMGenerator::generateInstruction(VmDirective directive, int arg1 /*= 0*/, ConstString arg2 /*= L""_cs*/)
{
	m_instructions->push_back({ directive, arg1, arg2 });
	return static_cast<uint32_t>(m_instructions->size() - 1);
}

VmInstruction& VMGenerator::getInstruction(uint32_t addr)
{
	return m_instructions->at(addr);
}

uint32_t VMGenerator::getCurrentAddress() const
{
	return static_cast<uint32_t>(m_instructions->size());
}

void VMGenerator::popInstruction()
{
	m_instructions->pop_back();
}

void VMGenerator::swapInstructions(std::vector<VmInstruction>* tempInstructions)
{
	if (tempInstructions == nullptr)
		m_instructions = &m_instructionsBuffer;
	else
		m_instructions = tempInstructions;
}

void VMGenerator::appendInstructions(std::vector<VmInstruction>* instructions)
{
	m_instructions->insert(m_instructions->end(), instructions->begin(), instructions->end());
}

VM* VMGenerator::makeVM()
{
	return new VM(&m_instructions->front(), m_instructions->size());
}
