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

#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sstream>
#include <cctype>

#include <vector>
#include <list>
#include <set>
#include <algorithm>

#ifdef __GNUC__
  #if __GNUC__ >= 4 && __GNUC_MINOR__ >= 3
    #include <unordered_map>
  #else
    #include <ext/hash_map>
  #endif
#else
  #include <unordered_map>
#endif

#include <boost/foreach.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/functional/hash.hpp>
#include <boost/tokenizer.hpp>


namespace kappa
{
  using namespace boost::lambda;

  /// @brief Compares every possible pair of elements within a container until a pair satisfies the condition,
  /// and returns true. If no pair of elements satisfies the condition, returns false.
  /// The condition must be a lambda functor, a boost.function object or a function pointer (really?).
  template <typename Container, typename Function>
  bool compare_every_two_elems_until(
      const Container & list,
      Function condition
      )
  {
    typedef typename Container::const_iterator Titer;
    // The first iterator goes from the first element to the next to the last
    // and the second iterator from the next to the first iterator to the last
    // So we can compare every pair just once.
    Titer i1_end = list.end() - 1,
          i2_end = list.end();
    for (Titer i1 = list.begin(); i1 != i1_end; ++i1)
      for (Titer i2 = i1 + 1; i2 != i2_end; ++i2)
        if ( condition(*i1, *i2) )
          return true;
    return false;
  }

  /// @brief Exception raised when conversion to and from strings isn't possible.
  struct ConversionFailure { };

  /// @brief Tries to convert anything to string
  template <typename T>
  inline std::string to_string(const T & value)
  {
    std::stringstream ss;
    if (ss << value)
      return ss.str();
    throw ConversionFailure();
  }

  template <typename InputIterator>
  inline std::string to_string(InputIterator i1, const InputIterator & i2)
  {
    std::string out = "{ ";
    InputIterator end = i2; --end;
    for (; i1 != end; ++i1) out += to_string(*i1) + ", ";
    return out + to_string(*end) + " }";
  }

  /// @brief Tries to convert a string to a given data type
  template <typename T>
  inline T from_string(const std::string& s)
  {
    T result;
    std::istringstream stream(s);
    if (stream >> result)
      return result;
    throw ConversionFailure();
  }
} // kappa

#endif // CONFIG_HPP
