/*
 * MST.cc
 *
 *  Created on: Jan 6, 2012
 *      Author: slynen
 */

#include <gtest/gtest.h>
#include <slynen_tools/kruskal_MST.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <list>
#include <stack>

typedef std::pair<std::pair<int,int>, double > edge_T;

void create_edges(std::list<edge_T>& edges, int vertices){
	for(int i = 0;i<vertices;++i){
		for(int j = 0;j<vertices;++j){
			if(i==j) continue;
			edge_T edge;
			edge.first.first = i;
			edge.first.second = j;
			edge.second = rand() % 200;
			edges.push_back(edge);
		}
	}
}

TEST(MST, find_root){
	std::vector<int> P;

	P.push_back(7); //0
	P.push_back(3); //1
	P.push_back(1); //2
	P.push_back(3); //3
	P.push_back(3); //4
	P.push_back(3); //5
	P.push_back(5); //6
	P.push_back(5); //7

	EXPECT_EQ(find_root(0,P), 3);
	EXPECT_EQ(find_root(1,P), 3);
	EXPECT_EQ(find_root(2,P), 3);
	EXPECT_EQ(find_root(3,P), 3);
	EXPECT_EQ(find_root(4,P), 3);
	EXPECT_EQ(find_root(5,P), 3);
	EXPECT_EQ(find_root(6,P), 3);
	EXPECT_EQ(find_root(7,P), 3);

	for(int i = 0;i<8;++i){
		EXPECT_EQ(P.at(i),3); //path compression
	}
}


TEST(MST, union_clusters){

	std::vector<int> P;

	P.push_back(7); //0 //C2
	P.push_back(3); //1 //C1
	P.push_back(1); //2 //C1
	P.push_back(3); //3 //C1
	P.push_back(3); //4 //C1
	P.push_back(5); //5 //C2
	P.push_back(5); //6 //C2
	P.push_back(6); //7 //C2
	P.push_back(7); //8 //C2

	std::vector<int> Rank;
	Rank.push_back(0); //0
	Rank.push_back(1); //1
	Rank.push_back(0); //2
	Rank.push_back(3); //3
	Rank.push_back(0); //4
	Rank.push_back(4); //5
	Rank.push_back(0); //6
	Rank.push_back(2); //7
	Rank.push_back(0); //8

	union_clusters(find_root(8, P), find_root(1, P), P, Rank); //link 8 and 1, 3 should get linked to 5

	EXPECT_EQ(P.at(3), 5); //correctly linked to larger cluster

	for(int i = 0;i<8;++i){
		EXPECT_EQ(find_root(i, P),5); //correct parent link
	}
	for(int i = 0;i<8;++i){
		EXPECT_EQ(P.at(i), 5); //path compressed
	}
}

TEST(MST, kruskal){
	std::list<edge_T> edges;
	srand(time(NULL));
	int vertices = 9;
	create_edges(edges, vertices);

	kruskal(edges, vertices, boost::bind(&edge_T::second, _1) > boost::bind(&edge_T::second, _2)); //MAX SPAN TREE

	//all connected
	{
		std::stack<int> nodes_stack;

		//BF search
		bool* visited = new bool[vertices];
		for(int i = 0;i<vertices;++i){ //start
			for(int j = 0;j<vertices;++j){ //endpoint
				std::list<edge_T> edges2 = edges; //fresh copy of edges
				int nvisited = 0;
				memset(visited,0,vertices*sizeof(bool)); //reset visited

				nodes_stack.push(i);
				bool found = false;

				while(!nodes_stack.empty() && !found){
					int current_node = nodes_stack.top();
					if(current_node==j) found = true;
					nodes_stack.pop();
					EXPECT_FALSE(visited[current_node]);
					visited[current_node] = true;
					for(std::list<edge_T>::iterator it = edges2.begin();it!=edges2.end();){
						if(it->first.first == current_node){
							nodes_stack.push(it->first.second);
							it = edges2.erase(it);
						}else if(it->first.second == current_node){
							nodes_stack.push(it->first.first);
							it = edges2.erase(it);
						}else{
							++it;
						}
					}
				}
				EXPECT_TRUE(found);
			}
		}
	}


	//no circles
	{
		std::stack<int> nodes_stack;

		//BF search
		bool* visited = new bool[vertices];
		for(int i = 0;i<vertices;++i){
			std::list<edge_T> edges2 = edges; //fresh copy of edges
			int nvisited = 0;
			memset(visited,0,vertices*sizeof(bool)); //reset visited

			nodes_stack.push(i);

			while(!nodes_stack.empty()){
				int current_node = nodes_stack.top();
				nodes_stack.pop();
				EXPECT_FALSE(visited[current_node]);
				visited[current_node] = true;
				for(std::list<edge_T>::iterator it = edges2.begin();it!=edges2.end();){
					if(it->first.first == current_node){
						nodes_stack.push(it->first.second);
						it = edges2.erase(it);
					}else if(it->first.second == current_node){
						nodes_stack.push(it->first.first);
						it = edges2.erase(it);
					}else{
						++it;
					}
				}
			}
		}
	}
}


// Run all the tests that were declared with TEST()
int main(int argc, char **argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
