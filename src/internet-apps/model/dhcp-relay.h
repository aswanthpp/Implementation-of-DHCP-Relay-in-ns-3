#ifndef DHCP_RELAY_H
#define DHCP_RELAY_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-value.h"
#include "ns3/inet-socket-address.h"
#include "dhcp-header.h"
#include <map>
#include "ns3/ipv4-address.h"
#include "ns3/random-variable-stream.h"
#include <list>

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup dhcp
 *
 * \class DhcpRelay
 * \brief Implements the functionality of a DHCP Relay
*/
class DhcpRelay : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  DhcpRelay ();

  virtual ~DhcpRelay ();

  /**
   * \brief Get the NetDevice DHCP should work on
   * \return the NetDevice DHCP should work on
   */
  Ptr<NetDevice> GetDhcpRelayNetDevice (void);

  /**
   * \brief Set the NetDevice DHCP should work on
   * \param netDevice the NetDevice DHCP should work on
   */
  void SetDhcpRelayNetDevice (Ptr<NetDevice> netDevice);

  /**
   * \brief Get the IPv4Address of current DHCP server
   * \return Ipv4Address of current DHCP server
   */
  Ipv4Address GetDhcpServer (void);

  /**
   * \brief Starts the DHCP Relay application
   */
  void StartApplication (void);

  /**
   * \brief Stops the DHCP Relay application
   */
  void StopApplication (void);

  /**
   * \brief Add address and mask of DHCP relay interface that communicates with a client subnet without a DHCP server
   * \param addr Ipv4Address of the interface
   * \param mask Ipv4mask of the client subnet
   */
  void AddRelayInterfaceAddress (Ipv4Address addr, Ipv4Mask mask);

protected:
  virtual void DoDispose (void);

private:
  static const int PORT_CLIENT = 68;   //!< Port number of DHCP client
  static const int PORT_SERVER = 67;   //!< Port number of DHCP server

  /**
   * \brief Handles incoming packets from the network
   * \param socket Socket bound to port 67 of the DHCP server
   */
  void NetHandlerClient (Ptr<Socket> socket);

  /**
   * \brief Handles incoming packets from the network
   * \param socket Socket bound to port 68 of the DHCP client
   */
  void NetHandlerServer (Ptr<Socket> socket);

  /**
   * \brief Sends DHCP DISCOVER to server as a unicast message
   * \param iDev incoming NetDevice
   * \param header DHCP header of the received message
   */
  void SendDiscover (Ptr<NetDevice> iDev, DhcpHeader header);

  /**
   * \brief Sends DHCP REQUEST to server as a unicast message
   * \param header DHCP header of the received message
   */
  void SendReq (DhcpHeader header);

  /**
   * \brief Sends DHCP OFFER coming from server to client
   * \param header DHCP header of the received message
   */
  void SendOffer (DhcpHeader header);

  /**
   * \brief Sends DHCP ACK coming from server to client
   * \param header DHCP header of the received message
   */
  void SendAckClient (DhcpHeader header);

  /// Client subnet container - gateway address / subnet mask
  typedef std::list< std::pair<Ipv4Address, Ipv4Mask> > RelayCInterface;
  /// Client subnet iterator - gateway address / subnet mask
  typedef std::list< std::pair<Ipv4Address, Ipv4Mask> >::iterator  RelayCInterfaceIter;

  Ptr<Socket> m_socket_client;               //!< Socket bound to port 67
  Ptr<Socket> m_socket_server;                   //!< Socket bound to port 68
  Ptr<NetDevice> m_device;                               //!< NetDevice pointer
  Ipv4Address m_relayServerSideAddress;  //!< Address assigned to the server side interface of relay
  Ipv4Address m_dhcps;                                   //!< Address of the DHCP server
  Ipv4Mask m_subMask;                                    //!< Mask of the subnet to which server belongs
  RelayCInterface m_relayCInterfaces;    //!< Client side gateway address and subnet mask
};

} // namespace ns3

#endif /* DHCP_RELAY_H */


