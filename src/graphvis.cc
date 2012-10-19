/*
 * graphvis.cc
 *
 *  Created on: Dec 7, 2011
 *      Author: slynen
 */

#include <slynen_tools/graphvis.h>

namespace graphvis{
void FruchtermanReingold(Graph& g, int niter, double maxdelta, double volume, double coolexp, double repulserad){

	//	int niter = 500;
	//	double maxdelta = 1000;//max move per iteration, about same value as vertices
	//	double volume = 1000*1000*1000; //should be nvertices^3
	//	double coolexp = 1.5; //cooling exponent of the simulated annealing
	//	double repulserad = volume * maxdelta; //Determines the radius at which   vertex-vertex repulsion cancels out attraction of adjacent vertices. A reasonable default is \p volume times the number of vertices.

	layout_random_3d(g);

	double frk, t, ded;
	double rf, af;

	int nvertices = num_vertices(g);

	frk=pow(volume/(double)nvertices,1.0/3.0); //Define the F-R constant


	double totaldisplacement;
	for(int i = niter;i>=0;i--){
		totaldisplacement = 0;
		//Set the temperature (maximum move/iteration)
		t=maxdelta*pow(i/(double)niter,coolexp);
		//Clear the deltas
		graph_traits<Graph>::vertex_iterator vertit, vertit_end;
		for (boost::tie(vertit, vertit_end) = vertices(g); vertit != vertit_end; ++vertit) {
			vertex_T& v = g[*vertit];
			v.displacement.fill(0);
		}

		//Increment deltas for each undirected pair
		graph_traits<Graph>::vertex_iterator viter, viter2, viter_end;
		for (boost::tie(viter, viter_end) = vertices(g); viter != viter_end; ++viter) {
			vertex_T& vj = g[*viter];
			viter2 = viter;
			for (++viter2; viter2 != viter_end; ++viter2) {
				vertex_T& vk = g[*viter2];
				//Obtain difference vector
				Vector3f diff = vj.pos - vk.pos;
				ded = diff.norm();
				diff.normalize();
				//Calculate repulsive "force"
				rf=frk*frk*(1.0/ded-ded*ded/repulserad);
				vj.displacement += diff * rf;//Add to the position change vector
				vk.displacement -= diff * rf;
			}
			if(vj.ID%100==0)
				cout<<"Layouting graph FruchtermanReingold... iteration:"<<niter-i<<" vertex: "<<vj.ID<<"              \r";
		}

		//Calculate the attractive "force"
		graph_traits<Graph>::edge_iterator eiter, eiter_end;
		for (boost::tie(eiter, eiter_end) = edges(g); eiter != eiter_end; ++eiter) {
			edge_T& edge = g[*eiter];
			vertex_T& vj = edge.vertexA;
			vertex_T& vk = edge.vertexB;
			Vector3f diff = vj.pos - vk.pos;
			float ded = diff.norm();
			diff.normalize();
			af=ded*ded/frk;
			vj.displacement -= diff*af;//Add to the position change vector
			vk.displacement += diff*af;
		}

		//Dampen motion, if needed, and move the points
		for (boost::tie(viter, viter_end) = vertices(g); viter != viter_end; ++viter) {
			vertex_T& v = g[*viter];
			float ded = v.displacement.norm();
			if(ded>t){                 //Dampen to t
				ded=t/ded;
				v.displacement *= ded;
			}
			totaldisplacement += v.displacement.norm();
			v.pos += v.displacement; //Update positions
		}
		//		cout<<"totaldisplacement: "<<totaldisplacement<<endl;
	}
}


void exportRViz(graphvis::Graph& g){

	double nvert = num_vertices(g);
	double vol = nvert*nvert*100;
	cout<<"Layouting graph FruchtermanReingold...     \r";
	FruchtermanReingold(g, 300, nvert * nvert, vol, 1.5, vol);//3d tree spanning

	ros::NodeHandle n("~");
	ros::Rate r(30);
	ros::Publisher marker_pub = n.advertise<visualization_msgs::Marker>("visualization_marker", 100);


	visualization_msgs::Marker edge_marker;
	visualization_msgs::Marker vertex_marker;

	// Set the frame ID and timestamp.  See the TF tutorials for information on these.
	edge_marker.header.frame_id = "/tree";
	edge_marker.header.stamp = ros::Time::now();

	// Set the namespace and id for this marker.  This serves to create a unique ID
	// Any edge_marker sent with the same namespace and id will overwrite the old one
	edge_marker.ns = "basic_tree";
	// Set the edge_marker action.  Options are ADD and DELETE
	edge_marker.action = visualization_msgs::Marker::ADD;
	// Set the scale of the marker -- 1x1x1 here means 1m on a side
	edge_marker.scale.x = 1.0;
	edge_marker.scale.y = 1.0;
	edge_marker.scale.z = 1.0;

	edge_marker.lifetime = ros::Duration();
	vertex_marker = edge_marker; //cpy all props
	edge_marker.type = visualization_msgs::Marker::ARROW;
	vertex_marker.type = visualization_msgs::Marker::SPHERE;

	int ID = 0;
	graph_traits<graphvis::Graph>::vertex_iterator viter, viter_end;
	for (boost::tie(viter, viter_end) = vertices(g); viter != viter_end; ++viter) {
		vertex_marker.color.r = 0.0f;
		vertex_marker.color.g = 1.0f;
		vertex_marker.color.b = 0.0f;
		vertex_marker.color.a = 1.0;
		vertex_marker.pose.position.x = g[*viter].pos(0);
		vertex_marker.pose.position.y = g[*viter].pos(1);
		vertex_marker.pose.position.z = g[*viter].pos(2);
		vertex_marker.id = ID++;
		// Publish the edge_marker
		marker_pub.publish(vertex_marker);
	}

	//	edge_marker.color.r = 0.0f;
	//	edge_marker.color.g = 1.0f;
	//	edge_marker.color.b = 0.0f;
	//	edge_marker.color.a = 1.0;
	//	edge_marker.pose.position.x = 0;
	//	edge_marker.pose.position.y = 0;
	//	edge_marker.pose.position.z = 0;
	//	edge_marker.pose.orientation.x = 0.0;
	//	edge_marker.pose.orientation.y = 0.0;
	//	edge_marker.pose.orientation.z = 0.0;
	//	edge_marker.pose.orientation.w = 1.0;
	//	edge_marker.id = 0;
	//
	//	// Publish the edge_marker
	//	marker_pub.publish(edge_marker);

	cout<<"done"<<endl;
	while(1){sleep(10);};

}

void exportGraphViz(Graph& g, std::string& path, std::string nodeprefix, std::string graphname){

	cout<<"sending "<<num_edges(g)<<" edges to graphviz export "<<path<<endl;

	std::ofstream fout(path.c_str());

	fout << "digraph " << graphname << " {\n"
			<< " rankdir=TB\n"
			<< " size=\"10,10\"\n"
			<< " ratio=\"filled\"\n"
			<< " node[shape=\"circle\"]\n";

	graph_traits<Graph>::vertex_iterator viter, viter_end;
	for (boost::tie(viter, viter_end) = vertices(g); viter != viter_end; ++viter) {
		if(g[*viter].ID == -1) continue;
		fout << nodeprefix + boost::lexical_cast<string>(g[*viter].ID);
		fout << " [color=\""<<g[*viter].Color<<"\", label=\"" << (g[*viter].Label!=""?g[*viter].Label:"deflbl") << "\"];\n";
	}
	graph_traits<Graph>::edge_iterator eiter, eiter_end;
	fout<<setprecision(2);
	for (boost::tie(eiter, eiter_end) = edges(g); eiter != eiter_end; ++eiter) {
		fout << nodeprefix + boost::lexical_cast<string>(source(*eiter, g)) << " -> " <<
				nodeprefix + boost::lexical_cast<string>(target(*eiter, g));
		fout << " [color=\""<<g[*eiter].Color<<"\", penwidth="<<g[*eiter].PenWidth<<", style=\""<< g[*eiter].Style<<"\", label=\"" << (g[*eiter].Label!=""?g[*eiter].Label:"-") << "\"];\n";
	}
	fout << "}\n";
	cout<<"done."<<endl;
}

int layout_random_3d(Graph& g) {
	graph_traits<Graph>::vertex_iterator viter, viter_end;
	for (boost::tie(viter, viter_end) = vertices(g); viter != viter_end; ++viter) {
		vertex_T& v = g[*viter];
		v.pos.setRandom();
	}
	return 0;
}


}
