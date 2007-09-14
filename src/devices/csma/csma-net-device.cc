/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 Emmanuelle Laprise
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
 * Author: Emmanuelle Laprise <emmanuelle.laprise@bluekazoo.ca>
 */

#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/composite-trace-resolver.h"
#include "csma-net-device.h"
#include "csma-channel.h"
#include "ns3/ethernet-header.h"
#include "ns3/ethernet-trailer.h"
#include "ns3/llc-snap-header.h"

NS_LOG_COMPONENT_DEFINE ("CsmaNetDevice");

namespace ns3 {

CsmaTraceType::CsmaTraceType (enum Type type)
  : m_type (type)
{
  NS_LOG_FUNCTION;
}

CsmaTraceType::CsmaTraceType ()
  : m_type (RX)
{
  NS_LOG_FUNCTION;
}

void 
CsmaTraceType::Print (std::ostream &os) const
{
  switch (m_type) {
  case RX:
    os << "dev-rx";
    break;
  case DROP:
    os << "dev-drop";
    break;
  }
}

uint16_t 
CsmaTraceType::GetUid (void)
{
  NS_LOG_FUNCTION;
  static uint16_t uid = AllocateUid<CsmaTraceType> ("CsmaTraceType");
  return uid;
}

std::string 
CsmaTraceType::GetTypeName (void) const
{
  NS_LOG_FUNCTION;
  return "ns3::CsmaTraceType";
}

enum CsmaTraceType::Type 
CsmaTraceType::Get (void) const
{
  NS_LOG_FUNCTION;
  return m_type;
}

CsmaNetDevice::CsmaNetDevice (Ptr<Node> node)
  : NetDevice (node, Mac48Address::Allocate ()),
    m_bps (DataRate (0xffffffff))
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << node << ")");
  m_encapMode = IP_ARP;
  Init(true, true);
}

CsmaNetDevice::CsmaNetDevice (Ptr<Node> node, Mac48Address addr, 
                              CsmaEncapsulationMode encapMode) 
  : NetDevice(node, addr), 
    m_bps (DataRate (0xffffffff))
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << node << ")");
  m_encapMode = encapMode;

  Init(true, true);
}

CsmaNetDevice::CsmaNetDevice (Ptr<Node> node, Mac48Address addr, 
                              CsmaEncapsulationMode encapMode,
                              bool sendEnable, bool receiveEnable) 
  : NetDevice(node, addr), 
    m_bps (DataRate (0xffffffff))
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << node << ")");
  m_encapMode = encapMode;

  Init(sendEnable, receiveEnable);
}

CsmaNetDevice::~CsmaNetDevice()
{
  NS_LOG_FUNCTION;
  m_queue = 0;
}

void 
CsmaNetDevice::DoDispose ()
{
  NS_LOG_FUNCTION;
  m_channel = 0;
  NetDevice::DoDispose ();
}

//
// Assignment operator for CsmaNetDevice.
//
// This uses the non-obvious trick of taking the source net device passed by
// value instead of by reference.  This causes the copy constructor to be
// invoked (where the real work is done -- see above).  All we have to do
// here is to return the newly constructed net device.
//
/*
CsmaNetDevice&
CsmaNetDevice::operator= (const CsmaNetDevice nd)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << &nd << ")");
  return *this;
}
*/

void 
CsmaNetDevice::Init(bool sendEnable, bool receiveEnable)
{
  NS_LOG_FUNCTION;
  m_txMachineState = READY;
  m_tInterframeGap = Seconds(0);
  m_channel = 0; 
  m_queue = 0;

  EnableBroadcast (Mac48Address ("ff:ff:ff:ff:ff:ff"));
  EnableMulticast (Mac48Address ("01:00:5e:00:00:00"));

  SetSendEnable (sendEnable);
  SetReceiveEnable (receiveEnable);
}

void
CsmaNetDevice::SetSendEnable (bool sendEnable)
{
  NS_LOG_FUNCTION;
  m_sendEnable = sendEnable;
}

void
CsmaNetDevice::SetReceiveEnable (bool receiveEnable)
{
  NS_LOG_FUNCTION;
  m_receiveEnable = receiveEnable;
}

bool
CsmaNetDevice::IsSendEnabled (void)
{
  NS_LOG_FUNCTION;
  return (m_sendEnable);
}

bool
CsmaNetDevice::IsReceiveEnabled (void)
{
  NS_LOG_FUNCTION;
  return (m_receiveEnable);
}

void 
CsmaNetDevice::SetDataRate (DataRate bps)
{
  NS_LOG_FUNCTION;
  m_bps = bps;
}

void 
CsmaNetDevice::SetInterframeGap (Time t)
{
  NS_LOG_FUNCTION;
  m_tInterframeGap = t;
}

