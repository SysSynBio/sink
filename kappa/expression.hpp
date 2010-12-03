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

#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP
#include <kappa/agent.hpp>
#include <kappa/bond_map.hpp>
#include <kappa/complex.hpp>
#include <kappa/pattern.hpp>

namespace kappa
{

  struct Expression
    : public std::list<Agent>
  {
    typedef std::list<Agent> Base;
    typedef Agent agent;
    typedef Base::reference reference;
    typedef Base::const_reference const_reference;
    typedef Base::iterator iterator;
    typedef Base::iterator agent_iterator;
    typedef Base::const_iterator const_iterator;
    typedef Base::size_type size_type;
    typedef Base::difference_type difference_type;
    typedef Base::value_type value_type;
    typedef Base::allocator_type allocator_type;
    typedef Base::pointer pointer;
    typedef Base::const_pointer const_pointer;
    typedef Base::reverse_iterator reverse_iterator;
    typedef Base::const_reverse_iterator const_reverse_iterator;

    // Ctors
    Expression()
      : Base(), bond_map(), next_bond_label(1) { }

    Expression(size_type n, const Agent & value)
      : Base(n, value)
    {
      bond_map = construct_bond_map(*this, &next_bond_label);
      ++next_bond_label;
    }

    Expression(const Agent & value)
      : Base(1, value)
    {
      bond_map = construct_bond_map(*this, &next_bond_label);
      ++next_bond_label;
    }

    template<typename InputIterator>
    Expression(InputIterator first, InputIterator last)
      : Base(first, last)
    {
      bond_map = construct_bond_map(*this, &next_bond_label);
      ++next_bond_label;
    }

    Expression(const Expression & other) // copy ctor
      : Base(other)
    {
      // there are cheaper ways of doing this... maybe a map between other's agents memory addresses and this's ones.
      bond_map = construct_bond_map(*this, &next_bond_label);
      ++next_bond_label;
    }

    Expression(const Base & other) // copy ctor
      : Base(other)
    {
      bond_map = construct_bond_map(*this, &next_bond_label);
      ++next_bond_label;
    }

    // Operators
    /*
    Expression &
    operator=(const Expression & other) // assignment operator
    {
      if (this != &other)
      {
        Base::operator=(other);
        bond_map = construct_bond_map(*this, &next_bond_label);
        ++next_bond_label;
      }
      return *this;
    }
    */

    Expression &
    operator=(const Base & other) // assignment operator
    {
      if (this != &other)
      {
        Base::operator=(other);
        bond_map = construct_bond_map(*this, &next_bond_label);
        ++next_bond_label;
      }
      return *this;
    }

    Agent & operator[](size_t index)
    {
      if ( index >= size() )
        throw std::out_of_range("Expression::operator[]");
      iterator it = begin();
      for (size_t cnt = 0; cnt < index; ++cnt, ++it);
      return *it;
    }

    const Agent & operator[](size_t index) const
    {
      if ( index >= size() )
        throw std::out_of_range("Expression::operator[]");
      const_iterator it = begin();
      for (size_t cnt = 0; cnt < index; ++cnt, ++it);
      return *it;
    }

    // Functions
    Agent & find(const std::string & agent_name)
    {
      BOOST_FOREACH( Agent & a, *this)
        if ( a.name == agent_name )
          return a;
      throw std::runtime_error("Agent '" + agent_name + "' not found in expression.");
    }

    template <typename LambdaFunctor>
    Agent & find(const std::string & agent_name, LambdaFunctor condition)
    {
      BOOST_FOREACH( Agent & a, *this)
        if ( a.name == agent_name and condition(a) )
          return a;
      throw std::runtime_error("Agent '" + agent_name + "' not found in expression.");
    }

    // Data members
    BondMap bond_map;
    int next_bond_label;
  };


  // Elementary actions over agents in expressions
  template <typename Expr>
  int bind_agents(
      Agent & a1,
      const std::string & s1_name,
      Agent & a2,
      const std::string & s2_name,
      Expr & expr
      )
  {
    std::string bond_label = to_string( expr.next_bond_label );
    Site & s1 = *a1.find_site( s1_name );
    Site & s2 = *a2.find_site( s2_name );
    s1.binding_state = bond_label;
    s2.binding_state = bond_label;
    Complex c;
    c.push_back( &a1 );
    c.push_back( &a2 );
    expr.bond_map[ expr.next_bond_label++ ] = c;
    return expr.next_bond_label - 1;
  }

