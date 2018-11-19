/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2018 NITK Surathkal
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
 * Authors: Shikha Bakshi <shikhabakshi912@gmail.com>
 *          Mohit P. Tahiliani <tahiliani@nitk.edu.in>
 */

#include "ns3/log.h"
#include "ns3/tcp-westwood.h"
#include "tcp-general-test.h"
#include "ns3/simple-channel.h"
#include "ns3/node.h"
#include "tcp-error-model.h"
#include "ns3/tcp-header.h"
#include "ns3/tcp-tx-buffer.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("TcpFackTest");

class FackPermittedTestCase : public TcpGeneralTest
{
public:
  /*
   * \brief Configuration of the test
  */
  enum Configuration
  {
    DISABLED,
    ENABLED_RECEIVER,
    ENABLED_SENDER,
    ENABLED
  };

  /**
   * \brief Constructor
   * \param conf Test configuration.
   *
  */
  FackPermittedTestCase (FackPermittedTestCase::Configuration conf);

protected:
  virtual Ptr<TcpSocketMsgBase> CreateReceiverSocket (Ptr<Node> node);
  virtual Ptr<TcpSocketMsgBase> CreateSenderSocket (Ptr<Node> node);

  Configuration m_configuration; //!< The configuration
};

FackPermittedTestCase::FackPermittedTestCase (FackPermittedTestCase::Configuration conf)
  : TcpGeneralTest ("Testing the TCP Fack Permitted option")
{
  m_configuration = conf;
}

Ptr<TcpSocketMsgBase>
FackPermittedTestCase::CreateReceiverSocket (Ptr<Node> node)
{
  Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateReceiverSocket (node);

  switch (m_configuration)
    {
    case DISABLED:
      socket->SetAttribute ("Fack", BooleanValue (false));
      break;

    case ENABLED_RECEIVER:
      socket->SetAttribute ("Fack", BooleanValue (true));
      break;

    case ENABLED_SENDER:
      socket->SetAttribute ("Fack", BooleanValue (false));
      break;

    case ENABLED:
      socket->SetAttribute ("Fack", BooleanValue (true));
      break;
    }

  return socket;
}

Ptr<TcpSocketMsgBase>
FackPermittedTestCase::CreateSenderSocket (Ptr<Node> node)
{
  Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateSenderSocket (node);

  switch (m_configuration)
    {
    case DISABLED:
      socket->SetAttribute ("Fack", BooleanValue (false));
      break;

    case ENABLED_RECEIVER:
      socket->SetAttribute ("Fack", BooleanValue (false));
      break;

    case ENABLED_SENDER:
      socket->SetAttribute ("Fack", BooleanValue (true));
      break;

    case ENABLED:
      socket->SetAttribute ("Fack", BooleanValue (true));
      break;
    }

  return socket;
}

class TcpFackTest : public TcpGeneralTest
{
public:

  TcpFackTest (TypeId congControl, uint32_t seqToKill, uint32_t numOfPkts, const std::string &msg);

  virtual Ptr<ErrorModel> CreateSenderErrorModel ();
  virtual Ptr<ErrorModel> CreateReceiverErrorModel ();

  virtual Ptr<TcpSocketMsgBase> CreateSenderSocket (Ptr<Node> node);

protected:
  virtual void RcvAck (const Ptr<const TcpSocketState> tcb,
                            const TcpHeader& h, SocketWho who);

  virtual void CongStateTrace (const TcpSocketState::TcpCongState_t oldValue,
                               const TcpSocketState::TcpCongState_t newValue);

  void PktDropped (const Ipv4Header &ipH, const TcpHeader& tcpH, Ptr<const Packet> p);
  virtual void ConfigureProperties ();
  virtual void ConfigureEnvironment ();

  uint32_t m_pktDropped;      //!< The packet has been dropped.
  uint32_t m_startSeqToKill;  //!< Sequence number of the first packet to drop.
  uint32_t m_seqToKill;       //!< Sequence number to drop.
  uint32_t m_pkts;            //!< Number of packets to drop.
  uint32_t m_dupAckReceived;  //!< DupACk received.
  uint32_t m_sndFack;         //!< Forward AckManagement.
  uint32_t m_unacked;         //!< First Byte of Unacknowledged data
  uint32_t m_pktSize;         //!< Sender Packet Size

  Ptr<TcpSeqErrorModel> m_errorModel; //!< Error model.
};

