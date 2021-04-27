

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

  std::string deviceName ("eno1");
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
    //LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    //LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  }
  /*
  LogComponentEnable ("ArpL3Protocol", LOG_FUNCTION);
  LogComponentEnable ("ArpL3Protocol", LOG_PREFIX_TIME);

  LogComponentEnable ("StaWifiMac", LOG_FUNCTION);
  LogComponentEnable ("StaWifiMac", LOG_PREFIX_TIME);
  //LogComponentEnable ("RegularWifiMac", LOG_FUNCTION);
  //LogComponentEnable ("RegularWifiMac", LOG_PREFIX_TIME);
  
  LogComponentEnable ("YansWifiChannel", LOG_FUNCTION);
  LogComponentEnable ("YansWifiChannel", LOG_PREFIX_TIME);

  LogComponentEnable ("WifiRemoteStationManager", LOG_DEBUG);
  LogComponentEnable ("MacLow", LOG_FUNCTION);

  LogComponentEnable ("Txop", LOG_FUNCTION);
  LogComponentEnable ("Txop", LOG_DEBUG);
  LogComponentEnable ("Txop", LOG_PREFIX_TIME);

  LogComponentEnable ("WifiPhy", LOG_FUNCTION);
  LogComponentEnable ("WifiPhy", LOG_DEBUG);
  LogComponentEnable ("WifiPhy", LOG_PREFIX_TIME);

*/
// Server : 10.1.3.1
// Client : 10.1.3.2
/*
  ///////////// Set Application : echo server ////////////////
  if (serverMode)
  {
    ///////////// Create node ////////////////
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

    ///////////// Set ApNode (Mac) ////////////////  
    
    mac.SetType ("ns3::ApWifiMac",
                  "Ssid", SsidValue (ssid),
                  "BeaconInterval", TimeValue(MicroSeconds(1024000)),
                  "ApMacAddress", Mac48AddressValue (Mac48Address ("00:00:00:00:00:04"))
                  );

    mac.SetType ("ns3::ApWifiMac",
              "Ssid", SsidValue (ssid),
              "BeaconInterval", TimeValue(MicroSeconds(1024000))
              );

    NetDeviceContainer apDevices;
    apDevices = wifi.Install (phy, mac, wifiApNodes);

    Ptr<NetDevice> dev = apDevices.Get (0);
    //nd->SetAddress ("00:00:00:00:00:04");
    //Ptr<WifiNetDevice> wd = DynamicCast<WifiNetDevice> (dev); 
    //Ptr<WifiNetDevice> wd = nd->GetObject<WifiNetDevice> ();
    //wd->SetAddress ("00:00:00:00:00:04");

    // Mobility model 
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (0.0, 0.0, 0.0));
    positionAlloc->Add (Vector (1.0, 1.0, 0.0));

    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiApNodes);
    
    ///////////// Set Internet stack ////////////////
    InternetStackHelper stack;
    stack.Install (wifiApNodes);
    
    ///////////// Set wifi IP address ////////////////
    Ipv4AddressHelper address;
    address.SetBase ("10.1.3.0", "255.255.255.0"); 
    
    Ipv4InterfaceContainer apInterface;
    apInterface = address.Assign (apDevices);
    
    NS_LOG_INFO ("Create Applications.");
    UdpEchoServerHelper echoServer (9);

    ApplicationContainer serverApps = echoServer.Install (wifiApNodes.Get(0));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));

    phy.EnablePcap ("WifiFdNetdeiveTest1-wifi", apDevices.Get (0));
  }
*/


  ///////////// Set Application : echo client ////////////////
  //else if (clientMode)
  
    ///////////// Create node ////////////////
    NodeContainer wifiStaNodes;
    wifiStaNodes.Create (nStaWifi);

    ///////////// Set Wifi channel ////////////////
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
    phy.SetChannel (channel.Create ());

    ///////////// Set Netdevice (Mac) ////////////////
    WifiHelper wifi;
    wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
    //wifi.EnableLogComponents ();

    WifiMacHelper mac;
    Ssid ssid = Ssid ("ns-3-ssid");

    ///////////// Set StaNode (Mac) ////////////////     
    mac.SetType ("ns3::StaWifiMac",
                   "Ssid", SsidValue (ssid),
                   "ActiveProbing", BooleanValue (false));
    //mac.SetAddress ("00:00:00:00:00:04");

    NetDeviceContainer staDevices;
    staDevices = wifi.Install (phy, mac, wifiStaNodes);

    //staDevices.SetAddress ("00:00:00:00:00:04");


    

    /* Mobility model */
    MobilityHelper mobility;
    Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
    positionAlloc->Add (Vector (0.0, 0.0, 0.0));
    positionAlloc->Add (Vector (1.0, 1.0, 0.0));

    mobility.SetPositionAllocator (positionAlloc);
    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (wifiStaNodes);
    

  ///////////// Set Internet stack ////////////////
    InternetStackHelper stack;

    stack.Install (wifiStaNodes);
    
  ///////////// Set wifi IP address ////////////////
    Ipv4AddressHelper address;
    address.SetBase ("10.1.3.0", "255.255.255.0");  

    Ipv4InterfaceContainer staInterface;
    address.NewAddress();
    staInterface = address.Assign (staDevices);


    UdpClientHelper echoClient (Ipv4Address ("10.1.3.1"), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (1000000000));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (5)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1257));

    ApplicationContainer clientApps = 
      echoClient.Install (wifiStaNodes.Get (0));
    clientApps.Start (Seconds (8.0));
    clientApps.Stop (Seconds (30.0));

    phy.EnablePcap ("WifiFdNetdeiveTest2-wifi", staDevices.Get (0));
  
    

  Simulator::Stop (Seconds (35.0));

  Simulator::Schedule(Seconds (10.0), Config::Set, "/NodeList/*/ApplicationList/*/$ns3::UdpClient/Interval", TimeValue(Seconds (0.0075)));
  Simulator::Run ();
  // uint64_t totalPacketSend = DynamicCast<UdpClient> (clientApps.Get (0))->GetSend ();
  // std::cout << "Throughtput : " << 1257*8/0.0075 << " bit/s" << std::endl;
  // std::cout << "Total number of packet sent : "<< totalPacketSend << std::endl;
  Simulator::Destroy ();
  return 0;
}











