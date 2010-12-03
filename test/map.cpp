#include <kappa/expression.hpp>
#include <kappa/map.hpp>
#include <kappa/lexer.hpp>
#define BOOST_TEST_MODULE map
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( map_test )
{ 
  using namespace kappa;
  typedef Rule<Expression> rule;
  typedef std::vector<rule> RuleSet;
  typedef ActivationMap<rule>::Type ActivationMap;
  typedef InhibitionMap<rule>::Type InhibitionMap;

  // 1 //
  RuleSet rules;
  rules.push_back( from_string<rule>("A(a!1),A(a!1) -> A(a),A(a) [0.1]") );
  rules.push_back( from_string<rule>("A(a!1),A(a!1),B(c) -> A(a!1),(a!1),B(c~p) [0.1]") );
  rules.push_back( from_string<rule>("A(a),C(a) -> A(a!1),C(a!1) [0.1]") );
  std::cout << "rule 1: " << &rules[ 0 ] << std::endl; // DEBUG
  std::cout << "rule 2: " << &rules[ 1 ] << std::endl; // DEBUG
  std::cout << "rule 3: " << &rules[ 2 ] << std::endl; // DEBUG

  ActivationMap ram;
  InhibitionMap rim;
  boost::tie(ram, rim) = compute_activation_inhibition_map(rules);
  BOOST_CHECK( ram[ &rules[0] ].size() != 0  );
  if ( ram[ &rules[0] ].size() != 0 )
  {
    BOOST_CHECK_EQUAL( ram[ &rules[0] ][0], &rules[2] );
    std::cout << "Activation Map:" << std::endl;
    BOOST_FOREACH( ActivationMap::value_type & i, ram )
    {
      std::cout << to_string(i.first) << " ->";
      BOOST_FOREACH( ActivationMap::value_type::second_type::value_type & j, i.second )
        std::cout << " " << to_string(j);
      std::cout << std::endl;
    }
  }

  BOOST_CHECK( rim[ &rules[0] ].size() != 0 );
  if ( rim[ &rules[0] ].size() != 0 )
  {
    BOOST_CHECK_EQUAL( rim[ &rules[0] ][0], &rules[1] ); // TODO Problem here!!
    std::cout << "Inhibition Map:" << std::endl;
    BOOST_FOREACH( InhibitionMap::value_type & i, rim )
    {
      std::cout << to_string(i.first) << " ->";
      BOOST_FOREACH( InhibitionMap::value_type::second_type::value_type & j, i.second )
        std::cout << " " << to_string(j);
      std::cout << std::endl;
    }
  }

  // 2 //
  typedef MatchingMap<rule>::Type MatchingMap;
  typedef MatchingMap::value_type MatchingMapItem;
  typedef MatchingMapItem::second_type ComplexMatchings;
  typedef ComplexMatchings::iterator ComplexMatchingsIter;

  Expression e1 = from_string<Expression>("A(a!1),A(a!1),B(c),C(a)");
  ComplexSet mixture_complexes = get_complexes(e1);
  MatchingMap mm( compute_matching_map(rules, e1) );

  const Complex * c1 = NULL,
                * c2 = NULL,
                * c3 = NULL;
  BOOST_FOREACH( const Complex & c, mixture_complexes )
  {
    BOOST_FOREACH( const Agent * a, c )
    {
      Expression::iterator i = e1.begin();
      if ( a == &*(i++) )
      {
        c1 = &c;
        break;
      }
      if ( a == &*(++i) )
      {
        c2 = &c;
        break;
      }
      if ( a == &*(++i) )
      {
        c3 = &c;
        break;
      }
    }
  }

  BOOST_CHECK( c1 != NULL );
  BOOST_CHECK( c2 != NULL );
  BOOST_CHECK( c3 != NULL );

  // check for first rule
  ComplexMatchingsIter cmi = mm[ &rules[0] ].begin(); // first: rule's Complex, second: mixture's ComplexSet
  BOOST_CHECK( *((*cmi).second.begin()) == *c1 );

  // check for second rule
  for (cmi = mm[ &rules[1] ].begin(); cmi != mm[ &rules[1] ].end(); ++cmi)
    BOOST_CHECK( *((*cmi).second.begin()) == *c1 or
                 *((*cmi).second.begin()) == *c2 );

  // check for third rule
  for (cmi = mm[ &rules[2] ].begin(); cmi != mm[ &rules[1] ].end(); ++cmi)
    BOOST_CHECK( (*cmi).second.size() == 0 or *((*cmi).second.begin()) == *c3 );
}
