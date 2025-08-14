#include "TableStream.h"

#include <iostream>

namespace Utilities
{
	/////PUBLIC///////////////////////////////////////////////////////////////////////////////////////////////////////

	TableStream::TableStream(const std::string& outFilePath, int32_t streamMode, int32_t flags)
		: m_OutStream(outFilePath, streamMode), m_Flags(flags)
	{

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

		if (!m_Label.empty())
		{
			int32_t spaceLeft = tableWidth - GetStringLineLength(m_Label);
			int32_t leftSpaces = spaceLeft / 2;
			int32_t rightSpaces = spaceLeft - leftSpaces;
			switch (m_LabelHorizontalAlignment)
			{
			case HorizontalAlignment::Left:
			{
				m_OutStream << m_Label;
				Space(spaceLeft);
				break;
			}
			case HorizontalAlignment::Center:
			{
				Space(leftSpaces);
				m_OutStream << m_Label;
				Space(rightSpaces);
				break;
			}
			case HorizontalAlignment::Right:
			{
				Space(spaceLeft);
				m_OutStream << m_Label;
				break;
			}
			}
			m_OutStream << '\n';
		}

		for (int32_t i = 0; i < tableWidth; i++)
		{
			m_OutStream << m_HorizontalBorderSymbol;
		}
		m_OutStream << '\n';

		m_OutStream << m_VerticalBorderSymbol1 << m_VerticalBorderSymbol2;
		if (m_Flags & TableStreamFlags_RowsLabel && m_Flags & TableStreamFlags_ColumnsLabel)
		{
			Space(m_MaxRowWidth);
			m_OutStream << m_VerticalBorderSymbol1 << m_VerticalBorderSymbol2;
		}

		// TODO: Print column labels
		if (m_Flags & TableStreamFlags_ColumnsLabel)
		{
			for (size_t column = 0; column < totalColumns; column++)
			{
				PrintColumnElement(column, m_GetColumnLabel(column), maxColumnWidth);
			}
			m_OutStream << m_VerticalBorderSymbol2 << '\n';
			for (int32_t i = 0; i < tableWidth; i++)
			{
				m_OutStream << m_HorizontalBorderSymbol;
			}
			m_OutStream << '\n' << m_VerticalBorderSymbol1 << m_VerticalBorderSymbol2;
		}

		for (size_t row = 0; row < totalRows; row++)
		{
			if (!m_GetElementString) break;
			
			CalculateMaxRowHeight(row);
			for (size_t line = 0; line < m_MaxRowHeight; line++)
			{
				if (m_Flags & TableStreamFlags_RowsLabel)
				{
					const std::string& rowElement = GetStringToDraw(m_GetRowLabel(row), line);

					int32_t spaceLeft = m_MaxRowWidth - GetStringLineLength(rowElement);
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
					m_OutStream << m_VerticalBorderSymbol1 << m_VerticalBorderSymbol2;
				}
				for (size_t column = 0; column < totalColumns; column++)
				{
					const std::string& elementString = GetStringToDraw(m_GetElementString(row, column), line);
					PrintColumnElement(column, elementString, maxColumnWidth);
				}
				m_OutStream << m_VerticalBorderSymbol2;
				if (line < m_MaxRowHeight - 1 || row < totalRows - 1)
					m_OutStream << '\n';
				if (line < m_MaxRowHeight - 1)
					m_OutStream << m_VerticalBorderSymbol1 << m_VerticalBorderSymbol2;
			}
			if (row < totalRows - 1)
			{
				if (m_Flags & TableStreamFlags_RowSeperator)
				{
					m_OutStream << m_VerticalBorderSymbol1 << m_VerticalBorderSymbol2;
					for (int32_t i = 0; i < m_MaxRowWidth; i++)
					{
						m_OutStream << m_HorizontalBorderSymbol;
					}
					m_OutStream << m_VerticalBorderSymbol1 << m_VerticalBorderSymbol2;
					for (int32_t col = 0; col < totalColumns; col++)
					{
						if (m_Flags & TableStreamFlags_ColumnsSameWidth)
							m_MaxColumnWidth = maxColumnWidth;
						else
							CalculateMaxColumnWidth(col);
						for (int32_t i = 0; i < m_MaxColumnWidth; i++)
						{
							m_OutStream << m_HorizontalBorderSymbol;
						}
						m_OutStream << m_VerticalBorderSymbol1;
					}

					m_OutStream << m_VerticalBorderSymbol2;
					m_OutStream << '\n';
				}
				m_OutStream << m_VerticalBorderSymbol1 << m_VerticalBorderSymbol2;
			}
		}

		m_OutStream << '\n';
		for (int32_t i = 0; i < tableWidth; i++)
		{
			m_OutStream << m_HorizontalBorderSymbol;
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
			m_MaxRowWidth = std::max(m_MaxRowWidth, GetStringLineLength(m_GetRowLabel(row)));
		}
		m_MaxRowWidth += m_RowHorizontalSpacing;
	}

