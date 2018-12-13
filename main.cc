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
#include "controladorSlice1.h"
#include "controladorSlice2.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SliceExample");


void regraZero(Ptr<OFSwitch13Controller> c, uint64_t datap){
  //Slice 1
  c->DpctlExecute (datap, "flow-mod cmd=add,table=0,prio=1 in_port=1 goto:1");
  c->DpctlExecute (datap, "flow-mod cmd=add,table=0,prio=1 in_port=2 goto:1");
  //Slice 2
  c->DpctlExecute (datap, "flow-mod cmd=add,table=0,prio=1 in_port=3 goto:2");
  c->DpctlExecute (datap, "flow-mod cmd=add,table=0,prio=1 in_port=4 goto:2");
  //Table miss
  c->DpctlExecute (datap, "flow-mod cmd=add,table=0,prio=0 apply:output=ctrl");
}

int
main (int argc, char *argv[])
{
  uint16_t simTime = 10;
  bool verbose = false;
  bool trace = false;

  uint16_t numberOfHosts = 1;

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
  OFSwitch13Helper::EnableDatapathLogs ();
  if (verbose)
    {
      OFSwitch13Helper::EnableDatapathLogs (); //log de tudo no switch
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
  // Slice 1
  NodeContainer hostG0S1;
  hostG0S1.Create (numberOfHosts);

  NodeContainer hostG1S1;
  hostG1S1.Create (numberOfHosts);

  // Slice 2
  NodeContainer hostG0S2;
  hostG0S2.Create (numberOfHosts);

  NodeContainer hostG1S2;
  hostG1S2.Create (numberOfHosts);

  // Roteadores
  // Slice 1
  NodeContainer routersS1;
  routersS1.Create(2);

  // Slice 2
  NodeContainer routersS2;
  routersS2.Create(2);

  // Create the switch node
  Ptr<Node> switchNode = CreateObject<Node> ();

  // Use the CsmaHelper to connect host nodes to the switch node
  CsmaHelper csmaHelper;
  csmaHelper.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
  csmaHelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  // Install the TCP/IP stack into hosts nodes
  InternetStackHelper internet;
  //Slice 1
  internet.Install (hostG0S1);
  internet.Install (hostG1S1);
  internet.Install (routersS1);
  //Slice 2
  internet.Install (hostG0S2);
  internet.Install (hostG1S2);
  internet.Install (routersS2);

  //Slice 1
  NetDeviceContainer hostG0S1Devices;
  NetDeviceContainer hostG1S1Devices;
  //Slice 2
  NetDeviceContainer hostG0S2Devices;
  NetDeviceContainer hostG1S2Devices;

  Ipv4AddressHelper ipv4helpr;
  //Slice 1
  Ipv4InterfaceContainer IpG0S1;
  Ipv4InterfaceContainer IpG1S1;
  //Slice 2
  Ipv4InterfaceContainer IpG0S2;
  Ipv4InterfaceContainer IpG1S2;

  /* Dividindo os dois grupos de Hosts de cada slice */
  //Slice 1 - Grupo 0
  ipv4helpr.SetBase ("10.1.1.0", "255.255.255.0");
  for (size_t i = 0; i < hostG0S1.GetN(); i++){
    NetDeviceContainer temp = csmaHelper.Install(hostG0S1.Get(i), routersS1.Get(0));
    Ipv4InterfaceContainer iptemp = ipv4helpr.Assign (temp);
    IpG0S1.Add(iptemp.Get(0));
    hostG0S1Devices.Add(temp.Get(0));
    ipv4helpr.NewNetwork();
  }
  //Slice 1 - Grupo 1
  ipv4helpr.SetBase ("10.2.1.0", "255.255.255.0");
  for (size_t i = 0; i < hostG1S1.GetN(); i++){
    NetDeviceContainer temp = csmaHelper.Install(hostG1S1.Get(i), routersS1.Get(1));
    Ipv4InterfaceContainer iptemp = ipv4helpr.Assign (temp);
    IpG1S1.Add(iptemp.Get(0));
    hostG1S1Devices.Add(temp.Get(0));
    ipv4helpr.NewNetwork();
  }
  //Slice 2 - Grupo 0
  ipv4helpr.SetBase ("10.3.1.0", "255.255.255.0");
  for (size_t i = 0; i < hostG0S2.GetN(); i++){
    NetDeviceContainer temp = csmaHelper.Install(hostG0S2.Get(i), routersS2.Get(0));
    Ipv4InterfaceContainer iptemp = ipv4helpr.Assign (temp);
    IpG0S2.Add(iptemp.Get(0));
    hostG0S2Devices.Add(temp.Get(0));
    ipv4helpr.NewNetwork();
  }
  //Slice 2 - Grupo 1
  ipv4helpr.SetBase ("10.4.1.0", "255.255.255.0");
  for (size_t i = 0; i < hostG1S2.GetN(); i++){
    NetDeviceContainer temp = csmaHelper.Install(hostG1S2.Get(i), routersS2.Get(1));
    Ipv4InterfaceContainer iptemp = ipv4helpr.Assign (temp);
    IpG1S2.Add(iptemp.Get(0));
    hostG1S2Devices.Add(temp.Get(0));
    ipv4helpr.NewNetwork();
  }

  //Conexao entre os routersS1 e S2 (sem o switch)
  /*
  NetDeviceContainer roteador01Devices;
  ipv4helpr.SetBase ("10.11.1.0", "255.255.255.0");
  roteador01Devices = csmaHelper.Install(routersS1.Get(0), routersS1.Get(1));
  ipv4helpr.Assign (roteador01Devices);
  
  NetDeviceContainer roteador23Devices;
  ipv4helpr.SetBase ("10.22.1.0", "255.255.255.0");
  roteador23Devices = csmaHelper.Install(routersS2.Get(0), routersS2.Get(1));
  ipv4helpr.Assign (roteador23Devices);
  */
    
  // Create the controller node
  NodeContainer controllers;
  controllers.Create (2);
  Ptr<ControladorSlice1> controllerSlice1 = CreateObject<ControladorSlice1> ();
  Ptr<ControladorSlice2> controllerSlice2 = CreateObject<ControladorSlice2> ();


  // Configure the OpenFlow network domain
  NetDeviceContainer switchPorts;
  Ptr<OFSwitch13InternalHelper> of13Helper = CreateObject<OFSwitch13InternalHelper> ();
  Ptr<OFSwitch13Controller> controller1 = of13Helper->InstallController (controllers.Get (0), controllerSlice1);
  Ptr<OFSwitch13Controller> controller2 = of13Helper->InstallController (controllers.Get (1), controllerSlice2);
  OFSwitch13DeviceContainer switches = of13Helper->InstallSwitch (switchNode);
  of13Helper->CreateOpenFlowChannels ();

  //Conectando roteadores com o switch  
  //Slice 1
  ipv4helpr.SetBase ("10.11.1.0", "255.255.255.0");
  NetDeviceContainer roteador01Devices;
  for (size_t i = 0; i < routersS1.GetN (); i++)
  {
    NetDeviceContainer temp = csmaHelper.Install(routersS1.Get(i), switchNode);
    ipv4helpr.Assign (temp);
    roteador01Devices.Add(temp.Get(0));
    uint32_t portaSlice1 = switches.Get(0)->AddSwitchPort (temp.Get(1))->GetPortNo ();
    switchPorts.Add(temp.Get(1));
    std::cout<<"Slice 1, porta: " << portaSlice1 << std::endl;
    ipv4helpr.NewNetwork();
  }
  //Slice 2
  ipv4helpr.SetBase ("10.22.1.0", "255.255.255.0");
  NetDeviceContainer roteador23Devices;
  for (size_t i = 0; i < routersS2.GetN (); i++)
  {
    NetDeviceContainer temp = csmaHelper.Install(routersS2.Get(i), switchNode);
    ipv4helpr.Assign (temp);
    roteador23Devices.Add(temp.Get(0));
    uint32_t portaSlice2 = switches.Get(0)->AddSwitchPort (temp.Get(1))->GetPortNo ();
    switchPorts.Add(temp.Get(1));
    std::cout<<"Slice 2, porta: " << portaSlice2 << std::endl;
    ipv4helpr.NewNetwork();
  }

  //Separando os slices em suas tabelas
  uint64_t datap = switches.Get(0)->GetDatapathId();
  Simulator::Schedule (Seconds(0.5), &regraZero, controller1, datap);

  //Aplicacoes http
  //Group 0 = Clientes; Group 1 = Servidores
  //uint16_t httpServerPort = 80;

  //Slice 1
  //Servidores
  /*for (size_t i = 0; i < hostG1S1.GetN(); i++){
    HttpServerHelper httpServer (httpServerPort);
    ApplicationContainer httpServerApps;
    httpServerApps.Add (httpServer.Install (hostG1S1.Get (i)));
    httpServerApps.Start (Seconds(1.0));
    httpServerApps.Stop (Seconds(simTime));
  }
  //Clientes
  for (size_t i = 0; i < hostG0S1.GetN(); i++){
    ApplicationContainer httpClientApps;
    HttpClientHelper httpClient (IpG1S1.GetAddress (i), httpServerPort);
    httpClientApps.Add (httpClient.Install (hostG0S1.Get (i)));
    httpClientApps.Start (Seconds(2.0));
    httpClientApps.Stop (Seconds(simTime));
  }*/

  //Slice 2
  //Servidores
  /*for (size_t i = 0; i < hostG1S2.GetN(); i++){
    HttpServerHelper httpServer (httpServerPort);
    ApplicationContainer httpServerApps;
    httpServerApps.Add (httpServer.Install (hostG1S2.Get (i)));
    httpServerApps.Start (Seconds(1.0));
    httpServerApps.Stop (Seconds(simTime));
  }
  //Clientes
  for (size_t i = 0; i < hostG0S2.GetN(); i++){
    ApplicationContainer httpClientApps;
    HttpClientHelper httpClient (IpG1S2.GetAddress (i), httpServerPort);
    httpClientApps.Add (httpClient.Install (hostG0S2.Get (i)));
    httpClientApps.Start (Seconds(2.0));
    httpClientApps.Stop (Seconds(simTime));
  }*/

  //Slice 1 <---> Slice 2 (funciona por enquanto, mas nao deveria pois a ideia de slice é isolar as duas redes)
  //Servidores
  /*for (size_t i = 0; i < hostG1S2.GetN(); i++){
    HttpServerHelper httpServer (httpServerPort);
    ApplicationContainer httpServerApps;
    httpServerApps.Add (httpServer.Install (hostG1S2.Get (i)));
    httpServerApps.Start (Seconds(1.0));
    httpServerApps.Stop (Seconds(simTime));
  }
  //Clientes
  for (size_t i = 0; i < hostG0S1.GetN(); i++){
    ApplicationContainer httpClientApps;
    HttpClientHelper httpClient (IpG1S2.GetAddress (i), httpServerPort);
    httpClientApps.Add (httpClient.Install (hostG0S1.Get (i)));
    httpClientApps.Start (Seconds(2.0));
    httpClientApps.Stop (Seconds(simTime));
  }*/

  // Configure ping application between hosts
  V4PingHelper pingHelper = V4PingHelper (IpG1S1.GetAddress (0));
  pingHelper.SetAttribute ("Verbose", BooleanValue (true));
  ApplicationContainer pingApps = pingHelper.Install (hostG0S1.Get (0));
  pingApps.Start (Seconds (1));
  pingApps.Stop (Seconds (5));

  //Ping entre todos os hosts
  /*for (int p = 0; p < numberOfHosts; p++){
    V4PingHelper pingHelper = V4PingHelper (IpG0S1.GetAddress (p));
    pingHelper.SetAttribute ("Verbose", BooleanValue (true));
    for (int t = 0; t < numberOfHosts; t++){
      ApplicationContainer pingApps = pingHelper.Install (hostG1S1.Get (t));
      pingApps.Start (Seconds (1));
      pingApps.Stop (Seconds (5));
    }
  }

  for (int p = 0; p < numberOfHosts; p++){
    V4PingHelper pingHelper = V4PingHelper (IpG1S2.GetAddress (p));
    pingHelper.SetAttribute ("Verbose", BooleanValue (true));
    for (int t = 0; t < numberOfHosts; t++){
      ApplicationContainer pingApps = pingHelper.Install (hostG0S2.Get (t));
      pingApps.Start (Seconds (1));
      pingApps.Stop (Seconds (5));
    }
  }*/

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  //ArpCache::PopulateArpCaches ();

  //Imprime as tabelas
  //Simulator::Schedule (Seconds(9.9), &OFSwitch13Device::PrintFlowTables, switches.Get(0));

  // Enable datapath stats and pcap traces at hosts, switch(es), and controller(s)
  if (trace)
    {
      //of13Helper->EnableOpenFlowPcap ("openflow");
      //of13Helper->EnableDatapathStats ("switch-stats");
      csmaHelper.EnablePcap ("switchPorts", switchPorts, true);
      csmaHelper.EnablePcap ("SLICE1_grupo0", hostG0S1);
      csmaHelper.EnablePcap ("SLICE1_grupo1", hostG1S1);
      csmaHelper.EnablePcap ("SLICE1_routersS1", routersS1);

     /* csmaHelper.EnablePcap ("SLICE2_grupo0", hostG0S2);
      csmaHelper.EnablePcap ("SLICE2_grupo1", hostG1S2);
      csmaHelper.EnablePcap ("SLICE2_routersS2", routersS2);*/
    }
  // Run the simulation
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  Simulator::Destroy ();
}