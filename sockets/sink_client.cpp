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

#include <iostream>
#include <cstdlib>
#include <fstream>

#include <kappa/expression.hpp>
#include <kappa/lexer.hpp>
#include <kappa/system.hpp>
#include <kappa/debug.hpp>

#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/device/back_inserter.hpp>

#include <boost/foreach.hpp>
#include <boost/asio.hpp>

#ifdef WITH_BOINC
  #include <boinc/boinc_api.h>
#endif

#define USAGE usage(argv[0])

using namespace std;
using namespace boost::asio;
using boost::asio::ip::tcp;
using namespace kappa;


std::string compress(
    const std::string & input
    )
{
  // Zlib compressor
  namespace io = boost::iostreams;
  io::filtering_ostream encode;
  encode.push(io::zlib_compressor());

  std::string encoded;
  encode.push(io::back_inserter(encoded));

  encode.write( input.c_str(), static_cast<std::streamsize>(input.size()) );
  encode.pop();
  return encoded;
}


struct Connection
{
  struct ConnectionFinished { };

private:
  io_service io_service_;
  ip::tcp::socket socket;
  static int const buf_size = 8192;

public:
  // Constructor
  Connection(std::string server_url)
    : io_service_(), socket(io_service_)
  {
    tcp::resolver resolver(io_service_);
    tcp::resolver::query query(server_url, "4444"); // EDIT PORT NUMBER HERE
    tcp::resolver::iterator iterator = resolver.resolve(query);
    socket.connect(*iterator);
  }

  // Destructor
  ~Connection()
  {
    socket.close();
  }

  // Member functions
  std::string communicate(
      const std::string & state
      )
  {
    // ********** Compress data ***********
    std::string encoded = compress(state);

    // ************ Send data *************
    send(encoded);
    cout << "Sent: " << encoded << endl;

    // *********** Receive data **********
    std::string updated_objs = receive();
    updated_objs = updated_objs.substr(0, updated_objs.length() - 1);
    cout << "Received: \"" << updated_objs << '"' << endl;
    return updated_objs;
  }

  std::string receive()
  {
    char reply[buf_size];
    size_t reply_length = read(socket, buffer(reply, buf_size), transfer_at_least(1));
    return std::string(reply, reply_length);
  }

  void send(
      const std::string & state
      )
  {
    write(socket, buffer(state, state.length()));
  }

  void die()
  {
    // Send a signal to the server

    throw ConnectionFinished();
  }

  void send_and_die(
      const std::string & state
      )
  {
    send(state);
    die();
  }
};


template <class System>
std::string get_state(
    const System & s
    )
{
  std::string state( to_string(s.mixture) );

  // Replace the commas that separates agents in the
  // mixture by semicolons, to make it easier to split
  std::string parseable_state;
  int level = 0;
  BOOST_FOREACH( char c, state )
  {
    if (c == ',' and level == 0)
      c = ';';
    else if (c == '(')
      ++level;
    else if (c == ')')
      --level;
    parseable_state += c;
  }
  return parseable_state;
}


template <class System>
void update_system(
    System & s,
    std::string updated_objs
    )
{
  if (updated_objs == "Die")
    throw Connection::ConnectionFinished();

  updated_objs += '+';

  // Parse input string
  std::list< kappa::Agent > added_agents;
  std::list< kappa::Agent > removed_agents;
  int add_remove_flag = 0; // 0: undefined
  std::string agent;
  BOOST_FOREACH( char c, updated_objs )
  {
    if (c == '+' || c == '-')
    {
      if (add_remove_flag == 1)
      { // add agent
        added_agents.push_back(
            kappa::from_string<kappa::Agent>(agent) );
      }
      else if (add_remove_flag == 2)
      { // remove agent
        removed_agents.push_back(
            kappa::from_string<kappa::Agent>(agent) );
      }

      agent.clear();

      if (c == '+')
        add_remove_flag = 1; // 1: add
      else
        add_remove_flag = 2; // 2: remove
    }
    else
    {
      agent += c;
    }
  }

  // Create new agents
  kappa::ModifiedSet created_set, removed_set, modified_set;
  BOOST_FOREACH( kappa::Agent & a, added_agents )
  {
    cout << "Creating agent: " << kappa::to_string(a) << endl;
    created_set.push_back( std::make_pair(
      &kappa::create_agent(a, s.mixture), "") );
  }

  // Delete agents
  BOOST_FOREACH( kappa::Agent & a, removed_agents )
  {
    BOOST_FOREACH( kappa::Agent & mixture_agent, s.mixture )
      if ( kappa::match_agent(a, mixture_agent) )
      {
        cout << "Destroying agent: " << kappa::to_string(mixture_agent) << endl;
        removed_set.push_back( std::make_pair( &mixture_agent, "" ) );
        kappa::destroy_agent( mixture_agent, s.mixture );
        break;
      }
  }

  // Update matching maps
  typedef typename kappa::Rule<Expression> Rule;
  typename System::RulePtrSet modified_rules;
  if (not created_set.empty())
  {
    std::vector<const Rule *> all_rules;
    BOOST_FOREACH( const Rule & r, s.rules )
      all_rules.push_back( &r );

    typename System::RulePtrSet modified_rules_add(
      s.positive_update(created_set, all_rules) );
    BOOST_FOREACH( const Rule * r, modified_rules_add )
      modified_rules.insert(r);
  }
  if (not removed_set.empty())
  {
    typename System::RulePtrSet modified_rules_del(
      s.negative_update(modified_set, removed_set) );
    BOOST_FOREACH( const Rule * r, modified_rules_del )
      modified_rules.insert(r);
  }

  // Update rule activities
  s.update_activities( modified_rules );

  return;
}


int main(
    int argc,
    char * argv []
    )
{
  typedef std::vector< Rule<Expression> > RuleSet;
  typedef System<LinearGillespie, RuleSet> system;
  
  #ifdef WITH_BOINC
  std::string resolved_input_filename;
  #endif

  if (argc != 4)
  {
    cerr << "Usage: " << argv[0] << " <system_description_file> <server_url> <rpd>" << endl;
    exit(EXIT_FAILURE);
  }
  
  std::string input_filename( argv[1] ),
              server_url( argv[2] );
  const int rpd = from_string<int>( argv[3] );

  cout << "Reading input file " << input_filename << endl;

  // ********** Read input file ***********
  #ifdef WITH_BOINC
  // Do this with boinc_fopen when we do it finally
  // Do more checks of file existance
  boinc_init(); // Where should this really go?
  boinc_resolve_filename_s(input_filename.c_str(), resolved_input_filename);
  std::ifstream infile(resolved_input_filename.c_str());
  #else
  std::ifstream infile(input_filename.c_str());
  #endif

  system s = read_system<LinearGillespie, RuleSet>(infile);
  infile.close();

  cout << "Readed" << endl;
  cout << "Connection to the server " << server_url << endl;

  // Create a Connection object to manage the socket
  Connection conn(server_url);

  cout << "Connected" << endl;

  // ************ Iteration ***************
  try {
    int i = 0;
    while (true)
    {
      if ( i % rpd == 0 ) // Send current state
        update_system(s, conn.communicate( get_state(s) ));
      s.iterate();
      ++i;
    }
  }
  catch (Connection::ConnectionFinished &) {
  }

  #ifdef WITH_BOINC
  boinc_finish(0);
  #endif
  
  return(0);
}
