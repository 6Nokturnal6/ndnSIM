/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#ifndef NDN_CXX_LP_SPEED_TAG_HPP
#define NDN_CXX_LP_SPEED_TAG_HPP

#include "ndn-cxx/encoding/block-helpers.hpp"
#include "ndn-cxx/encoding/encoding-buffer.hpp"
#include "ndn-cxx/tag.hpp"

#include <iostream>
#include <fstream>
#include <cstdio>

namespace ndn {
namespace lp {

/**
 * \brief represents a SpeedTag header field
 */
class SpeedTag : public Tag
{
public:
  static constexpr int
  getTypeId() noexcept
  {
    return 0x60000002;
  }

  SpeedTag() = default;

  explicit
  SpeedTag(std::tuple<double, double, double> speed)
    : m_speed(std::move(speed))
  {
  }

  explicit
  SpeedTag(const Block& block);

  /**
   * \brief prepend SpeedTag to encoder
   */
  template<encoding::Tag TAG>
  size_t
  wireEncode(EncodingImpl<TAG>& encoder) const;

  /**
   * \brief encode SpeedTag into wire format
   */
  const Block&
  wireEncode() const;

  /**
   * \brief get SpeedTag from wire format
   */
  void
  wireDecode(const Block& wire);

public: // get & set SpeedTag
  /**
   * \return position x of GeoTag
   */
  const std::tuple<double, double, double>
  getSpeed() const
  {
    return m_speed;
  }

//   const double
//   getX() const
//   {
//     double x = std::get<0>(m_speed);
//     return x;
//     std::cout << "Speed X: " << std::get<0>(m_speed) << std::endl;
//     return 0;
//   }
//
//   const double
//   getY() const
//   {
//     auto x = std::get<1>(m_speed);
//     return x;
//   }
//
//   const double
//   getZ() const
//   {
//     return std::get<2>(m_speed);
//   }



  /**
   * \brief set speed x
   */
  SpeedTag*
  setSpeed(std::tuple<double, double, double> speed)
  {
    m_speed = speed;
    return this;
  }

private:
  std::tuple<double, double, double> m_speed = {0.0, 0.0, 0.0};
  mutable Block m_wire;

//   friend std::ostream& operator<<(std::ostream& o, const std::tuple<double, double, double>& t);
  friend std::ostream& operator<<(std::ostream& o, const std::tuple<double, double, double>& t) {
    o << "(" << std::get<0>(t) << "," << std::get<1>(t) << "," << std::get<2>(t) << ")";
    return o;
}
};

} // namespace lp
} // namespace ndn

// std::ostream& operator<<(std::ostream& o, const std::tuple<double, double, double>& t) {
//     o << "(" << std::get<0>(t) << "," << std::get<1>(t) << "," << std::get<2>(t) << ")";
//     return o;
// }
#endif // NDN_CXX_LP_SPEED_TAG_HPP
