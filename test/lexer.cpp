#include <kappa/lexer.hpp>
#define BOOST_TEST_MODULE lexer
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( lexer_test )
{ 
  using namespace kappa;
  typedef Rule<Expression> rule;

  std::string site_str = "s~u!2";
  Site s = from_string<Site>(site_str);
  BOOST_CHECK_EQUAL( to_string(s), site_str );

  std::string agent_str = "A(s~u!2,a!1)";
  Agent a = from_string<Agent>(agent_str);
  BOOST_CHECK_EQUAL( to_string(a), agent_str );

  std::string expr_str = "A(a!1),A(a!1),B,C(s~u)";
  Expression e1 = from_string<Expression>(expr_str);
  BOOST_CHECK_EQUAL( e1.size(), size_t(4) );
  BOOST_CHECK_EQUAL( valid_expression(e1), true );
  BOOST_CHECK_EQUAL( to_string(e1), expr_str );

  std::string rule_str = "A(a!1),A(a!1),B,C(s~u) -> A(a),A(a),B,C(s~u) [0.001]";
  rule r = from_string<rule>(rule_str);
  BOOST_CHECK_EQUAL( to_string(r), rule_str );
  BOOST_CHECK_EQUAL( to_string(r.left_hand_side), "A(a!1),A(a!1),B,C(s~u)" );
  BOOST_CHECK_EQUAL( to_string(r.right_hand_side), "A(a),A(a),B,C(s~u)" );
}
