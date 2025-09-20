#include "TypeConvertionManager.h"

#include "TestParser.h"

TypeConvertionManager::TypeConvertionManager(TestParser* parserRef)
	: m_ParserRef(parserRef)
{

}

void TypeConvertionManager::RegisterType(const EDataType& type)
{
	if (m_ConvertorMap.find(type) != m_ConvertorMap.end()) return;
	m_ConvertorMap.try_emplace(type);
	auto& emplacedPair = m_ConvertorMap.find(type);
	for (auto& [keyDataType, connections] : m_ConvertorMap)
	{
		if (type == keyDataType) continue;
		connections.try_emplace(type);
		emplacedPair->second.try_emplace(keyDataType);
	}
}

void TypeConvertionManager::RegisterConversion(const EDataType& typeLeft, const EDataType& typeRight,
	const EConvertionType& convertionType, const ConstConvertionCallback& constConvertionCallback)
{
	switch (convertionType)
	{
	case EConvertionType::Left:
	{
		m_ConvertorMap.at(typeLeft).at(typeRight).Convertion = 
		[this, typeLeft, typeRight, constConvertionCallback](TExpression* left, TExpression* right) {
			return TConvertionResult{ new TExpression(*left),
				SubmitConversion(right, typeLeft, typeRight, constConvertionCallback, false)};
		};
		m_ConvertorMap.at(typeLeft).at(typeRight).DataType = typeLeft;
		m_ConvertorMap.at(typeLeft).at(typeRight).ConvertionType = convertionType;

		m_ConvertorMap.at(typeRight).at(typeLeft).Convertion =
		[this, typeLeft, typeRight, constConvertionCallback](TExpression* left, TExpression* right) {
			return TConvertionResult{ SubmitConversion(left, typeRight, typeLeft, constConvertionCallback, true),
				new TExpression(*right) };
		};
		m_ConvertorMap.at(typeRight).at(typeLeft).DataType = typeLeft;
		m_ConvertorMap.at(typeRight).at(typeLeft).ConvertionType = convertionType;
		break;
	}
	case EConvertionType::Right:
	{
		m_ConvertorMap.at(typeLeft).at(typeRight).Convertion =
		[this, typeLeft, typeRight, constConvertionCallback](TExpression* left, TExpression* right) {
			return TConvertionResult{ SubmitConversion(left, typeLeft, typeRight, constConvertionCallback, false),
				new TExpression(*right) };
		}; 
		m_ConvertorMap.at(typeLeft).at(typeRight).DataType = typeRight;
		m_ConvertorMap.at(typeLeft).at(typeRight).ConvertionType = convertionType;

		m_ConvertorMap.at(typeRight).at(typeLeft).Convertion =
		[this, typeLeft, typeRight, constConvertionCallback](TExpression* left, TExpression* right) {
			return TConvertionResult{ new TExpression(*left),
				SubmitConversion(right, typeRight, typeLeft, constConvertionCallback, true) };
		}; 
		m_ConvertorMap.at(typeRight).at(typeLeft).DataType = typeRight;
		m_ConvertorMap.at(typeRight).at(typeLeft).ConvertionType = convertionType;
		break;
	}
	case EConvertionType::Order:
	{
		m_ConvertorMap.at(typeLeft).at(typeRight).Convertion = 
		[this, typeLeft, typeRight, constConvertionCallback](TExpression* left, TExpression* right) {
			return TConvertionResult{ new TExpression(*left),
				SubmitConversion(right, typeLeft, typeRight, constConvertionCallback, true) };
		};
		m_ConvertorMap.at(typeLeft).at(typeRight).DataType = typeLeft;
		m_ConvertorMap.at(typeLeft).at(typeRight).ConvertionType = convertionType;

		m_ConvertorMap.at(typeRight).at(typeLeft).Convertion =
		[this, typeLeft, typeRight, constConvertionCallback](TExpression* left, TExpression* right) {
			return TConvertionResult{ new TExpression(*left),
				SubmitConversion(right, typeRight, typeLeft, constConvertionCallback, false) };
		}; 
		m_ConvertorMap.at(typeRight).at(typeLeft).DataType = typeRight;
		m_ConvertorMap.at(typeRight).at(typeLeft).ConvertionType = convertionType;
		break;
	}
	}
}

TConvertionResult TypeConvertionManager::InvokeConversion(TExpression* left, TExpression* right)
{
	const TDataTypeProperties* leftTypeProps = m_ParserRef->GetDataTypeProperties(*left);
	const TDataTypeProperties* rightTypeProps = m_ParserRef->GetDataTypeProperties(*right);

	if (*leftTypeProps == *rightTypeProps) return { new TExpression(*left), new TExpression(*right) };

	auto& conversionData = m_ConvertorMap.at(leftTypeProps->Type).at(rightTypeProps->Type);
	return conversionData.Convertion(left, right);
}

bool TypeConvertionManager::CanConvert(TExpression* left, TExpression* right)
{
	const TDataTypeProperties* leftTypeProps = m_ParserRef->GetDataTypeProperties(*left);
	const TDataTypeProperties* rightTypeProps = m_ParserRef->GetDataTypeProperties(*right);
	return CanConvert(*leftTypeProps, *rightTypeProps);
}

bool TypeConvertionManager::CanConvert(const TDataTypeProperties& left, const TDataTypeProperties& right)
{
	return CanConvert(left.Type, right.Type);
}

bool TypeConvertionManager::CanConvert(const EDataType& left, const EDataType& right)
{
	return m_ConvertorMap.at(left).at(right).ConvertionType != EConvertionType::Error;
}

TExpression* TypeConvertionManager::SubmitConversion(TExpression* expression,
	const EDataType& srcType, const EDataType& dstType, const ConstConvertionCallback& constConvertionCallback,
	bool isOpposite)
{
	if (expression == nullptr) return nullptr;

	TExpression* convertedExpression = new TExpression();
	auto& convertionData = m_ConvertorMap.at(srcType).at(dstType);
	switch (expression->Type)
	{
	case EExpressionType::Lvalue:
	{
		const std::string tempID = m_ParserRef->CreateTempVariable(ELvalueType::Modifiable,
			TDataTypeProperties{convertionData.DataType, 0}, m_ParserRef->m_Scope, m_ParserRef->GetLexer()->GetLineCount());
		m_ParserRef->m_SymbolTable.LookUp(tempID, m_ParserRef->m_Scope);
		convertedExpression->Type = EExpressionType::Lvalue;
		convertedExpression->SymbolTableEntry = tempID;
		break;
	}
	case EExpressionType::Constant:
	{
		convertedExpression->Type = EExpressionType::Constant;
		convertedExpression->ConstantValue = expression->ConstantValue;
		constConvertionCallback(convertedExpression->ConstantValue, isOpposite);
		convertedExpression->ConstantValue.DataTypeProps.Type = convertionData.DataType;
		convertedExpression->ConstantValue.DataTypeProps.PointerDepth = 0;
		break;
	}
	}
	TExpression* convertedTypeExpression = new TExpression();
	convertedTypeExpression->Type = EExpressionType::Type;
	convertedTypeExpression->ConstantValue.DataTypeProps.Type = convertionData.DataType;
	convertedTypeExpression->ConstantValue.DataTypeProps.PointerDepth = 0;

	m_ParserRef->m_QuadGenerator.AddInstruction(OperationCode_Cast, new TExpression(*expression), convertedTypeExpression,
		convertedExpression, m_ParserRef->GetLexer()->GetLineCount());
	return convertedExpression;
}
