#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <sstream>
#include <fstream>

#include "Utilities/Macros.h"

namespace Iry
{
	namespace InstructionFlag
	{
		enum Enum : uint8_t
		{
			None = 0,
			Incomplete = BIT(0)
		};
	}

	template<typename InstructionData>
	class IRGenerator
	{
	public:
		using OperationID_t = int32_t;
		struct Instruction
		{
			OperationID_t OperationID;
			InstructionData Data;
			int32_t Flags = InstructionFlag::None;

			Instruction() = default;
			template<typename ... Args>
			Instruction(OperationID_t operationID, int32_t flags, Args&& ... args)
				: OperationID(operationID), Flags(flags), Data(std::forward<Args>(args)...)
			{

			}
			Instruction(const Instruction&) = default;
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
		void AddInstruction(OperationID_t operationID, int32_t flags, Args&& ... args)
		{
			m_InstructionSet.emplace_back(operationID, flags, std::forward<Args>(args)...);
		}

		template<typename ... Args>
		void AddInstructionBefore(size_t instructionIndex, OperationID_t operationID, int32_t flags, Args&& ... args)
		{
			if (instructionIndex < 0 || instructionIndex >= m_InstructionSet.size()) return;
			m_InstructionSet.emplace(m_InstructionSet.begin() + instructionIndex, operationID, flags, std::forward<Args>(args)...);
		}

		template<typename ... Args>
		void AddInstructionAfter(size_t instructionIndex, OperationID_t operationID, int32_t flags, Args&& ... args)
		{
			if (instructionIndex < 0 || instructionIndex >= m_InstructionSet.size()) return;
			m_InstructionSet.emplace(m_InstructionSet.begin() + instructionIndex + 1, operationID, flags, std::forward<Args>(args)...);
		}

		void RemoveInstruction(Instruction* instructionRef)
		{
			if (instructionRef == nullptr) return;
			m_InstructionSet.erase(std::remove_if(m_InstructionSet.begin(), m_InstructionSet.end(), 
				[instructionRef](const Instruction& instruction)
				{
					return &instruction == instructionRef;
				}), m_InstructionSet.end());
		}

		inline const std::string& GetFileName() const { return m_FileName; }
		std::vector<size_t> GetInstructionIndicesOfOperation(OperationID_t operation, int32_t flags = InstructionFlag::Enum::None)
		{
			std::vector<size_t> instructionIndices;
			for (size_t i = 0; i < m_InstructionSet.size(); i++)
			{
				Instruction& instruction = m_InstructionSet[i];
				if ((instruction.Flags & flags) != flags) continue;
				instructionIndices.push_back(i);
			}
			return instructionIndices;
		}
		inline Instruction& GetInstruction(size_t index) { return m_InstructionSet[index]; }
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