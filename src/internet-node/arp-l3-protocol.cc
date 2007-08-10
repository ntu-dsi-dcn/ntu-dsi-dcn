/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006 INRIA
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
#include "ns3/packet.h"
#include "ns3/debug.h"
#include "ns3/empty-trace-resolver.h"
#include "ns3/node.h"
#include "ns3/net-device.h"

#include "ipv4-l3-protocol.h"
#include "arp-l3-protocol.h"
#include "arp-header.h"
#include "arp-cache.h"
#include "ipv4-interface.h"

NS_DEBUG_COMPONENT_DEFINE ("ArpL3Protocol");

namespace ns3 {

const InterfaceId ArpL3Protocol::iid = MakeInterfaceId ("ArpL3Protocol", Object::iid);
const uint16_t ArpL3Protocol::PROT_NUMBER = 0x0806;

ArpL3Protocol::ArpL3Protocol (Ptr<Node> node)
  : m_node (node)
{
  SetInterfaceId (ArpL3Protocol::iid);
}

ArpL3Protocol::~ArpL3Protocol ()
{}

void 
ArpL3Protocol::DoDispose (void)
{
  for (CacheList::const_iterator i = m_cacheList.begin (); i != m_cacheList.end (); i++)
    {
      delete *i;
    }
  m_cacheList.clear ();
  m_node = 0;
  Object::DoDispose ();
}

TraceResolver *
ArpL3Protocol::CreateTraceResolver (TraceContext const &context)
{
  return new EmptyTraceResolver (context);
}

ArpCache *
ArpL3Protocol::FindCache (Ptr<NetDevice> device)
{
  for (CacheList::const_iterator i = m_cacheList.begin (); i != m_cacheList.end (); i++)
    {
      if ((*i)->GetDevice () == device)
	{
	  return *i;
	}
    }
  Ptr<Ipv4L3Protocol> ipv4 = m_node->QueryInterface<Ipv4L3Protocol> (Ipv4L3Protocol::iid);
  Ipv4Interface *interface = ipv4->FindInterfaceForDevice (device);
  ArpCache * cache = new ArpCache (device, interface);
  NS_ASSERT (device->IsBroadcast ());
  device->SetLinkChangeCallback (MakeCallback (&ArpCache::Flush, cache));
  m_cacheList.push_back (cache);
  return cache;
}

void 
ArpL3Protocol::Receive(Ptr<NetDevice> device, const Packet& p, uint16_t protocol, const Address &from)
{
  ArpCache *cache = FindCache (device);
  ArpHeader arp;
  Packet packet = p;
  packet.RemoveHeader (arp);
  
  NS_DEBUG ("ARP: received "<< (arp.IsRequest ()? "request" : "reply") <<
            " node="<<m_node->GetId ()<<", got request from " <<
            arp.GetSourceIpv4Address () << " for address " <<
            arp.GetDestinationIpv4Address () << "; we have address " <<
            cache->GetInterface ()->GetAddress ());

  if (arp.IsRequest () && 
      arp.GetDestinationIpv4Address () == cache->GetInterface ()->GetAddress ()) 
    {
      NS_DEBUG ("node="<<m_node->GetId () <<", got request from " << 
                arp.GetSourceIpv4Address () << " -- send reply");
      SendArpReply (cache, arp.GetSourceIpv4Address (),
                    arp.GetSourceHardwareAddress ());
    } 
  else if (arp.IsReply () &&
           arp.GetDestinationIpv4Address ().IsEqual (cache->GetInterface ()->GetAddress ()) &&
           arp.GetDestinationHardwareAddress () == device->GetAddress ()) 
    {
      Ipv4Address from = arp.GetSourceIpv4Address ();
      ArpCache::Entry *entry = cache->Lookup (from);
      if (entry != 0)
        {
          if (entry->IsWaitReply ()) 
            {
              NS_DEBUG ("node="<<m_node->GetId ()<<", got reply from " << 
                        arp.GetSourceIpv4Address ()
                     << " for waiting entry -- flush");
              Address from_mac = arp.GetSourceHardwareAddress ();
              Packet waiting = entry->MarkAlive (from_mac);
	      cache->GetInterface ()->Send (waiting, arp.GetSourceIpv4Address ());
            } 
          else 
            {
              // ignore this reply which might well be an attempt 
              // at poisening my arp cache.
              NS_DEBUG ("node="<<m_node->GetId ()<<", got reply from " << 
                        arp.GetSourceIpv4Address () << 
                        " for non-waiting entry -- drop");
	      // XXX report packet as dropped.
            }
        } 
      else 
        {
          NS_DEBUG ("node="<<m_node->GetId ()<<", got reply for unknown entry -- drop");
	  // XXX report packet as dropped.
        }
    }
  else
    {
      NS_DEBUG ("node="<<m_node->GetId ()<<", got request from " <<
                arp.GetSourceIpv4Address () << " for unknown address " <<
                arp.GetDestinationIpv4Address () << " -- drop");
    }
}
bool 
ArpL3Protocol::Lookup (Packet &packet, Ipv4Address destination, 
                       Ptr<NetDevice> device,
                       Address *hardwareDestination)
{
  ArpCache *cache = FindCache (device);
  ArpCache::Entry *entry = cache->Lookup (destination);
  if (entry != 0)
    {
      if (entry->IsExpired ()) 
        {
          if (entry->IsDead ()) 
            {
              NS_DEBUG ("node="<<m_node->GetId ()<<
                        ", dead entry for " << destination << " expired -- send arp request");
              entry->MarkWaitReply (packet);
              SendArpRequest (cache, destination);
            } 
          else if (entry->IsAlive ()) 
            {
              NS_DEBUG ("node="<<m_node->GetId ()<<
                        ", alive entry for " << destination << " expired -- send arp request");
              entry->MarkWaitReply (packet);
              SendArpRequest (cache, destination);
            } 
          else if (entry->IsWaitReply ()) 
            {
              NS_DEBUG ("node="<<m_node->GetId ()<<
                        ", wait reply for " << destination << " expired -- drop");
              entry->MarkDead ();
	      // XXX report packet as 'dropped'
            }
        } 
      else 
        {
          if (entry->IsDead ()) 
            {
              NS_DEBUG ("node="<<m_node->GetId ()<<
                        ", dead entry for " << destination << " valid -- drop");
	      // XXX report packet as 'dropped'
            } 
          else if (entry->IsAlive ()) 
            {
              NS_DEBUG ("node="<<m_node->GetId ()<<
                        ", alive entry for " << destination << " valid -- send");
	      *hardwareDestination = entry->GetMacAddress ();
              return true;
            } 
          else if (entry->IsWaitReply ()) 
            {
              NS_DEBUG ("node="<<m_node->GetId ()<<
                        ", wait reply for " << destination << " valid -- drop previous");
              Packet old = entry->UpdateWaitReply (packet);
	      // XXX report 'old' packet as 'dropped'
            }
        }

    }
  else
    {
      // This is our first attempt to transmit data to this destination.
      NS_DEBUG ("node="<<m_node->GetId ()<<
                ", no entry for " << destination << " -- send arp request");
      entry = cache->Add (destination);
      entry->MarkWaitReply (packet);
      SendArpRequest (cache, destination);
    }
  return false;
}

void
ArpL3Protocol::SendArpRequest (ArpCache const *cache, Ipv4Address to)
{
  ArpHeader arp;
  NS_DEBUG ("ARP: sending request from node "<<m_node->GetId ()<<
            " || src: " << cache->GetDevice ()->GetAddress () <<
            " / " << cache->GetInterface ()->GetAddress () <<
            " || dst: " << cache->GetDevice ()->GetBroadcast () <<
            " / " << to);
  arp.SetRequest (cache->GetDevice ()->GetAddress (),
		  cache->GetInterface ()->GetAddress (), 
                  cache->GetDevice ()->GetBroadcast (),
                  to);
  Packet packet;
  packet.AddHeader (arp);
  cache->GetDevice ()->Send (packet, cache->GetDevice ()->GetBroadcast (), PROT_NUMBER);
}

void
ArpL3Protocol::SendArpReply (ArpCache const *cache, Ipv4Address toIp, Address toMac)
{
  ArpHeader arp;
  NS_DEBUG ("ARP: sending reply from node "<<m_node->GetId ()<<
            "|| src: " << cache->GetDevice ()->GetAddress () << 
            " / " << cache->GetInterface ()->GetAddress () <<
            " || dst: " << toMac << " / " << toIp);
  arp.SetReply (cache->GetDevice ()->GetAddress (),
                cache->GetInterface ()->GetAddress (),
                toMac, toIp);
  Packet packet;
  packet.AddHeader (arp);
  cache->GetDevice ()->Send (packet, toMac, PROT_NUMBER);
}

}//namespace ns3