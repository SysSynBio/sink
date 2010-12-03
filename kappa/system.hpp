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

#ifndef SYSTEM_HPP
#define SYSTEM_HPP
#include <kappa/rule.hpp>
#include <kappa/pattern.hpp>
#include <kappa/random.hpp>
#include <kappa/gillespie.hpp>
#include <kappa/map.hpp>
#include <kappa/lift_map.hpp>
#include <kappa/lexer.hpp>
#include <fstream>

namespace kappa
{

  template <typename Gillespie, typename RuleSet>
  struct System
  {
    typedef typename RuleSet::value_type Rule;
    typedef typename std::set<const Rule *> RulePtrSet;
    typedef typename Rule::Expression Expr;
    typedef typename std::vector<double> ActivityList;
    typedef typename ActivationMap<Rule>::Type ActivationMap;
    typedef typename InhibitionMap<Rule>::Type InhibitionMap;
    typedef typename MatchingMap<Rule>::Type MatchingMap;
    typedef typename LiftMap<Rule>::Type LiftMap;

    System(
        const Expr & m,
        const RuleSet & r
        )
      : mixture(m), rules(r), t(0.0), iteration_cnt(0),
        num_void_events(0), deadlock_criteria(20)
    {
      boost::tie(activation_map, inhibition_map) =
        compute_activation_inhibition_map(rules);
#ifdef KAPPA_DEBUG
      std::cerr << "System<Gillespie, RuleSet>::System(mixture, rule_set): Initializing system." << std::endl;
      std::cerr << "System<Gillespie, RuleSet>::System(mixture, rule_set): Rules:" << std::endl; int i = 0;
      BOOST_FOREACH( Rule & r, rules )
        std::cerr << "System<Gillespie, RuleSet>::System(mixture, rule_set):   rule "
                  << i++ << ": " << &r << std::endl;
      std::cerr << "System<Gillespie, RuleSet>::System(mixture, rule_set): Activation Map = ";
      print_activation_map(activation_map); std::cerr << std::endl;
      std::cerr << "System<Gillespie, RuleSet>::System(mixture, rule_set): Inhibition Map = ";
      print_activation_map(inhibition_map); std::cerr << std::endl;
#endif
      matching_map = compute_matching_map(rules, mixture);
      lift_map = compute_lift_map(mixture, matching_map);
      gillespie = Gillespie( compute_initial_activities(), rules );
    }

    System() // default ctor
      : mixture(), rules(), t(0.0), iteration_cnt(0),
      num_void_events(0), deadlock_criteria(20),
      activation_map(), inhibition_map(), matching_map(),
      lift_map(), gillespie()
    { }

    System( // copy ctor
        const System<Gillespie, RuleSet> & other
        )
      : mixture(other.mixture), rules(other.rules), 
        t(0.0), iteration_cnt(0), num_void_events(0),
        deadlock_criteria(20)
    { // review this!!
      // recompute maps and activities
#ifdef KAPPA_DEBUG
      std::cerr << "System<Gillespie, RuleSet>::System(other): Copy constructor called." << std::endl;
#endif
      boost::tie(activation_map, inhibition_map) =
        compute_activation_inhibition_map(rules);
      matching_map = compute_matching_map(rules, mixture);
      lift_map = compute_lift_map(mixture, matching_map);
      gillespie = Gillespie( compute_initial_activities(), rules );
    }

    System<Gillespie, RuleSet> &
    operator=( // assignment operator
        const System<Gillespie, RuleSet> & other
        )
    { // review this!!
#ifdef KAPPA_DEBUG
      std::cerr << "System<Gillespie, RuleSet>::operator=(other): Assignment operator called." << std::endl;
#endif
      if (this != &other)
      {
        mixture = other.mixture;
        rules = other.rules;
        t = 0.0; iteration_cnt = 0;
        num_void_events = 0;
        deadlock_criteria = 20;

        // recompute maps and activities
        boost::tie(activation_map, inhibition_map) =
          compute_activation_inhibition_map(rules);
        matching_map = compute_matching_map(rules, mixture);
        lift_map = compute_lift_map(mixture, matching_map);
        gillespie = Gillespie( compute_initial_activities(), rules );
      }
      return *this;
    }

