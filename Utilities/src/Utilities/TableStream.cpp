#include "TableStream.h"

#include <iostream>

namespace Utilities
{
	/////STATIC///////////////////////////////////////////////////////////////////////////////////////////////////////

	static const char s_HorizontalBorderCharacter = '-';
	static const char s_VerticalBorderCharacter = '|';

	/////PUBLIC///////////////////////////////////////////////////////////////////////////////////////////////////////

	TableStream::TableStream(const std::string& outFilePath, int32_t flags)
		: m_OutStream(outFilePath), m_Flags(flags)
	{
		if (m_OutStream.good())
		{
			std::cout << "Good" << std::endl;
		}
	}

	TableStream::TableStream(std::ofstream& outStream, int32_t flags)
		: m_Flags(flags)
	{
		m_OutStream = std::move(outStream);
	}

	TableStream::TableStream(FILE* file, int32_t flags)
		: m_OutStream(file), m_Flags(flags)
	{

	}

	bool TableStream::Export()
	{
		size_t totalRows = m_GetTotalRows();
		size_t totalColumns = m_GetTotalColumns();
		m_MaxWidthPerColumn.resize(totalColumns);
		size_t maxColumnWidth = 0;

		if (m_Flags & TableStreamFlags_ColumnsSameWidth)
		{
			CalculateMaxColumnWidth();
			maxColumnWidth = m_MaxColumnWidth;
		}

		if (m_Flags & TableStreamFlags_RowsLabel)
		{
			CalculateMaxRowWidth();
		}

		size_t tableWidth = CalculateTableWidth(m_MaxRowWidth, maxColumnWidth) + 1;
		for (int32_t i = 0; i < tableWidth; i++)
		{
			m_OutStream << s_HorizontalBorderCharacter;
		}
		m_OutStream << '\n';

		m_OutStream << s_VerticalBorderCharacter << s_VerticalBorderCharacter;
		if (m_Flags & TableStreamFlags_RowsLabel && m_Flags & TableStreamFlags_ColumnsLabel)
		{
			Space(m_MaxRowWidth);
			m_OutStream << s_VerticalBorderCharacter << s_VerticalBorderCharacter;
		}

		// TODO: Print column labels
		if (m_Flags & TableStreamFlags_ColumnsLabel)
		{
			for (size_t column = 0; column < totalColumns; column++)
			{
				PrintColumnElement(column, m_GetColumnLabel(column), maxColumnWidth);
			}
			m_OutStream << s_VerticalBorderCharacter << '\n';
			for (int32_t i = 0; i < tableWidth; i++)
			{
				m_OutStream << s_HorizontalBorderCharacter;
			}
			m_OutStream << '\n' << s_VerticalBorderCharacter << s_VerticalBorderCharacter;
		}

		for (size_t row = 0; row < totalRows; row++)
		{
			if (!m_GetElementString) break;
			// TODO: Print row labels
			if (m_Flags & TableStreamFlags_RowsLabel)
			{
				const std::string& rowElement = m_GetRowLabel(row);
				int32_t spaceLeft = m_MaxRowWidth - rowElement.length();
				int32_t leftSpaces = spaceLeft / 2;
				int32_t rightSpaces = spaceLeft - leftSpaces;
				switch (m_HorizontalAlignment)
				{
				case HorizontalAlignment::Left:
				{
					m_OutStream << rowElement;
					Space(spaceLeft);
					break;
				}
				case HorizontalAlignment::Center:
				{
					Space(leftSpaces);
					m_OutStream << rowElement;
					Space(rightSpaces);
					break;
				}
				case HorizontalAlignment::Right:
				{
					Space(spaceLeft);
					m_OutStream << rowElement;
					break;
				}
				}
				m_OutStream << s_VerticalBorderCharacter << s_VerticalBorderCharacter;
			}
			for (size_t column = 0; column < totalColumns; column++)
			{
				const std::string& elementString = m_GetElementString(row, column);
				PrintColumnElement(column, elementString, maxColumnWidth);
			}
			m_OutStream << s_VerticalBorderCharacter;
			if (row < totalRows - 1)
				m_OutStream << '\n' << s_VerticalBorderCharacter << s_VerticalBorderCharacter;
		}

		m_OutStream << '\n';
		for (int32_t i = 0; i < tableWidth; i++)
		{
			m_OutStream << s_HorizontalBorderCharacter;
		}

		return true;
	}

	void TableStream::BindGetRowLabelCallback(const std::function<const std::string& (size_t row)>& callback)
	{
		m_GetRowLabel = callback;
	}

	void TableStream::BindGetColumnLabelCallback(const std::function<const std::string& (size_t col)>& callback)
	{
		m_GetColumnLabel = callback;
	}

