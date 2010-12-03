#include <kappa/expression.hpp>
#include <kappa/complex.hpp>
#include <kappa/pattern.hpp>
#include <kappa/lexer.hpp>
#define BOOST_TEST_MODULE pattern
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( pattern_test )
{ 
  using namespace kappa;

  // 1 //
  Agent partial_agent = from_string<Agent>("A(a!-,b~p!1)");
  Agent complete_agent1 = from_string<Agent>("A(a!1,b~p!2)");
  Agent complete_agent2 = from_string<Agent>("A(a!1,b~p)");
  BOOST_CHECK_EQUAL( match_agent(partial_agent, complete_agent1), true );
  BOOST_CHECK_EQUAL( match_agent(partial_agent, complete_agent2), false );

  // 2 //
  Expression expr1 = from_string<Expression>("A(b!1,c!2),B(a!1,d~u),C(a!2)");
  Complex ec1 = *(get_complexes(expr1).begin());
  Expression pattern1 = from_string<Expression>("A(c!-,b!1),B(a!1)");
  Complex pc1 = *(get_complexes(pattern1).begin());
  BOOST_CHECK_EQUAL( match_complex(pc1, pattern1, ec1, expr1), true );
}
