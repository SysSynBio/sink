## Makefile.am -- Process this file with automake to produce Makefile.in
INCLUDES = -I../
LIBS = -lboost_unit_test_framework
AM_CPPFLAGS = -DBOOST_TEST_DYN_LINK -DBOOST_NO_SLIST -DBOOST_NO_HASH
AM_CXXFLAGS = -std=c++0x
noinst_PROGRAMS = agent expression complex pattern rule lexer maps lift_map \
    gillespie system
agent_SOURCES = agent.cpp
expression_SOURCES = expression.cpp
complex_SOURCES = complex.cpp
pattern_SOURCES = pattern.cpp
rule_SOURCES = rule.cpp
lexer_SOURCES = lexer.cpp
maps_SOURCES = map.cpp
lift_map_SOURCES = lift_map.cpp
gillespie_SOURCES = gillespie.cpp
system_SOURCES = system.cpp
TESTS = agent expression complex pattern rule lexer maps lift_map \
    gillespie system
##XFAIL_TESTS = map
