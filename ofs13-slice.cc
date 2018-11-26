/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 University of Campinas (Unicamp)
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
 *
 * The switch is managed by the default learning controller application.
 *
 *                       Learning Controller
 *                                |
 *                       +-----------------+
 *      Host group 0 === |     OpenFlow    | === Server group 0
 *                       |                 |
 *      Host group 1 === |      switch     | === Server group 1
 *                       +-----------------+
 * 
 * Links:
 * Host group 0 -> Server group 0
 * Host group 1 -> Server group 1
 * 
 */

#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/ofswitch13-module.h>
#include <ns3/internet-apps-module.h>
#include "ns3/applications-module.h"

using namespace ns3;

int
main (int argc, char *argv[])
{
  uint16_t simTime = 10;
  bool verbose = false;
  bool trace = false;

  uint16_t numberOfHosts = 10;
  //uint16_t numberOfServers = 4;

  // Configure command line parameters
  CommandLine cmd;
  cmd.AddValue ("simTime", "Simulation time (seconds)", simTime);
  cmd.AddValue ("verbose", "Enable verbose output", verbose);
  cmd.AddValue ("trace", "Enable datapath stats and pcap traces", trace);
  cmd.AddValue ("numberOfHosts", "Number of hosts", numberOfHosts);
  cmd.Parse (argc, argv);

  if (verbose)
    {
      OFSwitch13Helper::EnableDatapathLogs ();
      LogComponentEnable ("OFSwitch13Interface", LOG_LEVEL_ALL);
      LogComponentEnable ("OFSwitch13Device", LOG_LEVEL_ALL);
      LogComponentEnable ("OFSwitch13Port", LOG_LEVEL_ALL);
      LogComponentEnable ("OFSwitch13Queue", LOG_LEVEL_ALL);
      LogComponentEnable ("OFSwitch13SocketHandler", LOG_LEVEL_ALL);
      LogComponentEnable ("OFSwitch13Controller", LOG_LEVEL_ALL);
      LogComponentEnable ("OFSwitch13LearningController", LOG_LEVEL_ALL);
      LogComponentEnable ("OFSwitch13Helper", LOG_LEVEL_ALL);
      LogComponentEnable ("OFSwitch13InternalHelper", LOG_LEVEL_ALL);
    }

  // Enable checksum computations (required by OFSwitch13 module)
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  // Create host nodes
  NodeContainer hosts;
  hosts.Create (numberOfHosts);

  // Create the switch node
  Ptr<Node> switchNode = CreateObject<Node> ();

  // Use the CsmaHelper to connect host nodes to the switch node
  CsmaHelper csmaHelper;
  csmaHelper.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
  csmaHelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  NetDeviceContainer roteador0;
  NetDeviceContainer roteador1;
  NodeContainer hostGroup0;
  NodeContainer hostGroup1;
  NetDeviceContainer switchPorts;

  /* Dividindo os dois grupos de Hosts
  * Ainda não tentamos fazer com os servidores, somente conectar os dois grupos de host
  */
  for (size_t i = 0; i < hosts.GetN ()/2; i++)
      hostGroup0.Add (hosts.Get (i));
  roteador0 = csmaHelper.Install(hostGroup0);

  for (size_t i = hosts.GetN ()/2; i < hosts.GetN (); i++)
      hostGroup1.Add (hosts.Get (i));
  roteador1 = csmaHelper.Install(hostGroup1);

  /*
  * Conecta os roteadores no switch (não funciona)
  */
  NodeContainer pair (hostGroup0.Get(1), switchNode);
  NetDeviceContainer link = csmaHelper.Install(pair);
  switchPorts.Add(link.Get(1));

  NodeContainer pair1 (hostGroup1.Get(4), switchNode);
  NetDeviceContainer link1 = csmaHelper.Install(pair1);
  switchPorts.Add(link1.Get(1));
  
  /*NodeContainer par;
  NetDeviceContainer pairDevs;
  par = NodeContainer (hostGroup0, hostGroup1);
  pairDevs = csmaHelper.Install (par);
  switchPorts.Add (pairDevs.Get (0));
  switchPorts.Add (pairDevs.Get (1));*/
    
  // Create the controller node
  Ptr<Node> controllerNode = CreateObject<Node> ();

  // Configure the OpenFlow network domain
  Ptr<OFSwitch13InternalHelper> of13Helper = CreateObject<OFSwitch13InternalHelper> ();
  Ptr<OFSwitch13Controller> controller = of13Helper->InstallController (controllerNode);
  OFSwitch13DeviceContainer switches = of13Helper->InstallSwitch (switchNode, switchPorts);
  of13Helper->CreateOpenFlowChannels ();

  // Install the TCP/IP stack into hosts nodes
  InternetStackHelper internet;
  internet.Install (hosts);

  // Set IPv4 host addresses
  // Divide os dois grupos em ips diferentes
  Ipv4AddressHelper ipv4helpr;
  Ipv4InterfaceContainer hostIpIfacesGroup0;
  Ipv4InterfaceContainer hostIpIfacesGroup1;
  ipv4helpr.SetBase ("10.1.1.0", "255.255.255.0");
  hostIpIfacesGroup0 = ipv4helpr.Assign (roteador0);
  ipv4helpr.SetBase ("10.1.2.0", "255.255.255.0");
  hostIpIfacesGroup1 = ipv4helpr.Assign (roteador1);

  // Aplicação bulk-send
  /*BulkSendHelper clienteC1 ("ns3::TcpSocketFactory",
                           InetSocketAddress (hostIpIfacesGroup0.GetAddress (1), 9));
  clienteC1.SetAttribute ("MaxBytes", UintegerValue (0));
  ApplicationContainer c1Apps = clienteC1.Install (hosts.Get (9));
  c1Apps.Start (Seconds (2.0));
  c1Apps.Stop (Seconds (10.0));

  PacketSinkHelper sinkS1 ("ns3::TcpSocketFactory",
                          InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer s1Apps = sinkS1.Install (hosts.Get (1));
  s1Apps.Start (Seconds (1.0));
  s1Apps.Stop (Seconds (10.0));*/


  // Configure ping application between hosts
  V4PingHelper pingHelper = V4PingHelper (hostIpIfacesGroup0.GetAddress (1));
  pingHelper.SetAttribute ("Verbose", BooleanValue (true));
  ApplicationContainer pingApps = pingHelper.Install (hosts.Get (9));
  pingApps.Start (Seconds (1));

  //Imprime as tabelas
  //Simulator::Schedule (Seconds(9.9), &OFSwitch13Device::PrintFlowTables, switches.Get(0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // Enable datapath stats and pcap traces at hosts, switch(es), and controller(s)
  if (trace)
    {
      of13Helper->EnableOpenFlowPcap ("openflow");
      of13Helper->EnableDatapathStats ("switch-stats");
      csmaHelper.EnablePcap ("switch", switchPorts, true);
      csmaHelper.EnablePcap ("host0", hostGroup0);
      csmaHelper.EnablePcap ("host1", hostGroup1);
    }
  // Run the simulation
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  Simulator::Destroy ();

  //Resultados do bulk-send
  //Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (s1Apps.Get (0));
  //std::cout << "(S1) Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
}
