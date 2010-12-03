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

#ifndef COMPLEX_HPP
#define COMPLEX_HPP
#include <kappa/agent.hpp>
#include <kappa/bond_map.hpp>

namespace kappa
{

  // Complex is defined in bond_map.hpp
  typedef std::set<Complex>
      ComplexSet;


  template <typename Expr>
  Complex get_complex(
      Agent & agent,
      Expr & expr,
      std::unordered_map<Agent *, bool> & visited_agents
      )
  {
    Complex neighbours = Complex(1, &agent);
    visited_agents[ &agent ] = true;
    BOOST_FOREACH( Site & s, agent.interface )
    {
      if ( is_bond_label(s.binding_state) )
      {
        Agent & nb = binding_partner(agent, s.binding_state, expr);
        if ( visited_agents.find(&nb) == visited_agents.end() )
        {
          BOOST_FOREACH( Agent * nnb, get_complex(nb, expr, visited_agents) )
            neighbours.push_back(nnb);
        }
      }
    }
    return neighbours;
  }


  template <typename Expr>
  Complex get_complex(
      Agent & agent,
      Expr & expr
      )
  {
    std::unordered_map<Agent *, bool> visited_agents;
    return get_complex(agent, expr, visited_agents);
  }


  template <typename Expr>
  ComplexSet get_complexes(
      Expr & expr
      )
  {
    ComplexSet complexes;
    std::unordered_map<Agent *, bool> visited_agents;

    BOOST_FOREACH( Agent & a, expr )
      if ( visited_agents.find(&a) == visited_agents.end() )
        complexes.insert( get_complex(a, expr, visited_agents) );

    return complexes;
  }


  enum ComplexOutputFormat
  {
    AgentsAndPtrs = 0,
    JustPtrs = 1,
    AsExpr = 2
  };


  std::string to_string(
      const Complex & c,
      ComplexOutputFormat format = AgentsAndPtrs
      )
  {
    if ( c.empty() )
    {
      return "";
    }
    if (format == AsExpr)
    {
      std::string output;
      BOOST_FOREACH( const Agent * a, c )
      {
        output += to_string(*a) + ",";
      }
      output.erase( output.size() - 1 );
      return output;
    }
    else if (format == AgentsAndPtrs)
    {
      std::string output = "{";
      BOOST_FOREACH( Agent * a, c )
        output += " " + to_string(*a) + "[" + to_string(a) + "]";
      return output + " }";
    }
    else
    {
      std::string output = "{";
      BOOST_FOREACH( Agent * a, c )
        output += " " + to_string(a);
      return output + " }";
    }
  }


  template <typename Expr>
  Expr minimize_bond_labels(
      Expr & expr
      );


  typedef std::unordered_map<std::string, long long int>
      ComplexCount;


  template <typename Expr>
  ComplexCount get_complex_count(
      const Expr & expr,
      bool minimize_labels = false
      )
  {
    ComplexCount cc;
    ComplexSet expr_complexes = get_complexes(const_cast<Expr &>(expr));
    BOOST_FOREACH( const Complex & c, expr_complexes )
    {
      std::string s = to_string(c, AsExpr);
      if ( cc.find(s) == cc.end() )
        cc[ s ] = 1;
      else
        ++(cc[ s ]);
    }
    return cc;
  }

} // kappa

#endif // COMPLEX_HPP
