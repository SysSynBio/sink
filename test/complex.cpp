#include <kappa/complex.hpp>
#include <kappa/expression.hpp>
#include <kappa/lexer.hpp>
#define BOOST_TEST_MODULE _complex
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( _complex_test )
{ 
  using namespace kappa;

  Expression e1 = from_string<Expression>("A(a!1),A(a!1),B,C(s~u)");
  Agent & a1 = e1.find("A", bind(&Site::binding_state, bind(&Agent::interface, _1)[0]) == "1" );
  Complex c1 = get_complex(a1, e1);
  BOOST_CHECK_EQUAL( c1.size(), size_t(2) );
  BOOST_FOREACH( Agent * a_ptr, c1 )
  {
    BOOST_CHECK_EQUAL( a_ptr->name, "A" );
    BOOST_CHECK_EQUAL( a_ptr->interface[0].binding_state, "1" );
  }

  ComplexSet complexes1 = get_complexes(e1);
  BOOST_CHECK_EQUAL( complexes1.size(), size_t(3) );
  BOOST_FOREACH( const Complex & c, complexes1 )
  {
    BOOST_CHECK( c.size() == 1 or c.size() == 2 );
    if ( c.size() == 1 )
    {
      BOOST_CHECK( c[0]->name == "B" or c[0]->name == "C" );
    }
    else // c.size() == 2
    {
      BOOST_CHECK_EQUAL( c[0]->name, "A" );
      BOOST_CHECK_EQUAL( c[1]->name, "A" );
    }
  }
  //BOOST_CHECK_EQUAL( );
}
