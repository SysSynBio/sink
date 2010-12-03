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

#ifndef AGENT_HPP
#define AGENT_HPP
#include <kappa/config.hpp>

namespace kappa
{

  struct Site
  {
    /*
    Site() // default ctor
      : name(), internal_state(), binding_state() { }
    */

    Site(const std::string & _name, const std::string & _is, const std::string & _bs)
      : name(_name), internal_state(_is), binding_state(_bs) { }

    Site(const std::string & _name)
      : name(_name), internal_state(), binding_state() { }

    std::string name;
    std::string internal_state;
    std::string binding_state;
  };

  struct Agent
  {
    typedef std::vector<Site> Interface;
    typedef Interface::iterator Iiter;
    typedef Interface::const_iterator Iconst_iter;
    typedef Site site;

    /*
    Agent() // default ctor
      : name(), interface() { }
    */

    Agent(const std::string & _name, const std::vector<Site> & _interface)
      : name(_name), interface(_interface) { }

    Agent(const std::string & _name)
      : name(_name), interface() { }

    void add_site(const Site & s)
    { interface.push_back(s); }

    Iiter find_site(const std::string & site_name)
    {
      for (Iiter i = interface.begin(), end = interface.end(); i != end; ++i)
        if (i->name == site_name)
          return i;
      return interface.end();
    }

    Iconst_iter find_site(const std::string & site_name) const
    {
      for (Iconst_iter i = interface.begin(), end = interface.end(); i != end; ++i)
        if (i->name == site_name)
          return i;
      return interface.end();
    }

    void remove_site(const std::string & site_name)
    {
      for (Iiter i = interface.begin(), end = interface.end(); i != end; ++i)
        if (i->name == site_name)
          interface.erase(i);
      return;
    }

    std::string name;
    Interface interface;
  };

  // to_string specializations for Site and Agent
  template <>
  inline std::string to_string<Site>(const Site & s)
  {
    return s.name + (s.internal_state.empty()? "" : "~" + s.internal_state) + (s.binding_state.empty()? "" : "!" + s.binding_state);
  }

  template <>
  inline std::string to_string<Agent>(const Agent & a)
  {
    if ( a.interface.empty() )
      return a.name;

    std::string output = a.name + "(";
    BOOST_FOREACH( const Site & s, a.interface )
      output += to_string(s) + ",";
    output.erase( output.size() - 1 );
    output += ")";
    return output;
  }

  // Validation functions
  inline bool valid_internal_state(
      const std::string & is
      )
  {
    BOOST_FOREACH( const char & c, is )
      if ( not isalnum(c) )
        return false;
    return true;
  }

  inline bool is_bond_label(
      const std::string & bs
      )
  {
    if (bs.empty())
      return false;

    BOOST_FOREACH( const char & c, bs )
      if ( not isdigit(c) )
        return false;
    return true;
  }
  
  inline bool valid_binding_state(
      const std::string & bs,
      bool as_pattern = false
      )
  {
    // binding states "-" and "?" are only accepted in patterns
    if (bs == "" or ((bs == "-" or bs == "?") and as_pattern))
      return true;
    return is_bond_label(bs);
  }

  inline bool valid_site(
      const Site & s,
      bool as_pattern = false
      )
  {
    if ( not valid_internal_state(s.internal_state) )
      return false;
    if ( not valid_binding_state(s.binding_state, as_pattern) )
      return false;
    return true;
  }

  bool same_site_name(const Site & s1, const Site & s2)
  { return s1.name == s2.name; }

  bool valid_agent(
      const Agent & a,
      bool as_pattern = false
      )
  {
    if ( a.interface.empty() and not a.name.empty() )
      return true;

    // every site need to have a unique name in the interface (definition 2.3, x, Kappa language basics)
    // compare_every_two_elems_until returns true if there's found two element of interface
    // which satisfies the condition elem1.name == elem2.name
    if ( compare_every_two_elems_until( a.interface, &same_site_name ) )
      return false;

    // agent's sites must be valid
    BOOST_FOREACH( const Site & s, a.interface )
      if ( not valid_site(s, as_pattern) )
        return false;

    // if we've passed all checks, the agent is well formed
    return true;
  }

} // kappa

#endif // AGENT_HPP
