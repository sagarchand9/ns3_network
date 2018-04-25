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
 *
 */

// This script exercises global routing code in a mixed point-to-point
// and csma/cd environment
//
// Network topology
//
//  n0
//     \ p-p
//      \          (shared csma/cd)
//       n2 -------------------------n3
//      /            |        | 
//     / p-p        n4        n5 ---------- n6
//   n1                             p-p
//
// - CBR/UDP flows from n0 to n6
// - Tracing of queues and packet receptions to file "mixed-global-routing.tr"

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MixedGlobalRoutingExample");

int main (int argc, char *argv[])
{
  uint32_t nPackets = 1;//Default value of nPackets

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", UintegerValue (210));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue ("448kb/s"));

  // Allow the user to override any of the defaults and the above
  // Bind ()s at run-time, via command-line arguments
  CommandLine cmd;
  cmd.AddValue("nPackets", "Number of packets to echo", nPackets);//Taking number of packets to be transmitted as cmd line argument
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes.");
  NodeContainer c;
  c.Create (10);
  NodeContainer n0n2 = NodeContainer (c.Get (0), c.Get (2));
  NodeContainer n1n2 = NodeContainer (c.Get (1), c.Get (2));
  NodeContainer n5n6 = NodeContainer (c.Get (5), c.Get (6));
  NodeContainer n2345 = NodeContainer (c.Get (2), c.Get (3), c.Get (4), c.Get (5));
  NodeContainer n6789 = NodeContainer (c.Get (6), c.Get (7), c.Get (8), c.Get (9));

  InternetStackHelper internet;
  internet.Install (c);

  // We create the channels first without any IP addressing information
  NS_LOG_INFO ("Create channels.");
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer d0d2 = p2p.Install (n0n2);

  NetDeviceContainer d1d2 = p2p.Install (n1n2);

  p2p.SetDeviceAttribute ("DataRate", StringValue ("1500kbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));
  NetDeviceContainer d5d6 = p2p.Install (n5n6);

  // We create the channels first without any IP addressing information
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("5Mbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer d2345 = csma.Install (n2345);

  csma.SetChannelAttribute ("DataRate", StringValue ("5Mbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer d6789 = csma.Install (n6789);
  // Later, we add IP addresses.
  NS_LOG_INFO ("Assign IP Addresses.");
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  ipv4.Assign (d0d2);

  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  ipv4.Assign (d1d2);

  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer i5i6 = ipv4.Assign (d5d6);

  ipv4.SetBase ("10.250.1.0", "255.255.255.0");
  ipv4.Assign (d2345);


  ipv4.SetBase ("10.250.2.0", "255.255.255.0");
  Ipv4InterfaceContainer i6789 = ipv4.Assign (d6789);;

  // Create router nodes, initialize routing database and set up the routing
  // tables in the nodes.
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Create the OnOff application to send UDP datagrams of size
  // 210 bytes at a rate of 448 Kb/s
  NS_LOG_INFO ("Create Applications.");
/*  uint16_t port = 9;   // Discard port (RFC 863)
  OnOffHelper onoff ("ns3::UdpSocketFactory",
                     InetSocketAddress (i6789.GetAddress (3), port));
  onoff.SetConstantRate (DataRate ("300bps"));
  onoff.SetAttribute ("PacketSize", UintegerValue (50));

  ApplicationContainer apps = onoff.Install (c.Get (0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));
*/
  UdpEchoServerHelper echoServer (9);
  
  ApplicationContainer serverApps = echoServer.Install (c.Get(0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (i6789.GetAddress (3), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (nPackets));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (c.Get (1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));




  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("mixed-global-routing.tr");
  p2p.EnableAsciiAll (stream);
  csma.EnableAsciiAll (stream);

  p2p.EnablePcapAll ("mixed-global-routing");
  csma.EnablePcapAll ("mixed-global-routing", false);

  AnimationInterface anim ("anim2.xml");

  anim.SetConstantPosition(n2345.Get(0), 50.0, 0.0);
  anim.SetConstantPosition(n2345.Get(1), 55.0, 0.0);
  anim.SetConstantPosition(n2345.Get(2), 60.0, 0.0);
  anim.SetConstantPosition(n2345.Get(3), 65.0, 0.0);
  anim.SetConstantPosition(n5n6.Get(1), 65.0, 10.0);
  anim.SetConstantPosition(n1n2.Get(0), 50.0, 10.0);
  anim.SetConstantPosition(n0n2.Get(0), 55.0, 10.0);
  anim.SetConstantPosition(n6789.Get(1), 65.0, 20.0);
  anim.SetConstantPosition(n6789.Get(2), 65.0, 25.0);
  anim.SetConstantPosition(n6789.Get(3), 65.0, 30.0);

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
