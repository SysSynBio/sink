## Sink

Sink is a open-source implementation of the Kappa languange.
Kappa language is a formal lenguage for the analysis and simulation
of rule-based models of biochemical interaction networks.

#### Installation
To install it run the following commands inside the main directory:

    ./bootstrap.sh
    cp /usr/share/libtool/config/ltmain.sh . # this may depend on your Unix distribution
    ./bootstrap.sh
    ./configure
    make
    make install

Note that Sink's compilation requires GCC version 4.3 or higher!
Also, Sink requires Autotools and the Boost libraries to be properly setted up in your computer.
