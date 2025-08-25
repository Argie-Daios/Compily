#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

namespace Iry
{
	template<typename InstructionData>
	class IRGenerator
	{
		using OperationID_t = int32_t;
		struct Instruction
		{
			OperationID_t OperationID;
			InstructionData Data;

			Instruction() = default;
			template<typename ... Args>
			Instruction(OperationID_t operationID, Args&& ... args)
				: OperationID(operationID), Data(std::forward<Args>(args)...)
			{

			}
		};
		using InstructionSet = std::vector<Instruction>;
	public:
		IRGenerator() = default;

		void Export();
		template<typename ... Args>
		void AddInstruction(OperationID_t operationID, Args&& ... args)
		{
			m_InstructionSet.emplace_back(operationID, std::forward<Args>(args)...);
		}
		const InstructionSet& GetInstructionSet() const { return m_InstructionSet; }
	protected:
		virtual void ExportInstruction(InstructionData& instruction) = 0;
		virtual const std::string OperationToStr(OperationID_t operation) { return std::to_string(operation); }
	private:
		InstructionSet m_InstructionSet;
	};
}