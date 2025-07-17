#pragma once

#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>

namespace Utilities
{
	template<typename VertexDataType, typename EdgeDataType>
	class Graph
	{
	public:
		struct Edge;
		struct Vertex;
	public:
		Graph()
		{
			s_ConstructCount++;
		}

		Graph(const Graph& graph)
			: m_Vertices(graph.m_Vertices)
		{
			s_CopyCount++;
		}

		Graph(Graph&& graph)
			: m_Vertices(std::move(graph.m_Vertices))
		{
			s_CopyCount++;
		}

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
		inline Vertex& GetVertex(int32_t id) { return m_Vertices.at(id); }

		std::vector<Edge>& GetEdgesOfVertex(const int32_t& source)
		{
			auto it = m_Vertices.find(source);

			return it->second.Edges;
		}

		size_t GetTotalVertices() const { return m_Vertices.size(); }
		bool IsEmpty() const { return m_Vertices.empty(); }

		Graph<typename VertexDataType, typename EdgeDataType>& operator|=(
				Graph<typename VertexDataType, typename EdgeDataType>& graph)
		{
			if (this == &graph) return *this;

			size_t size = m_Vertices.size();
			for (int32_t i = 0U; i < graph.m_Vertices.size(); i++)
			{
				Vertex& vertex = graph.m_Vertices.at(i);
				int32_t id = (int32_t)m_Vertices.size();
				m_Vertices.emplace(id, std::move(vertex));

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

			Edge()
			{
				s_EdgeConstructCount++;
			}

			Edge(const int32_t& source, const int32_t& destination,
				const EdgeDataType& data)
				: Source(source), Destination(destination), Data(data)
			{
				s_EdgeConstructCount++;
			}

			Edge(int32_t&& source, int32_t&& destination, EdgeDataType&& data) noexcept
			{
				Source = std::move(source);
				Destination = std::move(destination);
				Data = std::move(data);
				s_EdgeConstructCount++;
			}

			Edge(const Edge& edge)
				: Source(edge.Source), Destination(edge.Destination), Data(edge.Data)
			{
				s_EdgeCopyCount++;
			}

			Edge(Edge&& edge) noexcept
				: Source(std::move(edge.Source)),
				Destination(std::move(edge.Destination)),
				Data(std::move(edge.Data))
			{
				s_EdgeMoveCount++;
			}
		};

		struct Vertex
		{
			VertexDataType Data;

			Vertex()
			{
				s_VertexConstructCount++;
			}

			Vertex(const VertexDataType& data)
				: Data(data)
			{
				s_VertexConstructCount++;
			}

			Vertex(VertexDataType&& data) noexcept
			{
				Data = std::move(data);
				s_VertexConstructCount++;
			}

			Vertex(const Vertex& vertex)
				: Data(vertex.Data), Edges(vertex.Edges)
			{
				s_VertexCopyCount++;
			}

			Vertex(Vertex&& vertex) noexcept
				: Data(std::move(vertex.Data)),
				Edges(std::move(vertex.Edges))
			{
				s_VertexMoveCount++;
			}
		private:
			std::vector<Edge> Edges;

			friend class Graph<VertexDataType, EdgeDataType>;
		};
	private:
		std::unordered_map<int32_t, Vertex> m_Vertices;
	public:
		inline static uint32_t s_ConstructCount = 0U;
		inline static uint32_t s_CopyCount = 0U;
		inline static uint32_t s_VertexConstructCount = 0U;
		inline static uint32_t s_VertexCopyCount = 0U;
		inline static uint32_t s_VertexMoveCount = 0U;
		inline static uint32_t s_EdgeConstructCount = 0U;
		inline static uint32_t s_EdgeCopyCount = 0U;
		inline static uint32_t s_EdgeMoveCount = 0U;
	};
}