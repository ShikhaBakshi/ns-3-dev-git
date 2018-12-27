/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 NITK Surathkal
 *
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
 * Authors: Shikha Bakshi <shikhabakshi912@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

#include <iostream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/traffic-control-module.h"

using namespace ns3;

// All the pcaps will be collected in a folder named dsack in the ns-3 root directory. DSACK block can be found in the entries of acknowledgement for duplicate packets in the pcaps.
Ptr<UniformRandomVariable> uv = CreateObject<UniformRandomVariable> ();
std::string dir = "dsack/";
double stopTime = 10;

int main (int argc, char *argv[])
{
  uint32_t stream = 1;
  uint32_t dataSize = 1000;
  uint32_t delAckCount = 1;
  bool dsack = true;

  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime (&rawtime);

  strftime (buffer,sizeof(buffer),"%d-%m-%Y-%I-%M-%S",timeinfo);
  std::string currentTime (buffer);

  dir += (currentTime + "/");
  std::string dirToSave = "mkdir -p " + dir;
  system (dirToSave.c_str ());
  system ((dirToSave + "/pcap/").c_str ());

  CommandLine cmd;
  cmd.AddValue ("stream", "Seed value for random variable", stream);
  cmd.AddValue ("delAckCount", "Delayed ack count", delAckCount);
  cmd.AddValue ("stopTime", "Stop time for applications / simulation time will be stopTime", stopTime);
  cmd.AddValue ("dsack", "DSACK mode", dsack);
  cmd.Parse (argc,argv);

  uv->SetStream (stream);

  // Create nodes
  NodeContainer senders, routers, receivers;
  routers.Create (2);
  senders.Create (1);
  receivers.Create (1);

  // Create point-to-point channels
  PointToPointHelper p2p;
  p2p.SetQueue ("ns3::ReorderQueue");      // Apply Reorder Queue to enable packet reordering
  p2p.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("2ms"));

  // Create netdevices
  std::vector <NetDeviceContainer> leftToRouter;
  std::vector <NetDeviceContainer> routerToRight;

  // Router1 and Router2
  NetDeviceContainer r1r2ND = p2p.Install (routers.Get (0), routers.Get (1));

  // Sender i and Receiver i
  for (int i = 0; i < 1; i++)
    {
      leftToRouter.push_back (p2p.Install (senders.Get (i), routers.Get (0)));
      routerToRight.push_back (p2p.Install (routers.Get (1), receivers.Get (i)));
    }

  // Install Stack
  InternetStackHelper stack;
  stack.Install (routers);
  stack.Install (senders);
  stack.Install (receivers);

  Ipv4AddressHelper ipAddresses ("10.0.0.0", "255.255.255.0");

  Ipv4InterfaceContainer r1r2IPAddress = ipAddresses.Assign (r1r2ND);
  ipAddresses.NewNetwork ();

  std::vector <Ipv4InterfaceContainer> leftToRouterIPAddress;

  for (int i = 0; i < 1; i++)
    {
      leftToRouterIPAddress.push_back (ipAddresses.Assign (leftToRouter [i]));
      ipAddresses.NewNetwork ();
    }

  std::vector <Ipv4InterfaceContainer> routerToRightIPAddress;

  for (int i = 0; i < 1; i++)
    {
      routerToRightIPAddress.push_back (ipAddresses.Assign (routerToRight [i]));
      ipAddresses.NewNetwork ();
    }

  Config::SetDefault ("ns3::TcpSocket::DelAckCount", UintegerValue (delAckCount));
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (dataSize));
  Config::SetDefault ("ns3::TcpSocketBase::Dsack", BooleanValue (dsack));
  Config::SetDefault ("ns3::TcpSocketBase::Sack", BooleanValue (true));

  AsciiTraceHelper asciiTraceHelper;
  Ptr<OutputStreamWrapper> streamWrapper;

  uint16_t port = 50000;

  // Install Sink Applications
  PacketSinkHelper sink ("ns3::TcpSocketFactory",
                         InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = sink.Install (receivers.Get (0));
  sinkApps.Start (Seconds (Seconds (0)));
  sinkApps.Stop (Seconds (stopTime));


  // Install BulkSend Application
  BulkSendHelper source ("ns3::TcpSocketFactory",
                         InetSocketAddress (routerToRightIPAddress [0].GetAddress (1), port));

  source.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer sourceApps = source.Install (senders.Get (0));
  sourceApps.Start (Seconds (0));
  sourceApps.Stop (Seconds (stopTime));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  p2p.EnablePcapAll (dir + "pcap/N", true);

  Simulator::Stop (Seconds (stopTime));
  Simulator::Run ();

  Simulator::Destroy ();
  return 0;
}


