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

NS_LOG_COMPONENT_DEFINE ("SliceExample");


int
main (int argc, char *argv[])
{
  uint16_t simTime = 10;
  bool verbose = false;
  bool trace = false;

  uint16_t numberOfHosts = 5;

  // Configure command line parameters
  CommandLine cmd;
  cmd.AddValue ("simTime", "Simulation time (seconds)", simTime);
  cmd.AddValue ("verbose", "Enable verbose output", verbose);
  cmd.AddValue ("trace", "Enable datapath stats and pcap traces", trace);
  cmd.AddValue ("numberOfHosts", "Number of hosts", numberOfHosts);
  cmd.Parse (argc, argv);

  LogComponentEnable ("SliceExample", LOG_LEVEL_INFO);
  LogComponentEnable ("HttpClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("HttpServerApplication", LOG_LEVEL_INFO);

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

  //CSMA Full Duplex
  Config::SetDefault ("ns3::CsmaChannel::FullDuplex", BooleanValue (true));

  // Create host nodes
  NodeContainer hostG0S1;
  hostG0S1.Create (numberOfHosts);

  NodeContainer hostG1S1;
  hostG1S1.Create (numberOfHosts);

  //routersS1
  NodeContainer routersS1;
  routersS1.Create(2);

  // Create the switch node
  Ptr<Node> switchNode = CreateObject<Node> ();

  // Use the CsmaHelper to connect host nodes to the switch node
  CsmaHelper csmaHelper;
  csmaHelper.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
  csmaHelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  // Install the TCP/IP stack into hosts nodes
  InternetStackHelper internet;
  internet.Install (hostG0S1);
  internet.Install (hostG1S1);
  internet.Install (routersS1);

  NetDeviceContainer switchPorts;
  NetDeviceContainer hostG0S1Devices;
  NetDeviceContainer hostG1S1Devices;

  Ipv4AddressHelper ipv4helpr;
  Ipv4InterfaceContainer IpG0S1;
  Ipv4InterfaceContainer IpG1S1;
  /* Dividindo os dois grupos de Hosts
  * Ainda não tentamos fazer com os servidores, somente conectar os dois grupos de host
  */
  ipv4helpr.SetBase ("10.1.1.0", "255.255.255.0");
  for (size_t i = 0; i < hostG0S1.GetN(); i++){
    NetDeviceContainer temp = csmaHelper.Install(hostG0S1.Get(i), routersS1.Get(0));
    Ipv4InterfaceContainer iptemp = ipv4helpr.Assign (temp);
    IpG0S1.Add(iptemp.Get(0));
    hostG0S1Devices.Add(temp.Get(0));
    ipv4helpr.NewNetwork();
  }
  ipv4helpr.SetBase ("10.2.1.0", "255.255.255.0");
  for (size_t i = 0; i < hostG1S1.GetN(); i++){
    NetDeviceContainer temp = csmaHelper.Install(hostG1S1.Get(i), routersS1.Get(1));
    Ipv4InterfaceContainer iptemp = ipv4helpr.Assign (temp);
    IpG1S1.Add(iptemp.Get(0));
    hostG1S1Devices.Add(temp.Get(0));
    ipv4helpr.NewNetwork();
  }
  ipv4helpr.SetBase ("10.3.1.0", "255.255.255.0");
  //Conexao entre os routersS1
  NetDeviceContainer roteador01Devices;
  roteador01Devices = csmaHelper.Install(routersS1.Get(0), routersS1.Get(1));
  ipv4helpr.Assign (roteador01Devices);
    
  // Create the controller node
  Ptr<Node> controllerNode = CreateObject<Node> ();

  //Conectando roteadores no switch
  /*NetDeviceContainer roteador01Devices;
  for (size_t i = 0; i < routersS1.GetN (); i++)
  {
      NodeContainer pair (routersS1.Get (i), switchNode);
      NetDeviceContainer link = csmaHelper.Install (pair);
      //roteador01Devices.Add (link.Get (0));
      switchPorts.Add (link.Get (1));
  }*/

  // Configure the OpenFlow network domain
  Ptr<OFSwitch13InternalHelper> of13Helper = CreateObject<OFSwitch13InternalHelper> ();
  Ptr<OFSwitch13Controller> controller = of13Helper->InstallController (controllerNode);
  OFSwitch13DeviceContainer switches = of13Helper->InstallSwitch (switchNode, switchPorts);
  of13Helper->CreateOpenFlowChannels ();


  //Aplicacoes http
  //Group 0 = Clientes; Group 1 = Servidores
  uint16_t httpServerPort = 80;
  //Servidores
  for (size_t i = 0; i < hostG1S1.GetN(); i++){
    HttpServerHelper httpServer (httpServerPort);
    ApplicationContainer httpServerApps;
    httpServerApps.Add (httpServer.Install (hostG1S1.Get (i)));
    httpServerApps.Start (Seconds(1.0));
    httpServerApps.Stop (Seconds(10.0));
  }
  //Clientes
  for (size_t i = 0; i < hostG0S1.GetN(); i++){
    ApplicationContainer httpClientApps;
    HttpClientHelper httpClient (IpG1S1.GetAddress (i), httpServerPort);
    httpClientApps.Add (httpClient.Install (hostG0S1.Get (i)));
    httpClientApps.Start (Seconds(2.0));
    httpClientApps.Stop (Seconds(10.0));
  }

  // Configure ping application between hosts
  /*V4PingHelper pingHelper = V4PingHelper (IpG0S1.GetAddress (0));
  pingHelper.SetAttribute ("Verbose", BooleanValue (true));
  ApplicationContainer pingApps = pingHelper.Install (hostG1S1.Get (0));
  pingApps.Start (Seconds (1));
  pingApps.Stop (Seconds (2));*/

  //Ping entre todos os hosts
  /*for (int p = 0; p < numberOfHosts; p++){
    V4PingHelper pingHelper = V4PingHelper (IpG0S1.GetAddress (p));
    pingHelper.SetAttribute ("Verbose", BooleanValue (true));
    for (int t = 0; t < numberOfHosts; t++){
      ApplicationContainer pingApps = pingHelper.Install (hostG1S1.Get (t));
      pingApps.Start (Seconds (1));
      pingApps.Stop (Seconds (2));
    }
  }

  for (int p = 0; p < numberOfHosts; p++){
    V4PingHelper pingHelper = V4PingHelper (IpG1S1.GetAddress (p));
    pingHelper.SetAttribute ("Verbose", BooleanValue (true));
    for (int t = 0; t < numberOfHosts; t++){
      ApplicationContainer pingApps = pingHelper.Install (hostG0S1.Get (t));
      pingApps.Start (Seconds (1));
      pingApps.Stop (Seconds (2));
    }
  }*/

  //Imprime as tabelas
  //Simulator::Schedule (Seconds(9.9), &OFSwitch13Device::PrintFlowTables, switches.Get(0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  ArpCache::PopulateArpCaches ();

  // Enable datapath stats and pcap traces at hosts, switch(es), and controller(s)
  if (trace)
    {
      //of13Helper->EnableOpenFlowPcap ("openflow");
      //of13Helper->EnableDatapathStats ("switch-stats");
      //csmaHelper.EnablePcap ("switch", switchPorts, true);
      csmaHelper.EnablePcap ("host0", hostG0S1);
      csmaHelper.EnablePcap ("host1", hostG1S1);
      csmaHelper.EnablePcap ("routersS1", routersS1);
    }
  // Run the simulation
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  Simulator::Destroy ();
}
