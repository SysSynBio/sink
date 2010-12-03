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

#ifndef MAP_HPP
#define MAP_HPP
#include <kappa/complex.hpp>
#include <kappa/rule.hpp>

namespace kappa
{

  // *** Matching Map ***

  // pattern complex -> set of mixture complexes
  typedef std::unordered_map< Complex, ComplexSet, boost::hash<Complex> >
      ComplexMatchings;

  template <typename Rule>
  struct MatchingMap
  {
    // rule index -> rule complex matchings in mixture
    typedef std::unordered_map< const Rule *, ComplexMatchings >
        Type;
  };


  template <typename Expr, typename RuleSet>
  typename MatchingMap<typename RuleSet::value_type>::Type
  compute_matching_map(
      RuleSet & rules,
      Expr & mixture
      )
  {
    typedef typename RuleSet::value_type Rule;
    typedef typename MatchingMap<Rule>::Type MatchingMap;

    MatchingMap mm;
    BOOST_FOREACH( Rule & r, rules )
    {
      mm[ &r ] = ComplexMatchings();
      BOOST_FOREACH( const Complex & rule_complex, r.complexes() )
        mm[ &r ][ rule_complex ] = ComplexSet();
    }

    ComplexSet mixture_complexes = get_complexes(mixture);
    BOOST_FOREACH( const Complex & mixture_complex, mixture_complexes )
      BOOST_FOREACH( Rule & r, rules )
        BOOST_FOREACH( const Complex & rule_complex, r.complexes() )
          if ( match_complex(rule_complex, r.left_hand_side, mixture_complex, mixture) )
            mm[ &r ][ rule_complex ].insert( mixture_complex );

    return mm;
  }


  // *** Activation and Inhibition Map ***
  namespace detail
  {

    template <typename T1, typename T2>
    bool find_match(
        const T1 & agents1,
        const T2 & agents2
        )
    {
      BOOST_FOREACH( const Agent * a1, agents1 )
        BOOST_FOREACH( const Agent * a2, agents2 )
          if ( match_agent(*a2, *a1) )
            return true;
      return false;
    }

  } // detail


  template <typename Rule>
  struct ActivationMap
  {
    typedef typename std::unordered_map< const Rule *, std::vector<const Rule *> >
        Type;
  };

  template <typename Rule>
  struct InhibitionMap
  {
    typedef typename std::unordered_map< const Rule *, std::vector<const Rule *> >
        Type;
  };


  template <typename RuleSet>
  std::pair<typename ActivationMap<typename RuleSet::value_type>::Type,
            typename InhibitionMap<typename RuleSet::value_type>::Type>
  compute_activation_inhibition_map(
      const RuleSet & rules
      )
  {
    using namespace detail;
    typedef typename RuleSet::value_type Rule;
    typedef typename ActivationMap<Rule>::Type ActivationMap;
    typedef typename InhibitionMap<Rule>::Type InhibitionMap;
    typedef typename std::list<const Agent *> AgentPtrSet;
    typedef typename std::unordered_map<const Rule *, AgentPtrSet> RuleAgentMap;

    ActivationMap ram;
    InhibitionMap rim;

    RuleAgentMap agents_lhs;
    BOOST_FOREACH( const Rule & r, rules )
    {
      agents_lhs[ &r ] = AgentPtrSet();
      BOOST_FOREACH( const Agent & a, r.left_hand_side )
        agents_lhs[ &r ].push_back(&a);
    }

    BOOST_FOREACH( const Rule & r1, rules )
    {
      ram[ &r1 ] = std::vector<const Rule *>();
      rim[ &r1 ] = std::vector<const Rule *>();
      BOOST_FOREACH( const Rule & r2, rules )
      {
        if ( find_match(r1.modified_agents().second, agents_lhs[ &r2 ]) )
          ram[ &r1 ].push_back( &r2 );

        if ( find_match(r1.modified_agents().first, agents_lhs[ &r2 ]) )
          rim[ &r1 ].push_back( &r2 );
      }
    }

    return std::make_pair(ram, rim);
  }

} // kappa

#endif // MAP_HPP
