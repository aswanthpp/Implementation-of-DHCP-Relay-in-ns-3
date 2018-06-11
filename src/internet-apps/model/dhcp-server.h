/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 UPB
 * Copyright (c) 2017 NITK Surathkal
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
 * Author: Radu Lupu <rlupu@elcom.pub.ro>
 *         Ankit Deepak <adadeepak8@gmail.com>
 *         Deepti Rajagopal <deeptir96@gmail.com>
 *
 */

#ifndef DHCP_SERVER_H
#define DHCP_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/traced-value.h"
#include "ns3/inet-socket-address.h"
#include "dhcp-header.h"
#include <map>

namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup dhcp
 *
 * \class DhcpServer
 * \brief Implements the functionality of a DHCP server
 */
class DhcpServer : public Application
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);   

  DhcpServer ();     
  virtual ~DhcpServer ();  

  /**
   * \brief Add a static entry to the pool.
   * \param chaddr The client chaddr.
   * \param addr The address to handle to the client.
   */
  void AddStaticDhcpEntry (Address chaddr, Ipv4Address addr);

  /**
   * \brief Assign an address pool to the DHCP server 
   * \param poolAddr The Ipv4Address (network part) of the address pool
   * \param poolMask The mask of the address pool
   * \param minAddr The lower bound of the address pool
   * \param maxAddr The upper bound of the address pool
   */
  void AddSubnets (Ipv4Address poolAddr, Ipv4Mask poolMask, Ipv4Address minAddr, Ipv4Address maxAddr);


protected:
  virtual void DoDispose (void);

private:
  static const int PORT = 67;   //!< Port number of DHCP server  

  /**
   * \brief Handles incoming packets from the network
   * \param socket Socket bound to port 67 of the DHCP server
   */
  void NetHandler (Ptr<Socket> socket);

  /**
   * \brief Sends DHCP offer after receiving DHCP Discover
   * \param iDev incoming NetDevice
   * \param header DHCP header of the received message
   * \param from Address of the DHCP client
   */
  void SendOffer (Ptr<NetDevice> iDev, DhcpHeader header, InetSocketAddress from);

  /**
   * \brief Sends DHCP ACK (or NACK) after receiving Request
   * \param iDev incoming NetDevice
   * \param header DHCP header of the received message
   * \param from Address of the DHCP client
   */
  void SendAck (Ptr<NetDevice> iDev, DhcpHeader header, InetSocketAddress from);

  /**
   * \brief Modifies the remaining lease time of addresses
   */
  void TimerHandler (void);

  /**
   * \brief Starts the DHCP Server application
   */
  virtual void StartApplication (void);

  /**
   * \brief Stops the DHCP Server application
   */
  virtual void StopApplication (void);

  /**
   * \brief Check whether the given Ipv4Address is in valid range
   * \param reqAddr Ipv4Address to be checked for validity
   * \return true if address is within the range
   */
  bool CheckIfValid (Ipv4Address reqAddr);

  Ptr<Socket> m_socket;                  //!< The socket bound to port 67
  Ipv4Address m_poolAddress;             //!< The network address available to the server
  Ipv4Address m_minAddress;              //!< The first address in the address pool
  Ipv4Address m_maxAddress;              //!< The last address in the address pool
  Ipv4Mask m_poolMask;                   //!< The network mask of the pool
  Ipv4Address m_gateway;                 //!< The gateway address

  /// Pool address conatainer - pool address / pool mask + min address / max address
  typedef std::list < std::pair < std::pair <Ipv4Address,Ipv4Mask> , std::pair <Ipv4Address,Ipv4Address> > > PoolAddress; 
  /// Pool address iterator - pool address / pool mask + min address / max address
  typedef std::list < std::pair < std::pair <Ipv4Address,Ipv4Mask> , std::pair <Ipv4Address,Ipv4Address> > >::iterator PoolAddressIter; 
  /// Pool address const iterator - pool address / pool mask + min address / max address
  typedef std::list < std::pair < std::pair <Ipv4Address,Ipv4Mask> , std::pair <Ipv4Address,Ipv4Address> > >::const_iterator PoolAddressCIter; 
  
  /// Leased address container - chaddr + IP addr / lease time
  typedef std::map<Address, std::pair<Ipv4Address, uint32_t> > LeasedAddress;
  /// Leased address iterator - chaddr + IP addr / lease time
  typedef std::map<Address, std::pair<Ipv4Address, uint32_t> >::iterator LeasedAddressIter;
  /// Leased address const iterator - chaddr + IP addr / lease time
  typedef std::map<Address, std::pair<Ipv4Address, uint32_t> >::const_iterator LeasedAddressCIter;

  /// Expired address container - chaddr
  typedef std::list<Address> ExpiredAddress;
  /// Expired address iterator - chaddr
  typedef std::list<Address>::iterator ExpiredAddressIter;
  /// Expired address const iterator - chaddr
  typedef std::list<Address>::const_iterator ExpiredAddressCIter;

  /// Available address container - IP address / subnet mask
  typedef std::list< std::pair<Ipv4Address, Ipv4Mask> > AvailableAddress;
  /// Available address iterator - IP address / subnet mask
  typedef std::list< std::pair<Ipv4Address, Ipv4Mask> >::iterator  AvailableAddressIter;
  /// Available address const iterator - IP address / subnet mask
  typedef std::list< std::pair<Ipv4Address, Ipv4Mask> >::const_iterator AvailableAddressCIter;

  PoolAddress m_poolAddresses;           //!< Pool address and their range and subnet mask
  LeasedAddress m_leasedAddresses;       //!< Leased address and their status (cache memory)
  ExpiredAddress m_expiredAddresses;     //!< Expired addresses to be reused (chaddr of the clients)
  AvailableAddress m_availableAddresses; //!< Available addresses to be used (IP addresses)
  Time m_lease;                          //!< The granted lease time for an address
  Time m_renew;                          //!< The renewal time for an address
  Time m_rebind;                         //!< The rebinding time for an address
  EventId m_expiredEvent;                //!< The Event to trigger TimerHandler
};

} // namespace ns3

#endif /* DHCP_SERVER_H */
