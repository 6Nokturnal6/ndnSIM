/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2019 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 */

#include "ndn-cxx/lp/speed-tag.hpp"
#include "ndn-cxx/lp/tlv.hpp"

namespace ndn {
namespace lp {

SpeedTag::SpeedTag(const Block& block)
{
  wireDecode(block);
}

template<encoding::Tag TAG>
size_t
SpeedTag::wireEncode(EncodingImpl<TAG>& encoder) const
{
  size_t length = 0;
  length += prependDoubleBlock(encoder, tlv::SpeedTagPos, std::get<2>(m_speed));
  length += prependDoubleBlock(encoder, tlv::SpeedTagPos, std::get<1>(m_speed));
  length += prependDoubleBlock(encoder, tlv::SpeedTagPos, std::get<0>(m_speed));
  length += encoder.prependVarNumber(length);
  length += encoder.prependVarNumber(tlv::SpeedTag);
  return length;
}

template size_t
SpeedTag::wireEncode<encoding::EncoderTag>(EncodingImpl<encoding::EncoderTag>& encoder) const;

template size_t
SpeedTag::wireEncode<encoding::EstimatorTag>(EncodingImpl<encoding::EstimatorTag>& encoder) const;

const Block&
SpeedTag::wireEncode() const
{
  if (m_wire.hasWire()) {
    return m_wire;
  }

  EncodingEstimator estimator;
  size_t estimatedSize = wireEncode(estimator);

  EncodingBuffer buffer(estimatedSize, 0);
  wireEncode(buffer);

  m_wire = buffer.block();

  return m_wire;
}

void
SpeedTag::wireDecode(const Block& wire)
{
  if (wire.type() != tlv::SpeedTag) {
    NDN_THROW(ndn::tlv::Error("expecting SpeedTag block"));
  }

  m_wire = wire;
  m_wire.parse();

  if (m_wire.elements().size() < 3 ||
      m_wire.elements()[0].type() != tlv::SpeedTagPos ||
      m_wire.elements()[1].type() != tlv::SpeedTagPos ||
      m_wire.elements()[2].type() != tlv::SpeedTagPos) {
    NDN_THROW(ndn::tlv::Error("Unexpected input while decoding SpeedTag"));
  }
  m_speed = {encoding::readDouble(m_wire.elements()[0]),
           encoding::readDouble(m_wire.elements()[1]),
           encoding::readDouble(m_wire.elements()[2])};
}

} // namespace lp
} // namespace ndn