  template <typename Expr>
  void unbind_agents(
      Agent & a1,
      const std::string & s1_name,
      Agent & a2,
      const std::string & s2_name,
      Expr & expr
      )
  {
    Site & s1 = *a1.find_site( s1_name );
    Site & s2 = *a2.find_site( s2_name );
    if ( s1.binding_state != s2.binding_state )
      throw std::runtime_error("Cannot unbind not-connected sites.");
    if ( expr.next_bond_label == from_string<int>(s1.binding_state) )
      --expr.next_bond_label;
    expr.bond_map.erase( from_string<int>(s1.binding_state) );
    s1.binding_state = "";
    s2.binding_state = "";
    return;
  }

  template <typename Expr>
  std::pair<Agent *, std::string> unbind_agent(
      Agent & a1,
      const std::string & s1_name,
      Expr & expr
      )
  {
    Site & s1 = *a1.find_site( s1_name );
    Agent & a2 = binding_partner(a1, s1, expr);
    Site * s2_ptr = NULL;
    BOOST_FOREACH( Site & s2, a2.interface )
      if (s2.binding_state == s1.binding_state)
      {
        s2_ptr = &s2;
        break;
      }
    if ( expr.next_bond_label == from_string<int>(s1.binding_state) )
      --expr.next_bond_label;
    expr.bond_map.erase( from_string<int>(s1.binding_state) );
    s1.binding_state = "";
    s2_ptr->binding_state = "";
    return std::make_pair(&a2, s2_ptr->name);
  }

  template <typename Expr>
  void destroy_agent(
      const Agent & a,
      Expr & expr
      )
  {
    typedef typename Expr::iterator Eiter;
    std::vector<int> bonds;
    for (Eiter ei = expr.begin(), ei_end = expr.end(); ei != ei_end; ++ei)
    {
      if ( &(*ei) == &a )
      {
        BOOST_FOREACH( const Site & s, a.interface )
          if ( is_bond_label(s.binding_state) )
            bonds.push_back( from_string<int>(s.binding_state) );

        expr.erase(ei);
        break;
      }
    }

    // Delete links from bond_map
    BOOST_FOREACH( int bond_label, bonds )
      expr.bond_map.erase(bond_label);

    return;
  }

  template <typename Expr>
  Agent & create_agent(
      const Agent & a,
      Expr & expr
      )
  {
    expr.push_back(a);
    typename Expr::iterator ei = expr.end(); --ei;
    Agent & new_agent = *ei;
    BOOST_FOREACH( Site & s, new_agent.interface )
    {
      if ( is_bond_label(s.binding_state) )
      {
        if ( expr.bond_map.find( from_string<int>(s.binding_state) ) == expr.bond_map.end() )
        { // s.binding_state does not exist in expr.bond_map
          expr.bond_map[ from_string<int>(s.binding_state) ] = Complex(1, &new_agent);
        }
        else
        {
          expr.bond_map[ from_string<int>(s.binding_state) ].push_back( &new_agent );
        }
      }
    }
    return new_agent;
  }

  void modify_internal_state(
      const std::string & new_internal_state,
      Site & s
      )
  {
    s.internal_state = new_internal_state;
    return;
  }


  // Validation function
  /// @brief Returns a bool value indicating whether the expression is a well-formed expression under the rules of Kappa language (ref.)
  template <typename Expr>
  bool valid_expression(
      const Expr & expr,
      bool as_pattern = false
      )
  {
    BOOST_FOREACH( const Agent & a, expr )
      if ( not valid_agent(a, as_pattern) )
        return false;

    // check if bond labels are present exactly twice in the expression (definition 2.3, xi 'binary binding', Kappa language basics)
    BondMap bond_map = construct_bond_map( const_cast<Expr &>(expr), NULL ); // I swear I won't modify my own data structures in this function =P
    BOOST_FOREACH( BondMapItem & label_and_agents, bond_map )
      if (label_and_agents.second.size() != 2)
        return false;

    return true;
  }


  // to_string specialization
  /// @brief Returns the string representation of the expression
  template <>
  inline std::string to_string<Expression>(const Expression & expr)
  {
    if ( expr.empty() )
      return "";

    std::string output;
    BOOST_FOREACH( const Agent & a, expr )
    {
      output += to_string(a) + ",";
    }
    output.erase( output.size() - 1 );
    return output;
  }

} // kappa

#endif // EXPRESSION_HPP