    ActivityList compute_initial_activities() //const
    {
      ActivityList activities( rules.size() );
      size_t cnt = 0;
#ifdef KAPPA_DEBUG
      std::cerr << "System<Gillespie, RuleSet>::compute_initial_activities(): Computing initial activities:" << std::endl;
#endif
      BOOST_FOREACH( const Rule & r, rules )
      {
        activities[ cnt++ ] = compute_activity(r);
#ifdef KAPPA_DEBUG
        std::cerr << "System<Gillespie, RuleSet>::compute_initial_activities():   rule "
                  << cnt-1 << ": " << to_string(r) << " -> " << activities[cnt-1]
                  << "  outer_automorphisms = " << r.outer_automorphisms << std::endl;
#endif
      }
      return activities;
    }

    double compute_activity(const Rule & r) //const
    {
      double a = r.kinetic_constant / r.outer_automorphisms;
      BOOST_FOREACH( const Complex & c, r.complexes() )
        a *= matching_map[ &r ][ c ].size();
      return a;
    }

    void update_all_activities()
    {
      BOOST_FOREACH( const Rule & r, rules )
        gillespie[ &r ] = compute_activity(r);
    }

    void iterate()
    {
      // draw rule R for the next event and advance time
      int rule_index = 0; double dt = 0.0;
      boost::tie(rule_index, dt) = gillespie();
      if ( rule_index == -1 ) // system activity is zero, so no rule can be applied to the mixture
        return;
      t += dt; ++iteration_cnt;
#ifdef KAPPA_DEBUG
      std::cerr << "System<Gillespie, RuleSet>::iterate(): Activities:" << std::endl;
      BOOST_FOREACH( Rule & rule, rules )
        std::cerr << "System<Gillespie, RuleSet>::iterate():   rule '" << to_string(rule) << "'[" << &rule << "]: " << gillespie[ rule ] << std::endl;
      std::cerr << "System<Gillespie, RuleSet>::iterate(): selected rule index = " << rule_index << ", dt = " << dt << std::endl;
#endif

      Rule & r = rules[ rule_index ];
      ComplexSet matchings = select_random_matchings( r, matching_map );
      if ( matchings.empty() ) // there are no matchings for rule r in mixture, this should not happen
        return;
      if ( check_clashes(matchings) )
      {
#ifdef KAPPA_DEBUG
        std::cerr << "System<Gillespie, RuleSet>::iterate(): Clash found! Void event generated. Number of consecutive void events = " << num_void_events+1 << std::endl;
#endif
        if (++num_void_events >= deadlock_criteria)
          throw std::runtime_error("System<Gillespie, RuleSet>::iterate(): Deadlock found!");
        return;
      }
      else
      {
        num_void_events = 0;
      }
#ifdef KAPPA_DEBUG
      std::cerr << "System<Gillespie, RuleSet>::iterate(): Selected random matchings: {";
      BOOST_FOREACH( const Complex & c, matchings )
        std::cerr << " " << to_string(c);
      std::cerr << " }" << std::endl;
#endif

      // apply rule R and update R-related counts
      ModifiedSet modified_set, created_set, removed_set;
      BOOST_FOREACH( const typename Rule::ElementaryAction & action, r.elementary_actions() )
      {
        if ( action.target_type() == typeid(Create) )
        {
          ModifiedSet created_agents( action(mixture, matchings) );
          BOOST_FOREACH( AgentSitePair & created_agent, created_agents )
            created_set.push_back( created_agent );
        }
        else if ( action.target_type() == typeid(Destroy) )
        {
          ModifiedSet removed_agents( action(mixture, matchings) );
          BOOST_FOREACH( AgentSitePair & removed_agent, removed_agents )
            removed_set.push_back( removed_agent );
        }
        else
        {
          ModifiedSet modified_agents( action(mixture, matchings) );
          BOOST_FOREACH( AgentSitePair & modified_agent, modified_agents )
            modified_set.push_back( modified_agent );
        }
      }

#ifdef KAPPA_DEBUG
      if (not modified_set.empty())
      {
        std::cerr << "System<Gillespie, RuleSet>::iterate(): Modified agents:";
        BOOST_FOREACH( AgentSitePair & ma, modified_set )
          std::cerr << " " << ma.first;
        std::cerr << std::endl;
      }
      if (not removed_set.empty())
      {
        std::cerr << "System<Gillespie, RuleSet>::iterate(): Removed agents:";
        BOOST_FOREACH( AgentSitePair & ra, removed_set )
          std::cerr << " " << ra.first;
        std::cerr << std::endl;
      }
      if (not created_set.empty())
      {
        std::cerr << "System<Gillespie, RuleSet>::iterate(): Created agents:";
        BOOST_FOREACH( AgentSitePair & ca, created_set )
          std::cerr << " " << ca.first;
        std::cerr << std::endl;
      }
#endif

      // negative update via RIM (inhibition map) or via matching map
      RulePtrSet modified_rules(
        negative_update( modified_set, removed_set ) );

      // positive update via RAM (activation map)
      BOOST_FOREACH( AgentSitePair & created_agent, created_set )
        modified_set.push_back( created_agent );
      std::vector<const Rule *> activated_rules( activation_map[ &r ] );
      RulePtrSet modified_rules_aux(
        positive_update( modified_set, activated_rules ) );

      BOOST_FOREACH( const Rule * r, modified_rules_aux )
        modified_rules.insert(r);

      // Update rule activities
      update_activities(modified_rules);

      // Debug
#if KAPPA_DEBUG > 1
      std::unordered_map<const Agent *, bool> existent_agents;
      BOOST_FOREACH( const Agent & a, mixture )
        existent_agents[ &a ] = true;

      BOOST_FOREACH( typename MatchingMap::value_type & i, matching_map ) // foreach rule 'i.first'
      {
        const Rule & rule = *i.first;
        ComplexMatchings & matchings = i.second;
        BOOST_FOREACH( typename ComplexMatchings::value_type & j, matchings ) // foreach (rule complex -> mixture complexes) pair
        {
          const Complex & rule_complex = j.first;
          BOOST_FOREACH( const Complex & c, j.second )
          {
            BOOST_FOREACH( const Agent * a, c )
            {
              if ( existent_agents.find(a) == existent_agents.end() )
                std::cerr << "System<Gillespie, RuleSet>::iterate(): Warning! Agent " << a << " was not found in mixture."
                          << std::endl;

              if ( lift_map.find(a) == lift_map.end() or
                   lift_map[a].find(&rule) == lift_map[a].end() or
                   lift_map[a][&rule].find(rule_complex) == lift_map[a][&rule].end() or
                   lift_map[a][&rule][rule_complex].find(c) == lift_map[a][&rule][rule_complex].end()
                   )
                std::cerr << "System<Gillespie, RuleSet>::iterate(): Warning! Lift map ( " << to_string(*a) << "["
                          << a << "], '" << to_string(rule) << "', " << to_string(rule_complex) << " ) does not contain "
                          << to_string(c) << std::endl;
            }

            if ( not match_complex(rule_complex, rule.left_hand_side, c, mixture) )
              std::cerr << "System<Gillespie, RuleSet>::iterate(): Warning! Complex " << to_string(rule_complex, AsExpr)
                        << " of rule " << to_string(rule) << " does not match mixture complex " << to_string(c) << std::endl;
          }
        }
      }
#endif
      return;
    }

