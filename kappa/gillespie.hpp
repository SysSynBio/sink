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

#ifndef GILLESPIE_HPP
#define GILLESPIE_HPP
#include <kappa/random.hpp>
#include <kappa/rule.hpp>
#include <cmath>

namespace kappa
{

  struct LinearGillespie
  {
    typedef std::vector<double> ActivityList;
    typedef std::unordered_map<const void *, size_t> RuleIndexMap;

    LinearGillespie()
      : activity(), system_activity(0.0), rule_index() { }

    LinearGillespie(const ActivityList & al) // mainly for testing
      : activity(al), system_activity(0.0), rule_index() { }

    template <typename T>
    LinearGillespie(const ActivityList & al, const T & rule_set)
      : activity(al), system_activity(0.0)
    {
      size_t i = 0;
      BOOST_FOREACH( const typename T::value_type & rule, rule_set )
        rule_index[ (const void *)&rule ] = i++;
    }

    std::pair<int, double> // (rule_index, dt)
    operator()()
    {
      compute_system_activity();
      if (system_activity == 0.0)
        return std::make_pair(-1, 0.0);
      double r1 = random_uniform01(),
             r2 = random_uniform01();
      double tau = logf( 1.0 / r1 ) / system_activity;
      double r2a0 = r2 * system_activity;
      double sum = 0.0; int mu = 0;
      BOOST_FOREACH( double a, activity )
      {
        sum += a;
        if ( sum >= r2a0 )
          return std::make_pair(mu, tau);
        ++mu;
      }
      return std::make_pair(activity.size() - 1, tau);
    }

    double &
    operator[](size_t index)
    {
      if ( index >= activity.size() )
        throw std::out_of_range("LinearGillespie::operator[]");
      return activity[index];
    }

    const double &
    operator[](size_t index) const
    {
      if ( index >= activity.size() )
        throw std::out_of_range("LinearGillespie::operator[]");
      return activity[index];
    }

    template <typename Rule>
    double &
    operator[](const Rule & rule)
    {
      typename RuleIndexMap::iterator it = rule_index.find( (const void *)&rule );
      if ( it == rule_index.end() )
        throw std::out_of_range("LinearGillespie::operator[]");
      size_t index = (*it).second;
      return activity[index];
    }

    template <typename Rule>
    const double &
    operator[](const Rule & rule) const
    {
      typename RuleIndexMap::const_iterator it = rule_index.find( (const void *)&rule );
      if ( it == rule_index.end() )
        throw std::out_of_range("LinearGillespie::operator[]");
      size_t index = (*it).second;
      return activity[index];
    }

  private:
    void compute_system_activity()
    {
      system_activity = 0.0;
      BOOST_FOREACH( double a, activity )
        system_activity += a;
    }

    ActivityList activity;
    double system_activity;
    RuleIndexMap rule_index;
  };

} // kappa

#endif // GILLESPIE_HPP
