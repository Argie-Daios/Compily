#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>

namespace Lexy
{
	template<typename VertexDataType, typename EdgeDataType>
	class Graph
	{
	public:
		struct Edge;
		struct Vertex;
	public:
		Graph() = default;

		int32_t PushVertex(const VertexDataType& data)
		{
			int32_t id = (int32_t)m_Vertices.size();
			m_Vertices.emplace(id, data);
			return id;
		}

		void PushEdge(const int32_t& source, const int32_t& destination,
			const EdgeDataType& edge)
		{
			auto sourceIt = m_Vertices.find(source);
			if (sourceIt == m_Vertices.end()) return;

			auto destinationIt = m_Vertices.find(destination);
			if (destinationIt == m_Vertices.end()) return;

			sourceIt->second.Edges.emplace_back(source, destination, edge);
		}

		inline std::unordered_map<int32_t, Vertex>& GetVertices() { return m_Vertices; }

		std::vector<Edge>& GetEdgesOfVertex(const int32_t& source)
		{
			auto it = m_Vertices.find(source);

			return it->second.Edges;
		}

		size_t GetTotalVertices() const { return m_Vertices.size(); }
		bool IsEmpty() const { return m_Vertices.empty(); }

		Graph<typename VertexDataType, typename EdgeDataType>& operator|=(const
				Graph<typename VertexDataType, typename EdgeDataType>& graph)
		{
			if (this == &graph) return *this;

			size_t size = m_Vertices.size();
			for (int32_t i = 0U; i < graph.m_Vertices.size(); i++)
			{
				const Vertex& vertex = graph.m_Vertices.at(i);
				int32_t id = (int32_t)m_Vertices.size();
				m_Vertices.emplace(id, vertex);

				std::vector<Edge>& edges = m_Vertices.at(id).Edges;
				for (Edge& edge : edges)
				{
					edge.Source += (int32_t)size;
					edge.Destination += (int32_t)size;
				}
			}

			return *this;
		}

	public:
		struct Edge
		{
			int32_t Source;
			int32_t Destination;
			EdgeDataType Data;

			Edge(const int32_t& source, const int32_t& destination,
				const EdgeDataType& data)
				: Source(source), Destination(destination), Data(data)
			{
				
			}

			Edge(int32_t&& source, int32_t&& destination, EdgeDataType&& data)
			{
				Source = std::move(source);
				Destination = std::move(destination);
				Data = std::move(data);
			}
		};

		struct Vertex
		{
			VertexDataType Data;

			Vertex( const VertexDataType& data)
				: Data(data)
			{

			}

			Vertex(VertexDataType&& data)
			{
				Data = std::move(data);
			}
		private:
			std::vector<Edge> Edges;

			friend class Graph<VertexDataType, EdgeDataType>;
		};
	private:
		std::unordered_map<int32_t, Vertex> m_Vertices;
	};
}