    RulePtrSet negative_update(
        ModifiedSet & modified_set,
        ModifiedSet & removed_set
        )
    {
      RulePtrSet modified_rules;
      BOOST_FOREACH( AgentSitePair & i, modified_set )
      {
        if ( lift_map.find(i.first) == lift_map.end() )
        { // this should not happen!! ... agents should always be in lift map,
          // even when they aren't associated to anything
#ifdef KAPPA_DEBUG
          std::cerr << "System<Gillespie, RuleSet>::negative_update(): Warning! " << i.first
                    << " not found in lift map and present in modified_set." << std::endl;
#endif
          continue;
        }

        BOOST_FOREACH( typename MatchingMap::value_type & lme, lift_map[ i.first ] )
        {
          const Rule * const r_ = lme.first;
          BOOST_FOREACH( typename ComplexMatchings::value_type & cm, lme.second )
          {
            const Complex & c_ = cm.first;
            ComplexSet & matchings_ = cm.second;
            std::list<typename ComplexSet::iterator> to_remove;

            //BOOST_FOREACH( const Complex & codomain_complex, matchings_ )
            for (ComplexSet::iterator codomain_complex = matchings_.begin(), matchings__end = matchings_.end();
                 codomain_complex != matchings__end; ++codomain_complex)
            {
              if ( not match_complex(c_, (*r_).left_hand_side, *codomain_complex, mixture) ) // FIXME can this be optimized?
              { // if c_ doesnt match codomain_complex anymore, then codomain_complex should not be in the lift map of any agent
                // nor should it be in the matching map
                BOOST_FOREACH( Agent * const a, *codomain_complex )
                { // remove codomain_complex from the matchings in the lift map of every agent in codomain_complex
                  if ( lift_map.find(a) == lift_map.end() ) // error
                    continue;
                  if ( lift_map[ a ].find( r_ ) == lift_map[ a ].end() or
                       lift_map[ a ][ r_ ].find( c_ ) == lift_map[ a ][ r_ ].end() )
                    continue;

                  ComplexSet & m_a = lift_map[ a ][ r_ ][ c_ ];
#ifdef KAPPA_DEBUG
                  int n = m_a.erase( *codomain_complex );
                  std::cerr << "System<Gillespie, RuleSet>::negative_update(): Removing (" << n << ") "
                            << to_string(*codomain_complex, JustPtrs) << " from lift_map ( " << a << ", '"
                            << to_string(*r_) << "', " << to_string(c_) << " ) (size = " << m_a.size() << ")" << std::endl;
#else
                  m_a.erase( *codomain_complex );
#endif
                }

#ifdef KAPPA_DEBUG
                int nremoved = matching_map[ r_ ][ c_ ].erase( *codomain_complex );
                std::cerr << "System<Gillespie, RuleSet>::negative_update(): Removing (" << nremoved << ") "
                          << to_string(*codomain_complex, JustPtrs) << " from matching_map[ '" << to_string(*r_) << "' ][ "
                          << to_string(c_) << " ] (size = " << matching_map[ r_ ][ c_ ].size() << ")" << std::endl;
#else
                matching_map[ r_ ][ c_ ].erase( *codomain_complex );
#endif
                to_remove.push_back(codomain_complex);
              }
            }

            BOOST_FOREACH( typename ComplexSet::iterator & j, to_remove )
            {
#ifdef KAPPA_DEBUG
              matchings_.erase(j);
              std::cerr << "System<Gillespie, RuleSet>::negative_update(): Removing " << to_string(*j, JustPtrs)
                        << " from lift_map ( " << i.first << ", '" << to_string(*r_) << "', " << to_string(c_)
                        << " ) (size = " << matchings_.size() << ")" << std::endl;
#else
              matchings_.erase(j);
#endif
            }
            modified_rules.insert( r_ );
          }
        }
      }

      BOOST_FOREACH( AgentSitePair & i, removed_set )
      {
        if ( lift_map.find(i.first) == lift_map.end() ) // this should not happen!!
        {
#ifdef KAPPA_DEBUG
          std::cerr << "System<Gillespie, RuleSet>::negative_update(): Warning! " << i.first
                    << " not found in lift map and present in removed_set." << std::endl;
#endif
          continue;
        }

        BOOST_FOREACH( typename MatchingMap::value_type & lme, lift_map[ i.first ] )
        {
          const Rule * const r_ = lme.first;
          BOOST_FOREACH( typename ComplexMatchings::value_type & cm, lme.second )
          {
            const Complex & c_ = cm.first;
            ComplexSet & matchings_ = cm.second;

            BOOST_FOREACH( const Complex & codomain_complex, matchings_ )
            {
#ifdef KAPPA_DEBUG
              int nremoved = matching_map[ r_ ][ c_ ].erase( codomain_complex );
              std::cerr << "System<Gillespie, RuleSet>::negative_update(): Removing (" << nremoved << ") "
                        << to_string(codomain_complex, JustPtrs) << " from matching_map[ '" << to_string(*r_) << "' ][ "
                        << to_string(c_) << " ]  (size = " << matching_map[ r_ ][ c_ ].size() << ")" << std::endl;
#else
              matching_map[ r_ ][ c_ ].erase( codomain_complex );
#endif
            }

            modified_rules.insert(r_);
          }
        }

        lift_map.erase(i.first);
#ifdef KAPPA_DEBUG
        std::cerr << "System<Gillespie, RuleSet>::negative_update(): Removing agent " << i.first << " from lift map" << std::endl;
#endif
      }
      return modified_rules;
    }