void 
CsmaNetDevice::SetBackoffParams (Time slotTime, uint32_t minSlots, 
                                 uint32_t maxSlots, uint32_t ceiling, 
                                 uint32_t maxRetries)
{
  NS_LOG_FUNCTION;
  m_backoff.m_slotTime = slotTime;
  m_backoff.m_minSlots = minSlots;
  m_backoff.m_maxSlots = maxSlots;
  m_backoff.m_ceiling = ceiling;
  m_backoff.m_maxRetries = maxRetries;
}

void 
CsmaNetDevice::AddHeader (Packet& p, Mac48Address dest,
                            uint16_t protocolNumber)
{
  NS_LOG_FUNCTION;
  if (m_encapMode == RAW)
    {
      return;
    }
  EthernetHeader header (false);
  EthernetTrailer trailer;
  Mac48Address source = Mac48Address::ConvertFrom (GetAddress ());
  header.SetSource(source);
  header.SetDestination(dest);

  uint16_t lengthType = 0;
  switch (m_encapMode) 
    {
    case ETHERNET_V1:
      lengthType = p.GetSize() + header.GetSerializedSize() + trailer.GetSerializedSize();
      break;
    case IP_ARP:
      lengthType = protocolNumber;
      break;
    case LLC: {
      LlcSnapHeader llc;
      llc.SetType (protocolNumber);
      p.AddHeader (llc);
    } break;
    case RAW:
      NS_ASSERT (false);
      break;
    }
  header.SetLengthType (lengthType);
  p.AddHeader(header);
  trailer.CalcFcs(p);
  p.AddTrailer(trailer);
}

bool 
CsmaNetDevice::ProcessHeader (Packet& p, uint16_t & param)
{
  NS_LOG_FUNCTION;
  if (m_encapMode == RAW)
    {
      return true;
    }
  EthernetHeader header (false);
  EthernetTrailer trailer;
      
  p.RemoveTrailer(trailer);
  trailer.CheckFcs(p);
  p.RemoveHeader(header);

  if ((header.GetDestination() != GetBroadcast ()) &&
      (header.GetDestination() != GetAddress ()))
    {
      return false;
    }

  switch (m_encapMode)
    {
    case ETHERNET_V1:
    case IP_ARP:
      param = header.GetLengthType();
      break;
    case LLC: {
      LlcSnapHeader llc;
      p.RemoveHeader (llc);
      param = llc.GetType ();
    } break;
    case RAW:
      NS_ASSERT (false);
      break;
    }
  return true;
}

bool
CsmaNetDevice::DoNeedsArp (void) const
{
  NS_LOG_FUNCTION;
  if ((m_encapMode == IP_ARP) || (m_encapMode == LLC))
    {
      return true;
    } 
  else 
    {
      return false;
    }
}

bool
CsmaNetDevice::SendTo (
  const Packet& packet, 
  const Address& dest, 
  uint16_t protocolNumber)
{
  NS_LOG_FUNCTION;
  Packet p = packet;
  NS_LOG_LOGIC ("p=" << &p);
  NS_LOG_LOGIC ("UID is " << p.GetUid () << ")");

  NS_ASSERT (IsLinkUp ());

  // Only transmit if send side of net device is enabled
  if (!IsSendEnabled())
    return false;

  Mac48Address destination = Mac48Address::ConvertFrom (dest);
  AddHeader(p, destination, protocolNumber);

  // Place the packet to be sent on the send queue
  if (m_queue->Enqueue(p) == false )
    {
      return false;
    }
  // If the device is idle, we need to start a transmission. Otherwise,
  // the transmission will be started when the current packet finished
  // transmission (see TransmitCompleteEvent)
  if (m_txMachineState == READY) 
    {
      // Store the next packet to be transmitted
      if (m_queue->Dequeue (m_currentPkt))
        {
          TransmitStart();
        }
    }
  return true;
}

