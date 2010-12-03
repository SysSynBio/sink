#include <kappa/gillespie.hpp>
#define BOOST_TEST_MODULE gillespie
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( gillespie_test )
{ 
  using namespace kappa;

  // 1 //
  std::vector<double> activities(1);
  activities[0] = 2000; // TODO Problem here!! index out of range
  LinearGillespie gillespie(activities);
  int rule_index = 0; double dt = 0.0;
  boost::tie(rule_index, dt) = gillespie();
  BOOST_CHECK_EQUAL( rule_index, 0 );
  BOOST_CHECK_EQUAL( gillespie[0], 2000 );

  // check concerning the distribution of dt is missing
}
