#include "QuadGenerator.h"

#include <Utilities.h>
#include <sstream>

QuadGenerator::QuadGenerator(const std::string& fileName)
	: IRGenerator<TQuad>(fileName)
{
}

void QuadGenerator::ExportDebugFormat()
{
	return;
	const InstructionSet& instructionSet = GetInstructionSet();
	std::vector<std::string> labels = { "Operation", "Arg1", "Arg2", "Result", "Line" };
	Utilities::TableStream tableStream(GetFileName() + ".i", std::ios::out, Utilities::TableStreamFlags_RowsLabel |
		Utilities::TableStreamFlags_ColumnsLabel);

	tableStream.BindGetRowLabelCallback([&](size_t row) -> const std::string& {
		static std::string helperString;
		helperString.clear();
		helperString = std::to_string(row);
		return helperString;
	});

	tableStream.BindGetColumnLabelCallback([&](size_t col) -> const std::string& {
		return labels.at(col);
	});

	tableStream.BindGetTotalRowsCallback([&]() {
		return instructionSet.size();
	});

	tableStream.BindGetTotalColumnsCallback([&]() {
		return labels.size();
	});

	tableStream.BindGetElementStringCallback([&](size_t row, size_t col) -> const std::string& {
		static std::string helperString;
		helperString.clear();
		const Instruction& instruction = instructionSet.at(row);
		switch (col)
		{
		case 0:
		{
			helperString = OperationToStr(instruction.OperationID);
			return helperString;
		}
		case 1:
		{
			const TExpression* arg1 = instruction.Data.Arg1;
			if (arg1 == nullptr)
			{
				helperString = "----";
				return helperString;
			}
			switch (arg1->Type)
			{
			case EExpressionType::Constant:
			{
				std::stringstream ss;
				arg1->ConstantValue.Visit([&](auto&& value) {
					ss << value;
				});
				helperString = ss.str();
				return helperString;
			}
			case EExpressionType::Lvalue:
			{
				return arg1->SymbolTableEntry;
			}
			case EExpressionType::Type:
			{
				helperString = arg1->ConstantValue.DataTypeProps.ToString();
				return helperString;
			}
			}
			break;
		}
		case 2:
		{
			const TExpression* arg2 = instruction.Data.Arg2;
			if (arg2 == nullptr)
			{
				helperString = "----";
				return helperString;
			}
			switch (arg2->Type)
			{
			case EExpressionType::Constant:
			{
				std::stringstream ss;
				arg2->ConstantValue.Visit([&](auto&& value) {
					ss << value;
				});
				helperString = ss.str();
				return helperString;
			}
			case EExpressionType::Lvalue:
			{
				return arg2->SymbolTableEntry;
			}
			case EExpressionType::Type:
			{
				helperString = arg2->ConstantValue.DataTypeProps.ToString();
				return helperString;
			}
			}
			break;
		}
		case 3:
		{
			const TExpression* result = instruction.Data.Result;
			if (result == nullptr)
			{
				helperString = "----";
				return helperString;
			}
			switch (result->Type)
			{
			case EExpressionType::Constant:
			{
				std::stringstream ss;
				result->ConstantValue.Visit([&](auto&& value) {
					ss << value;
				});
				helperString = ss.str();
				return helperString;
			}
			case EExpressionType::Lvalue:
			{
				return result->SymbolTableEntry;
			}
			case EExpressionType::Type:
			{
				helperString = result->ConstantValue.DataTypeProps.ToString();
				return helperString;
			}
			}
			break;
		}
		case 4:
		{
			helperString = std::to_string(instruction.Data.Line);
			return helperString;
		}
		}
	});

	tableStream.SetLabel("Quads");
	tableStream.SetLabelHorizontalAlignment(Utilities::HorizontalAlignment::Center);
	tableStream.SetHorizontalAlignment(Utilities::HorizontalAlignment::Center);
	tableStream.SetRowHorizontalSpacing(4);
	tableStream.SetColumnHorizontalSpacing(4);

	tableStream.Export();
}

void QuadGenerator::ExportInstruction(std::ofstream& exportStream, OperationID_t operation, TQuad& instruction)
{
	exportStream << OperationToStr(operation) << " | ";
	switch (instruction.Arg1->Type)
	{
	case EExpressionType::Constant:
	{
		exportStream << instruction.Arg1->ConstantValue.Get<int32_t>();
		break;
	}
	case EExpressionType::Lvalue:
	{
		exportStream << instruction.Arg1->SymbolTableEntry;
		break;
	}
	}
	exportStream << " | ";
	switch (instruction.Arg2->Type)
	{
	case EExpressionType::Constant:
	{
		exportStream << instruction.Arg2->ConstantValue.Get<int32_t>();
		break;
	}
	case EExpressionType::Lvalue:
	{
		exportStream << instruction.Arg2->SymbolTableEntry;
		break;
	}
	}
	exportStream << " | ";
	switch (instruction.Result->Type)
	{
	case EExpressionType::Constant:
	{
		exportStream << instruction.Result->ConstantValue.Get<int32_t>();
		break;
	}
	case EExpressionType::Lvalue:
	{
		exportStream << instruction.Result->SymbolTableEntry;
		break;
	}
	}
	exportStream << std::endl;
}

const std::string QuadGenerator::OperationToStr(OperationID_t operation)
{
	switch (operation)
	{
	case OperationCode_Add: return "ADD";
	case OperationCode_Substract: return "SUB";
	case OperationCode_Multiply: return "MUL";
	case OperationCode_Divide: return "DIV";
	case OperationCode_Assign: return "ASSIGN";
	case OperationCode_Cast: return "CAST";
	}
	return IRGenerator<TQuad>::OperationToStr(operation);
}
