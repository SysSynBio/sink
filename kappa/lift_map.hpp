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

#ifndef LIFT_MAP_HPP
#define LIFT_MAP_HPP
#include <kappa/rule.hpp>
#include <kappa/map.hpp>

namespace kappa
{

  template <typename Rule>
  struct LiftMap
  {
    typedef typename MatchingMap<Rule>::Type MatchingMap;
    typedef typename std::unordered_map<const Agent *, MatchingMap> Type;
  };


  template <typename Expr, typename MatchingMap>
  typename LiftMap< Rule<Expr> >::Type
  compute_lift_map(
      Expr & expr,
      MatchingMap & mm
      )
  {
    typedef typename LiftMap< Rule<Expr> >::Type LiftMap;
    LiftMap lm;

    //BOOST_FOREACH( typename MatchingMap::value_type & i, mm ) // foreach rule 'i.first'
    for (typename MatchingMap::iterator i = mm.begin(), mm_end = mm.end(); i != mm_end; ++i)
    {
      ComplexMatchings & matchings = i-> second;

      //BOOST_FOREACH( typename ComplexMatchings::value_type & j, matchings ) // foreach (rule complex -> mixture complexes) pair
      for (typename ComplexMatchings::iterator j = matchings.begin(), m_end = matchings.end(); j != m_end; ++j)
      {
        typedef typename std::unordered_map<const Agent *, ComplexSet> EmbeddingsMap;
        EmbeddingsMap embeddings;

        //BOOST_FOREACH( const Complex & codomain_complex, j.second ) // foreach mixture complex in this matching
        for (typename ComplexSet::const_iterator cod_complex = j-> second.begin(), codc_end = j-> second.end();
             cod_complex != codc_end; ++cod_complex) // cod: codomain
        {
          //BOOST_FOREACH( const Agent * codomain_agent, codomain_complex )
          for (typename Complex::const_iterator cod_agent = cod_complex-> begin(), coda_end = cod_complex-> end();
               cod_agent != coda_end; ++cod_agent)
          {
            if ( embeddings.find(*cod_agent) == embeddings.end() )
              embeddings[*cod_agent] = ComplexSet();
            embeddings[*cod_agent].insert(*cod_complex);
          } // foreach cod_agent
        } // foreach cod_complex

        BOOST_FOREACH( EmbeddingsMap::value_type & emb, embeddings )
        {
          const Agent * cod_agent = emb.first;
          lm[ cod_agent ][ i-> first ][ j-> first ] = emb.second;
#if KAPPA_DEBUG > 2
          std::cerr << "compute_lift_map: Inserting { ";
          BOOST_FOREACH( const Complex & emb_complex, emb.second )
            std::cerr << " " << to_string(emb_complex);
          std::cerr << " } to lift map (" << cod_agent << ", " << i-> first << to_string(j-> first) << ")" << std::endl;
#endif
        }
      } // foreach j in matchings
    } // foreach i in matching_map

    return lm;
  }

} // kappa

#endif // LIFT_MAP_HPP
