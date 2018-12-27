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

#include "tcp-general-test.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/tcp-option-sack-permitted.h"
#include "ns3/tcp-header.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("DsackPermittedTestSuite");

/**
 * \ingroup internet-test
 * \ingroup tests
 *
 * \brief Test case for checking the DSACK-PERMITTED option.
 *
 */
class DsackTestCase : public TcpGeneralTest
{
public:
  /** \brief Configuration of the test */
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
   * */
  DsackTestCase (DsackTestCase::Configuration conf);
protected:
  virtual Ptr<TcpSocketMsgBase> CreateReceiverSocket (Ptr<Node> node);
  virtual Ptr<TcpSocketMsgBase> CreateSenderSocket (Ptr<Node> node);

  Configuration m_configuration; //!< The configuration
};

DsackTestCase::DsackTestCase (DsackTestCase::Configuration conf)
  : TcpGeneralTest ("Testing the TCP Dsack Permitted option")
{
  m_configuration = conf;
}

Ptr<TcpSocketMsgBase>
DsackTestCase::CreateReceiverSocket (Ptr<Node> node)
{
  Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateReceiverSocket (node);

  switch (m_configuration)
    {
    case DISABLED:
      socket->SetAttribute ("Dsack", BooleanValue (false));
      break;

    case ENABLED_RECEIVER:
      socket->SetAttribute ("Dsack", BooleanValue (true));
      break;

    case ENABLED_SENDER:
      socket->SetAttribute ("Dsack", BooleanValue (false));
      break;

    case ENABLED:
      socket->SetAttribute ("Dsack", BooleanValue (true));
      break;
    }

  return socket;
}

Ptr<TcpSocketMsgBase>
DsackTestCase::CreateSenderSocket (Ptr<Node> node)
{
  Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateSenderSocket (node);

  switch (m_configuration)
    {
    case DISABLED:
      socket->SetAttribute ("Dsack", BooleanValue (false));
      break;

    case ENABLED_RECEIVER:
      socket->SetAttribute ("Dsack", BooleanValue (false));
      break;

    case ENABLED_SENDER:
      socket->SetAttribute ("Dsack", BooleanValue (true));
      break;

    case ENABLED:
      socket->SetAttribute ("Dsack", BooleanValue (true));
      break;
    }

  return socket;
}

/**
 * \ingroup internet-test
 *  \ingroup tests
 *
 * The test case for testing the TCP DSACK option.
 */
class TcpDsackTestSuite : public TestSuite
{
public:
  /** \brief Constructor */
  TcpDsackTestSuite ()
    : TestSuite ("tcp-dsack", UNIT)
  {
    AddTestCase (new DsackTestCase (DsackTestCase::DISABLED), TestCase::QUICK);
    AddTestCase (new DsackTestCase (DsackTestCase::ENABLED_RECEIVER), TestCase::QUICK);
    AddTestCase (new DsackTestCase (DsackTestCase::ENABLED_SENDER), TestCase::QUICK);
    AddTestCase (new DsackTestCase (DsackTestCase::ENABLED), TestCase::QUICK);
  }

};

static TcpDsackTestSuite g_tcpDsackTestSuite; //!< Static variable for test initialization
