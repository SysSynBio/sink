#include <kappa/rule.hpp>
#include <kappa/lexer.hpp>
#define BOOST_TEST_MODULE rule
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( rule_test )
{ 
  using namespace kappa;
  typedef Rule<Expression> rule;

  rule r1 = from_string<rule>( "A(a!1),A(a!1),B,C(s~u) -> A(a),A(a),B,C(s~u) [0.001]" );

  int modify_internal_state_cnt = 0, bind_cnt = 0, unbind_cnt = 0,
      create_cnt = 0, destroy_cnt = 0;

  BOOST_FOREACH( const rule::ElementaryAction & ea, r1.elementary_actions() )
  {
    if ( ea.target_type() == typeid(ModifyInternalState) )
    {
      const ModifyInternalState * ea_ptr = (const ModifyInternalState *)(ea.target<const ModifyInternalState>());
      std::cout << "Modify Intenal State: agent = " << to_string(ea_ptr->agent) << ", site name = " << ea_ptr->site_name
                << ", new internal state = " << ea_ptr->new_internal_state << std::endl;
      ++modify_internal_state_cnt;
    }
    else if ( ea.target_type() == typeid(Bind) )
    {
      const Bind * ea_ptr = (const Bind *)(ea.target<Bind>());
      std::cout << "Bind: agents = " << to_string(ea_ptr->agents.first) << " -- " << to_string(ea_ptr->agents.second)
                << ", site names = " << ea_ptr->site_names.first << " -- " << ea_ptr->site_names.second << std::endl;
      ++bind_cnt;
    }
    else if ( ea.target_type() == typeid(Unbind) )
    {
      const Unbind * ea_ptr = (const Unbind *)(ea.target<Unbind>());
      if ( ea_ptr->agents.second.name.empty() )
        std::cout << "Unbind: agent = " << to_string(ea_ptr->agents.first)
                  << ", site name = " << ea_ptr->site_names.first << std::endl;
      else
        std::cout << "Unbind: agents = " << to_string(ea_ptr->agents.first) << " -|- " << to_string(ea_ptr->agents.second)
                  << ", site names = " << ea_ptr->site_names.first << " -|- " << ea_ptr->site_names.second << std::endl;
      ++unbind_cnt;
    }
    else if ( ea.target_type() == typeid(Create) )
    {
      const Create * ea_ptr = (const Create *)(ea.target<Create>());
      std::cout << "Create: agent = " << to_string(ea_ptr->agent) << std::endl;
      ++create_cnt;
    }
    else if ( ea.target_type() == typeid(Destroy) )
    {
      const Destroy * ea_ptr = (const Destroy *)(ea.target<Destroy>());
      std::cout << "Destroy: agent = " << to_string(ea_ptr->agent) << std::endl;
      ++destroy_cnt;
    }
  }

  BOOST_CHECK_EQUAL( modify_internal_state_cnt, 0 );
  BOOST_CHECK_EQUAL( bind_cnt, 0 );
  BOOST_CHECK_EQUAL( unbind_cnt, 1 );
  BOOST_CHECK_EQUAL( create_cnt, 0 );
  BOOST_CHECK_EQUAL( destroy_cnt, 0 );
}
