/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University of Campinas (Unicamp)
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
 * Author:  Luciano Chaves <luciano@lrc.ic.unicamp.br>
 */

#include "controladorSlice2.h"
#include <ns3/network-module.h>
#include <ns3/internet-module.h>

NS_LOG_COMPONENT_DEFINE ("ControladorSlice2");
NS_OBJECT_ENSURE_REGISTERED (ControladorSlice2);

ControladorSlice2::ControladorSlice2 ()
{
  NS_LOG_FUNCTION (this);
}

ControladorSlice2::~ControladorSlice2 ()
{
  NS_LOG_FUNCTION (this);
}

void
ControladorSlice2::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  OFSwitch13Controller::DoDispose ();
}

void
ControladorSlice2::AddRegra (uint32_t portNo, Ipv4Address ipAddr)
{
  NS_LOG_FUNCTION (this << portNo << ipAddr);

  // Inserindo na tabela 2 a regra que mapeia IP de destino na porta de saída.
  std::ostringstream cmd;
  cmd << "flow-mod cmd=add,prio=64,table=2"
      << " eth_type=0x800,ip_dst=" << ipAddr
      << " apply:output=" << portNo;
  DpctlSchedule (1, cmd.str ());
}