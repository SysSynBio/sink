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

#ifndef BOND_MAP_HPP
#define BOND_MAP_HPP
#include <kappa/agent.hpp>

namespace kappa
{

  typedef std::vector<Agent *>
      AgentSet;
  typedef std::vector<Agent *>
      Complex;
  typedef std::unordered_map<int, Complex>
      BondMap;
  typedef std::pair<const int, Complex>
      BondMapItem;

  /// @brief Returns a mapping from each bond label to every pair of bounded agents in an expression.
  /// Used internally.
  template <typename Expr>
  BondMap construct_bond_map(
      Expr & expr,
      int * last_bond_label
      )
  {
    BondMap output;
    if ( last_bond_label != NULL )
      *last_bond_label = 0;

    BOOST_FOREACH( Agent & a, expr )
      BOOST_FOREACH( Site & s, a.interface )
        if ( is_bond_label(s.binding_state) )
        {
          if ( last_bond_label != NULL and *last_bond_label < from_string<int>(s.binding_state) )
            *last_bond_label = from_string<int>(s.binding_state);

          // check if s.binding_state already exists in output as a key
          BondMap::iterator i = output.find( from_string<int>(s.binding_state) );
          if ( i == output.end() )
            output[ from_string<int>(s.binding_state) ] = Complex(1, &a);
          else
            output[ from_string<int>(s.binding_state) ].push_back(&a);
        }

    return output;
  }

  /// @brief Returns the bond pair of a certain agent in an expression. The bond_map has to be made from the expression.
  template <typename Expr>
  inline Agent & binding_partner(
      const Agent & agent,
      int bond_label,
      Expr & expr
      )
  {
    BondMap::const_iterator i = expr.bond_map.find( bond_label );
    if ( i != expr.bond_map.end() )
    {
      if ( i->second.size() != 2 )
        throw std::runtime_error("Key " + to_string(bond_label) + " in bond map should be associated to two agents.");

      if ( i->second[ 0 ] == &agent )
        return *(i->second[ 1 ]);
      else if ( i->second[ 1 ] == &agent )
        return *(i->second[ 0 ]);
      else
        throw std::runtime_error("Agent '" + to_string(agent) + "' does not have any site participating in bond " + to_string(bond_label) + ".");
    }
    throw std::runtime_error("Could not find binding partner for agent '" + to_string(agent) + "'.");
  }

  template <typename Expr>
  inline Agent & binding_partner(
      const Agent & agent,
      const std::string & bond_label,
      Expr & expr
      )
  {
    return binding_partner(agent, from_string<int>(bond_label), expr);
  }

  template <typename Expr>
  inline Agent & binding_partner(
      const Agent & agent,
      const Site & site,
      Expr & expr
      )
  {
    if ( not is_bond_label(site.binding_state) )
      throw std::runtime_error("Agent '" + to_string(agent) + "' is not bound to anything at site '" + site.name + "'.");
    return binding_partner(agent, site.binding_state, expr);
  }

} // kappa

#endif // BOND_MAP_HPP
