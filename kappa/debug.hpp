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

#ifndef DEBUG_UTILS_HPP
#define DEBUG_UTILS_HPP
#include <kappa/rule.hpp>

namespace kappa
{

  template <typename BondMap>
  void print_bond_map(
      const BondMap & bond_map
      )
  {
    typedef typename BondMap::value_type BondMapItem;
    std::cerr << "{";
    BOOST_FOREACH( const BondMapItem & i, bond_map )
    {
      std::cerr << " " << i.first << ": [";
      BOOST_FOREACH( Agent * a, i.second )
        std::cerr << " " << a; //to_string(*a);
      std::cerr << " ]";
    }
    std::cerr << " }";
  }

  template <typename Expr>
  void print_expr(
      const Expr & expr
      )
  {
    std::cerr << "[";
    BOOST_FOREACH( const Agent & a, expr )
      std::cerr << " " << &a;
    std::cerr << " ]";
  }

  template <typename ActivationMap>
  void print_activation_map(
      const ActivationMap & ram
      )
  {
    typedef typename ActivationMap::value_type ActivationMapItem;
    typedef typename ActivationMapItem::second_type RuleSet;
    typedef typename RuleSet::value_type Rule;
    std::cerr << "{";
    BOOST_FOREACH( const ActivationMapItem & i, ram )
    {
      std::cerr << " " << to_string(i.first) << ": ["; // the rule which is activating the others
      BOOST_FOREACH( const Rule & r, i.second )
        std::cerr << " " << to_string(r);
      std::cerr << " ]";
    }
    std::cerr << " }";
  }

  template <typename MatchingMap>
  void print_matching_map(
      const MatchingMap & mm
      )
  {
    typedef typename MatchingMap::value_type MatchingMapItem;
    typedef typename MatchingMapItem::second_type ComplexMatchings;
    typedef typename ComplexMatchings::value_type ComplexMatchingsItem;
    typedef typename ComplexMatchingsItem::second_type ComplexSet;
    typedef typename ComplexSet::value_type Complex;

    std::cerr << "{";
    BOOST_FOREACH( const MatchingMapItem & mmi, mm )
    {
      std::cerr << " " << to_string(*mmi.first) << ": {";
      BOOST_FOREACH( const ComplexMatchingsItem & cmi, mmi.second )
      {
        std::cerr << " " << to_string(cmi.first) << ": [";
        BOOST_FOREACH( const Complex & cs, cmi.second )
          std::cerr << " " << to_string(cs);
        std::cerr << " ]";
      }
      std::cerr << " }";
    }
    std::cerr << " }";
  }

  template <typename LiftMap>
  void print_lift_map(
      const LiftMap & lm
      )
  {
    using std::cerr; using std::endl; using std::string;
    typedef typename LiftMap::value_type::second_type AgentLiftMap;
    typedef typename AgentLiftMap::mapped_type LiftMapMappedType;
    typedef typename LiftMapMappedType::value_type LiftMapElement;
    typedef typename boost::tuples::element<0, LiftMapElement>::type RulePtr;
    typedef typename boost::tuples::element<1, LiftMapElement>::type Complex;
    typedef typename boost::tuples::element<2, LiftMapElement>::type ComplexSet;

    cerr << "Lift Map:" << endl;
    BOOST_FOREACH( const typename LiftMap::value_type & i, lm )
    {
      const Agent * a = i.first;
      BOOST_FOREACH( const typename AgentLiftMap::value_type & j, i.second )
      {
        string site_name = j.first;
        cerr << "  (" << a << ", " << site_name << ") -> {";
        BOOST_FOREACH( const LiftMapElement & k, j.second )
        {
          const RulePtr r = boost::get<0>(k);
          const Complex & c = boost::get<1>(k);
          const ComplexSet & matchings = boost::get<2>(k);
          cerr << " (" << r << ", " << to_string(c) << ", {";
          BOOST_FOREACH( const Complex & m, matchings )
            cerr << " " << to_string(m);
          cerr << " })";
        }
        cerr << " }" << endl;
      }
    }
    return;
  }


  template <typename ElemActionSet>
  void print_elementary_actions(
      const ElemActionSet & eas
      )
  {
    BOOST_FOREACH( const typename ElemActionSet::value_type & ea, eas )
    {
      if ( ea.target_type() == typeid(ModifyInternalState) )
      {
        const ModifyInternalState * ea_ptr = (const ModifyInternalState *)(ea.template target<const ModifyInternalState>());
        std::cout << "Modify Intenal State: agent = " << to_string(ea_ptr->agent) << ", site name = " << ea_ptr->site_name
                  << ", new internal state = " << ea_ptr->new_internal_state << std::endl;
      }
      else if ( ea.target_type() == typeid(Bind) )
      {
        const Bind * ea_ptr = (const Bind *)(ea.template target<const Bind>());
        std::cout << "Bind: agents = " << to_string(ea_ptr->agents.first) << " -- " << to_string(ea_ptr->agents.second)
                  << ", site names = " << ea_ptr->site_names.first << " -- " << ea_ptr->site_names.second << std::endl;
      }
      else if ( ea.target_type() == typeid(Unbind) )
      {
        const Unbind * ea_ptr = (const Unbind *)(ea.template target<const Unbind>());
        if ( ea_ptr->agents.second.name.empty() )
          std::cout << "Unbind: agent = " << to_string(ea_ptr->agents.first)
                    << ", site name = " << ea_ptr->site_names.first << std::endl;
        else
          std::cout << "Unbind: agents = " << to_string(ea_ptr->agents.first) << " -|- " << to_string(ea_ptr->agents.second)
                    << ", site names = " << ea_ptr->site_names.first << " -|- " << ea_ptr->site_names.second << std::endl;
      }
      else if ( ea.target_type() == typeid(Create) )
      {
        const Create * ea_ptr = (const Create *)(ea.template target<const Create>());
        std::cout << "Create: agent = " << to_string(ea_ptr->agent) << std::endl;
      }
      else if ( ea.target_type() == typeid(Destroy) )
      {
        const Destroy * ea_ptr = (const Destroy *)(ea.template target<const Destroy>());
        std::cout << "Destroy: agent = " << to_string(ea_ptr->agent) << std::endl;
      }
    }
  }

} // kappa

#endif // DEBUG_UTILS_HPP
