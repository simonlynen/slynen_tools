/*
 * graphtools.h
 *
 *  Created on: Nov 28, 2011
 *      Author: slynen
 */


#include <fstream>
#include <iostream>
#include <utility>
#include <algorithm>
#include <string>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <time.h>
#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <boost/foreach.hpp>
#include <slynen_tools/STLhelpers.h>

#ifndef GRAPHVIS_H_
#define GRAPHVIS_H_

using namespace std;
using namespace boost;

namespace graphvis{


enum edge_value{
	eentropy,
	esplit_bit
};
enum vertex_value{
	ventropy,
	vsplit_bit,
	node_id,
	traindata
};

typedef Eigen::Matrix<float, 3, 1> Vector3f;
typedef Eigen::Matrix< float , 3 , 3 > Matrix3f;

struct vertex_T{
	friend ostream& operator<<(ostream& output, const vertex_T& p);
public:
	int ID;
	std::string Color;
	std::string Label;
	Vector3f pos;
	Vector3f displacement;
	vertex_T(){
		ID = -1;
		Label = "NA";
		Color = "black";
	}
	bool operator<(const vertex_T& lhs) const {
		return lhs.ID < ID;
	}
	bool operator==(const vertex_T& lhs) const {
		return lhs.ID == ID;
	}
	vertex_T& operator=(const vertex_T& lhs){
		this->ID = lhs.ID;
		this->Color = lhs.Color;
		this->Label = lhs.Label;
		return *this;
	}


};

ostream& operator<<(ostream& os, const vertex_T& p) {
	os << "Vertex: ID: "<< p.ID << " Color: "<<p.Color<<" Label: "<<p.Label<<" ";
	return os;  // for multiple << operators.
}

struct edge_T{
	friend ostream& operator<<(ostream& output, const edge_T& e);
public:
	vertex_T vertexA;
	vertex_T vertexB;
	std::string Label;
	std::string Color;
	std::string Style;
	double PenWidth;
	edge_T(){
		Label = "lbl";
		Color = "black";
		Style = "bold";
		PenWidth = 1.0;
	}
	edge_T& operator=(edge_T& lhs){
		vertexA = lhs.vertexA;
		vertexB = lhs.vertexB;
		Label = lhs.Label;
		Color = lhs.Color;
		Style = lhs.Style;
		PenWidth = lhs.PenWidth;
		return *this;
	}
	bool operator==(const edge_T& lhs) const{
		return this->vertexA.ID == lhs.vertexA.ID && this->vertexB.ID == lhs.vertexB.ID;
	}
	bool operator<(const edge_T& lhs) const {
		return lhs.vertexA.ID < vertexA.ID || (lhs.vertexA.ID == vertexA.ID && lhs.vertexB.ID < vertexB.ID);
	}
};


ostream& operator<<(ostream& os, const edge_T& e) {
	os << "Edge: "<< e.vertexA.ID << "<->"<< e.vertexB.ID<<" Color: "<<e.Color<<" Label: "<<e.Label<<" ";
	return os;  // for multiple << operators.
}

typedef adjacency_list<vecS, vecS, bidirectionalS, vertex_T, edge_T > Graph;
typedef boost::graph_traits<Graph>::vertex_descriptor vertex_t;
typedef boost::graph_traits<Graph>::edge_descriptor edge_t;

int layout_random_3d(Graph& g);


template <template<typename, typename> class edge_C_T, template<typename> class edge_A_T>
void convertToBoostGraph(Graph& g, edge_C_T<edge_T, edge_A_T<edge_T> >& cEdges){
	if(cEdges.empty()){
		cout<<"[ERROR] Empty container provided to convertToBoostGraph function"<<endl;
		return;
	}

	//list of vertices already in the graph
	typedef pair<const int, vertex_T> vt;
	std::set<int> existingVertices;
	pair<set<int>::iterator,bool> ret1;
	BOOST_FOREACH(edge_T& edge, cEdges){
		edge_t e; bool be;
		vertex_t vA, vB;
		ret1 = existingVertices.insert(edge.vertexA.ID);
		if(ret1.second){
			vA = boost::add_vertex(g);
			g[vA] = edge.vertexA;
		}
		ret1 = existingVertices.insert(edge.vertexB.ID);
		if(ret1.second){
			vB = boost::add_vertex(g);
			g[vB] = edge.vertexB;
		}
		//addedge
		boost::tie(e,be) = boost::add_edge(edge.vertexA.ID, edge.vertexB.ID, g);
		g[e] = edge; //copy all properties
	};

}




/**
 * \brief 3D Fruchterman-Reingold algorithm.
 *
 * This is the 3D version of the force based
 * Fruchterman-Reingold layout
 *
 * </para><para>
 * This function was ported from the SNA R package.
 * \param graph Pointer to an initialized graph object.
 * \param niter The number of iterations to do. A reasonable
 *        default value is 500.
 * \param maxdelta The maximum distance to move a vertex in an
 *        iteration. A reasonable default value is the number of
 *        vertices.
 * \param volume The volume parameter of the algorithm. A reasonable
 *        default is the number of vertices^3.
 * \param coolexp The cooling exponent of the simulated annealing.
 *        A reasonable default is 1.5.
 * \param repulserad Determines the radius at which
 *        vertex-vertex repulsion cancels out attraction of
 *        adjacent vertices. A reasonable default is \p volume
 *        times the number of vertices.
 * \param use_seed Logical, if true the supplied values in the
 *        \p res argument are used as an initial layout, if
 *        false a random initial layout is used.
 * \param weight Pointer to a vector containing edge weights,
 *        the attraction along the edges will be multiplied by these.
 *        It will be ignored if it is a null-pointer.
 *
 */
void FruchtermanReingold(Graph& g, int niter, double maxdelta, double volume, double coolexp, double repulserad);
void exportRViz(graphvis::Graph& g);
void exportGraphViz(Graph& g, std::string& path, std::string nodeprefix = "nde_", std::string graphname = "unnamed");


}
#endif /* GRAPHVIS_H_ */
