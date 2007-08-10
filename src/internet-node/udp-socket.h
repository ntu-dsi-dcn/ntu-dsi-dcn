/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
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
#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <stdint.h>
#include "ns3/callback.h"
#include "ns3/socket.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

class Ipv4EndPoint;
class Node;
class Packet;
class UdpL4Protocol;

class UdpSocket : public Socket
{
public:
  /**
   * Create an unbound udp socket.
   */
  UdpSocket (Ptr<Node> node, Ptr<UdpL4Protocol> udp);
  virtual ~UdpSocket ();

  virtual enum SocketErrno GetErrno (void) const;
  virtual Ptr<Node> GetNode (void) const;
  virtual int Bind (void);
  virtual int Bind (const Address &address);
  virtual int ShutdownSend (void);
  virtual int ShutdownRecv (void);

private:
  virtual int DoClose(Callback<void, Ptr<Socket> > closeCompleted);
  virtual int DoConnect(const Address & address,
                        Callback<void, Ptr<Socket> > connectionSucceeded,
                        Callback<void, Ptr<Socket> > connectionFailed,
                        Callback<void, Ptr<Socket> > halfClose);
  virtual int DoAccept(Callback<bool, Ptr<Socket>, const Address&> connectionRequest,
		       Callback<void, Ptr<Socket>, const Address&> newConnectionCreated,
		       Callback<void, Ptr<Socket> > closeRequested);
  virtual int DoSend (const uint8_t* buffer,
                    uint32_t size,
                    Callback<void, Ptr<Socket>, uint32_t> dataSent);
  virtual int DoSendTo(const Address &address,
                      const uint8_t *buffer,
                      uint32_t size,
                      Callback<void, Ptr<Socket>, uint32_t> dataSent);
  virtual void DoRecv(Callback<void, Ptr<Socket>, const uint8_t*, uint32_t,const Address&>);
  virtual void DoRecvDummy(Callback<void, Ptr<Socket>, uint32_t,const Address&>);

private:
  friend class Udp;
  // invoked by Udp class
  int FinishBind (void);
  void ForwardUp (const Packet &p, Ipv4Address ipv4, uint16_t port);
  void Destroy (void);
  int DoSendPacketTo (const Packet &p, const Address &daddr,
		      Callback<void, Ptr<Socket>, uint32_t> dataSent);
  int DoSendPacketTo (const Packet &p, Ipv4Address daddr, uint16_t dport,
		      Callback<void, Ptr<Socket>, uint32_t> dataSent);

  Ipv4EndPoint *m_endPoint;
  Ptr<Node> m_node;
  Ptr<UdpL4Protocol> m_udp;
  Ipv4Address m_defaultAddress;
  uint16_t m_defaultPort;
  Callback<void,Ptr<Socket>,uint32_t,const Address &> m_dummyRxCallback;
  Callback<void,Ptr<Socket>,uint8_t const*,uint32_t,const Address &> m_rxCallback;
  enum SocketErrno m_errno;
  bool m_shutdownSend;
  bool m_shutdownRecv;
  bool m_connected;
};

}//namespace ns3

#endif /* UDP_SOCKET_H */