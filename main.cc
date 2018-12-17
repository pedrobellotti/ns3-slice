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
 *                        Slice 1 Controller
 *                                |
 *                       +-----------------+
 *      Host group 0 === |     OpenFlow    | === Server group 0
 *          (Slice 1)    |                 |     (Slice 1)
 *      -----------------|------SLICE------|-----------------
 *          (Slice 2)    |                 |     (Slice 2)
 *      Host group 1 === |      switch     | === Server group 1
 *                       +-----------------+
 *                                |
 *                        Slice 2 Controller
 * 
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
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SliceExample");

void regraZero(Ptr<OFSwitch13Controller> c, uint64_t datap, uint16_t numHosts){
  int numSlices = 2;
  int numPortas = numHosts*numSlices;
  int porta = 1;
  for (int s = 0; s < numSlices; s++){
    for (int h = 0; h < numPortas; h++){
      std::ostringstream cmd;
      cmd << "flow-mod cmd=add,prio=1,table=0"
      << " in_port=" << porta
      << " goto:" << s+1;
      c->DpctlExecute (datap, cmd.str());
      porta++;
    }
  }
}

int
main (int argc, char *argv[])
{
  uint16_t simTime = 10;
  bool verbose = false;
  bool trace = false;

  uint16_t numberOfHosts = 1; //flowmonitor

  // Configure command line parameters
  CommandLine cmd;
  cmd.AddValue ("simTime", "Simulation time (seconds)", simTime);
  cmd.AddValue ("verbose", "Enable verbose output", verbose);
  cmd.AddValue ("trace", "Enable datapath stats and pcap traces", trace);
  cmd.AddValue ("numHosts", "Number of hosts on each group", numberOfHosts);
  cmd.Parse (argc, argv);

  // HTTP app Logs
  LogComponentEnable ("SliceExample", LOG_LEVEL_INFO);
  LogComponentEnable ("HttpClientApplication", LOG_INFO);
  LogComponentEnable ("HttpClientApplication", LOG_PREFIX_TIME);
  LogComponentEnable ("HttpServerApplication", LOG_INFO);
  LogComponentEnable ("HttpServerApplication", LOG_PREFIX_TIME);
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
  //Slice 2
  internet.Install (hostG0S2);
  internet.Install (hostG1S2);

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

  /* Criando controladores */
  NodeContainer controllers;
  controllers.Create (2);
  Ptr<ControladorSlice1> controllerSlice1 = CreateObject<ControladorSlice1> ();
  Ptr<ControladorSlice2> controllerSlice2 = CreateObject<ControladorSlice2> ();
  Ptr<OFSwitch13InternalHelper> of13Helper = CreateObject<OFSwitch13InternalHelper> ();
  Ptr<OFSwitch13Controller> controller1 = of13Helper->InstallController (controllers.Get (0), controllerSlice1);
  Ptr<OFSwitch13Controller> controller2 = of13Helper->InstallController (controllers.Get (1), controllerSlice2);

  //Instala o switch
  NetDeviceContainer switchPorts;
  OFSwitch13DeviceContainer switches = of13Helper->InstallSwitch (switchNode);

  /* Dividindo os dois grupos de Hosts de cada slice */
  //Slice 1 - Grupo 0
  ipv4helpr.SetBase ("10.1.1.0", "255.255.255.0");
  for (size_t i = 0; i < hostG0S1.GetN(); i++){
    // Conectando hosts com o switch.
    NetDeviceContainer link = csmaHelper.Install (switchNode, hostG0S1.Get (i));
    uint32_t numPorta = switches.Get(0)->AddSwitchPort (link.Get (0))->GetPortNo ();
    switchPorts.Add(link.Get(0));
    hostG0S1Devices.Add (link.Get (1));
    // Colocando os ips nos hosts e notificando o controlador para adicionar as regras.
    Ipv4InterfaceContainer tempIpIface = ipv4helpr.Assign (link.Get (1));
    IpG0S1.Add (tempIpIface);
    controllerSlice1->AddRegra (numPorta, tempIpIface.GetAddress (0));
  }
  //Slice 1 - Grupo 1
  for (size_t i = 0; i < hostG1S1.GetN(); i++){
    // Conectando hosts com o switch.
    NetDeviceContainer link = csmaHelper.Install (switchNode, hostG1S1.Get (i));
    uint32_t numPorta = switches.Get(0)->AddSwitchPort (link.Get (0))->GetPortNo ();
    switchPorts.Add(link.Get(0));
    hostG1S1Devices.Add (link.Get (1));
    // Colocando os ips nos hosts e notificando o controlador para adicionar as regras.
    Ipv4InterfaceContainer tempIpIface = ipv4helpr.Assign (link.Get (1));
    IpG1S1.Add (tempIpIface);
    controllerSlice1->AddRegra (numPorta, tempIpIface.GetAddress (0));
  }
  //Slice 2 - Grupo 0
  ipv4helpr.SetBase ("10.2.2.0", "255.255.255.0");
  for (size_t i = 0; i < hostG0S2.GetN(); i++){
    // Conectando hosts com o switch.
    NetDeviceContainer link = csmaHelper.Install (switchNode, hostG0S2.Get (i));
    uint32_t numPorta = switches.Get(0)->AddSwitchPort (link.Get (0))->GetPortNo ();
    switchPorts.Add(link.Get(0));
    hostG0S2Devices.Add (link.Get (1));
    // Colocando os ips nos hosts e notificando o controlador para adicionar as regras.
    Ipv4InterfaceContainer tempIpIface = ipv4helpr.Assign (link.Get (1));
    IpG0S2.Add (tempIpIface);
    controllerSlice2->AddRegra (numPorta, tempIpIface.GetAddress (0));
  }
  //Slice 2 - Grupo 1
  for (size_t i = 0; i < hostG1S2.GetN(); i++){
    // Conectando hosts com o switch.
    NetDeviceContainer link = csmaHelper.Install (switchNode, hostG1S2.Get (i));
    uint32_t numPorta = switches.Get(0)->AddSwitchPort (link.Get (0))->GetPortNo ();
    switchPorts.Add(link.Get(0));
    hostG1S2Devices.Add (link.Get (1));
    // Colocando os ips nos hosts e notificando o controlador para adicionar as regras.
    Ipv4InterfaceContainer tempIpIface = ipv4helpr.Assign (link.Get (1));
    IpG1S2.Add (tempIpIface);
    controllerSlice2->AddRegra (numPorta, tempIpIface.GetAddress (0));
  }
    
  //Separando os slices em suas tabelas
  uint64_t datap = switches.Get(0)->GetDatapathId();
  Simulator::Schedule (Seconds(0.5), &regraZero, controller1, datap, numberOfHosts);

  //Aplicacoes http
  //Group 0 = Clientes; Group 1 = Servidores
  uint16_t httpServerPort = 80;

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
  }

  //Slice 2
  //Servidores
  for (size_t i = 0; i < hostG1S2.GetN(); i++){
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

  /* Todos os nós com aplicação de cliente e servidor */
  //Slice 1 - Grupo 0
  for (size_t i = 0; i < hostG0S1.GetN(); i++){
    //App Server
    HttpServerHelper httpServer (httpServerPort);
    ApplicationContainer httpServerApps;
    httpServerApps.Add (httpServer.Install (hostG0S1.Get (i)));
    httpServerApps.Start (Seconds(1.0));
    httpServerApps.Stop (Seconds(simTime));
    //App Cliente
    ApplicationContainer httpClientApps;
    HttpClientHelper httpClient (IpG1S1.GetAddress (i), httpServerPort);
    httpClientApps.Add (httpClient.Install (hostG0S1.Get (i)));
    httpClientApps.Start (Seconds(2.0));
    httpClientApps.Stop (Seconds(simTime));
  }
  //Slice 1 - Grupo 1
  for (size_t i = 0; i < hostG1S1.GetN(); i++){
    //App Server
    HttpServerHelper httpServer (httpServerPort);
    ApplicationContainer httpServerApps;
    httpServerApps.Add (httpServer.Install (hostG1S1.Get (i)));
    httpServerApps.Start (Seconds(1.0));
    httpServerApps.Stop (Seconds(simTime));
    //App Cliente
    ApplicationContainer httpClientApps;
    HttpClientHelper httpClient (IpG0S1.GetAddress (i), httpServerPort);
    httpClientApps.Add (httpClient.Install (hostG1S1.Get (i)));
    httpClientApps.Start (Seconds(2.0));
    httpClientApps.Stop (Seconds(simTime));
  }
  //Slice 2 - Grupo 0
  for (size_t i = 0; i < hostG0S2.GetN(); i++){
    //App Server
    HttpServerHelper httpServer (httpServerPort);
    ApplicationContainer httpServerApps;
    httpServerApps.Add (httpServer.Install (hostG0S2.Get (i)));
    httpServerApps.Start (Seconds(1.0));
    httpServerApps.Stop (Seconds(simTime));
    //App Cliente
    ApplicationContainer httpClientApps;
    HttpClientHelper httpClient (IpG1S2.GetAddress (i), httpServerPort);
    httpClientApps.Add (httpClient.Install (hostG0S2.Get (i)));
    httpClientApps.Start (Seconds(2.0));
    httpClientApps.Stop (Seconds(simTime));
  }
  //Slice 2 - Grupo 1
  for (size_t i = 0; i < hostG1S2.GetN(); i++){
    //App Server
    HttpServerHelper httpServer (httpServerPort);
    ApplicationContainer httpServerApps;
    httpServerApps.Add (httpServer.Install (hostG1S2.Get (i)));
    httpServerApps.Start (Seconds(1.0));
    httpServerApps.Stop (Seconds(simTime));
    //App Cliente
    ApplicationContainer httpClientApps;
    HttpClientHelper httpClient (IpG0S2.GetAddress (i), httpServerPort);
    httpClientApps.Add (httpClient.Install (hostG1S2.Get (i)));
    httpClientApps.Start (Seconds(2.0));
    httpClientApps.Stop (Seconds(simTime));
  }


  //Ping entre todos os hosts
  /*for (int p = 0; p < numberOfHosts; p++){
    V4PingHelper pingHelper = V4PingHelper (IpG0S1.GetAddress (p));
    pingHelper.SetAttribute ("Verbose", BooleanValue (true));
    for (int t = 0; t < numberOfHosts; t++){
      ApplicationContainer pingApps = pingHelper.Install (hostG1S1.Get (t));
      pingApps.Start (Seconds (1));
      pingApps.Stop (Seconds (5));
    }
  }*/

  /*for (int p = 0; p < numberOfHosts; p++){
    V4PingHelper pingHelper = V4PingHelper (IpG1S2.GetAddress (p));
    pingHelper.SetAttribute ("Verbose", BooleanValue (true));
    for (int t = 0; t < numberOfHosts; t++){
      ApplicationContainer pingApps = pingHelper.Install (hostG0S2.Get (t));
      pingApps.Start (Seconds (1));
      pingApps.Stop (Seconds (5));
    }
  }*/

  of13Helper->CreateOpenFlowChannels ();
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
  ArpCache::PopulateArpCaches ();

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
      csmaHelper.EnablePcap ("SLICE2_grupo0", hostG0S2);
      csmaHelper.EnablePcap ("SLICE2_grupo1", hostG1S2);
    }

  //Flowmonitor
  FlowMonitorHelper flowHelper;
  Ptr<FlowMonitor> fmS1_G0;
  Ptr<FlowMonitor> fmS1_G1;
  Ptr<FlowMonitor> fmS2_G0;
  Ptr<FlowMonitor> fmS2_G1;
  fmS1_G0 = flowHelper.Install(hostG0S1);
  fmS1_G1 = flowHelper.Install(hostG1S1);
  fmS2_G0 = flowHelper.Install(hostG0S2);
  fmS2_G1 = flowHelper.Install(hostG1S2);

  // Run the simulation
  Simulator::Stop (Seconds (simTime));
  Simulator::Run ();
  fmS1_G0->SerializeToXmlFile("slice1_G0.xml", true, true);
  fmS1_G1->SerializeToXmlFile("slice1_G1.xml", true, true);
  fmS2_G0->SerializeToXmlFile("slice2_G0.xml", true, true);
  fmS2_G1->SerializeToXmlFile("slice2_G1.xml", true, true);
  Simulator::Destroy ();
}
