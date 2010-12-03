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

#ifndef PATTERN_HPP
#define PATTERN_HPP
#include <kappa/agent.hpp>

namespace kappa
{

  // Pattern matching related functions
  bool is_binding_state_less_specific(
      std::string pattern_bs,
      std::string expr_bs
      )
  {
    if (pattern_bs == "?")
      return true;
    else if (pattern_bs == "" and expr_bs == "")
      return true;
    else if ( (pattern_bs == "-" or is_bond_label(pattern_bs)) and is_bond_label(expr_bs) )
      return true;
    else
      return false;
  }

  bool is_internal_state_less_specific(
      std::string pattern_is,
      std::string expr_is
      )
  {
    if (expr_is == "" and pattern_is != "")
      return false;
    return true;
    /*
    if (pattern_is == "")
      return true;
    else if (pattern_is == expr_is)
      return true;
    else
      return false;
    */
  }

  bool match_agent(
      const Agent & pattern_agent,
      const Agent & expr_agent
      )
  {
    if (pattern_agent.name != expr_agent.name)
      return false;

    BOOST_FOREACH( const Site & ps, pattern_agent.interface )
    {
      Agent::Iconst_iter i = expr_agent.find_site( ps.name );
      if ( i == expr_agent.interface.end() )
      {
        return false;
      }
      else
      {
        const Site & es = *i;
        if ( not (is_internal_state_less_specific(ps.internal_state, es.internal_state) or
                  ps.internal_state == es.internal_state) )
          return false;

        if ( not is_binding_state_less_specific(ps.binding_state, es.binding_state) )
          return false;
      }
    }

    return true;
  }


  namespace detail
  {

    template <typename E>
    bool check_neighbours(
        const Agent * Panchor,
        const E & pattern,
        const Agent * Eanchor,
        const E & expr,
        std::unordered_map<const Agent *, bool> & visited_agents
        )
    {
      visited_agents[ Panchor ] = true;
      BOOST_FOREACH( const Site & s_pattern, Panchor->interface )
      {
        if ( s_pattern.binding_state == "-" )
        {
          if ( not is_bond_label( Eanchor->find_site(s_pattern.name)->binding_state ) )
            return false;
        }
        else if ( is_bond_label(s_pattern.binding_state) and
                  visited_agents.find( &binding_partner(*Panchor, s_pattern, pattern) ) == visited_agents.end() )
        {
          BOOST_FOREACH( const Site & s_expr, Eanchor->interface )
          {
            if ( is_bond_label(s_expr.binding_state) )
            {
              Agent & Eanchor_nb = binding_partner( *Eanchor, s_expr, expr );
              Agent & Panchor_nb = binding_partner( *Panchor, s_pattern, pattern );
              if ( match_agent(Panchor_nb, Eanchor_nb) )
                if ( not check_neighbours(&Panchor_nb, pattern, &Eanchor_nb, expr, visited_agents) )
                  return false;
            }
          }
        }
      }
      return true;
    }

  } // detail


  template <typename E>
  bool match_complex(
      const Complex & pattern_complex,
      const E & pattern,
      const Complex & expr_complex,
      const E & expr
      )
  {
    using namespace detail;

    if ( pattern_complex.empty() )
      return true;
    else if ( pattern_complex.size() > expr_complex.size() )
      return false;

    // Choose an anchor agent for the pattern and search for it in the expression
    const Agent * Panchor = pattern_complex[0];
    std::vector<const Agent *> possible_expr_anchors;
    BOOST_FOREACH( const Agent * Eagent, expr_complex )
      if ( match_agent(*Panchor, *Eagent) )
        possible_expr_anchors.push_back( Eagent );

    if ( possible_expr_anchors.size() == 0 )
      return false;

#ifdef DEBUG
    std::cout << "Possible expression anchors:";
    BOOST_FOREACH( const Agent * Eanchor, possible_expr_anchors )
      std::cout << " " << to_string(*Eanchor);
    std::cout << std::endl;
#endif

    BOOST_FOREACH( const Agent * Eanchor, possible_expr_anchors )
    {
      std::unordered_map<const Agent *, bool> visited_agents;
      if ( check_neighbours( Panchor, pattern, Eanchor, expr, visited_agents ) == true )
        return true;
    }

    return false;
  }

} // kappa

#endif // PATTERN_HPP
