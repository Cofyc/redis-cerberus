A Redis cluster proxy.

Build
=====

    make

or compile with g艹

    make COMPILER=g++

Run
===

    ./cerberus example.conf

The argument is path of a configuration file, which should contains at least

* bind : (integer) local port to listen
* node : (address) one of active node in a cluster; format should be host:port