void
CsmaNetDevice::TransmitStart ()
{
  NS_LOG_FUNCTION;
  NS_LOG_LOGIC ("m_currentPkt=" << &m_currentPkt);
  NS_LOG_LOGIC ("UID is " << m_currentPkt.GetUid ());
//
// This function is called to start the process of transmitting a packet.
// We need to tell the channel that we've started wiggling the wire and
// schedule an event that will be executed when it's time to tell the 
// channel that we're done wiggling the wire.
//
  NS_ASSERT_MSG((m_txMachineState == READY) || (m_txMachineState == BACKOFF), 
                "Must be READY to transmit. Tx state is: " 
                << m_txMachineState);

  // Only transmit if send side of net device is enabled
  if (!IsSendEnabled())
    return;

  if (m_channel->GetState() != IDLE)
    { // Channel busy, backoff and rechedule TransmitStart()
      m_txMachineState = BACKOFF;
      if (m_backoff.MaxRetriesReached())
        { // Too many retries reached, abort transmission of packet
          TransmitAbort();
        } 
      else 
        {
          m_backoff.IncrNumRetries();
          Time backoffTime = m_backoff.GetBackoffTime();

          NS_LOG_LOGIC ("Channel busy, backing off for " << 
            backoffTime.GetSeconds () << " sec");

          Simulator::Schedule (backoffTime, 
                               &CsmaNetDevice::TransmitStart, 
                               this);
        }
    } 
  else 
    {
      // Channel is free, transmit packet
      m_txMachineState = BUSY;
      Time tEvent = Seconds (m_bps.CalculateTxTime(m_currentPkt.GetSize()));
      
      NS_LOG_LOGIC ("Schedule TransmitCompleteEvent in " << 
        tEvent.GetSeconds () << "sec");
      
      Simulator::Schedule (tEvent, 
                           &CsmaNetDevice::TransmitCompleteEvent, 
                           this);
      if (!m_channel->TransmitStart (m_currentPkt, m_deviceId))
        {
          NS_LOG_WARN ("Channel transmit start did not work at " << 
            tEvent.GetSeconds () << "sec");
          m_txMachineState = READY;
        } 
      else 
        {
          // Transmission success, reset backoff time parameters.
          m_backoff.ResetBackoffTime();
        }
    }
}


void
CsmaNetDevice::TransmitAbort (void)
{
  NS_LOG_FUNCTION;
  NS_LOG_LOGIC ("Pkt UID is " << m_currentPkt.GetUid () << ")");

  // Try to transmit a new packet
  bool found;
  found = m_queue->Dequeue (m_currentPkt);
  NS_ASSERT_MSG(found, "IsEmpty false but no Packet on queue?");
  m_backoff.ResetBackoffTime();
  m_txMachineState = READY;
  TransmitStart ();
}

void
CsmaNetDevice::TransmitCompleteEvent (void)
{
  NS_LOG_FUNCTION;
//
// This function is called to finish the  process of transmitting a packet.
// We need to tell the channel that we've stopped wiggling the wire and
// schedule an event that will be executed when it's time to re-enable
// the transmitter after the interframe gap.
//
  NS_ASSERT_MSG(m_txMachineState == BUSY, "Must be BUSY if transmitting");
  // Channel should be transmitting
  NS_ASSERT(m_channel->GetState() == TRANSMITTING);
  m_txMachineState = GAP;

  NS_LOG_LOGIC ("Pkt UID is " << m_currentPkt.GetUid () << ")");
  m_channel->TransmitEnd (); 

  NS_LOG_LOGIC ("Schedule TransmitReadyEvent in "
    << m_tInterframeGap.GetSeconds () << "sec");

  Simulator::Schedule (m_tInterframeGap, 
                       &CsmaNetDevice::TransmitReadyEvent, 
                       this);
}

void
CsmaNetDevice::TransmitReadyEvent (void)
{
  NS_LOG_FUNCTION;
//
// This function is called to enable the transmitter after the interframe
// gap has passed.  If there are pending transmissions, we use this opportunity
// to start the next transmit.
//
  NS_ASSERT_MSG(m_txMachineState == GAP, "Must be in interframe gap");
  m_txMachineState = READY;

  // Get the next packet from the queue for transmitting
  if (m_queue->IsEmpty())
    {
      return;
    }
  else
    {
      bool found;
      found = m_queue->Dequeue (m_currentPkt);
      NS_ASSERT_MSG(found, "IsEmpty false but no Packet on queue?");
      TransmitStart ();
    }
}

Ptr<TraceResolver>
CsmaNetDevice::GetTraceResolver (void) const
{
  NS_LOG_FUNCTION;
  Ptr<CompositeTraceResolver> resolver = Create<CompositeTraceResolver> ();
  resolver->AddComposite ("queue", m_queue);
  resolver->AddSource ("rx",
                       TraceDoc ("receive MAC packet",
                                 "const Packet &", "packet received"),
                       m_rxTrace,
                       CsmaTraceType (CsmaTraceType::RX));
  resolver->AddSource ("drop",
                       TraceDoc ("drop MAC packet",
                                 "const Packet &", "packet dropped"),
                       m_dropTrace,
                       CsmaTraceType (CsmaTraceType::DROP));
  resolver->SetParentResolver (NetDevice::GetTraceResolver ());
  return resolver;
}

bool
CsmaNetDevice::Attach (Ptr<CsmaChannel> ch)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << &ch << ")");

  m_channel = ch;

  m_deviceId = m_channel->Attach(this);
  m_bps = m_channel->GetDataRate ();
  m_tInterframeGap = m_channel->GetDelay ();

  /* 
   * For now, this device is up whenever a channel is attached to it.
   */
  NotifyLinkUp ();
  return true;
}

void
CsmaNetDevice::AddQueue (Ptr<Queue> q)
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << q << ")");

  m_queue = q;
}

