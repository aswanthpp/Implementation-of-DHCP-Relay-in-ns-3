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

#ifndef DHCP_HELPER_H
#define DHCP_HELPER_H

#include <stdint.h>
#include "ns3/application-container.h"
#include "ns3/net-device-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-interface-container.h"

namespace ns3 {

/**
 * \ingroup dhcp
 *
 * \class DhcpHelper
 * \brief The helper class used to configure and install DHCP applications on nodes
 */
class DhcpHelper
{
public:
  DhcpHelper ();

  /**
   * \brief Set DHCP client attributes
   * \param name Name of the attribute
   * \param value Value to be set
   */
  void SetClientAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Set DHCP server attributes
   * \param name Name of the attribute
   * \param value Value to be set
   */
  void SetServerAttribute (std::string name, const AttributeValue &value);

  /**
   * \brief Set DHCP relay attributes
   * \param name Name of the attribute
   * \param value Value to be set
   */
  void SetRelayAttribute (std::string name,const AttributeValue &value);

  /**
   * \brief Install DHCP client of a nodes / NetDevice
   * \param netDevice The NetDevice that the DHCP client will use
   * \return The application container with DHCP client installed
   */
  ApplicationContainer InstallDhcpClient (Ptr<NetDevice> netDevice) const;

  /**
   * \brief Install DHCP client of a set of nodes / NetDevices
   * \param netDevices The NetDevices that the DHCP client will use
   * \return The application container with DHCP client installed
   */
  ApplicationContainer InstallDhcpClient (NetDeviceContainer netDevices) const;

  /**
   * \brief Install DHCP server of a node / NetDevice
   *
   * Note: the server address must be coherent with the pool address, because
   * DHCP relays are not yet supported.
   *
   * \param netDevice The NetDevice on which DHCP server application has to be installed
   * \param serverAddr The Ipv4Address of the server
   * \param poolAddr The Ipv4Address (network part) of the allocated pool
   * \param poolMask The mask of the allocated pool
   * \param minAddr The lower bound of the Ipv4Address pool
   * \param maxAddr The upper bound of the Ipv4Address pool
   * \param gateway The Ipv4Address of default gateway (optional)
   * \return The application container with DHCP server installed
   */
  ApplicationContainer InstallDhcpServer (Ptr<NetDevice> netDevice, Ipv4Address serverAddr,
                                          Ipv4Address poolAddr, Ipv4Mask poolMask,
                                          Ipv4Address minAddr, Ipv4Address maxAddr,
                                          Ipv4Address gateway = Ipv4Address ());
  
  /**
   * \brief Install DHCP server of a node / NetDevice
   *
   * \param netDevice The NetDevice on which DHCP server application has to be installed
   * \param serverAddr The Ipv4Address of the server
   * \param netMask The mask of the allocated pool
   * \param gateway The Ipv4Address of default gateway (optional)
   * \return The application container with DHCP server installed
   */
  ApplicationContainer InstallDhcpServer (Ptr<NetDevice> netDevice, Ipv4Address serverAddr,
                                          Ipv4Mask netMask, Ipv4Address gateway = Ipv4Address ());

  /**
   * \brief Assign a fixed IP addresses to a net device.
   * \param netDevice The NetDevice on which the address has to be installed
   * \param addr The Ipv4Address
   * \param mask The network mask
   * \return the Ipv4 interface container
   */
  Ipv4InterfaceContainer InstallFixedAddress (Ptr<NetDevice> netDevice, Ipv4Address addr, Ipv4Mask mask);

  /**
   * \brief Install DHCP relay of a node / NetDevice
   * \param netDevice The NetDevice on which DHCP relay application has to be installed
   * \param serverSideAddress Ipv4Address of relay interface communicating with DHCP server
   * \param subMask Ipv4mask of the server subnet
   * \param dhcps Ip4Address of server
   * \return The application container with DHCP relay installed
   */
  ApplicationContainer InstallDhcpRelay (Ptr<NetDevice> netDevice, Ipv4Address serverSideAddress,
                                         Ipv4Mask subMask, Ipv4Address dhcps);

  /**
   * \brief Assign an address pool to the installed DHCP server
   * \param dhcpServerApp Pointer to the DHCP server application
   * \param poolAddr The Ipv4Address (network part) of the address pool
   * \param poolMask The mask of the address pool
   * \param minAddr The lower bound of the address pool
   * \param maxAddr The upper bound of the address pool
   */
   void AddAddressPool (ApplicationContainer * dhcpServerApp, Ipv4Address poolAddr, Ipv4Mask poolMask, Ipv4Address minAddr, 
                        Ipv4Address maxAddr);

   /**
   * \brief Add an interface to the DHCP relay to communicate with a client subnet without a DHCP server 
   * \param dhcpRelayApp Pointer to the DHCP relay application 
   * \param netDevice The NetDevice associated with the interface
   * \param addr Ipv4Address of the interface
   * \param mask Ipv4mask of the client subnet
   */
   void AddRelayInterface (ApplicationContainer * dhcpRelayApp, Ptr<NetDevice> netDevice, Ipv4Address addr, Ipv4Mask mask);

private:
  /**
   * \brief Function to install DHCP client on a node
   * \param netDevice The NetDevice on which DHCP client application has to be installed
   * \return The pointer to the installed DHCP client
   */
  Ptr<Application> InstallDhcpClientPriv (Ptr<NetDevice> netDevice) const;

  /// Address pool container - pool address / pool mask + min address / max address
  typedef std::list < std::pair < std::pair <Ipv4Address,Ipv4Mask> , std::pair <Ipv4Address,Ipv4Address> > > AddressPool; 

  ObjectFactory m_clientFactory;                 //!< DHCP client factory
  ObjectFactory m_serverFactory;                 //!< DHCP server factory
  ObjectFactory m_relayFactory;                  //!< DHCP relay factory
  std::list<Ipv4Address> m_fixedAddresses;       //!< list of fixed addresses already allocated.
  AddressPool m_addressPools;                    //!< list of address pools 
};

} // namespace ns3

#endif /* DHCP_HELPER_H */