	void TableStream::BindGetElementStringCallback(const std::function<const std::string& (size_t row, size_t col)>&
		callback)
	{
		m_GetElementString = callback;
	}

	void TableStream::BindGetTotalRowsCallback(const std::function<size_t()>& callback)
	{
		m_GetTotalRows = callback;
	}

	void TableStream::BindGetTotalColumnsCallback(const std::function<size_t()>& callback)
	{
		m_GetTotalColumns = callback;
	}

	void TableStream::BindGetTableSizeCallback(const std::function<size_t()>& callback)
	{
		m_GetTableSize = callback;
	}

	/////PRIVATE//////////////////////////////////////////////////////////////////////////////////////////////////////

	void TableStream::CalculateMaxRowWidth()
	{
		m_MaxRowWidth = 0;
		size_t totalRows = m_GetTotalRows();
		for (size_t row = 0; row < totalRows; row++)
		{
			if (!m_GetRowLabel) break;
			m_MaxRowWidth = std::max(m_MaxRowWidth, m_GetRowLabel(row).length());
		}
		m_MaxRowWidth += m_RowSpacing;
	}

	void TableStream::CalculateMaxColumnWidth()
	{
		m_MaxColumnWidth = 0;
		size_t totalRows = m_GetTotalRows();
		size_t totalColumns = m_GetTotalColumns();
		for (size_t column = 0; column < totalColumns; column++)
		{
			if (!m_GetColumnLabel) break;
			m_MaxColumnWidth = std::max(m_MaxColumnWidth, m_GetColumnLabel(column).length());
		}

		for (size_t row = 0; row < totalRows; row++)
		{
			if (!m_GetElementString) break;
			for (size_t column = 0; column < totalColumns; column++)
			{
				m_MaxColumnWidth = std::max(m_MaxColumnWidth, m_GetElementString(row, column).length());
			}
		}
		m_MaxColumnWidth += m_ColumnSpacing;
	}

	void TableStream::CalculateMaxColumnWidth(size_t column)
	{
		m_MaxColumnWidth = 0;
		size_t totalRows = m_GetTotalRows();
		if (m_GetColumnLabel)
			m_MaxColumnWidth = std::max(m_MaxColumnWidth, m_GetColumnLabel(column).length());

		for (size_t row = 0; row < totalRows; row++)
		{
			if (!m_GetElementString) break;
			m_MaxColumnWidth = std::max(m_MaxColumnWidth, m_GetElementString(row, column).length());
		}
		m_MaxColumnWidth += m_ColumnSpacing;
	}

	void TableStream::Space(size_t times)
	{
		for (size_t i = 0; i < times; i++)
		{
			m_OutStream << ' ';
		}
	}

	size_t TableStream::PrintColumnElement(size_t column, const std::string& element, size_t maxColumnWidth)
	{
		size_t characterCount = 0;
		if (m_Flags & TableStreamFlags_ColumnsSameWidth)
			m_MaxColumnWidth = maxColumnWidth;
		else
			CalculateMaxColumnWidth(column);
		size_t columnWidth = m_MaxWidthPerColumn.at(column) = m_MaxColumnWidth;

		size_t labelLength = element.length();
		size_t spaceLeft = columnWidth - labelLength;
		size_t leftSpaces = spaceLeft / 2;
		size_t rightSpaces = spaceLeft - leftSpaces;
		switch (m_HorizontalAlignment)
		{
		case HorizontalAlignment::Left:
		{
			m_OutStream << element;
			Space(spaceLeft);
			break;
		}
		case HorizontalAlignment::Center:
		{
			Space(leftSpaces);
			m_OutStream << element;
			Space(rightSpaces);
			break;
		}
		case HorizontalAlignment::Right:
		{
			Space(spaceLeft);
			m_OutStream << element;
			break;
		}
		}
		m_OutStream << s_VerticalBorderCharacter;
		return element.length() + spaceLeft + 1;
	}

	size_t TableStream::CalculateTableWidth(size_t maxRowWidth, size_t maxColumnWidth)
	{
		size_t counter = 0U;
		size_t totalColumns = m_GetTotalColumns();

		if (m_Flags & TableStreamFlags_RowsLabel && m_Flags & TableStreamFlags_ColumnsLabel)
		{
			counter += maxRowWidth + 2;
		}

		for (size_t column = 0; column < totalColumns; column++)
		{
			if (m_Flags & TableStreamFlags_ColumnsSameWidth)
				m_MaxColumnWidth = maxColumnWidth;
			else
				CalculateMaxColumnWidth(column);
			counter += m_MaxColumnWidth + 1;
		}

		return counter + 2;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}