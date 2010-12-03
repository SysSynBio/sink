#include <kappa/system.hpp>
#define BOOST_TEST_MODULE system
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( system_test )
{ 
  using namespace kappa;
  typedef Rule<Expression> rule;
  typedef std::vector<rule> RuleSet;
  typedef System<LinearGillespie, RuleSet> system;

  // 1 //
  std::string s1_str;
  s1_str = "init 10 X(a)\n";
  s1_str += "X -> Y(b) [0.1]\n";
  system s1 = read_system<LinearGillespie, RuleSet>(s1_str);
  std::cout << "Mixture:" << std::endl; int i = 0;
  BOOST_FOREACH( Agent & a, s1.mixture )
    std::cout << to_string(a) << " [" << &a << "] " << i++ << std::endl;
  std::cout << std::endl;
  std::cout << "t = " << s1.t << std::endl;
  std::cout << "mixture = " << to_string(s1.mixture) << std::endl;
  const int num_steps = 5;
  for (int i = 0; i < num_steps; ++i)
  {
    s1.iterate();
    std::cout << "t = " << s1.t << std::endl;
    std::cout << "mixture = " << to_string(s1.mixture) << std::endl;
  }

  // 2 //
  /*
  RuleSet rules;
  rules.push_back( from_string<rule>("A(a!1),A(a!1) -> A(a),A(a) [0.1]") );
  rules.push_back( from_string<rule>("A(a!1),A(a!1),B(c) -> A(a!1),(a!1),B(c~p) [0.1]") );
  rules.push_back( from_string<rule>("A(a),C(a) -> A(a!1),C(a!1) [0.1]") );

  Expression initial_mixture = from_string<Expression>("A(a!1),A(a!1),B(c),C(a)");

  System<LinearGillespie, RuleSet> system(initial_mixture, rules);
  */

  //BOOST_CHECK_EQUAL();
}
