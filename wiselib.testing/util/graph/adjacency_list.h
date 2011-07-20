/*
 * adjacency_list.h
 *
 *  Created on: 27-ago-2009
 *      Author: Juan Farré, jafarre@lsi.upc.edu
 */

#ifndef ADJACENCY_LIST_H_
#define ADJACENCY_LIST_H_

#include "util/pstl/pair.h"

namespace wiselib{

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
class AdjacencyList {
public:
	typedef OsModel_P OsModel;
	typedef VData VertexData;
	typedef EData EdgeData;
	typedef typename OsModel::size_t size_t;
	typedef size_t VerticesSize;
	typedef size_t EdgesSize;
	typedef EdgesSize DegreeSize;
	class VertexDescriptor;
	class EdgeDescriptor;
	class VertexIterator;
	class EdgeIterator;
	typedef pair<VertexIterator,VertexIterator> VertexIteratorRange;
	typedef pair<EdgeIterator,EdgeIterator> EdgeIteratorRange;

	AdjacencyList();

	VerticesSize num_vertices();
	EdgesSize num_edges();
	VertexIteratorRange vertices();
	EdgeIteratorRange edges();
	VertexDescriptor add_vertex();
	EdgeDescriptor add_edge(VertexDescriptor,VertexDescriptor);
	void remove_vertex(VertexDescriptor,bool remove_edges=false);
	void remove_edge(EdgeDescriptor);

