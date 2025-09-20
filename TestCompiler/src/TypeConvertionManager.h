#pragma once

#include "RuleTypes/Expression.h"
#include <unordered_map>
#include <functional>
#include <initializer_list>

enum class EConvertionType
{
	Error,
	Left,
	Right,
	Order
};

struct TConvertionResult
{
	TExpression* Left;
	TExpression* Right;
};

using ConvertionCallback = std::function<TConvertionResult(TExpression*, TExpression*)>;
using ConstConvertionCallback = std::function<void(TConstant&, bool)>;
class TypeConvertionManager
{
public:
	TypeConvertionManager(class TestParser* parserRef);

	void RegisterType(const EDataType& type);
	void RegisterConversion(const EDataType& typeLeft, const EDataType& typeRight,
		const EConvertionType& convertionType, const ConstConvertionCallback& constConvertionCallback);
	TConvertionResult InvokeConversion(TExpression* left, TExpression* right);
	bool CanConvert(TExpression* left, TExpression* right);
	bool CanConvert(const TDataTypeProperties& left, const TDataTypeProperties& right);
	bool CanConvert(const EDataType& left, const EDataType& right);
private:
	struct ConvertorData
	{
		ConvertionCallback Convertion;
		EDataType DataType;
		EConvertionType ConvertionType;

		ConvertorData() = default;
		ConvertorData(const ConvertionCallback& callback, const EDataType& dataType, const EConvertionType& type)
			: Convertion(callback), DataType(dataType), ConvertionType(type)
		{

		}
	};
private:
	TExpression* SubmitConversion(TExpression* expression, const EDataType& srcType, const EDataType& dstType,
		const ConstConvertionCallback& constConvertionCallback, bool isOpposite);
private:
	std::unordered_map<EDataType, std::unordered_map<EDataType, ConvertorData>> m_ConvertorMap;
	class TestParser* m_ParserRef = nullptr;
};