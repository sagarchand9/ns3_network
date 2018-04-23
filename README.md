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

It is of the format 

#### `<file_name>-<node>-<device>.pcap`

These files are created in parent directory

## TO RUN VISUAL ANIMATION 

Visual animation is run on netanim. Run ./Netanim in netanim folder. Then choose the .xml when UI appears. xml file is created by the code. 


