/*
 * test_sort_comp.cc
 *
 *  Created on: Jan 25, 2012
 *      Author: slynen
 */

#include <gtest/gtest.h>
#include <slynen_tools/sort_comp.h>
#include <string.h>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;


TEST(sort_comp, numeric_string_compare){

	std::string s1 = "s423.jpg";
	std::string s2 = "s123";
	std::string s3 = "s23.jpg";
	std::string s4 = "sd2023.png";
	std::string s5 = "123";
	std::string s6 = "asdg133.doc";
	EXPECT_TRUE(numeric_string_compare(s1, s2));
	EXPECT_TRUE(numeric_string_compare(s1, s3));
	EXPECT_TRUE(numeric_string_compare(s2, s3));
	EXPECT_TRUE(numeric_string_compare(s4, s3));
	EXPECT_TRUE(numeric_string_compare(s4, s2));
	EXPECT_TRUE(numeric_string_compare(s4, s1));
	EXPECT_TRUE(numeric_string_compare(s5, s3));
	EXPECT_TRUE(numeric_string_compare(s6, s2));
	EXPECT_TRUE(numeric_string_compare(s6, s3));
	EXPECT_TRUE(numeric_string_compare(s4, s6));

	std::string p1;
	std::string p2;
	std::string base = "/home/user/docs/datasets/image_and_49_to_be_tested";
	std::string ext = ".jpg";
	p1 = base + boost::lexical_cast<string>(0) + ext;
	for(int i = 1;i<10000;++i){
		p2 = base + boost::lexical_cast<string>(i) + ext;
		EXPECT_TRUE(numeric_string_compare(p2, p1));
		p1 = p2;
	}
}

// Run all the tests that were declared with TEST()
int main(int argc, char **argv){
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
