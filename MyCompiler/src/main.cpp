#include <iostream>

#include "TokenType.h"
#include "MyParser.h"

#include <Utilities.h>

int main()
{
	std::ifstream stream("SourceCode.src");
	//MyLexer lexer(stream);

	MyParser parser(stream);

	/*std::vector<std::string> labels = { "State", "Stack", "Rofl" };
	std::vector<std::string> elements = { 
		"0\nputas\nxd\nrofl", "#\n#\n#", "a" ,
		"01\n", "##\n", "ab" ,
		"012\n", "###\n", "abc" ,
		"0123\n", "####\n", "abcd" ,
		"01234\n", "#####\n", "abcde"
	};

	Utilities::TableStream tableStream("Test.txt", std::ios::out, Utilities::TableStreamFlags_ColumnsLabel |
		Utilities::TableStreamFlags_ColumnsSameWidth | Utilities::TableStreamFlags_RowsLabel |
		Utilities::TableStreamFlags_RowSeperator);

	tableStream.BindGetTotalRowsCallback([&]() {return elements.size() / labels.size(); });
	tableStream.BindGetTotalColumnsCallback([&]() {return labels.size(); });
	tableStream.BindGetRowLabelCallback([&](size_t row) -> const std::string& {
		static std::string helper;
		helper.clear();
		helper = std::to_string(row);
		return helper;
	});
	tableStream.BindGetColumnLabelCallback([&](size_t col) -> const std::string& { return labels.at(col); });
	tableStream.BindGetElementStringCallback([&](size_t row, size_t col) -> const std::string& {
		size_t index = labels.size() * row + col;
		return elements.at(index);
	});

	tableStream.SetLabel("Test");
	tableStream.SetLabelHorizontalAlignment(Utilities::HorizontalAlignment::Center);
	tableStream.SetHorizontalAlignment(Utilities::HorizontalAlignment::Center);
	tableStream.SetVerticalAlignment(Utilities::VericalAlignment::Bottom);
	tableStream.SetRowHorizontalSpacing(4);
	tableStream.SetRowVerticalSpacing(4);
	tableStream.SetColumnHorizontalSpacing(4);

	tableStream.Export();*/

	return 0;
}