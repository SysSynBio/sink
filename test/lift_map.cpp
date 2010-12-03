#include <kappa/lift_map.hpp>
#include <kappa/lexer.hpp>
#define BOOST_TEST_MODULE lift_map
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( lift_map_test )
{ 
  using namespace kappa;
  using namespace std;
  typedef Rule<Expression> rule;
  typedef MatchingMap<rule>::Type MM;
  typedef LiftMap<rule>::Type LiftMap;
  typedef LiftMap::value_type::second_type AgentLiftMap;

  // 1 //
  Expression e1 = from_string<Expression>("X,X,X,X");
  rule r1 = from_string<rule>("X -> Y [1]");
  std::vector<rule> rule_set1(1, r1);
  MM mm1 = compute_matching_map(rule_set1, e1);
  LiftMap lm1 = compute_lift_map(e1, mm1);
  cout << "Lift map:" << endl;
  BOOST_FOREACH( LiftMap::value_type & i, lm1 )
  {
    const Agent * a = i.first;
    BOOST_FOREACH( const MM::value_type & j, i.second )
    {
      const rule * r = j.first;
      BOOST_FOREACH( const ComplexMatchings::value_type & k, j.second )
      {
        const Complex & c = k.first;
        const ComplexSet & matchings = k.second;
        cout << "  " << a << " [" << to_string(*a) << "] -> (" << r << ", " << to_string(c) << ", {";
        BOOST_FOREACH( const Complex & m, matchings )
          cout << " " << to_string(m);
        cout << " }" << endl;
      }
    }
  }
}