    RulePtrSet positive_update(
        ModifiedSet & set,
        std::vector<const Rule *> activated_rules
        )
    {
      RulePtrSet modified_rules;
      ComplexSet modified_complexes;
      BOOST_FOREACH( AgentSitePair & i, set )
      {
        Complex c = get_complex(*i.first, mixture);
        modified_complexes.insert(c);
      }

      BOOST_FOREACH( const Rule * r_, activated_rules )
      {
        bool modified = false;
        BOOST_FOREACH( const Complex & c, modified_complexes )
        {
          BOOST_FOREACH( const Complex & c_, (*r_).complexes() )
          {
            if ( match_complex(c_, mixture, c, mixture) )
            {
              // add embedding to matching map
              matching_map[ r_ ][ c_ ].insert( c );
#ifdef KAPPA_DEBUG
              std::cerr << "System<Gillespie, RuleSet>::positive_update(): Inserting " << to_string(c) << " in matching_map[ '"
                        << to_string(*r_) << "' ][ " << to_string(c_) << " ] (size = " << matching_map[ r_ ][ c_ ].size()
                        << ")" << std::endl;
#endif

              BOOST_FOREACH( const Agent * a, c )
              {
                lift_map[ a ][ r_ ][ c_ ].insert( c );
#ifdef KAPPA_DEBUG
                std::cerr << "System<Gillespie, RuleSet>::positive_update(): Inserting " << to_string(c)
                          << " in matchings of lift map ( " << a << ", '" << to_string(*r_) << "', "
                          << to_string(c_) << " )" << std::endl;
#endif
              }
              modified = true;
            }
          }
        }

        if (modified)
          modified_rules.insert( r_ );
      }
      return modified_rules;
    }

