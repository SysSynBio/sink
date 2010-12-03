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

#ifndef KAPPA_RANDOM_HPP
#define KAPPA_RANDOM_HPP
#include <ctime>
#include <boost/random.hpp>

namespace kappa
{

  double random_uniform01()
  {
    static boost::mt19937 rng(time(NULL));
    static boost::uniform_real<> uniform01;
    static boost::variate_generator<boost::mt19937&, boost::uniform_real<> > random01(rng, uniform01);

    return random01();
  }

  double random_uniform_int(int start, int end)
  {
    static boost::mt19937 rng(time(NULL));
    boost::uniform_int<int> uniform_int(start, end);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<int> > random_int(rng, uniform_int);

    return random_int();
  }

} // kappa

#endif // KAPPA_RANDOM_HPP
