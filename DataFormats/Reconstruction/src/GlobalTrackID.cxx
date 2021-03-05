// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @file  GlobalTrackID.cxx
/// \brief Global index for barrel track: provides provenance (detectors combination), index in respective array and some number of bits
/// \author ruben.shahoyan@cern.ch

#include "ReconstructionDataFormats/GlobalTrackID.h"
#include "Framework/Logger.h"
#include <fmt/printf.h>
#include <iostream>
#include <bitset>

using namespace o2::dataformats;
using DetID = o2::detectors::DetID;

const std::array<DetID::mask_t, GlobalTrackID::NSources> GlobalTrackID::SourceDetectorsMasks = {
  DetID::getMask(DetID::ITS),
  DetID::getMask(DetID::TPC),
  DetID::getMask(DetID::TRD),
  DetID::getMask(DetID::TOF),
  DetID::getMask(DetID::PHS),
  DetID::getMask(DetID::CPV),
  DetID::getMask(DetID::EMC),
  DetID::getMask(DetID::HMP),
  DetID::getMask(DetID::MFT),
  DetID::getMask(DetID::MCH),
  DetID::getMask(DetID::MID),
  DetID::getMask(DetID::ZDC),
  DetID::getMask(DetID::FT0),
  DetID::getMask(DetID::FV0),
  DetID::getMask(DetID::FDD),
  //
  DetID::getMask(DetID::ITS) | DetID::getMask(DetID::TPC),
  DetID::getMask(DetID::TPC) | DetID::getMask(DetID::TOF),
  DetID::getMask(DetID::TPC) | DetID::getMask(DetID::TRD),
  DetID::getMask(DetID::ITS) | DetID::getMask(DetID::TPC) | DetID::getMask(DetID::TRD),
  DetID::getMask(DetID::ITS) | DetID::getMask(DetID::TPC) | DetID::getMask(DetID::TOF),
  DetID::getMask(DetID::TPC) | DetID::getMask(DetID::TRD) | DetID::getMask(DetID::TOF),
  DetID::getMask(DetID::ITS) | DetID::getMask(DetID::TPC) | DetID::getMask(DetID::TRD) | DetID::getMask(DetID::TOF)};

std::string GlobalTrackID::asString() const
{
  std::bitset<NBitsFlags()> bits{getFlags()};
  return fmt::format("[{:s}/{:d}/{:s}]", getSourceName(), getIndex(), bits.to_string());
}

GlobalTrackID::mask_t GlobalTrackID::getSourcesMask(const std::string_view srcList)
{
  mask_t mask;
  std::string ss(srcList), sname{};
  if (ss.find(NONE) != std::string::npos) {
    return mask;
  }
  if (ss.find(ALL) != std::string::npos) {
    mask.set();
    return mask;
  }
  std::replace(ss.begin(), ss.end(), ' ', ',');
  std::stringstream sss(ss);
  while (getline(sss, sname, ',')) {
    for (auto id = 0; id < NSources; id++) {
      if (sname == getSourceName(id)) {
        mask.set(id);
        sname = "";
        break;
      }
    }
    if (!sname.empty()) {
      throw std::runtime_error(fmt::format("Wrong entry {:s} in reco-sources list {:s}", sname, srcList));
    }
  }
  return mask;
}

bool GlobalTrackID::includesDet(DetID id, GlobalTrackID::mask_t srcm)
{
  for (int i = 0; i < NSources; i++) {
    if (includesSource(i, srcm) && getSourceDetectorsMask(i) == id.getMask()) {
      return true;
    }
  }
  return false;
}

std::ostream& o2::dataformats::operator<<(std::ostream& os, const o2::dataformats::GlobalTrackID& v)
{
  // stream itself
  os << v.asString();
  return os;
}

void GlobalTrackID::print() const
{
  LOG(INFO) << asString();
}