    template <typename RulePtrSet>
    void update_activities(
        const RulePtrSet & modified_rules
        )
    {
      BOOST_FOREACH( const Rule * r, modified_rules )
      {
#ifdef KAPPA_DEBUG
        double old_activity = gillespie[ *r ];
#endif
        gillespie[ *r ] = compute_activity( *r );
#ifdef KAPPA_DEBUG
        std::cerr << "System<Gillespie, RuleSet>::update_activities(): Updating activity of rule '" << to_string(*r) << "': "
                  << old_activity << " -> " << gillespie[ *r ] << std::endl;
#endif
      }
      return;
    }

    ComplexSet select_random_matchings(
        Rule & r,
        MatchingMap & mm
        ) const
    {
      ComplexSet out;
      BOOST_FOREACH( const Complex & c, r.complexes() )
      {
        int size = mm[ &r ][ c ].size();
#if KAPPA_DEBUG > 1
        std::cerr << "System<Gillespie, RuleSet>::select_random_matchings(): size = " << size << std::endl;
#endif
        if (size == 0)
          return ComplexSet();
        // choose random number between 0 and (size - 1)
        int index = random_uniform_int(0, size-1);
#if KAPPA_DEBUG > 1
        std::cerr << "System<Gillespie, RuleSet>::select_random_matchings(): index = " << index << std::endl;
#endif
        // insert the random-chosen complex in the output
        ComplexSet::iterator i = mm[ &r ][ c ].begin(),
                             end = mm[ &r ][ c ].end();
        for (; i != end and index != 0; ++i, --index);
#if KAPPA_DEBUG > 1
        std::cerr << "System<Gillespie, RuleSet>::select_random_matchings(): *i = " << to_string(*i) << std::endl;
#endif
        out.insert( *i );
      }
      return out;
    }

