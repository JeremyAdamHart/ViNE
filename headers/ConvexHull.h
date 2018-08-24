#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include <memory>

//Container which reserves space in middle
template<typename T>
class SlotMap {


	std::vector<T> data;
	std::vector<int> timestamp;
	std::vector<int> emptySlots;
	size_t dataSize;

public:
	SlotMap() :dataSize(0) {}

	struct Index {
		int index;
		int timestamp;

		Index() :index(-1), timestamp(0) {}
		Index(int index) :index(index), timestamp(0) {}
		Index(int index, int timestamp) :index(index), timestamp(timestamp) {}
		operator int() { return index; }
		operator size_t() { return index; }
		operator bool() { return index >= 0; }
		bool operator==(Index other) { return other.index == index && other.timestamp == timestamp; }
		bool operator<(Index other) { return index < other.index; }
		bool operator>(Index other) { return index > other.index; }
		bool operator<=(Index other) { return index <= other.index; }
		bool operator>=(Index other) { return index >= other.index; }
	};

	Index add(T value) {
		dataSize++;
		if (emptySlots.empty()) {
			data.push_back(value);
			timestamp.push_back(0);
			return Index(data.size()-1);
		}
		else {
			int index = emptySlots.back();
			data[index] = value;
			timestamp[index]++;
			emptySlots.pop_back();
			return Index(index, timestamp[index]);
		}
	}

	void remove(Index i) {
		emptySlots.push_back(i);
		timestamp[i]++;
		dataSize = (dataSize == 0) ? dataSize : dataSize-1;
	}

	T& operator[](Index i) {
		return data[i];
	}

	/*const T& operator[] const (Index i) {
		return data[i];
	}*/

	int size() { return dataSize; }

	//Iterator
	class Iterator {
		int index;
		SlotMap<T>* container;
		
		friend SlotMap<T>;

		Iterator(int index, SlotMap<T>* container) :index(index), container(container) {}
	public:
		Iterator& operator++() {
			do {
				index++;
			} while (index < container->data.size() && (container->timestamp[index] % 2) == 1);

			return (index < container->data.size()) ? *this : container->end();
		}

		Iterator& operator--() {
			do {
				index--;
			} while (index > 0 && (container->timestamp[index] % 2) == 1);

			return (index > 0) ? *this : container->end();
		}

		T& operator*() { return container->data[index]; }
		T* operator->() { return &container->data[index]; }

		bool operator==(const Iterator& it) const { return index == it.index && container == it.container; }
		bool operator!=(const Iterator& it) const { return !(*this == it); }
		bool operator<(const Iterator& it) const { return index < it.index; }
		bool operator>(const Iterator& it) const { return index > it.index; }
		bool operator<=(const Iterator& it) const { return index <= it.index; }
		bool operator>=(const Iterator& it) const { return index >= it.index; }

		//T operator
	};

	Iterator begin() {
		if (data.size() == 0) return Iterator(0, this);

		int i = 0;
		while (i < data.size() && (timestamp[i] % 2) == 1) {
			i++;
		}
		return (i < data.size()) ? Iterator(i, this)  : Iterator(0, this);
	}

	Iterator end() {
		if (data.size() == 0) return Iterator(0, this);

		int i = data.size() - 1;
		while (i > 0 && (timestamp[i] % 2) == 1) {
			i--;
		}
		return {i+1, this };
	}
};


template<typename P> class Face;
template<typename P> class Vertex;

template<typename P>
struct HalfEdge {
	typename SlotMap<Vertex<P>>::Index head;
	typename SlotMap<HalfEdge<P>>::Index next;
	typename SlotMap<HalfEdge<P>>::Index pair;
	typename SlotMap<Face<P>>::Index face;
};

template<typename P>
struct Vertex {
	P pos;
	typename SlotMap<HalfEdge<P>>::Index edge;
};

template<typename P>
struct Face {
	typename SlotMap<HalfEdge<P>>::Index edge;
};


template<typename P>
class HalfEdgeMesh {	
public:
	SlotMap<Vertex<P>> vertices;
	SlotMap<Face<P>> faces;
	SlotMap<HalfEdge<P>> edges;

	//Accessor methods
	HalfEdge<P>& next(HalfEdge<P> edge) { return edges[edge.next]; }
	const HalfEdge<P>& next(HalfEdge<P> edge) const { return edges[edge.next]; }
	HalfEdge<P>& pair(HalfEdge<P> edge) { return edges[edge.pair]; }
	const HalfEdge<P>& pair(HalfEdge<P> edge) const { return edges[edge.pair]; }
	Vertex<P>& head(HalfEdge<P> edge) { return vertices[edge.head]; }
	const Vertex<P>& head(HalfEdge<P> edge) const { return vertices[edge.head]; }
	Face<P>& face(HalfEdge<P> edge) { return vertices[edge.face]; }
	const Face<P>& face(HalfEdge<P> edge) const { return vertices[edge.face]; }

	HalfEdge<P>& edge(Vertex<P> vert) { return vertices[vert.edge]; }
	const HalfEdge<P>& edge(Vertex<P> vert) const { return vertices[vert.edge]; }
	HalfEdge<P>& edge(Face<P> face) { return vertices[face.edge]; }
	const HalfEdge<P>& edge(Face<P> face) const { return vertices[face.edge]; }

