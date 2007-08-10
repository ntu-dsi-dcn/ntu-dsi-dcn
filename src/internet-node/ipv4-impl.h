/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
#ifndef IPV4_IMPL_H
#define IPV4_IMPL_H

#include "ns3/ipv4.h"
#include "ns3/ptr.h"

namespace ns3 {

class Ipv4L3Protocol;

class Ipv4Impl : public Ipv4
{
public:
  Ipv4Impl (Ptr<Ipv4L3Protocol> ipv4);

  virtual ~Ipv4Impl ();

  virtual void AddRoutingProtocol (Ptr<Ipv4RoutingProtocol> routingProtocol,
                                   int16_t priority);

  virtual void AddHostRouteTo (Ipv4Address dest, 
			       Ipv4Address nextHop, 
			       uint32_t interface);
  virtual void AddHostRouteTo (Ipv4Address dest, 
			       uint32_t interface);
  virtual void AddNetworkRouteTo (Ipv4Address network, 
				  Ipv4Mask networkMask, 
				  Ipv4Address nextHop, 
				  uint32_t interface);
  virtual void AddNetworkRouteTo (Ipv4Address network, 
				  Ipv4Mask networkMask, 
				  uint32_t interface);
  virtual void SetDefaultRoute (Ipv4Address nextHop, 
				uint32_t interface);
  virtual uint32_t GetNRoutes (void);
  virtual Ipv4Route GetRoute (uint32_t i);
  virtual void RemoveRoute (uint32_t i);
  virtual uint32_t AddInterface (Ptr<NetDevice> device);
  virtual uint32_t GetNInterfaces (void);  
  virtual Ptr<NetDevice> GetNetDevice(uint32_t i);

  virtual void SetAddress (uint32_t i, Ipv4Address address);
  virtual void SetNetworkMask (uint32_t i, Ipv4Mask mask);
  virtual Ipv4Mask GetNetworkMask (uint32_t t) const;
  virtual Ipv4Address GetAddress (uint32_t i) const;
  virtual uint16_t GetMtu (uint32_t i) const;
  virtual bool IsUp (uint32_t i) const;
  virtual void SetUp (uint32_t i);
  virtual void SetDown (uint32_t i);
protected:
  virtual void DoDispose (void);
private:
  Ptr<Ipv4L3Protocol> m_ipv4;
};

} // namespace ns3

#endif /* IPV4_IMPL_H */