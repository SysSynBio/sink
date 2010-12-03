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

#ifndef LEXER_HPP
#define LEXER_HPP
#include <kappa/rule.hpp>
#include <boost/tokenizer.hpp>
//#include <boost/regex.hpp>

namespace kappa
{
  /*
  bool valid_site_string(
      const std::string & str
      )
  {
  }

  bool valid_agent_string(
      const std::string & str
      )
  {
  }

  bool valid_expression_string(
      const std::string & str
      )
  {
  }

  bool valid_rule_string(
      const std::string & str
      )
  {
  }
  */

  // from_string specializations for Site, Agent, Expression and Rule
  template <>
  inline Site from_string<Site>(
      const std::string & str
      )
  {
    std::string name, binding_state, internal_state;
    int selector = 0; // 0 = name, 1 = internal_state, 2 = binding_state
    BOOST_FOREACH( const char & c, str )
    {
      if (c == '~')
      {
        selector = 1;
      }
      else if (c == '!')
      {
        selector = 2;
      }
      else if (c == ' ')
      {
        continue;
      }
      else
      {
        if (selector == 0)
          name += c;
        else if (selector == 1)
          internal_state += c;
        else if (selector == 2)
          binding_state += c;
      }
    }
    return Site(name, internal_state, binding_state);
  }

  template <>
  inline Agent from_string<Agent>(
      const std::string & str
      )
  {
    Agent::Interface intf;
    std::string name, intf_str;
    int selector = 0; // 0 = name, 1 = interface
    BOOST_FOREACH( const char & c, str )
    {
      if (c == '(')
      {
        selector = 1;
      }
      else if (c == ')' or c == ' ')
      {
        continue;
      }
      else
      {
        if (selector == 0)
        {
          name += c;
        }
        else
        {
          intf_str += c;
        }
      }
    }

    typedef boost::tokenizer< boost::char_separator<char> > 
        tokenizer;
    boost::char_separator<char> sep(",");
    tokenizer tok(intf_str, sep);

    BOOST_FOREACH( const std::string & toki, tok )
      intf.push_back( from_string<Site>(toki) );

    return Agent(name, intf);
  }

  template <>
  inline Expression from_string<Expression>(
      const std::string & str
      )
  {
    Expression expr;
    std::string buf;
    int depth = 0;
    BOOST_FOREACH( const char & c, str )
    {
      if ( c == '(' )
      {
        buf += c;
        ++depth;
      }
      else if ( c == ')' )
      {
        buf += c;
        --depth;
      }
      else if ( depth == 0 and (c == ',' or c == '+') )
      {
        expr.push_back( from_string<Agent>(buf) );
        buf.clear();
      }
      else
      {
        buf += c;
      }
    }

    if ( not buf.empty() )
      expr.push_back( from_string<Agent>(buf) );

    int last_bond_label = 0;
    expr.bond_map = construct_bond_map(expr, &last_bond_label);
    expr.next_bond_label = last_bond_label + 1;

    return expr;
  }

  template <>
  inline Rule<Expression> from_string< Rule<Expression> >(
      const std::string & str
      )
  {
    Expression lhs, rhs;
    double k = 0.0;

    //boost::regex e("([a-zA-Z0-9_()~! ,]*)->([a-zA-Z0-9_()~! ,]*)\\[([\\d.Ee-]*)\\]");
    //boost::smatch m;
    //if ( boost::regex_match(str, m, e) )
    std::string lhs_str, rhs_str, k_str;
    int part = 0, // lhs
        skip = 0,
        i = 0;
    BOOST_FOREACH(char c, str)
    {
      if (skip == 1)
        skip = 0;
      else if (c == '-' and str[i+1] == '>')
        skip = part = 1; // rhs
      else if (c == '[')
        part = 2; // k
      else if (part == 0)
        lhs_str += c;
      else if (part == 1)
        rhs_str += c;
      else if (part == 2)
        k_str += c;
      else
        throw std::runtime_error("Malformatted input string for from_string< Rule<Expression> > [" + str + "].");
      ++i;
    }
    lhs = from_string<Expression>( lhs_str );
    rhs = from_string<Expression>( rhs_str );
    k = from_string<double>( k_str );

    return Rule<Expression>(lhs, rhs, k);
  }

} // kappa

#endif // LEXER_HPP
