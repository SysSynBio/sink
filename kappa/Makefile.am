## Makefile.am -- Process this file with automake to produce Makefile.in
INCLUDES = -I../
AM_CXXFLAGS = --std=c++0x
AM_CPPFLAGS = -DBOOST_NO_SLIST -DBOOST_NO_HASH 
bin_PROGRAMS = kappa
kappa_SOURCES = kappa.cpp
noinst_PROGRAMS = bench
bench_SOURCES = bench.cpp
pkginclude_HEADERS = config.hpp agent.hpp bond_map.hpp expression.hpp \
    complex.hpp pattern.hpp rule.hpp map.hpp lift_map.hpp lexer.hpp \
    gillespie.hpp system.hpp random.hpp debug.hpp