void
CsmaNetDevice::Receive (const Packet& packet)
{
  NS_LOG_FUNCTION;

  EthernetHeader header (false);
  EthernetTrailer trailer;
  Mac48Address broadcast;
  Mac48Address multicast;
  Mac48Address destination;
  Packet p = packet;

  NS_LOG_LOGIC ("UID is " << p.GetUid());

  // Only receive if send side of net device is enabled
  if (!IsReceiveEnabled())
    {
      m_dropTrace (p);
      return;
    }

  if (m_encapMode == RAW)
    {
      ForwardUp (packet, 0, GetBroadcast ());
      m_dropTrace (p);
      return;
    }
  p.RemoveTrailer(trailer);
  trailer.CheckFcs(p);
  p.RemoveHeader(header);

  NS_LOG_LOGIC ("Pkt destination is " << header.GetDestination ());
//
// An IP host group address is mapped to an Ethernet multicast address
// by placing the low-order 23-bits of the IP address into the low-order
// 23 bits of the Ethernet multicast address 01-00-5E-00-00-00 (hex).
//
// We are going to receive all packets destined to any multicast address,
// which means clearing the low-order 23 bits the header destination 
//
  Mac48Address mcDest;
  uint8_t      mcBuf[6];

  header.GetDestination ().CopyTo (mcBuf);
  mcBuf[3] &= 0x80;
  mcBuf[4] = 0;
  mcBuf[5] = 0;
  mcDest.CopyFrom (mcBuf);

  multicast = Mac48Address::ConvertFrom (GetMulticast ());
  broadcast = Mac48Address::ConvertFrom (GetBroadcast ());
  destination = Mac48Address::ConvertFrom (GetAddress ());

  if ((header.GetDestination () != broadcast) &&
      (mcDest != multicast) &&
      (header.GetDestination () != destination))
    {
      NS_LOG_LOGIC ("Dropping pkt ");
      m_dropTrace (p);
      return;
    }

  m_rxTrace (p);
//
// protocol must be initialized to avoid a compiler warning in the RAW
// case that breaks the optimized build.
//
  uint16_t protocol = 0;

  switch (m_encapMode)
    {
    case ETHERNET_V1:
    case IP_ARP:
      protocol = header.GetLengthType();
      break;
    case LLC: {
      LlcSnapHeader llc;
      p.RemoveHeader (llc);
      protocol = llc.GetType ();
    } break;
    case RAW:
      NS_ASSERT (false);
      break;
    }
  
  ForwardUp (p, protocol, header.GetSource ());
  return;
}

Address
CsmaNetDevice::MakeMulticastAddress(Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION;
  NS_LOG_PARAM ("(" << multicastGroup << ")");
//
// First, get the generic multicast address.
//
  Address hardwareDestination = GetMulticast ();

  NS_LOG_LOGIC ("Device multicast address: " << hardwareDestination);
//
// It's our address, and we know we're playing with an EUI-48 address here
// primarily since we know that by construction, but also since the parameter
// is an Ipv4Address.
//
  Mac48Address etherAddr = Mac48Address::ConvertFrom (hardwareDestination);
//
// We now have the multicast address in an abstract 48-bit container.  We 
// need to pull it out so we can play with it.  When we're done, we have the 
// high order bits in etherBuffer[0], etc.
//
  uint8_t etherBuffer[6];
  etherAddr.CopyTo (etherBuffer);
//
// Now we need to pull the raw bits out of the Ipv4 destination address.
//
  uint8_t ipBuffer[4];
  multicastGroup.Serialize (ipBuffer);
//
// RFC 1112 says that an Ipv4 host group address is mapped to an EUI-48
// multicast address by placing the low-order 23-bits of the IP address into 
// the low-order 23 bits of the Ethernet multicast address 
// 01-00-5E-00-00-00 (hex). 
//
  etherBuffer[3] |= ipBuffer[1] & 0x7f;
  etherBuffer[4] = ipBuffer[2];
  etherBuffer[5] = ipBuffer[3];
//
// Now, etherBuffer has the desired ethernet multicast address.  We have to
// suck these bits back into the Mac48Address,
//
  etherAddr.CopyFrom (etherBuffer);
//
// Implicit conversion (operator Address ()) is defined for Mac48Address, so
// use it by just returning the EUI-48 address which is automagically converted
// to an Address.
//
  NS_LOG_LOGIC ("multicast address is " << etherAddr);

  return etherAddr;
}

Ptr<Queue>
CsmaNetDevice::GetQueue(void) const 
{ 
  NS_LOG_FUNCTION;
  return m_queue;
}

Ptr<Channel>
CsmaNetDevice::DoGetChannel(void) const 
{ 
  NS_LOG_FUNCTION;
  return m_channel;
}

} // namespace ns3