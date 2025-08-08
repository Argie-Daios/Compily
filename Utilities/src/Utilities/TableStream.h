#pragma once

#include <fstream>
#include <functional>
#include <string>

#include "Macros.h"

namespace Utilities
{
	enum TableStreamFlags
	{
		TableStreamFlags_None = 0,
		TableStreamFlags_ColumnsSameWidth = BIT(0),
		TableStreamFlags_RowsLabel = BIT(1),
		TableStreamFlags_ColumnsLabel = BIT(2)
	};

	enum class HorizontalAlignment
	{
		Left,
		Center,
		Right
	};
	
	class TableStream
	{
	public:
		TableStream(const std::string& outFilePath, int32_t streamMode = std::ios::out, int32_t flags = TableStreamFlags_None);
		TableStream(std::ofstream& outStream, int32_t flags = TableStreamFlags_None);
		TableStream(FILE* file, int32_t flags = TableStreamFlags_None);

		bool Export();

		inline void SetLabel(const std::string& label) { m_Label = label; }
		inline void SetLabelHorizontalAlignment(const HorizontalAlignment& alignment)
			{ m_LabelHorizontalAlignment = alignment; }
		inline void SetHorizontalAlignment(const HorizontalAlignment& alignment) { m_HorizontalAlignment = alignment; }
		inline void SetRowSpacing(int32_t spacing) { m_RowSpacing = spacing; }
		inline void SetColumnSpacing(int32_t spacing) { m_ColumnSpacing = spacing; }

		void BindGetRowLabelCallback(const std::function<const std::string& (size_t row)>& callback);
		void BindGetColumnLabelCallback(const std::function<const std::string& (size_t col)>& callback);
		void BindGetElementStringCallback(const std::function<const std::string& (size_t row, size_t col)>& callback);
		void BindGetTotalRowsCallback(const std::function<size_t()>& callback);
		void BindGetTotalColumnsCallback(const std::function<size_t()>& callback);
		void BindGetTableSizeCallback(const std::function<size_t()>& callback);
	private:
		void CalculateMaxRowWidth();
		void CalculateMaxColumnWidth();
		void CalculateMaxColumnWidth(size_t column);
		void Space(size_t times);
		size_t PrintColumnElement(size_t column, const std::string& element, size_t maxColumnWidth);
		size_t CalculateTableWidth(size_t maxRowWidth, size_t maxColumnWidth);
	private:
		std::ofstream m_OutStream;
		int32_t m_Flags;

		std::string m_Label;
		HorizontalAlignment m_LabelHorizontalAlignment = HorizontalAlignment::Left;

		HorizontalAlignment m_HorizontalAlignment = HorizontalAlignment::Left;
		size_t m_RowSpacing = 0;
		size_t m_ColumnSpacing = 0;

		size_t m_MaxRowWidth = 0;
		size_t m_MaxColumnWidth = 0;
		std::vector<int32_t> m_MaxWidthPerColumn;

		std::function<const std::string& (size_t row)> m_GetRowLabel;
		std::function<const std::string& (size_t col)> m_GetColumnLabel;
		std::function<const std::string& (size_t row, size_t col)> m_GetElementString;
		std::function<size_t()> m_GetTotalRows;
		std::function<size_t()> m_GetTotalColumns;
		std::function<size_t()> m_GetTableSize;
	};
}