    bool check_clashes(
        const ComplexSet & matchings
        ) const
    {
      int i = 0;
      BOOST_FOREACH( const Complex & c1, matchings )
      {
        int j = 0;
        BOOST_FOREACH( const Complex & c2, matchings )
        {
          if ( j++ <= i )
            continue;
          if ( c1 == c2 )
            return true;
        }
        ++i;
      }
      return false;
    }

    Expr mixture;
    RuleSet rules;
    double t;
    size_t iteration_cnt;
    size_t num_void_events;
    size_t deadlock_criteria;
    ActivationMap activation_map;
    InhibitionMap inhibition_map;
    MatchingMap matching_map;
    LiftMap lift_map;
    Gillespie gillespie; // gillespie stores the activities
    //ModifiedSet pending_modifications;
  };


  template <typename Gillespie, typename RuleSet>
  System<Gillespie, RuleSet>
  read_system(
      const std::string & input
      )
  {
    typedef typename RuleSet::value_type Rule;
    typedef typename Rule::Expression Expr;
    if (input.empty())
      return System<Gillespie, RuleSet>();
    boost::char_separator<char> sep("\n");
    boost::tokenizer<boost::char_separator<char> > tok(input, sep);
    Expr mixture;
    RuleSet rules;
    BOOST_FOREACH( const std::string & line, tok )
    {
      if (line.empty() or line[0] == '%')
        continue;
      else if (line.substr(0, 4) == "init" or line.substr(0, 5) == "\%init")
      {
        boost::char_separator<char> sep2(" ");
        boost::tokenizer<boost::char_separator<char> > tok_line(line, sep2);
        std::vector<std::string> words;
        BOOST_FOREACH( const std::string & s, tok_line )
          words.push_back(s);
        if (words.size() == 3)
        {
          int num_agents = int(from_string<double>(words[1]));
          Expr e = from_string<Expr>(words[2]);
#ifdef KAPPA_DEBUG
          std::cerr << "read_system(input): Adding '" << words[2] << "' x " << num_agents << " to mixture." << std::endl;
#endif
          for (int i = 0; i < num_agents; ++i)
            BOOST_FOREACH( Agent & a, e )
              mixture.push_back(a); // FIXME buggy!! if agents are linked, this wont work
        }
        else
        {
#ifdef KAPPA_DEBUG
          std::cerr << "read_system(input): mixture = " << words[1] << std::endl;
#endif
          mixture = from_string<Expr>(words[1]); // FIXME buggy!! this part should work for Expression x Num notation
        }
      }
      else
      {
#ifdef KAPPA_DEBUG
        std::cerr << "read_system(input): Adding rule '" << line << "' to rule set." << std::endl;
#endif
        rules.push_back( from_string<Rule>(line) );
      }
    }
    System<Gillespie, RuleSet> output(mixture, rules);
    return output;
  }


  template <typename Gillespie, typename RuleSet>
  System<Gillespie, RuleSet>
  read_system(
      std::ifstream & input
      )
  {
    std::string file, line;
    while ( not std::getline(input, line).eof() )
      file += line + '\n';
    return read_system<Gillespie, RuleSet>(file);
  }

} // kappa

#endif // SYSTEM_HPP
