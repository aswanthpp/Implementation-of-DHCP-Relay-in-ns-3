#include "ns3/ipv4.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/ipv4-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/uinteger.h"
#include "ns3/random-variable-stream.h"
#include "ns3/pointer.h"
#include "ns3/string.h"
#include "ns3/ipv4-routing-table-entry.h"
#include "dhcp-relay.h"
#include "dhcp-header.h"
#include "ns3/assert.h"
#include "ns3/ipv4-packet-info-tag.h"
#include "ns3/config.h"
#include "ns3/ipv4-l3-protocol.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DhcpRelay");
NS_OBJECT_ENSURE_REGISTERED (DhcpRelay);

TypeId
DhcpRelay::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DhcpRelay")
    .SetParent<Application> ()
    .AddConstructor<DhcpRelay> ()
    .SetGroupName ("Internet-Apps")
    .AddAttribute ("ServerSideAddress",
                   "Relay address at the server side",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&DhcpRelay::m_relayServerSideAddress),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("DhcpServerAddress",
                   "Address of DHCP server",
                   Ipv4AddressValue (),
                   MakeIpv4AddressAccessor (&DhcpRelay::m_dhcps),
                   MakeIpv4AddressChecker ())
    .AddAttribute ("SubnetMask",
                   "Mask of the subnet",
                   Ipv4MaskValue (),
                   MakeIpv4MaskAccessor (&DhcpRelay::m_subMask),
                   MakeIpv4MaskChecker ())
  ;
  return tid;
}

DhcpRelay::DhcpRelay ()
{
  NS_LOG_FUNCTION (this);
}

DhcpRelay::~DhcpRelay ()
{
  NS_LOG_FUNCTION (this);
}

void
DhcpRelay::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

Ptr<NetDevice> DhcpRelay::GetDhcpRelayNetDevice (void)
{
  return m_device;
}

void DhcpRelay::SetDhcpRelayNetDevice (Ptr<NetDevice> netDevice)
{
  m_device = netDevice;
}

void DhcpRelay::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  TypeId tid_client = TypeId::LookupByName ("ns3::UdpSocketFactory");
  m_socket_client = Socket::CreateSocket (GetNode (), tid_client);
  InetSocketAddress local_client = InetSocketAddress (Ipv4Address::GetAny (), PORT_SERVER);
  m_socket_client->SetAllowBroadcast (true);
  m_socket_client->Bind (local_client);
  m_socket_client->SetRecvPktInfo (true);
  m_socket_client->SetRecvCallback (MakeCallback (&DhcpRelay::NetHandlerServer, this));

  TypeId tid_server = TypeId::LookupByName ("ns3::UdpSocketFactory");
  m_socket_server = Socket::CreateSocket (GetNode (), tid_server);
  InetSocketAddress local_server = InetSocketAddress (m_relayServerSideAddress, PORT_CLIENT);
  m_socket_server->SetAllowBroadcast (true);
  m_socket_server->Bind (local_server);
  m_socket_server->SetRecvPktInfo (true);
  m_socket_server->SetRecvCallback (MakeCallback (&DhcpRelay::NetHandlerClient, this));
}

