#include <kappa/system.hpp>

using namespace kappa;
typedef Expression expression;
typedef Rule<expression> rule;
typedef std::vector<rule> RuleSet;

int main(
    int argc,
    char * argv []
    )
{
  typedef System<LinearGillespie, RuleSet> system;

  if (argc != 2)
  {
    std::cerr << "Usage: " << argv[0] << " <system-description-file>" << std::endl;
    return(EXIT_FAILURE);
  }

  std::ifstream infile(argv[1]);
  system s = read_system<LinearGillespie, RuleSet>(infile);
  infile.close();
  std::clock_t start(clock());
  long num_steps = 10000000L;
  for (long i = 0L; i < num_steps; ++i)
  {
    s.iterate();
  }
  std::cout << (clock() - start) / CLOCKS_PER_SEC << " segs" << std::endl;

  return(0);
}
