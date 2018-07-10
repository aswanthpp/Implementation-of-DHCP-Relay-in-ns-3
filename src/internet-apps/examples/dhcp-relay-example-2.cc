#include <fstream>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DhcpExampleRelay");

int
main (int argc, char *argv[])
{
  CommandLine cmd;

  bool verbose = false;
  bool tracing = false;
  cmd.AddValue ("verbose", "turn on the logs", verbose);
  cmd.AddValue ("tracing", "turn on the tracing", tracing);

  cmd.Parse (argc, argv);

  if (verbose)
    {
      LogComponentEnable ("DhcpServer", LOG_LEVEL_ALL);
      LogComponentEnable ("DhcpClient", LOG_LEVEL_ALL);
      LogComponentEnable ("DhcpRelay", LOG_LEVEL_ALL);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    }

  Time stopTime = Seconds (20);

  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodes;
  NodeContainer relay;
  nodes.Create (3);
  relay.Create (1);

  NodeContainer net (nodes, relay);

  NS_LOG_INFO ("Create channels.");
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("5Mbps"));
  csma.SetChannelAttribute ("Delay", StringValue ("2ms"));
  csma.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  NetDeviceContainer devNet = csma.Install (net);

  NodeContainer p2pNodes;
  p2pNodes.Add (net.Get (2));
  p2pNodes.Create (1);

  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);

  InternetStackHelper tcpip;
  tcpip.Install (nodes);
  tcpip.Install (relay);
  tcpip.Install (p2pNodes.Get (1));

  Ipv4AddressHelper address;
  address.SetBase ("172.30.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  // manually add a routing entry because we don't want to add a dynamic routing
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4> ipv4Ptr = p2pNodes.Get (1)->GetObject<Ipv4> ();
  Ptr<Ipv4StaticRouting> staticRoutingA = ipv4RoutingHelper.GetStaticRouting (ipv4Ptr);
  staticRoutingA->AddNetworkRouteTo (Ipv4Address ("172.30.0.0"), Ipv4Mask ("/24"),
                                     Ipv4Address ("172.30.1.1"), 1);


  NS_LOG_INFO ("Setup the IP addresses and create DHCP applications.");
  DhcpHelper dhcpHelper;

  // DHCP server
  ApplicationContainer dhcpServerApp = dhcpHelper.InstallDhcpServer (devNet.Get (1), Ipv4Address ("172.30.1.12"),
                                                                     Ipv4Mask ("/24"));

  dhcpHelper.AddAddressPool (&dhcpServerApp, Ipv4Address ("172.30.1.0"), Ipv4Mask ("/24"), Ipv4Address ("172.30.1.10"),
                             Ipv4Address ("172.30.1.15"));

  dhcpHelper.AddAddressPool (&dhcpServerApp, Ipv4Address ("172.30.0.0"), Ipv4Mask ("/24"), Ipv4Address ("172.30.0.10"),
                             Ipv4Address ("172.30.0.15"));

  // DHCP Relay Agent
  ApplicationContainer dhcpRelayApp = dhcpHelper.InstallDhcpRelay (devNet.Get (2), Ipv4Address ("172.30.1.16"),
                                                                   Ipv4Mask ("/24"), Ipv4Address ("172.30.1.12"));

  dhcpHelper.AddRelayInterface (&dhcpRelayApp, p2pDevices.Get (0), Ipv4Address ("172.30.0.17"), Ipv4Mask ("/24"));

  // This is just to show how it can be done.
  /* DynamicCast<DhcpServer> (dhcpServerApp.Get (0))->AddStaticDhcpEntry (devNet.Get (2)->GetAddress (),
                                                                        Ipv4Address ("172.30.0.14"));
 */
  dhcpServerApp.Start (Seconds (0.0));
  dhcpServerApp.Stop (stopTime);

  dhcpRelayApp.Start (Seconds (0.0));
  dhcpRelayApp.Stop (stopTime);

  // DHCP clients
  NetDeviceContainer dhcpClientNetDevs;
  dhcpClientNetDevs.Add (devNet.Get (0));
  //dhcpClientNetDevs.Add (devNet.Get (1));
  //dhcpClientNetDevs.Add (devNet.Get (2));
  //dhcpClientNetDevs.Add (devNet.Get (1));


  ApplicationContainer dhcpClients = dhcpHelper.InstallDhcpClient (dhcpClientNetDevs);
  dhcpClients.Start (Seconds (1.0));
  dhcpClients.Stop (stopTime);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (p2pNodes.Get (1));
  serverApps.Start (Seconds (0.0));
  serverApps.Stop (stopTime);

  UdpEchoClientHelper echoClient (p2pInterfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (100));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get (0));
  clientApps.Start (Seconds (10.0));
  clientApps.Stop (stopTime);

  Simulator::Stop (stopTime + Seconds (10.0));

  if (tracing)
    {
      csma.EnablePcapAll ("dhcp-csma");
      pointToPoint.EnablePcapAll ("dhcp-p2p");
    }

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}
