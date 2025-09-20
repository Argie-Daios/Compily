#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <sstream>
#include <fstream>

namespace Iry
{
	template<typename InstructionData>
	class IRGenerator
	{
	public:
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
		IRGenerator(const std::string& fileName)
			: m_FileName(fileName)
		{
		}
		virtual ~IRGenerator()
		{
			for (Instruction& instruction : m_InstructionSet)
			{
				OnInstructionDestruct(instruction.Data);
			}
		}

		void Export()
		{
			std::ofstream stream(m_FileName + ".i");

			for (Instruction& instruction : m_InstructionSet)
			{
				ExportInstruction(stream, instruction.OperationID, instruction.Data);
			}

			stream.close();
		}
		template<typename ... Args>
		void AddInstruction(OperationID_t operationID, Args&& ... args)
		{
			m_InstructionSet.emplace_back(operationID, std::forward<Args>(args)...);
		}
		inline const std::string& GetFileName() const { return m_FileName; }
		std::vector<Instruction*> GetInstructionsOfOperation(OperationID_t operation)
		{
			std::vector<InstructionData*> instructions;
			for (Instruction& instruction : m_InstructionSet)
			{
				instructions.push_back(instruction.Data);
			}
			return instructions;
		}
		inline Instruction& GetLastInstruction() { return m_InstructionSet.back(); }
		InstructionSet& GetInstructionSet() { return m_InstructionSet; }
		const InstructionSet& GetInstructionSet() const { return m_InstructionSet; }
	protected:
		virtual void ExportInstruction(std::ofstream& exportStream, OperationID_t operation,
			InstructionData& instruction) = 0;
		virtual void OnInstructionDestruct(InstructionData& instructionData) {}
		virtual const std::string OperationToStr(OperationID_t operation) { return std::to_string(operation); }
	private:
		std::string m_FileName;
		InstructionSet m_InstructionSet;
	};
}