#include <kappa/agent.hpp>
#define BOOST_TEST_MODULE agents
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( agents_test )
{ 
  using namespace kappa;

  Agent a1("test_agent1");
  BOOST_CHECK_EQUAL( a1.name, "test_agent1" );
  BOOST_CHECK_EQUAL( a1.interface.size(), size_t(0) );
  BOOST_CHECK_EQUAL( to_string(a1), "test_agent1" );

  a1.add_site( Site("test_site1") );
  Site & s1 = *a1.find_site("test_site1");
  BOOST_CHECK_EQUAL( s1.name, "test_site1" );
  BOOST_CHECK_EQUAL( s1.binding_state, "" );
  BOOST_CHECK_EQUAL( s1.internal_state, "" );

  std::vector<Site> if1;
  if1.push_back( s1 );
  Site s2 = Site("test_site2", "gdp", "");
  if1.push_back( s2 );

  Agent a2("test_agent2", if1);
  BOOST_CHECK_EQUAL( a2.name, "test_agent2" );
  BOOST_CHECK_EQUAL( a2.interface.size(), size_t(2) );
  BOOST_CHECK_EQUAL( to_string(a2), "test_agent2(test_site1,test_site2~gdp)" );

  s1.internal_state = "u";
  BOOST_CHECK_EQUAL( a2.find_site("test_site1")->internal_state, "" );
  BOOST_CHECK_EQUAL( a1.find_site("test_site1")->internal_state, "u" );
}
