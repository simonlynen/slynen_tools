/*
 * kruskal_MST.h
 *
 *  Created on: 29 Aug 2011
 *      Author: slynen
 */

#ifndef KRUSKAL_MST_H_
#define KRUSKAL_MST_H_

#include <stack>
#include <algorithm>
#include <iostream>
#include <iomanip>
#include <slynen_tools/sort_comp.h>

using namespace std;


/*
 * find root. with path compression, returns root of v
 */
__inline__ int find_root(int v, std::vector<int>& P){
	std::stack<int> stck;
	stck.push(v);

	while (P[v] != v){ //find root
		v = P[v];
		stck.push(v);
	}
	while (!stck.empty()) //compress children
	{
		P[stck.top()] = v;
		stck.pop();
	}
	return v;
}

/*
 * union of tree-partitions by size
 */
__inline__ void union_clusters(int v, int w, std::vector<int>& Parent, std::vector<int>& Rank){
	if (Rank[v] < Rank[w]){
		Parent[v] = w;
	}else if (Rank[v] > Rank[w]){
		Parent[w] = v;
	}else{
		Parent[w] = v;
		++(Rank[v]);
	}
}

template <template<typename, typename> class Edges_T, template<typename> class alloc_edge_T, typename edge_T, class sort_T>
__inline__ bool kruskal(Edges_T<edge_T, alloc_edge_T<edge_T> >& edges, int inVertices, sort_T sort_functor){

	std::vector<int> viParent;
	viParent.reserve(inVertices);
	for(int i = 0;i<inVertices;++i)	viParent.push_back(i);

	std::vector<int> viRank;
	viRank.reserve(inVertices);
	viRank.insert(viRank.begin(), inVertices, 1);

	edges.sort(sort_functor); //sort by mutual information

	int inTreeEdges = 0;
	//	int cnt = 0;

	typename Edges_T<edge_T, alloc_edge_T<edge_T> >::iterator edgeIt = edges.begin();
	while(inTreeEdges<inVertices-1 && edgeIt!=edges.end()){
		edge_T & edge = *edgeIt;
		if(find_root(edge.first.first, viParent)!=find_root(edge.first.second, viParent)){
			union_clusters(viParent[edge.first.first],viParent[edge.first.second], viParent, viRank); //TODO should'n we put the parent here, or is this already the case after find_root?
			++inTreeEdges;
			++edgeIt;
		} else {
			edgeIt = edges.erase(edgeIt);
		}
		//		if(++cnt%50==1){
		//			cout << fixed << setprecision(1) << "Building max span tree: "<<
		//					100 * inTreeEdges / inVertices << "%\r";
		//			cout.flush();
		//		}
	}
	edges.erase(edgeIt,edges.end());
	if((int)edges.size()!=inVertices-1){
//		cout<<"Not enough vertices to build tree. Got only "<<edges.size()<<" edges in MST."<<endl;
		return false;
	}else{
		cout<<"Done. Got "<<edges.size()<<" edges in MST."<<endl;
		//		cout<<"Edges = [";
		//		for(edgeIt = edges.begin();edgeIt!=edges.end();++edgeIt){
		//			cout<<edgeIt->first.first<<" "<<edgeIt->first.second<<endl;
		//		}
		//		cout<<" ];"<<endl;
		return true;
	}
}

#endif /* KRUSKAL_MST_H_ */
