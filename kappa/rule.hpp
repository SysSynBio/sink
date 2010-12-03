/*
 * Sink - Open-Source Implementation of Kappa
 * Copyright (C) 2009  Ricardo Honorato Zimmer [rikardo.horo@gmail.com]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef RULE_HPP
#define RULE_HPP
#include <kappa/expression.hpp>
#include <kappa/complex.hpp>
#include <kappa/pattern.hpp>

namespace kappa
{
  long factorial (int num)
  {
    long result = 1;
    for (int i = 1; i <= num; ++i)
      result *= i;
    return result;
  }


  typedef std::pair<Agent *, std::string>
      AgentSitePair;
  typedef std::vector<AgentSitePair>
      ModifiedSet;


  struct ModifyInternalState
  {
    /*
    ModifyInternalState() // default ctor
      : agent(), site_name(), new_internal_state() { }
    */

    ModifyInternalState(const Agent & a, const Site & s, const std::string & new_is)
      : agent(a), site_name(s.name), new_internal_state(new_is) { }

    template <typename Expr> //, typename ComplexSet>
    ModifiedSet operator()(Expr &, ComplexSet & complex_set)
    {
      if ( site_name.empty() or agent.name.empty() )
        throw std::runtime_error("ModifyInternalState: Site name or agent was not specified");

      BOOST_FOREACH( const Complex & c, complex_set )
        BOOST_FOREACH( Agent * a, c )
          if ( match_agent(agent, *a) )
          {
            modify_internal_state( new_internal_state, *(a->find_site( site_name )) );
            return ModifiedSet(1, std::make_pair( a, site_name ));
          }
      return ModifiedSet();
    }

    Agent agent;
    std::string site_name;
    std::string new_internal_state;
  };

  struct Bind
  {
    /*
    Bind()
      : agents(), site_names() { }
    */

    Bind(const Agent & a1, const Site & s1, const Agent & a2, const Site & s2)
      : agents(a1, a2), site_names(s1.name, s2.name) { }

    template <typename Expr>
    ModifiedSet operator()(Expr & expr, ComplexSet & complex_set)
    {
      if ( agents.first.name.empty() or agents.second.name.empty() or
           site_names.first.empty() or site_names.second.empty() )
        throw std::runtime_error("Bind: Site names or agents were not specified");

      Agent * a1 = NULL,
            * a2 = NULL;
      BOOST_FOREACH( const Complex & c, complex_set )
      {
        BOOST_FOREACH( Agent * a, c )
        {
          if ( match_agent(agents.first, *a) )
            a1 = a;

          if ( match_agent(agents.second, *a) )
            a2 = a;
        }
      }

      if ( a1 == NULL or a2 == NULL )
        throw std::runtime_error("Bind: agents were not found in expression.");

      bind_agents( *a1, site_names.first, *a2, site_names.second, expr );
      ModifiedSet s(2);
      s[0] = std::make_pair(a1, site_names.first);
      s[1] = std::make_pair(a2, site_names.second);
      return s;
    }

    std::pair<Agent, Agent> agents;
    std::pair<std::string, std::string> site_names;
  };

  struct Unbind
  {
    /*
    Unbind()
      : agents(), site_names() { }
    */

    Unbind(const Agent & a1, const Site & s1, const Agent & a2, const Site & s2)
      : agents(a1, a2), site_names(s1.name, s2.name) { }

    Unbind(const Agent & a1, const Site & s1)
      : agents(a1, Agent("")), site_names(s1.name, "") { }

    template <typename Expr>
    ModifiedSet operator()(Expr & expr, ComplexSet & complex_set)
    {
      if ( agents.second.name.empty() or site_names.second.empty() )
      { // partial agent unbinding
        BOOST_FOREACH( const Complex & c, complex_set )
          BOOST_FOREACH( Agent * a1, c )
            if ( match_agent(agents.first, *a1) )
            {
              ModifiedSet s(2);
              s[0] = std::make_pair(a1, site_names.first);
              s[1] = unbind_agent(*a1, site_names.first, expr);
              return s;
            }
      }
      else if ( agents.first.name.empty() or site_names.first.empty() )
      {
        throw std::runtime_error("Unbind: Site names or agents were not specified");
      }

      Agent * a1 = NULL,
            * a2 = NULL;
      BOOST_FOREACH( const Complex & c, complex_set )
      {
        BOOST_FOREACH( Agent * a, c )
        {
          if ( match_agent(agents.first, *a) )
          {
            a1 = a;
            if (a2 != NULL)
              break;
          }

          if ( match_agent(agents.second, *a) )
          {
            a2 = a;
            if (a1 != NULL)
              break;
          }
        }
      }

      if ( a1 == NULL or a2 == NULL )
        throw std::runtime_error("Unbind: agents were not found in expression.");

      unbind_agents( *a1, site_names.first, *a2, site_names.second, expr );
      ModifiedSet s(2);
      s[0] = std::make_pair(a1, site_names.first);
      s[1] = std::make_pair(a2, site_names.second);
      return s;
    }

    std::pair<Agent, Agent> agents;
    std::pair<std::string, std::string> site_names;
  };

  struct Create
  {
    /*
    Create()
      : agent() { }
    */

    Create(const Agent & a)
      : agent(a) { }

    template <typename Expr>
    ModifiedSet operator()(Expr & expr, ComplexSet &)
    {
      if ( agent.name.empty() )
        throw std::runtime_error("Create: Agent was not specified");
      return ModifiedSet(1, std::make_pair( &create_agent(agent, expr), "") );
    }

    Agent agent;
  };
  
  struct Destroy
  {
    /*
    Destroy()
      : agent() { }
    */

    Destroy(const Agent & a)
      : agent(a) { }

    template <typename Expr>
    ModifiedSet operator()(Expr & expr, ComplexSet & complex_set)
    {
      if ( agent.name.empty() )
        throw std::runtime_error("Destroy: Agent was not specified");

      BOOST_FOREACH( const Complex & c, complex_set )
        BOOST_FOREACH( Agent * a, c )
          if ( match_agent(agent, *a) )
          {
            destroy_agent(*a, expr);
            return ModifiedSet(1, std::make_pair( a, "" ));
          }
      return ModifiedSet();
    }

    Agent agent;
  };


  template <typename Expr>
  struct Rule
  {
    typedef Expr Expression;
    typedef typename boost::function<ModifiedSet (Expr &, ComplexSet &)> ElementaryAction;
    typedef typename std::list<ElementaryAction> ElementaryActionSet;
    typedef typename std::set<Agent *> AgentPtrSet;
    typedef typename std::pair<AgentPtrSet, AgentPtrSet> ModifiedAgents;
    //typedef typename std::vector<std::string> SiteSet;

    Rule(const Expr & lhs, const Expr & rhs, double kc)
      : left_hand_side(lhs), right_hand_side(rhs), kinetic_constant(kc),
        eas(), _complexes( get_complexes(left_hand_side) ), _modified_agents()
    {
      outer_automorphisms = count_automorphisms_between_components();
      compute_elementary_actions();
    }

    Rule() // default ctor
      : left_hand_side(), right_hand_side(), kinetic_constant(0.0),
        outer_automorphisms(0), eas(), _complexes(), _modified_agents() { }

    Rule(const Rule<Expr> & other) // copy ctor
      : left_hand_side( other.left_hand_side ),
        right_hand_side( other.right_hand_side ),
        kinetic_constant( other.kinetic_constant ),
        outer_automorphisms( other.outer_automorphisms ),
        eas( other.eas ),
        _complexes( get_complexes(left_hand_side) ), // is there a cheaper way to do this?
        _modified_agents()
    {
        // copy modified agents pointers
        std::unordered_map<const Agent *, Agent *> agent_map;
        typename Expr::iterator i = left_hand_side.begin();
        typename Expr::const_iterator other_i = other.left_hand_side.begin(),
                                      other_end = other.left_hand_side.end();
        for (; other_i != other_end; ++other_i, ++i)
          agent_map[ &*other_i ] = &*i;
        BOOST_FOREACH( Agent * a, other._modified_agents.first )
          _modified_agents.first.insert( agent_map[a] );

        agent_map.clear();
        i = right_hand_side.begin();
        other_i = other.right_hand_side.begin(),
        other_end = other.right_hand_side.end();
        for (; other_i != other_end; ++other_i, ++i)
          agent_map[ &*other_i ] = &*i;
        BOOST_FOREACH( Agent * a, other._modified_agents.second )
          _modified_agents.second.insert( agent_map[a] );
    }

    Rule<Expr> & operator=(const Rule<Expr> & other) // assignment operator
    {
      if (this != &other)
      {
        left_hand_side = other.left_hand_side;
        right_hand_side = other.right_hand_side;
        kinetic_constant = other.kinetic_constant;
        outer_automorphisms = other.outer_automorphisms;
        eas = other.eas;
        _complexes = get_complexes(left_hand_side);

        // copy modified agents pointers
        _modified_agents.first.clear(); _modified_agents.second.clear();
        std::unordered_map<const Agent *, Agent *> agent_map;
        typename Expr::iterator i = left_hand_side.begin();
        typename Expr::const_iterator other_i = other.left_hand_side.begin(),
                                      other_end = other.left_hand_side.end();
        for (; other_i != other_end; ++other_i, ++i)
          agent_map[ &*other_i ] = &*i;
        BOOST_FOREACH( Agent * a, other._modified_agents.first )
          _modified_agents.first.insert( agent_map[a] );

        agent_map.clear();
        i = right_hand_side.begin();
        other_i = other.right_hand_side.begin(),
        other_end = other.right_hand_side.end();
        for (; other_i != other_end; ++other_i, ++i)
          agent_map[ &*other_i ] = &*i;
        BOOST_FOREACH( Agent * a, other._modified_agents.second )
          _modified_agents.second.insert( agent_map[a] );
      }
      return *this;
    }

    // Member functions
    const ElementaryActionSet & elementary_actions() const
    { return eas; }

    const ModifiedAgents & modified_agents() const
    { return _modified_agents; }

    const ComplexSet & complexes() const
    { return _complexes; }

  private:
    void compute_elementary_actions()
    {
      typedef std::unordered_map< Agent *, Agent * > AgentMap;
      AgentMap rhs_to_lhs;

      // map rhs agents to lhs agents
      BOOST_FOREACH( Agent & lhs_agent, left_hand_side )
      {
        bool agent_found = false;
        BOOST_FOREACH( Agent & rhs_agent, right_hand_side )
        {
          if ( lhs_agent.name == rhs_agent.name and
               rhs_to_lhs.find( &rhs_agent ) == rhs_to_lhs.end() )
          {
            rhs_to_lhs[ &rhs_agent ] = &lhs_agent;
            agent_found = true; break;
          }
        }

        if (not agent_found)
        { // agent deletion
          eas.push_back( Destroy(lhs_agent) );
          _modified_agents.first.insert( &lhs_agent );
        }
      }

      typedef std::pair<Agent *, std::string> AgentSite;
      typedef std::unordered_map<std::string, AgentSite> BondLabel2AgentSite;
      BondLabel2AgentSite to_bind, to_unbind;

      BOOST_FOREACH( Agent & rhs_agent, right_hand_side )
      {
        if ( rhs_to_lhs.find( &rhs_agent ) == rhs_to_lhs.end() )
        { // agent creation
          eas.push_back( Create(rhs_agent) );
          _modified_agents.second.insert( &rhs_agent );
        }
        else
        {
          Agent & lhs_agent = *rhs_to_lhs[ &rhs_agent ];
          BOOST_FOREACH( Site & rs, rhs_agent.interface )
          {
            Agent::Iiter ls_it = lhs_agent.find_site(rs.name);
            if ( ls_it == lhs_agent.interface.end() )
              continue; // this site is mentioned in rhs only for completeness, it has not been modified
            Site & ls = *ls_it;
            if (rs.internal_state != ls.internal_state)
            { // internal state modification
              eas.push_back( ModifyInternalState(lhs_agent, ls, rs.internal_state) );
              _modified_agents.first.insert( &lhs_agent );
              _modified_agents.second.insert( &rhs_agent );
            }

            if (rs.binding_state != ls.binding_state)
            {
              if ( ls.binding_state.empty() and is_bond_label(rs.binding_state) )
              { // binding
                if ( to_bind.find( rs.binding_state ) == to_bind.end() )
                {
                  to_bind[ rs.binding_state ] = std::make_pair(&lhs_agent, ls.name);
                  _modified_agents.second.insert( &rhs_agent );
                }
                else
                {
                  Agent * partner = NULL; std::string partner_site;
                  boost::tie(partner, partner_site) = to_bind[ rs.binding_state ];
                  eas.push_back( Bind(lhs_agent, ls.name, *partner, partner_site) );

                  _modified_agents.first.insert( &lhs_agent );
                  _modified_agents.first.insert( partner );
                  _modified_agents.second.insert( &rhs_agent );
                }
              }
              else if ( (is_bond_label(ls.binding_state) or ls.binding_state == "-" or ls.binding_state == "?") and
                        rs.binding_state.empty() )
              { // unbinding
                if ( is_bond_label(ls.binding_state) and to_unbind.find( ls.binding_state ) == to_unbind.end() )
                {
                  to_unbind[ ls.binding_state ] = std::make_pair(&lhs_agent, ls.name);
                  _modified_agents.second.insert( &rhs_agent );
                }
                else if ( ls.binding_state == "-" or ls.binding_state == "?" )
                {
                  eas.push_back( Unbind(lhs_agent, ls.name) );
                  _modified_agents.first.insert( &lhs_agent );
                  _modified_agents.second.insert( &rhs_agent );
                }
                else
                {
                  Agent * partner = NULL; std::string partner_site;
                  boost::tie(partner, partner_site) = to_unbind[ ls.binding_state ];
                  eas.push_back( Unbind(lhs_agent, ls.name, *partner, partner_site) );

                  _modified_agents.first.insert( &lhs_agent );
                  _modified_agents.first.insert( partner );
                  _modified_agents.second.insert( &rhs_agent );
                }
              }
            } // rs.binding_state != ls.binding_state
          } // foreach Site & rs, rhs_agent.interface
        } // if &rhs_agent in rhs_to_lhs
      } // foreach Agent & rhs_agent, right_hand_side

      return;
    }

    int count_inner_automorphisms(Complex & c) const
    { return 1; }

    int count_automorphisms_between_components() const // for left hand side
    {
      int n = 1, i = 0; // complexes_size = _complexes.size();
      BOOST_FOREACH( const Complex & c1, _complexes )
      {
        int j = 0;
        BOOST_FOREACH( const Complex & c2, _complexes )
        {
          if (j++ <= i)
            continue;
          if ( match_complex( c1, left_hand_side, c2, left_hand_side ) or
               match_complex( c2, left_hand_side, c1, left_hand_side ) )
            ++n;
        }
        ++i;
      }
      return factorial(n);
    }

    // Data members
  public:
    Expr left_hand_side;
    Expr right_hand_side;
    double kinetic_constant;
    size_t outer_automorphisms;
  private:
    ElementaryActionSet eas;
    ComplexSet _complexes;
    ModifiedAgents _modified_agents;
  };

  typedef Rule<Expression> SimpleRule;

  typedef std::pair<SimpleRule, SimpleRule>
    ReversibleRule;

  // Validation function
  template <typename Expr>
  bool valid_rule(
      const Rule<Expr> & rule
      )
  {
    return valid_expression(rule.left_hand_side, true) and valid_expression(rule.right_hand_side, true);
  }

  // to_string specialization
  template <typename Expr>
  inline std::string to_string(
      const Rule<Expr> & rule
      )
  {
    return to_string(rule.left_hand_side) + " -> " + to_string(rule.right_hand_side) + " [" + to_string(rule.kinetic_constant) + "]" ;
  }

} // kappa

#endif // RULE_HPP
