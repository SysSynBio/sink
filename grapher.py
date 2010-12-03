from sys import argv, exit
import re, math
from matplotlib import rcParams
rcParams.update( {'backend': 'Agg'} )
import pylab

golden_mean = (math.sqrt(5) - 1.0) / 2.0

arg_index = 1
filename = ''
complex_list = ''
rpd = 1
while arg_index < len(argv):
  if argv[ arg_index ] == '-c':
    complex_list = argv[ arg_index + 1 ]
    arg_index += 1
  elif argv[ arg_index ] == '-rpd':
    rpd = int( argv[ arg_index + 1 ] )
    arg_index += 1
  else:
    filename = argv[ arg_index ]
  arg_index += 1

complexes_to_plot = []
if complex_list != '':
  complexes_to_plot = complex_list.split(':')

if filename == '':
  print 'Usage:', argv[0], '<input_file> [ -c complex_list ]'
  print 'complex_list is a colon-separated list of complexes. Those complexes will be plot.'
  exit(1)

times = []
complexes = {}
iteration_cnt = 0
complex_re = r'[\w,|()]+' # this RE matches any agent
complex_count_re = complex_re + r' -> \d+' # this matches an agent plus its count info
complex_count_re = re.compile( complex_count_re )
time_re = re.compile( r'\(t = ([\d.eE\-]+)\)' ) # this RE matches the time information normally found at the end of every inputfile line and captures just the number in group 1
inputfile = open( filename, 'r' )
for line in inputfile:
  if line.startswith('iteration'):
    found_complexes = complexes.keys()
    complex_count_list = complex_count_re.findall( line )
    for complex_count in complex_count_list:
      complex = complex_count.split('->')[0].strip()
      count = int( complex_count.split('->')[1].strip() )
      if complex not in complexes:
        complexes[ complex ] = []
        for i in range(iteration_cnt):
          complexes[ complex ].append( 0 )
      complexes[ complex ].append( count )
      if complex in found_complexes:
        found_complexes.remove( complex )
    for complex in found_complexes:
      complexes[ complex ].append( 0 )
    times.append( float( time_re.search( line ).group(1) ) ) # get the '...' in (t = ...)
    iteration_cnt += 1
inputfile.close()

fig = pylab.figure( figsize=(20.0, golden_mean * 20.0) )
if not complexes_to_plot:
  complexes_to_plot = complexes.keys()

for complex in complexes_to_plot:
  pylab.plot( times[::rpd], complexes[ complex ][::rpd], label=complex )
pylab.legend()
fig.savefig( filename + '.png' )
