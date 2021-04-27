

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/fd-net-device-module.h"


// Network Topology
//
//   Wifi 10.1.3.0
//   STA          AP
//    *            *
//    |            |
//   n7           n0 
//  


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WifiFdNetdeiveTest");

int 
main (int argc, char *argv[])
{
  Packet::EnablePrinting ();

  bool logEnable = true;
  uint32_t nStaWifi = 1;     // # of Sta wifi node
  uint32_t nApWifi = 1;     // # of Sta wifi node
  bool tracing = true;

  std::string deviceName ("enp0s3");
  std::string encapMode ("Dix");
  bool clientMode = false;
  bool serverMode = false;

  CommandLine cmd;
  

///////////// wifi parameter setting ////////////////
  cmd.AddValue ("nStaWifi", "Number of wifi STA devices", nStaWifi);
  cmd.AddValue ("nApWifi", "Number of wifi STA devices", nApWifi);
  cmd.AddValue ("logEnable", "Tell echo applications to log if true", logEnable);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

///////////// Fd Netdevice parameter setting ////////////////
  cmd.AddValue ("client", "client mode", clientMode);
  cmd.AddValue ("server", "server mode", serverMode);
  cmd.AddValue ("deviceName", "device name", deviceName);
  cmd.AddValue ("encapsulationMode", "encapsulation mode of emu device (\"Dix\" [default] or \"Llc\")", encapMode);


  cmd.Parse (argc,argv);

  GlobalValue::Bind ("SimulatorImplementationType",
                     StringValue ("ns3::RealtimeSimulatorImpl"));

  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));

  if (clientMode && serverMode)
	{
      NS_FATAL_ERROR("Error, both client and server options cannot be enabled.");
    }
  if (logEnable)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

///////////// Create real Channels ////////////////
/*
  NS_LOG_INFO ("Create channels.");
  EmuFdNetDeviceHelper emu;
  emu.SetDeviceName (deviceName);
  emu.SetAttribute ("EncapsulationMode", StringValue (encapMode));
*/

///////////// Create node ////////////////
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nStaWifi);
  NodeContainer wifiApNodes;
  wifiApNodes.Create (nApWifi);

///////////// Set Wifi channel ////////////////
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

///////////// Set Netdevice (Mac) ////////////////
  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  wifi.EnableLogComponents ();

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");

  mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid),
                   "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

    mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid), 
               "BeaconInterval", TimeValue(MicroSeconds(1024000)));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNodes);


  /* Mobility model */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (1.0, 1.0, 0.0));

  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (wifiApNodes);
  mobility.Install (wifiStaNodes);
  

///////////// Set Internet stack ////////////////
  InternetStackHelper stack;

  stack.Install (wifiApNodes);
  stack.Install (wifiStaNodes);
  
///////////// Set wifi IP address ////////////////
  Ipv4AddressHelper address;
  address.SetBase ("10.1.3.0", "255.255.255.0");

  Ipv4InterfaceContainer apInterface;
  apInterface = address.Assign (apDevices);
      
  Ipv4InterfaceContainer staInterface;
  staInterface = address.Assign (staDevices);



///////////// Set Application : echo server ////////////////
 if (serverMode)
    {
      NS_LOG_INFO ("Create Applications.");
  	  UdpServerHelper echoServer (9);

  	  ApplicationContainer serverApps = echoServer.Install (wifiApNodes.Get(0));
  	  serverApps.Start (Seconds (1.0));
  	  serverApps.Stop (Seconds (50.0));
    }

///////////// Set Application : echo client ////////////////
  else if (clientMode)
    {
  	  UdpClientHelper echoClient (Ipv4Address ("10.1.3.1"), 9);
  	  echoClient.SetAttribute ("MaxPackets", UintegerValue (50));
  	  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  	  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  	  ApplicationContainer clientApps = 
  			echoClient.Install (wifiStaNodes.Get (0));
  	  clientApps.Start (Seconds (2.0));
  	  clientApps.Stop (Seconds (50.0));
    }
  
  //emulation in one host
  else
    {
      //
      // Create a UdpEchoServer application on node one.
      //
      NS_LOG_INFO ("Create Applications.");
///////////// Set Application : echo server ////////////////
  	  UdpServerHelper echoServer (9);

  	  ApplicationContainer serverApps = 
  	  		echoServer.Install (wifiApNodes.Get(0));
  	  serverApps.Start (Seconds (1.0));
  	  serverApps.Stop (Seconds (50.0));

///////////// Set Application : echo client //////////////// 
  	  UdpClientHelper echoClient (apInterface.GetAddress (0), 9);
  	  echoClient.SetAttribute ("MaxPackets", UintegerValue (50));
  	  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  	  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  	  ApplicationContainer clientApps = 
  	    	echoClient.Install (wifiStaNodes.Get (0));
  	  clientApps.Start (Seconds (2.0));
  	  clientApps.Stop (Seconds (50.0));
    }

  Simulator::Stop (Seconds (50.0));


///////////// Tracing Enable ////////////////
 
  if (tracing == false)
  {
    if(serverMode)
    	phy.EnablePcap ("WifiFdNetdeiveTest-wifi", apDevices.Get (0));
    else if(clientMode)
    	phy.EnablePcap ("WifiFdNetdeiveTest-wifi", staDevices.Get (0));
	// emu.EnablePcapAll ("WifiFdNetdeiveTest-fd", true);
  }   

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}











