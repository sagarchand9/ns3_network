/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h" //for netanim

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int main (int argc, char *argv[])
{
  uint32_t nPackets = 1;//Default value of nPackets
  CommandLine cmd;
  cmd.AddValue("nPackets", "Number of packets to echo", nPackets);//Taking number of packets to be transmitted as cmd line argument
  cmd.Parse (argc, argv);

  // Take logs
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  // Take n number of computer
  NodeContainer nodes;
  nodes.Create (2);

  // Choose your technology to communicate
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // Install technology on computers
  NetDeviceContainer devices;
  devices = pointToPoint.Install (nodes);

  // Asking to follow rules
  InternetStackHelper stack;
  stack.Install (nodes);

  // Assign IP address to communicate
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");//base IP and subnet. base IP = All IPs are to be assigned on forward of it
  Ipv4InterfaceContainer interfaces = address.Assign (devices);

 // Create a x type of server on port x
  UdpEchoServerHelper echoServer (9);

  // Install server on a node and then Start and stop server
  ApplicationContainer serverApps = echoServer.Install (nodes.Get (1));//Doing work on node index 1 (index starts with 0) Here we have 2 nodes
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  // Create x type of client and set its attributes
  UdpEchoClientHelper echoClient (interfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (nPackets));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  // Install the server then start and stop it
  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (2.0)); // Client should start after server has started
  clientApps.Stop (Seconds (10.0));
  
  /*// ASCII tracing. File is created in top level directory
  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("try_first.tr"));
  */
  
  // PCAP tracing
  pointToPoint.EnablePcapAll ("try_first");
  
  // Creating xml file for netanim before simulation runs
  AnimationInterface anim ("anim1.xml");
  anim.SetConstantPosition(nodes.Get(0), 1.0, 2.0);
  anim.SetConstantPosition(nodes.Get(1), 2.0, 3.0);

  // Run the simulation
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