	static VerticesSize const max_vertices=N;
	static EdgesSize const max_edges=M;
	static VertexDescriptor const null_vertex;
	static EdgeDescriptor const null_edge;

private:
	struct VertexEntry;
	struct EdgeEntry;
	VertexEntry vertex_set[max_vertices];
	EdgeEntry edge_set[max_edges];
	VerticesSize first_vertex;
	VerticesSize first_unused_vertex;
	EdgesSize first_edge;
	EdgesSize first_unused_edge;
	VerticesSize nvertices;
	EdgesSize nedges;
};

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::AdjacencyList():
	first_vertex(max_vertices),
	first_unused_vertex(0),
	first_edge(max_edges),
	first_unused_edge(0),
	nvertices(0),
	nedges(0){
	for(VerticesSize i=0;i<max_vertices;i++)
		vertex_set[i].next=i+1;
	for(EdgesSize i=0;i<max_edges;i++)
		edge_set[i].next=i+1;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
struct AdjacencyList<OsModel_P,N,M,VData,EData>::VertexEntry {
	VertexEntry();

	VertexData data;
	EdgesSize out_edges;
	EdgesSize in_edges;
	DegreeSize out_degree;
	DegreeSize in_degree;
	VerticesSize next;
	VerticesSize prev;
};

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::VertexEntry::VertexEntry():
	out_degree(max_edges){
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
struct AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeEntry {
	EdgeEntry();

	VerticesSize source;
	VerticesSize target;
	EdgeData data;
	EdgesSize next;
	EdgesSize prev;
	EdgesSize next_out;
	EdgesSize prev_out;
	EdgesSize next_in;
	EdgesSize prev_in;
};

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeEntry::EdgeEntry():
	source(max_vertices){
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
class AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor {
public:
	class OutEdgeIterator;
	class InEdgeIterator;
	typedef pair<OutEdgeIterator,OutEdgeIterator> OutEdgeIteratorRange;
	typedef pair<InEdgeIterator,InEdgeIterator> InEdgeIteratorRange;

	DegreeSize out_degree() const;
	DegreeSize in_degree() const;
	DegreeSize degree() const;
	OutEdgeIteratorRange out_edges();
	InEdgeIteratorRange in_edges();
	VertexData operator*();
	bool operator==(VertexDescriptor);
	bool operator!=(VertexDescriptor);

protected:
	VertexDescriptor(AdjacencyList &);
	VertexDescriptor(AdjacencyList &,VerticesSize);

	AdjacencyList &g;
	VerticesSize v;

	friend class AdjacencyList;
	friend class OutEdgeIterator;
	friend class InEdgeIterator;
};

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::VertexDescriptor(AdjacencyList &graph):
	g(graph),
	v(max_vertices){
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::VertexDescriptor(AdjacencyList &graph,VerticesSize i):
	g(graph),
	v(i){
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::DegreeSize AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::out_degree() const {
	return g.vertex_set[v].out_degree;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::DegreeSize AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::in_degree() const {
	return g.vertex_set[v].in_degree;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::DegreeSize AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::degree() const {
	return out_degree()+in_degree();
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::OutEdgeIteratorRange AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::out_edges() {
	return OutEdgeIteratorRange(OutEdgeIterator(*this),OutEdgeIterator(*this,max_edges));
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::InEdgeIteratorRange AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::in_edges() {
	return InEdgeIteratorRange(InEdgeIterator(*this),InEdgeIterator(*this,max_edges));
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
VData AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::operator*(){
	return g.vertex_set[v].data;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
bool AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::operator==(VertexDescriptor u){
	return u.v==v;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
bool AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::operator!=(VertexDescriptor u){
	return !operator==(u);
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
class AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeDescriptor {
public:
	VertexDescriptor source();
	VertexDescriptor target();
	EdgeData operator*();
	bool operator==(EdgeDescriptor);
	bool operator!=(EdgeDescriptor);

protected:
	EdgeDescriptor(AdjacencyList &);
	EdgeDescriptor(AdjacencyList &,EdgesSize);

	AdjacencyList &g;
	EdgesSize e;

	friend class AdjacencyList;
};

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeDescriptor::EdgeDescriptor(AdjacencyList &graph):
	g(graph),
	e(max_edges){
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeDescriptor::EdgeDescriptor(AdjacencyList &graph,EdgesSize i):
	g(graph),
	e(i){
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeDescriptor::source(){
	return VertexDescriptor(g,g.edge_set[e].source);
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeDescriptor::target(){
	return VertexDescriptor(g,g.edge_set[e].target);
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
EData AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeDescriptor::operator*(){
	return g.edge_set[e].data;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
bool AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeDescriptor::operator==(EdgeDescriptor e2){
	return e2.e==e;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
bool AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeDescriptor::operator!=(EdgeDescriptor e2){
	return !operator==(e2);
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
class AdjacencyList<OsModel_P,N,M,VData,EData>::VertexIterator: public VertexDescriptor {
public:
	VertexIterator operator++();

private:
	VertexIterator(AdjacencyList &);
	VertexIterator(AdjacencyList &,VerticesSize);

	friend class AdjacencyList;
};

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::VertexIterator::VertexIterator(AdjacencyList &g):
	VertexDescriptor(g,g.first_vertex) {
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::VertexIterator::VertexIterator(AdjacencyList &g,VerticesSize v):
	VertexDescriptor(g,v) {
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::VertexIterator AdjacencyList<OsModel_P,N,M,VData,EData>::VertexIterator::operator++(){
	if(operator!=(null_vertex))
		VertexDescriptor::v=vertex_set[VertexDescriptor::v].next;
	return *this;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
class AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeIterator: public EdgeDescriptor {
public:
	EdgeIterator operator++();

private:
	EdgeIterator(AdjacencyList &);
	EdgeIterator(AdjacencyList &,EdgesSize);

	friend class AdjacencyList;
};

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeIterator::EdgeIterator(AdjacencyList &g):
	EdgeDescriptor(g,g.first_edge) {
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeIterator::EdgeIterator(AdjacencyList &g,EdgesSize e):
	EdgeDescriptor(g,e) {
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeIterator AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeIterator::operator++(){
	if(operator!=(null_edge))
		EdgeDescriptor::e=edge_set[EdgeDescriptor::e].next;
	return *this;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
class AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::OutEdgeIterator: public EdgeDescriptor {
public:
	OutEdgeIterator operator++();

private:
	OutEdgeIterator(VertexDescriptor &);
	OutEdgeIterator(VertexDescriptor &,EdgesSize);

	VertexDescriptor &v;

	friend class AdjacencyList;
};

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::OutEdgeIterator::OutEdgeIterator(VertexDescriptor& vertex):
	EdgeDescriptor(vertex.g,vertex.g.vertex_set[vertex.v].out_edges),
	v(vertex){
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::OutEdgeIterator::OutEdgeIterator(VertexDescriptor& vertex,EdgesSize e):
	EdgeDescriptor(vertex.g,e),
	v(vertex){
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::OutEdgeIterator AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::OutEdgeIterator::operator++(){
	if(operator!=(null_edge))
		EdgeDescriptor::e=edge_set[EdgeDescriptor::e].next_out;
	return *this;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
class AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::InEdgeIterator: public EdgeDescriptor {
public:
	InEdgeIterator operator++();

private:
	InEdgeIterator(VertexDescriptor &);
	InEdgeIterator(VertexDescriptor &,EdgesSize);

	VertexDescriptor &v;

	friend class AdjacencyList;
};

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::InEdgeIterator::InEdgeIterator(VertexDescriptor &vertex):
	EdgeDescriptor(vertex.g,vertex.g.vertex_set[vertex.v].in_edges),
	v(vertex){
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::InEdgeIterator::InEdgeIterator(VertexDescriptor &vertex,EdgesSize e):
	EdgeDescriptor(vertex.g,e),
	v(vertex){
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::InEdgeIterator AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor::InEdgeIterator::operator++(){
	if(operator!=(null_edge))
		EdgeDescriptor::e=edge_set[EdgeDescriptor::e].next_in;
	return *this;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::VertexIteratorRange AdjacencyList<OsModel_P,N,M,VData,EData>::vertices() {
	return VertexIteratorRange(VertexIterator(*this),VertexIterator(*this,max_vertices));
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeIteratorRange AdjacencyList<OsModel_P,N,M,VData,EData>::edges() {
	return EdgeIteratorRange(EdgeIterator(*this),EdgeIterator(*this,max_edges));
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::VerticesSize AdjacencyList<OsModel_P,N,M,VData,EData>::num_vertices() {
	return nvertices;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::EdgesSize AdjacencyList<OsModel_P,N,M,VData,EData>::num_edges() {
	return nedges;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::VertexDescriptor AdjacencyList<OsModel_P,N,M,VData,EData>::add_vertex() {
	if(nvertices==max_vertices)
		return null_vertex;
	++nvertices;
	VerticesSize const v=first_unused_vertex;
	first_unused_vertex=vertex_set[v].next;
	vertex_set[v].prev=max_vertices;
	vertex_set[v].next=first_vertex;
	if(first_vertex!=max_vertices)
		vertex_set[first_vertex].prev=v;
	first_vertex=v;
	vertex_set[v].out_edges=vertex_set[v].in_edges=max_edges;
	vertex_set[v].in_degree=vertex_set[v].out_degree=0;
	return VertexDescriptor(*this,v);
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
typename AdjacencyList<OsModel_P,N,M,VData,EData>::EdgeDescriptor AdjacencyList<OsModel_P,N,M,VData,EData>::add_edge(VertexDescriptor source,VertexDescriptor target) {
	if(nedges==max_edges||source==null_vertex||target==null_vertex)
		return null_edge;
	EdgesSize const e=first_unused_edge;
	first_unused_edge=edge_set[e].next;
	edge_set[e].source=source.v;
	edge_set[e].target=target.v;
	edge_set[e].prev=max_edges;
	edge_set[e].next_out=first_edge;
	if(first_edge!=max_edges)
		edge_set[first_edge].prev_out=e;
	first_edge=e;
	++nedges;
	edge_set[e].prev_out=max_edges;
	{
		EdgesSize &first=vertex_set[source.v].out_edges;
		edge_set[e].next_out=first;
		if(first!=max_edges)
			edge_set[first].prev_out=e;
		first=e;
	}
	vertex_set[source.v].out_degree++;
	edge_set[e].prev_in=max_edges;
	{
		EdgesSize &first=vertex_set[target.v].in_edges;
		edge_set[e].next_in=first;
		if(first!=max_edges)
			edge_set[first].prev_in=e;
		first=e;
	}
	vertex_set[target.v].in_degree++;
	return EdgeDescriptor(*this,e);
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
void AdjacencyList<OsModel_P,N,M,VData,EData>::remove_edge(EdgeDescriptor edge) {
	if(edge==null_edge||edge_set[edge.e].source==max_vertices)
		return;
	EdgesSize const e=edge.e;
	edge_set[e].source=max_vertices;
	edge_set[e].next=first_unused_edge;
	first_unused_edge=e;
	{
		EdgesSize const prev=edge_set[e].prev_out;
		EdgesSize const next=edge_set[e].next_out;
		if(prev==max_edges)
			vertex_set[edge_set[e].source].out_edges=next;
		else
			edge_set[prev].next_out=next;
		if(next!=max_edges)
			edge_set[next].prev_out=prev;
	}
	vertex_set[edge_set[e].source].out_degree--;
	{
		EdgesSize const prev=edge_set[e].prev_in;
		EdgesSize const next=edge_set[e].next_in;
		if(prev==max_edges)
			vertex_set[edge_set[e].source].in_edges=next;
		else
			edge_set[prev].next_in=next;
		if(next!=max_edges)
			edge_set[next].prev_in=prev;
	}
	vertex_set[edge_set[e].target].in_degree--;
	{
		EdgesSize const prev=edge_set[e].prev;
		EdgesSize const next=edge_set[e].next;
		if(prev==max_edges)
			first_edge=next;
		else
			edge_set[prev].next=next;
		if(next!=max_edges)
			edge_set[next].prev=prev;
	}
	--nedges;
}

template<class OsModel_P,
	typename OsModel_P::size_t N,
	typename OsModel_P::size_t M,
	class VData,
	class EData>
void AdjacencyList<OsModel_P,N,M,VData,EData>::remove_vertex(VertexDescriptor vertex,bool remove_edges) {
	if(vertex==null_vertex||vertex_set[vertex.v].out_degree==max_edges)
		return;
	VerticesSize const v=vertex.v;
	vertex_set[v].out_degree=max_edges;
	vertex_set[v].next=first_unused_vertex;
	first_unused_vertex=v;
	{
		VerticesSize const prev=vertex_set[v].prev;
		VerticesSize const next=vertex_set[v].next;
		if(prev==max_vertices)
			first_vertex=next;
		else
			vertex_set[prev].next=next;
		if(next!=max_vertices)
			vertex_set[next].prev=prev;
	}
	--nvertices;
	if(remove_edges){
		while(vertex_set[v].out_edges!=max_edges)
			remove_edge(EdgeDescriptor(*this,vertex_set[v].out_edges));
		while(vertex_set[v].in_edges!=max_edges)
			remove_edge(EdgeDescriptor(*this,vertex_set[v].in_edges));
	}
}

}

#endif /* ADJACENCY_LIST_H_ */
