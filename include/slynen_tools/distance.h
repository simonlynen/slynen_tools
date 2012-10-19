#ifndef VOCABULARY_TREE_DISTANCE_H
#define VOCABULARY_TREE_DISTANCE_H

#include <stdint.h>
//#include <slynen_tools/BRISK_Feature.h>
#include <slynen_tools/popcnt.h>

namespace distance {

#ifdef USE_ORIGINAL_EIGEN3_NAMESPACE
namespace Eigen3 = Eigen;
#endif

/**
 * \brief Meta-function returning a type that can be used to accumulate many values of T.
 *
 * By default, the accumulator type is the same as \c T. Specializations for the basic types
 * are:
 * \li \c uint8_t -> \c uint32_t
 * \li \c uint16_t -> \c uint32_t
 * \li \c int8_t -> \c int32_t
 * \li \c int16_t -> \c int32_t
 * \li \c float -> \c double
 */
template<typename T>
struct Accumulator
{
	typedef T type;
};

/// \cond internal
template<> struct Accumulator<uint8_t>  { typedef uint32_t type; };
template<> struct Accumulator<uint16_t> { typedef uint32_t type; };
template<> struct Accumulator<int8_t>   { typedef int32_t type; };
template<> struct Accumulator<int16_t>  { typedef int32_t type; };
// Have noticed loss of precision when computing sum-squared-error with many input features.
template<> struct Accumulator<float>    { typedef double type; };
/// \endcond

/**
 * \brief Default implementation of L2 distance metric.
 *
 * Works with std::vector, boost::array, or more generally any container that has
 * a \c value_type typedef, \c size() and array-indexed element access.
 */
template<class Feature>
struct L2
{
	typedef typename Feature::value_type value_type;
	typedef typename Accumulator<value_type>::type result_type;

	result_type operator()(const Feature& a, const Feature& b) const
	{
		result_type result = result_type();
		for (size_t i = 0; i < a.size(); ++i) {
			result_type diff = a[i] - b[i];
			result += diff*diff;
		}
		return result;
	}
};
//slynen
/**
 * \brief Default implementation of HAMMING distance metric for the BRIEFFeature Class.
 *
 */
struct HAMMING
{

	typedef unsigned char value_type;
	typedef uint32_t result_type;

	int nWords;

	HAMMING(int _nWords){
		nWords = _nWords;
	}

//	__inline__ result_type operator()(const vt::BRISKFeature& a, const vt::BRISKFeature& b) const
//	{
//		return (result_type)popcnt::ssse3_popcntofXORed(a._signature, b._signature, a._size);
//	}
//	__inline__ result_type operator()(const vt::BRISKFeature& a, const vt::BRISKFeature& b, result_type max) const
//	{
//		return (result_type)popcnt::ssse3_popcntofXORed(a._signature, b._signature, a._size, max);
//	}
	__inline__ result_type operator()(const __m128i* a, const __m128i* b) const
	{
		return (result_type)popcnt::ssse3_popcntofXORed_intel(a, b, nWords);
	}
	__inline__ result_type operator()(__m128i*& a, __m128i*& b, result_type& max) const
	{
		return (result_type)popcnt::ssse3_popcntofXORed_intel(a, b, nWords);
	}

};
/// @todo Version for raw data pointers that knows the size of the feature
/// @todo Specialization for cv::Vec. Doesn't have size() so default won't work.

///// Specialization for Eigen3::Matrix types.
//template<typename Scalar, int Rows, int Cols, int Options, int MaxRows, int MaxCols>
//struct L2< Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> >
//{
//	typedef Eigen::Matrix<Scalar, Rows, Cols, Options, MaxRows, MaxCols> feature_type;
//	typedef Scalar value_type;
//	typedef typename Accumulator<Scalar>::type result_type;
//
//	result_type operator()(const feature_type& a, const feature_type& b) const
//	{
//		return (a - b).squaredNorm();
//	}
//};

} //namespace distance

#endif