void DhcpRelay::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket_client != 0)
    {
      m_socket_client->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }

  if (m_socket_client != 0)
    {
      m_socket_server->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void DhcpRelay::NetHandlerServer (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  DhcpHeader header;
  Ptr<Packet> packet = 0;
  Address from;
  packet = m_socket_client->RecvFrom (from);

  Ipv4PacketInfoTag interfaceInfo;

  if (!packet->RemovePacketTag (interfaceInfo))
    {
      NS_ABORT_MSG ("No incoming interface on DHCP message, aborting.");
    }

  uint32_t incomingIf = interfaceInfo.GetRecvIf ();
  Ptr<NetDevice> iDev = GetNode ()->GetDevice (incomingIf);

  if (packet->RemoveHeader (header) == 0)
    {
      return;
    }
  if (header.GetType () == DhcpHeader::DHCPDISCOVER)
    {
      SendDiscover (iDev,header);
    }
  if (header.GetType () == DhcpHeader::DHCPREQ)
    {
      SendReq (header);
    }
}

void DhcpRelay::NetHandlerClient (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  DhcpHeader header;
  Ptr<Packet> packet = 0;
  Address from;
  packet = m_socket_server->RecvFrom (from);

  Ipv4PacketInfoTag interfaceInfo;
  if (!packet->RemovePacketTag (interfaceInfo))
    {
      NS_ABORT_MSG ("No incoming interface on DHCP message, aborting.");
    }

  uint32_t incomingIf = interfaceInfo.GetRecvIf ();
  Ptr<NetDevice> iDev = GetNode ()->GetDevice (incomingIf);

  if (packet->RemoveHeader (header) == 0)
    {
      return;
    }
  if (header.GetType () == DhcpHeader::DHCPOFFER)
    {
      SendOffer (header);
    }
  if (header.GetType () == DhcpHeader::DHCPACK || header.GetType () == DhcpHeader::DHCPNACK)
    {
      SendAckClient (header);
    }
}

void DhcpRelay::SendDiscover (Ptr<NetDevice> iDev,DhcpHeader header)
{
  NS_LOG_FUNCTION (this << header);

  Ptr<Packet> packet = 0;
  packet = Create<Packet> ();
  DhcpHeader newDhcpHeader;
  uint32_t tran = header.GetTran ();
  Address sourceChaddr = header.GetChaddr ();
  uint32_t mask = header.GetMask ();

  Ptr<Ipv4L3Protocol> ipv4 = GetNode ()->GetObject< Ipv4L3Protocol > ();
  int32_t ifIndex = ipv4->GetInterfaceForDevice (iDev);

  Ipv4Address m_relayClientSideAddress;
  for (uint32_t i = 0; i < ipv4->GetNAddresses (ifIndex); i++)
    {
      m_relayClientSideAddress = ipv4->GetAddress (ifIndex, i).GetLocal ();
    }

  RelayCInterfaceIter i;
  for (i = m_relayCInterfaces.begin (); i != m_relayCInterfaces.end (); i++)
    {
      if (m_relayClientSideAddress.Get () == (*i).first.Get ())
        {
          mask = (*i).second.Get ();
          break;
        }
    }

  if (m_relayClientSideAddress.CombineMask (Ipv4Mask (mask)).Get () != m_relayServerSideAddress.CombineMask (m_subMask).Get ())
    {
      newDhcpHeader.ResetOpt ();
      newDhcpHeader.SetType (DhcpHeader::DHCPDISCOVER);
      newDhcpHeader.SetTran (tran);
      newDhcpHeader.SetChaddr (sourceChaddr);
      newDhcpHeader.SetTime ();
      newDhcpHeader.SetGiAddr (m_relayClientSideAddress);
      newDhcpHeader.SetMask (mask);
      packet->AddHeader (newDhcpHeader);

      if ((m_socket_server->SendTo (packet, 0, InetSocketAddress (m_dhcps, PORT_SERVER))) >= 0)
        {
          NS_LOG_INFO ("DHCP DISCOVER sent from relay to server");
        }
      else
        {
          NS_LOG_INFO ("Error while sending DHCP DISCOVER from relay to server");
        }
    }
}

void DhcpRelay::SendOffer (DhcpHeader header)
{
  NS_LOG_FUNCTION (this << header);

  Ptr<Packet> packet = 0;
  packet = Create<Packet> ();
  DhcpHeader newDhcpHeader;

  uint32_t tran = header.GetTran ();
  Address sourceChaddr = header.GetChaddr ();
  uint32_t mask = header.GetMask ();
  Ipv4Address offeredAddress = header.GetYiaddr ();
  Ipv4Address dhcpServerAddress = header.GetDhcps ();
  uint32_t lease = header.GetLease ();
  uint32_t renew = header.GetRenew ();
  uint32_t rebind = header.GetRebind ();
  Ipv4Address giaddress = header.GetGiAddr ();

  if (giaddress.CombineMask (Ipv4Mask (mask)).Get () != m_relayServerSideAddress.CombineMask (m_subMask).Get ())
    {
      newDhcpHeader.ResetOpt ();
      newDhcpHeader.SetType (DhcpHeader::DHCPOFFER);
      newDhcpHeader.SetTran (tran);
      newDhcpHeader.SetChaddr (sourceChaddr);
      newDhcpHeader.SetMask (mask);
      newDhcpHeader.SetYiaddr (offeredAddress);
      newDhcpHeader.SetDhcps (dhcpServerAddress);
      newDhcpHeader.SetLease (lease);
      newDhcpHeader.SetRenew (renew);
      newDhcpHeader.SetRebind (rebind);
      newDhcpHeader.SetGiAddr (giaddress);
      newDhcpHeader.SetTime ();

      packet->AddHeader (newDhcpHeader);

      if ((m_socket_client->SendTo (packet, 0, InetSocketAddress (Ipv4Address ("255.255.255.255"), PORT_CLIENT))) >= 0)
        {
          NS_LOG_INFO ("DHCP OFFER sent from relay to client");
        }
      else
        {
          NS_LOG_INFO ("Error while sending DHCP OFFER from relay to client");
        }
    }
}

void DhcpRelay::SendReq (DhcpHeader header)
{
  NS_LOG_FUNCTION (this << header);
  NS_LOG_FUNCTION (this);

  Ptr<Packet> packet = 0;
  packet = Create<Packet> ();

  uint32_t tran = header.GetTran ();
  Ipv4Address offeredAddress = header.GetReq ();
  Address sourceChaddr = header.GetChaddr ();
  uint32_t mask = header.GetMask ();
  Ipv4Address giaddress = header.GetGiAddr ();

  if (giaddress.CombineMask (Ipv4Mask (mask)).Get () != m_relayServerSideAddress.CombineMask (m_subMask).Get ())
    {

      DhcpHeader newDhcpHeader;

      newDhcpHeader.ResetOpt ();
      newDhcpHeader.SetType (DhcpHeader::DHCPREQ);
      newDhcpHeader.SetTime ();
      newDhcpHeader.SetTran (tran);
      newDhcpHeader.SetReq (offeredAddress);
      newDhcpHeader.SetChaddr (sourceChaddr);
      packet->AddHeader (newDhcpHeader);

      if (m_socket_server->SendTo (packet, 0, InetSocketAddress (m_dhcps, PORT_SERVER)) >= 0)
        {
          NS_LOG_INFO ("DHCP REQUEST sent from relay to server");
        }
      else
        {
          NS_LOG_INFO ("Error while sending DHCP REQUEST from relay to server");
        }
    }
}

void DhcpRelay::SendAckClient (DhcpHeader header)
{
  NS_LOG_FUNCTION (this << header);

  Ptr<Packet> packet = 0;
  packet = Create<Packet> ();
  Address sourceChaddr = header.GetChaddr ();
  uint32_t tran = header.GetTran ();
  Ipv4Address address = header.GetReq ();
  uint32_t type = header.GetType ();
  uint32_t mask = header.GetMask ();
  Ipv4Address giaddress = header.GetGiAddr ();
  if (giaddress.CombineMask (Ipv4Mask (mask)).Get () != m_relayServerSideAddress.CombineMask (m_subMask).Get ())
    {

      DhcpHeader newDhcpHeader;
      newDhcpHeader.ResetOpt ();
      newDhcpHeader.SetType (type);
      newDhcpHeader.SetYiaddr (address);
      newDhcpHeader.SetChaddr (sourceChaddr);
      newDhcpHeader.SetTran (tran);
      newDhcpHeader.SetDhcps (m_dhcps);
      newDhcpHeader.SetTime ();
      packet->AddHeader (newDhcpHeader);

      if ((m_socket_client->SendTo (packet, 0, InetSocketAddress (Ipv4Address ("255.255.255.255"), PORT_CLIENT))) >= 0)
        {
          NS_LOG_INFO ("DHCP ACK sent from relay to client");
        }
      else
        {
          NS_LOG_INFO ("Error while sending DHCP ACK from relay to client");
        }
    }
}

void DhcpRelay::AddRelayInterfaceAddress (Ipv4Address addr, Ipv4Mask mask)
{
  RelayCInterfaceIter i;
  for (i = m_relayCInterfaces.begin (); i != m_relayCInterfaces.end (); i++)
    {
      if (((*i).first.CombineMask ((*i).second).Get () == addr.CombineMask (mask).Get () ) || ((*i).first.Get () == addr.Get ()))
        {
          NS_ABORT_MSG ("Relay agent cannot have same gateway for two subnets ");
        }
    }
  m_relayCInterfaces.push_back (std::make_pair (addr,mask));
}

} // Namespace ns3