	void TableStream::CalculateMaxRowHeight(size_t row)
	{
		size_t totalRows = m_GetTotalRows();
		size_t totalColumns = m_GetTotalColumns();
		m_MaxRowHeight = 0;
		for (size_t row = 0; row < totalRows; row++)
		{
			if (!m_GetRowLabel) break;
			size_t labelHeight = CalculateStringHeight(m_GetRowLabel(row));
			m_MaxRowHeight = std::max(m_MaxRowHeight, labelHeight);
		}

		for (size_t column = 0; column < totalColumns; column++)
		{
			if (!m_GetElementString) break;
			size_t elementHeight = CalculateStringHeight(m_GetElementString(row, column));
			m_MaxRowHeight = std::max(m_MaxRowHeight, elementHeight);
		}
		m_MaxRowHeight += m_RowVerticalSpacing;
	}

	void TableStream::CalculateMaxColumnWidth()
	{
		m_MaxColumnWidth = 0;
		size_t totalRows = m_GetTotalRows();
		size_t totalColumns = m_GetTotalColumns();
		for (size_t column = 0; column < totalColumns; column++)
		{
			if (!m_GetColumnLabel) break;
			m_MaxColumnWidth = std::max(m_MaxColumnWidth, GetStringLineLength(m_GetColumnLabel(column)));
		}

		for (size_t row = 0; row < totalRows; row++)
		{
			if (!m_GetElementString) break;
			for (size_t column = 0; column < totalColumns; column++)
			{
				m_MaxColumnWidth = std::max(m_MaxColumnWidth, GetStringLineLength(m_GetElementString(row, column)));
			}
		}
		m_MaxColumnWidth += m_ColumnHorizontalSpacing;
	}

	void TableStream::CalculateMaxColumnWidth(size_t column)
	{
		m_MaxColumnWidth = 0;
		size_t totalRows = m_GetTotalRows();
		if (m_GetColumnLabel)
			m_MaxColumnWidth = std::max(m_MaxColumnWidth, GetStringLineLength(m_GetColumnLabel(column)));

		for (size_t row = 0; row < totalRows; row++)
		{
			if (!m_GetElementString) break;
			m_MaxColumnWidth = std::max(m_MaxColumnWidth, GetStringLineLength(m_GetElementString(row, column)));
		}
		m_MaxColumnWidth += m_ColumnHorizontalSpacing;
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

		size_t labelLength = GetStringLineLength(element);
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
		m_OutStream << m_VerticalBorderSymbol1;
		return labelLength + spaceLeft + 1;
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

	size_t TableStream::CalculateStringHeight(const std::string& string)
	{
		size_t height = 1;
		for (const char character : string)
		{
			if (character == '\n')
				height++;
		}
		return height;
	}

	void TableStream::QueryStringLineByIndex(const std::string& string, size_t index, size_t& offset, size_t& count)
	{
		offset = 0;

		if (index == 0)
		{
			offset = string.find_first_of('\n', offset);
			if (offset == std::string::npos)
			{
				count = string.length();
			}
			else
			{
				count = offset;
			}
			offset = 0;
			return;
		}

		for (size_t i = 0; i < index; i++)
		{
			offset = string.find_first_of('\n', offset) + 1;
		}

		if (offset == std::string::npos)
		{
			offset = count = -1;
			return;
		}

		size_t lastNewLine = string.find_first_of('\n', offset);
		if (lastNewLine == std::string::npos)
		{
			count = string.length() - offset + 1;
			return;
		}

		count = lastNewLine - offset;
	}

	std::string TableStream::GetStringToDraw(const std::string& string, size_t line)
	{
		std::string resultStr;
		size_t stringHeight = CalculateStringHeight(string);
		size_t spaceLeft = m_MaxRowHeight - stringHeight;
		size_t upSpaces = spaceLeft / 2;
		size_t bottomSpaces = spaceLeft - upSpaces;
		switch (m_VericalAlignment)
		{
		case VericalAlignment::Up:
		{
			if (line < stringHeight)
			{
				size_t offset, count;
				QueryStringLineByIndex(string, line, offset, count);
				return string.substr(offset, count);
			}
			break;
		}
		case VericalAlignment::Center:
		{
			if (line >= upSpaces && line - upSpaces < stringHeight)
			{
				size_t offset, count;
				QueryStringLineByIndex(string, line - upSpaces, offset, count);
				return string.substr(offset, count);
			}
			break;
		}
		case VericalAlignment::Bottom:
		{
			if (line >= spaceLeft && line - spaceLeft < stringHeight)
			{
				size_t offset, count;
				QueryStringLineByIndex(string, line - spaceLeft, offset, count);
				return string.substr(offset, count);
			}
			break;
		}
		}
		return std::string();
	}

	size_t TableStream::GetStringLineLength(const std::string& string)
	{
		size_t newLineIndex = string.find_first_of('\n');
		if (newLineIndex == std::string::npos) return string.length();
		return newLineIndex;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}