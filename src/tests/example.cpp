#define BOOST_TEST_MODULE Example Test
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(test_one){
    int i = 1;
    BOOST_TEST(i);
    BOOST_TEST(i == 2);
}

