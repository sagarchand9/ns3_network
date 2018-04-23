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

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Wireless");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nCsma = 3;
  uint32_t nWifi = 3;
  bool tracing = true;

  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

  NodeContainer c;
  c.Create (9);
  NodeContainer n0n1 = NodeContainer (c.Get (0), c.Get (1));
  NodeContainer n1234 = NodeContainer (c.Get (1), c.Get (2), c.Get (3), c.Get (4));
  NodeContainer n4n5 = NodeContainer (c.Get (4), c.Get (5));
  NodeContainer n5678 = NodeContainer (c.Get (5), c.Get (6), c.Get (7), c.Get (8));

  InternetStackHelper internet;
  internet.Install (c.Get(1));
  internet.Install (c.Get(2));
  internet.Install (c.Get(3));
  internet.Install (c.Get(4));
  internet.Install (c.Get(5));
  internet.Install (c.Get(6));
  internet.Install (c.Get(7));
  internet.Install (c.Get(8));

  PointToPointHelper p2p_1;
  p2p_1.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p_1.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer d0d1 = p2p_1.Install (n0n1);

  CsmaHelper csma_1;
  csma_1.SetChannelAttribute ("DataRate", StringValue ("5Mbps"));
  csma_1.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer d1234 = csma_1.Install (n1234);

  PointToPointHelper p2p_2;
  p2p_2.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  p2p_2.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer d4d5 = p2p_2.Install (n4n5);

  CsmaHelper csma_2;
  csma_2.SetChannelAttribute ("DataRate", StringValue ("5Mbps"));
  csma_2.SetChannelAttribute ("Delay", StringValue ("2ms"));
  NetDeviceContainer d5678 = csma_2.Install (n5678);

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = c.Get (0);

  //Interconnection between channel and wifi node
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_1;
  p2pInterfaces_1 = address.Assign (d0d1);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces_1;
  csmaInterfaces_1 = address.Assign (d1234);

  address.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces_2;
  p2pInterfaces_2 = address.Assign (d4d5);

  address.SetBase ("10.1.5.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces_2;
  csmaInterfaces_2 = address.Assign (d5678);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (c.Get(0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (csmaInterfaces_1.GetAddress (nCsma), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = 
    echoClient.Install (wifiStaNodes.Get (nWifi - 1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (10.0));

  if (tracing == true)
    {
      p2p_1.EnablePcapAll ("try_thrd");
      p2p_2.EnablePcapAll ("try_third");
      phy.EnablePcap ("try_thrd", apDevices.Get (0));
      csma_1.EnablePcap ("try_thrd", d1234.Get (0));
      csma_2.EnablePcap ("try_third", d5678.Get (0));
    }

  AnimationInterface anim ("anim2.xml");
  //anim.SetConstantPosition(c.Get(0), 0.0, 20.0);
//  anim.SetConstantPosition(p2pNodes.Get(1), 17.0, 1.0);
  anim.SetConstantPosition(n1234.Get(0), 50.0, 0.0);
  anim.SetConstantPosition(n1234.Get(1), 55.0, 0.0);
  anim.SetConstantPosition(n1234.Get(2), 60.0, 0.0);
  anim.SetConstantPosition(n1234.Get(3), 65.0, 0.0);
  anim.SetConstantPosition(n5678.Get(0), 65.0, 5.0);
  anim.SetConstantPosition(n5678.Get(1), 65.0, 10.0);
  anim.SetConstantPosition(n5678.Get(2), 65.0, 15.0);
  anim.SetConstantPosition(n5678.Get(3), 65.0, 20.0);
//
//  anim.SetConstantPosition(wifiStaNodes.Get(0), 0.0, 5.0);
//  anim.SetConstantPosition(wifiStaNodes.Get(1), 0.0, 10.0);
//  anim.SetConstantPosition(wifiStaNodes.Get(2), 0.0, 15.0);

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
