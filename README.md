## Memento: Making Sliding Windows Efficient for Heavy Hitters

### We release an open source of all our algorithms.

In (Memento/hhh_source/windowHHH) you can find the Makefile that compiles:
hhh1d - which is the implementation of H-Memento for 1D IP hierarchies.
hhh2d - which is the implementation of H-Memento for 2D IP hierarchies.

In (Memento/hhh_source/windowHH) we have the source code for the (non-hierarchical) Memento. We will shortly add a Makefile that compiles that.

The executables have a command line of the format:
<exec_name> <nr_packets> <window_size> <nr_counters> <insert_prob> 

Here:
<exec_name> is the path for the executable (e.g., hhh1d ).
<nr_packets> is the length of the stream (in packets).
<window_size> is the size of the window (in packets).
<nr_counters> the number of counters per hierarchy node (e.g., in 1D the overall number of counters will be 5*nr_counters).
<insert_prob> is the parameter tau from the paper -- the rate in which each packet is sampled.

The executables read the input trace from stdin (redirect from a file).
The trace, for 1D traces, needs to be of the format
%d %d %d %d
Where these are the four bytes of the IP address.

For 2D traces, each line has the format
%d %d %d %d %d %d %d %d
Where the first four bytes are the src-ip and the next four are the dst-ip.

You can see an example trace here: https://github.com/ranbenbasat/RHHH/blob/master/dataSample

### HAProxy extension:
A basic tutorial: https://www.howtoforge.com/tutorial/ubuntu-load-balancer-haproxy/

#### Compile HAProxy:

cd ./Memento/haproxy/haproxy-1.8.1
make TARGET=linux2628 CPU=native USE_STATIC_PCRE=1 USE_OPENSSL=1 USE_ZLIB=1

#### Start HAProxy:

sudo ./haproxy -f PATH_TO_CONFIG_FILE/haproxy.cfg

#### About:

For each entering HTTP request, HAProxy prints out the source IP address of HTTP request and the number of its appearances over the hierarchies.

#### Most relevant parts of the code: 

proto_http.c (lines 3406-3489) - here, it is possible to add mitigation (e.g., deny or tarpit) and control the export of data (e.g., print to a terminal, write to file, etc.).

haproxy.c (lines 2449-2485)  - Initialization. Here, it is possible to change the window size, open an additional socket to a controller, etc.

