/*
 * STLhelpers.h
 *
 *  Created on: 18 Aug 2011
 *      Author: slynen
 */

#ifndef STLHELPERS_H_
#define STLHELPERS_H_

#include <vector>
#include <utility>
#include <sstream>
#include <string>


template < typename T>
std::string string_format(const T & t, int decimals = 4)
{
	using namespace std;
	std::ostringstream oss;
	oss<<fixed<<setprecision(decimals);
	oss << t;
	return oss.str();
}

//a map that works a bit more efficient than the original STL implementation (original map creates unnecessary objects on insertion)
template <typename MapType, typename KeyArgType, typename ValueArgType>
typename MapType::iterator MapAddOrUpdate(MapType& m, const KeyArgType& k, const ValueArgType& v){
	typename MapType::iterator lb = m.lower_bound(k);
	if(lb!=m.end() && !(m.key_comp()(k,lb->first))){
		lb->second = v;
		return lb;
	}else{
		typedef typename MapType::value_type MVT;
		return m.insert(lb,MVT(k,v));
	}
}
//a map that returns references to the value if the key exists (original map overwrites existing value)
template <typename MapType, typename KeyArgType, typename ValueArgType>
typename MapType::iterator MapAddOrRef(MapType& m, const KeyArgType& k, const ValueArgType& v){
	typename MapType::iterator lb = m.lower_bound(k);
	if(lb!=m.end() && !(m.key_comp()(k,lb->first))){
		return lb;
	}else{
		typedef typename MapType::value_type MVT;
		return m.insert(lb,MVT(k,v));
	}
}

//a vector for scores and indices that can be sorted
template<typename index_T, class distance_T>
struct VectorWithScore
{
	typedef std::vector<std::pair<index_T, distance_T> > Type; //common workaround for typedefs can't be templated in c++
	bool reverse;
public:
	VectorWithScore(const bool& revparam=false) {reverse=revparam;}
	bool operator() (const std::pair<index_T,distance_T>& lhs, const std::pair<index_T,distance_T>& rhs) const{
		if (reverse) return (lhs.second>rhs.second);
		else return (lhs.second<rhs.second);
	};
};


#endif /* STLHELPERS_H_ */