TcpFackTest::TcpFackTest (TypeId typeId, uint32_t startSeqToKill, uint32_t numOfPkts,
                                  const std::string &msg)
  : TcpGeneralTest (msg),
    m_pktDropped (0),
    m_startSeqToKill (startSeqToKill),
    m_seqToKill (startSeqToKill),
    m_pkts (numOfPkts),
    m_dupAckReceived (0),
    m_sndFack (5001),
    m_unacked (1),
    m_pktSize (500)
{
  m_congControlTypeId = typeId;
}

void
 TcpFackTest::ConfigureProperties ()
 {
   TcpGeneralTest::ConfigureProperties ();
   SetInitialSsThresh (SENDER, 0);
   SetInitialCwnd (SENDER, 10);
   SetSegmentSize (SENDER, m_pktSize);
 }

 void
 TcpFackTest::ConfigureEnvironment ()
 {
   TcpGeneralTest::ConfigureEnvironment ();
   SetAppPktCount (10);
 }

Ptr<ErrorModel>
 TcpFackTest::CreateSenderErrorModel ()
 {
   return 0;
 }

Ptr<ErrorModel>
 TcpFackTest::CreateReceiverErrorModel ()
 {
   m_errorModel = CreateObject<TcpSeqErrorModel> ();

  for (uint32_t i=0; i < m_pkts; i++)
     {
       m_seqToKill = m_startSeqToKill + i * m_pktSize;
       m_errorModel->AddSeqToKill (SequenceNumber32 (m_seqToKill));
       m_errorModel->SetDropCallback (MakeCallback (&TcpFackTest::PktDropped, this));
     }

  return m_errorModel;
 }

 Ptr<TcpSocketMsgBase>
 TcpFackTest::CreateSenderSocket (Ptr<Node> node)
{
  Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateSenderSocket (node);
  socket->SetAttribute ("MinRto", TimeValue (Seconds (10.0)));
  socket->SetAttribute ("Fack", BooleanValue (true));
  return socket;
}

 void
 TcpFackTest::RcvAck (const Ptr<const TcpSocketState> tcb, const TcpHeader &h,
                         SocketWho who)
 {
   NS_LOG_FUNCTION (this << tcb << h << who);

   if (h.GetAckNumber ().GetValue () == m_startSeqToKill && GetDupAckCount (SENDER) == 1 && m_pktDropped == m_pkts)
     {
       m_sndFack = 5001;
     }
   Ptr<TcpTxBuffer> tx = GetTxBuffer (SENDER);
   m_unacked = tx->HeadSequence ().GetValue ();
 }

// Trace TCP congestion state
 void
 TcpFackTest::CongStateTrace (const TcpSocketState::TcpCongState_t oldValue,
                               const TcpSocketState::TcpCongState_t newValue)
 {
   NS_LOG_FUNCTION (this << oldValue << newValue);

   if (oldValue == TcpSocketState::CA_DISORDER && newValue == TcpSocketState::CA_RECOVERY)
     {
       NS_TEST_ASSERT_MSG_GT ( m_sndFack-m_unacked, 3 * m_pktSize, "DISORDER to RECOVERY Invalid");
     }
 }

// Count number of packets dropped
void
TcpFackTest::PktDropped (const Ipv4Header &ipH, const TcpHeader& tcpH, Ptr<const Packet> p)
{
  NS_LOG_FUNCTION (this << ipH << tcpH);
  m_pktDropped ++;
}

class TcpFackTestSuite : public TestSuite
{
public:
  TcpFackTestSuite () : TestSuite ("tcp-fack-test", UNIT)
  {
    std::list<TypeId> types;
    types.insert (types.begin (), TcpWestwood::GetTypeId ());
    types.insert (types.begin (), TcpNewReno::GetTypeId ());

    AddTestCase (new FackPermittedTestCase (FackPermittedTestCase::DISABLED), TestCase::QUICK);
    AddTestCase (new FackPermittedTestCase (FackPermittedTestCase::ENABLED_RECEIVER), TestCase::QUICK);
    AddTestCase (new FackPermittedTestCase (FackPermittedTestCase::ENABLED_SENDER), TestCase::QUICK);
    AddTestCase (new FackPermittedTestCase (FackPermittedTestCase::ENABLED), TestCase::QUICK);

    for (std::list<TypeId>::iterator it = types.begin (); it != types.end (); ++it)
      {
        AddTestCase (new TcpFackTest ((*it), 2501, 4, "Fack testing"), TestCase::QUICK);
      }
  }
};

static TcpFackTestSuite g_TcpFackTestSuite; //!< Static variable for test initialization
