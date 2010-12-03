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

#include <kappa/system.hpp>
#include <kappa/debug.hpp>
#define USAGE usage(argv[0])

using namespace kappa;
typedef Expression expression;
typedef Rule<expression> rule;
typedef std::vector<rule> RuleSet;

void usage(char * prog_name)
{
  std::cout << "Usage: " << prog_name << " [ -t time | -n num_steps ] <system_description_file>" << std::endl;
  exit(EXIT_FAILURE);
}

template <typename S>
void print_line(
    int iter_cnt,
    const S & s
    )
{
  std::cout << "iteration " << iter_cnt << ": ";
  ComplexCount cc( get_complex_count(s.mixture) );
  int cnt = 0, size = cc.size();
  long long int last_int = 0; std::string last_str;
  if (size == 1)
  {
    BOOST_FOREACH( const typename ComplexCount::value_type & i, cc )
      std::cout << i.first << " -> " << i.second; // print the only one complex count
  }
  else
  {
    BOOST_FOREACH( const typename ComplexCount::value_type & i, cc )
    {
      if (++cnt >= size)
      {
        last_str = i.first;
        last_int = i.second;
        break;
      }
      std::cout << i.first << " -> " << i.second << ", ";
    }
    std::cout << last_str << " -> " << last_int;
  }
  std::cout << " (t = " << s.t << ")" << std::endl;
#ifdef KAPPA_DEBUG
  std::cout << "Activities:" << std::endl;
  BOOST_FOREACH( const rule & r, s.rules )
    std::cout << "  rule '" << to_string(r) << "'[" << &r << "]: " << s.gillespie[ r ] << std::endl;
#endif
  return;
}

int main(
    int argc,
    char * argv []
    )
{
  typedef System<LinearGillespie, RuleSet> system;

  double time = 0.0;
  int num_steps = 0, rpd = 1;
  std::string input_filename;
  try {
    for (int i = 1; i < argc; ++i)
    {
      if ( strcmp(argv[i], "-t") == 0 )
      {
        if (num_steps != 0)
          USAGE;

        std::string time_str = argv[++i];
        int last_pos = time_str.size() - 1;
        if ( time_str[last_pos] == 's' ) // seconds
          time = from_string<double>(time_str.substr(0, last_pos));
        else if ( time_str[last_pos] == 'm' ) // minutes
          time = from_string<double>(time_str.substr(0, last_pos)) * 60;
        else if ( time_str[last_pos] == 'h' ) // hours
          time = from_string<double>(time_str.substr(0, last_pos)) * 3600;
        else if ( time_str[last_pos] == 'd' ) // days
          time = from_string<double>(time_str.substr(0, last_pos)) * 86400;
        else
          time = from_string<double>(time_str);
      }
      else if ( strcmp(argv[i], "-n") == 0 )
      {
        if (time != 0.0)
          USAGE;

        num_steps = from_string<int>(argv[++i]);
      }
      else if ( strcmp(argv[i], "-rpd") == 0 )
      {
        rpd = from_string<int>(argv[++i]);
      }
      else
      {
        input_filename = argv[i];
      }
    }
  }
  catch (ConversionFailure &)
  {
    USAGE;
  }

  if (input_filename.empty())
    USAGE;

  std::ifstream infile(input_filename.c_str());
  system s = read_system<LinearGillespie, RuleSet>(infile);
  /*
  std::cout << "Elementary actions:" << std::endl;
  BOOST_FOREACH( rule & r, s.rules )
  {
    std::cout << "  rule '" << to_string(r) << "': ";
    print_elementary_actions( r.elementary_actions() );
  }
  */
  infile.close();
  int i = 0;
  if (num_steps != 0)
  {
    for (; i < num_steps; ++i)
    {
      if ( i % rpd == 0 )
      { // print current state
        print_line(i, s);
      }
      s.iterate();
    }
  }
  else
  {
    while (s.t < time)
    {
      if ( i % rpd == 0 )
      { // print current state
        print_line(i, s);
      }
      s.iterate();
      ++i;
    }
  }
  if ( i % rpd == 0 )
  { // print final state
    print_line(i, s);
  }
  return(0);
}
