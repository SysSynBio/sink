#include <kappa/expression.hpp>
#define BOOST_TEST_MODULE expression
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( expression_test )
{ 
  using namespace kappa;

  Expression e1;
  BOOST_CHECK_EQUAL( e1.size(), size_t(0) );
  BOOST_CHECK_EQUAL( valid_expression(e1), true );
  BOOST_CHECK_EQUAL( to_string(e1), "" );

  Agent a1("a1");
  a1.add_site( Site("s1", "gtp", "") );
  e1.push_back(a1);
  BOOST_CHECK_EQUAL( e1.size(), size_t(1) );
  BOOST_CHECK_EQUAL( e1[0].name, "a1" );
  BOOST_CHECK_EQUAL( e1[0].interface[0].name, "s1" );
  BOOST_CHECK_EQUAL( e1[0].interface[0].internal_state, "gtp" );
  BOOST_CHECK_EQUAL( e1[0].interface[0].binding_state, "" );
  BOOST_CHECK_EQUAL( to_string(e1), "a1(s1~gtp)" );

  a1.name = "a2";
  // since e1 has a copy of a1, changing the name of a1 shouldn't change the name of e1's a1 copy.
  BOOST_CHECK_EQUAL( e1[0].name, "a1" );

  Expression e2( a1 );
  BOOST_CHECK_EQUAL( e2.size(), size_t(1) );
  BOOST_CHECK_EQUAL( e2[0].name, "a2" );

  // test copy ctor
  Expression e3( e2 );
  BOOST_CHECK_EQUAL( e3.size(), size_t(1) );
  BOOST_CHECK_EQUAL( e3[0].name, "a2" );

  // test iterator-based ctor
  std::list<Agent> agent_list;
  a1.name = "a1";
  agent_list.push_back(a1);
  Agent a2("a2");
  a2.add_site( Site("s1") );
  agent_list.push_back(a2);
  Expression e4( agent_list.begin(), agent_list.end() );
  BOOST_CHECK_EQUAL( e4.size(), size_t(2) );
  BOOST_CHECK_EQUAL( e4[0].name, "a1" );
  BOOST_CHECK_EQUAL( e4[1].name, "a2" );

  Expression e5( agent_list );
  BOOST_CHECK_EQUAL( to_string(e4), to_string(e5) );
  BOOST_CHECK_EQUAL( to_string(e5), "a1(s1~gtp),a2(s1)" );

  // bind site s1 of agent a1 to site s1 of agent a2
  bind_agents( e5[ 0 ], "s1", e5[ 1 ], "s1", e5 );
  BOOST_CHECK_EQUAL( to_string(e5), "a1(s1~gtp!1),a2(s1!1)" );
  BOOST_CHECK_EQUAL( valid_expression(e5), true );
  BOOST_CHECK_EQUAL( e5.bond_map.size(), size_t(1) );
  Complex c1; c1.push_back(&e5[ 0 ]); c1.push_back(&e5[ 1 ]);
  BOOST_CHECK( e5.bond_map[ 1 ] == c1 );
  BOOST_FOREACH( BondMapItem label_and_agents, e5.bond_map )
  {
    BOOST_CHECK_EQUAL( label_and_agents.second.size(), size_t(2) );
  }
  Agent & a1_pair = binding_partner(e5[ 0 ], 1, e5);
  BOOST_CHECK_EQUAL( &a1_pair, &e5[ 1 ] );
  Agent & a2_pair = binding_partner(e5[ 1 ], 1, e5);
  BOOST_CHECK_EQUAL( &a2_pair, &e5[ 0 ] );

  // unbind them
  unbind_agents( e5[ 0 ], "s1", e5[ 1 ], "s1", e5 );
  BOOST_CHECK_EQUAL( to_string(e5), "a1(s1~gtp),a2(s1)" );
  BOOST_CHECK_EQUAL( valid_expression(e5), true );

  destroy_agent( e5[ 0 ], e5 );
  BOOST_CHECK_EQUAL( to_string(e5), "a2(s1)" );
  BOOST_CHECK_EQUAL( valid_expression(e5), true );

  create_agent( a1, e5 );
  BOOST_CHECK_EQUAL( to_string(e5), "a2(s1),a1(s1~gtp)" );
  BOOST_CHECK_EQUAL( valid_expression(e5), true );
  //BOOST_CHECK_EQUAL( );
}