	Vertex<P>& nextOnBoundary(Vertex<P> v) {
		HalfEdge<P> e = next(edge(v));
		while (!pair(e) && head(e != v)) {
			e = next(pair(e));
		}

		return head(e);
	}
};

//P is vertex type, I is index type
template <typename P, typename I>
SlotMap<Vertex<P>>::Index halfEdgeFromFaceList(HalfEdgeMesh<P>* mesh, const std::vector<P>& vertices, const std::vector<I>& indices) {
	std::vector<SlotMap<Vertex<P>>::Index> vertIndices;

	struct VertexPair {
		I a, b;
		VertexPair(I a, I b) :a(a), b(b) {}
		bool operator<(VertexPair other) { return (a == other.a) ? b < other.b : a < other.a; }
	};

	for (int i = 0; i < vertices.size(); i++)
		vertIndices.push_back(mesh->vertices.add(Vertex<P>(vertices[i])));

	std::map <VertexPair, SlotMap<Vertex<P>>::Index> edgeMap;

	for (int i = 0; i+2 < indices.size(); i+=3) {

		auto face = mesh->faces.add({});

		SlotMap<HalfEdge<P>>::Index edges[3];

		for (int j = 0; j < 3; j++) {
			I a = indices[i + j];
			I b = indices[i + (j + 1) % 3];

			auto v_a = vertIndices[a];
			auto v_b = vertIndices[b];

			if (edgeMap.find(VertexPair(a, b)) == edgeMap.end()) {
				edges[j] = mesh->edges.add({
					.head = v_a,
					.face = face
				});
				edgeMap[VertexPair(b, a)] = mesh->edges.add({
					.head = v_b,
					.pair = edges[j],
				});
			}
		}

		mesh->next(edge[0]) = edge[1];
	}
}

template <typename P>
SlotMap<Vertex<P>>::Index generateTetrahedron(HalfEdgeMesh<P>* mesh, P a, P b, P c, P d) {
	//Make sure ordering is such that faces point out
	if (dot(cross(b - a, c - a), d - a) > 0) {
		P temp = a;
		a = b;
		b = temp;
	}

	auto v_a = mesh->vertices.add({ .pos = a });
	auto v_b = mesh->vertices.add({ .pos = b });
	auto v_c = mesh->vertices.add({ .pos = c });
	auto v_d = mesh->vertices.add({ .pos = d });

	auto f_abc = mesh->faces.add({});
	auto e_ab = mesh->edges.add({});
	auto e_bc = mesh->edges.add({});
	auto e_ca = mesh->edges.add({});

	auto f_adb = mesh->faces.add({});
	auto e_ad = mesh->edges.add({});
	auto e_db = mesh->edges.add({});
	auto e_ba = mesh->edges.add({});

	auto f_bdc = mesh->faces.add({});
	auto e_bd = mesh->edges.add({});
	auto e_dc = mesh->edges.add({});
	auto e_cb = mesh->edges.add({});

	auto f_cda = mesh->faces.add({});
	auto e_cd = mesh->edges.add({});
	auto e_da = mesh->edges.add({});
	auto e_ac = mesh->edges.add({});

	mesh->edge(v_a) = e_ca;
	mesh->edge(v_b) = e_db;
	mesh->edge(v_c) = e_dc;
	mesh->edge(v_d) = e_cd;
	//Vertices complete

	mesh->edge(f_abc) = e_ab;
	mesh->edge(f_adb) = e_ad;
	mesh->edge(f_bdc) = e_bd;
	mesh->edge(f_cda) = e_cd;
	//Faces complete

	mesh->edges[e_ab] = {
		.head = v_b,
		.next = e_bc,
		.pair = e_ba,
		.face = f_abc
	};

	mesh->edges[e_bc] = {
		.head = v_c,
		.next = e_ca,
		.pair = e_cb,
		.face = f_abc
	};

	mesh->edges[e_ca] = {
		.head = v_a,
		.next = e_ab,
		.pair = e_ac,
		.face = f_abc
	};

	mesh->edges[e_ad] = {
		.head = v_d,
		.next = e_db,
		.pair = e_da,
		.face = f_adb
	};

	mesh->edges[e_db] = {
		.head = v_b,
		.next = e_ba,
		.pair = e_ab,
		.face = f_adb
	};

	mesh->edges[e_ba] = {
		.head = v_a,
		.next = e_ad,
		.pair = e_ab,
		.face = f_adb
	};

	mesh->edges[e_bd] = {
		.head = v_d,
		.next = e_dc,
		.pair = e_db,
		.face = f_bdc
	};

	mesh->edges[e_dc] = {
		.head = v_c,
		.next = e_cb,
		.pair = e_cd,
		.face = f_bdc
	};

	mesh->edges[e_cb] = {
		.head = v_b,
		.next = e_bd,
		.pair = e_bc,
		.face = f_bdc
	};

	mesh->edges[e_cd] = {
		.head = v_d,
		.next = e_da,
		.pair = e_dc,
		.face = f_cda
	};

	mesh->edges[e_da] = {
		.head = v_a,
		.next = e_ac,
		.pair = e_ad,
		.face = f_cda
	};

	mesh->edges[e_ac] = {
		.head = v_c,
		.next = e_cd,
		.pair = e_ca,
		.face = f_cda
	};

	return v_a;
}