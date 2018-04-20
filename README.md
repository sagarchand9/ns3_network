## TO RUN

goto folder

#### `~/ns3/repos/ns-3-allinone/ns-3-dev`


now run the command

#### `./waf --run examples/tutorial/try_first`


Here **try_first.cc** is file and you run **try_first**

## VIEW TRACE FILE LOGS

To view contents of trace file(.pcap), either use tools like wireshark or run

#### `tcpdump -nn -tt -r try_first-0-0.pcap`


#### `tcpdump -nn -tt -r try_first-1-0.pcap` 

and so on for all the nodes


These files are created in parent directory
