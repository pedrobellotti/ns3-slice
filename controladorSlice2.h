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

#ifndef CONTROLADORSLICE2_H
#define CONTROLADORSLICE2_H

#include <ns3/ofswitch13-module.h>

using namespace ns3;

/**
 * \brief An border OpenFlow 1.3 controller
 */
class ControladorSlice2 : public OFSwitch13Controller
{
public:
  ControladorSlice2 ();          //!< Default constructor.
  virtual ~ControladorSlice2 (); //!< Dummy destructor.

  /** Destructor implementation */
  virtual void DoDispose ();

protected:
  void HandshakeSuccessful (Ptr<const RemoteSwitch> swtch);

};

#endif /* CONTROLADORSLICE2_H */