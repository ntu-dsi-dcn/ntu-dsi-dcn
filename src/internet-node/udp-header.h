/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2005 INRIA
 * All rights reserved.
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef UDP_HEADER_H
#define UDP_HEADER_H

#include <stdint.h>
#include <string>
#include "ns3/header.h"
#include "ns3/ipv4-address.h"

namespace ns3 {
/**
 * \brief Packet header for UDP packets
 */
class UdpHeader : public Header 
{
public:
  static uint32_t GetUid (void);

  /**
   * \brief Constructor
   *
   * Creates a null header
   */
  UdpHeader ();
  ~UdpHeader ();

  /**
   * \brief Enable checksum calculation for UDP (XXX currently has no effect)
   */
  static void EnableChecksums (void);
  /**
   * \param port the destination port for this UdpHeader
   */
  void SetDestination (uint16_t port);
  /**
   * \param port The source port for this UdpHeader
   */
  void SetSource (uint16_t port);
  /**
   * \return The source port for this UdpHeader
   */
  uint16_t GetSource (void) const;
  /**
   * \return the destination port for this UdpHeader
   */
  uint16_t GetDestination (void) const;
  /**
   * \param size The payload size in bytes
   */
  void SetPayloadSize (uint16_t size);

  /**
   * \param source the ip source to use in the underlying
   *        ip packet.
   * \param destination the ip destination to use in the
   *        underlying ip packet.
   * \param protocol the protocol number to use in the underlying
   *        ip packet.
   *
   * If you want to use udp checksums, you should call this
   * method prior to adding the header to a packet.
   */
  void InitializeChecksum (Ipv4Address source, 
                           Ipv4Address destination,
                           uint8_t protocol);

  std::string GetName (void) const;
  void Print (std::ostream &os) const;
  uint32_t GetSerializedSize (void) const;
  void Serialize (Buffer::Iterator start) const;
  uint32_t Deserialize (Buffer::Iterator start);

private:
  uint16_t m_sourcePort;
  uint16_t m_destinationPort;
  uint16_t m_payloadSize;
  uint16_t m_initialChecksum;

  static bool m_calcChecksum;
};

} // namespace ns3

#endif /* UDP_HEADER */