// Copyright CERN and copyright holders of ALICE O2. This software is
// distributed under the terms of the GNU General Public License v3 (GPL
// Version 3), copied verbatim in the file "COPYING".
//
// See http://alice-o2.web.cern.ch/license for full licensing information.
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

/// @file   PartialNoiseCalibratorSpec.cxx

#include "CCDB/CcdbApi.h"
#include "CommonUtils/StringUtils.h"
#include "DetectorsCalibration/Utils.h"
#include "MFTCalibration/PartialNoiseCalibratorSpec.h"
#include "DataFormatsITSMFT/Digit.h"
#include "DataFormatsITSMFT/CompCluster.h"
#include "DataFormatsITSMFT/ROFRecord.h"

#include "FairLogger.h"
#include "Framework/ControlService.h"
#include "Framework/ConfigParamRegistry.h"

using namespace o2::framework;
using namespace o2::utils;

namespace o2
{
namespace mft
{

PartialNoiseCalibratorSpec::PartialNoiseCalibratorSpec(bool useDigits)
  : mDigits(useDigits)
{
}

void PartialNoiseCalibratorSpec::init(InitContext& ic)
{
  auto probT = ic.options().get<float>("prob-threshold");
  LOG(INFO) << "Setting the probability threshold to " << probT;

  mCalibrator = std::make_unique<CALIBRATOR>(probT);
}

void PartialNoiseCalibratorSpec::run(ProcessingContext& pc)
{
  if (mDigits) {
    const auto digits = pc.inputs().get<gsl::span<o2::itsmft::Digit>>("digits");
    const auto rofs = pc.inputs().get<gsl::span<o2::itsmft::ROFRecord>>("digitsROF");
    const auto tfcounter = o2::header::get<o2::framework::DataProcessingHeader*>(pc.inputs().get("digitsROF").header)->startTime;

    if (mCalibrator->processTimeFrame(tfcounter, digits, rofs)) {
      LOG(INFO) << "Minimum number of noise counts has been reached !";
      sendOutput(pc.outputs());
      pc.services().get<ControlService>().readyToQuit(QuitRequest::All);
    }
  } else {
    const auto compClusters = pc.inputs().get<gsl::span<o2::itsmft::CompClusterExt>>("compClusters");
    gsl::span<const unsigned char> patterns = pc.inputs().get<gsl::span<unsigned char>>("patterns");
    const auto rofs = pc.inputs().get<gsl::span<o2::itsmft::ROFRecord>>("ROframes");
    const auto tfcounter = o2::header::get<o2::framework::DataProcessingHeader*>(pc.inputs().get("ROframes").header)->startTime;

    if (mCalibrator->processTimeFrame(tfcounter, compClusters, patterns, rofs)) {
      LOG(INFO) << "Minimum number of noise counts has been reached !";
      mCalibrator->finalize();
      mNoiseMap = mCalibrator->getNoiseMap();
      std::vector<o2::itsmft::NoiseMap> noiseVec;
      noiseVec.emplace_back(mNoiseMap);
//      sendOutput(pc.outputs());
      o2::header::DataOrigin detOrig = o2::header::gDataOriginMFT;
      pc.outputs().snapshot(Output{detOrig, "NOISEMAP", 0}, noiseVec);

      pc.services().get<ControlService>().readyToQuit(QuitRequest::All);
    }
  }
}

void PartialNoiseCalibratorSpec::sendOutput(DataAllocator& outputs)
{
//  mCalibrator->finalize();
//  mNoiseMap = mCalibrator->getNoiseMap();
//  std::vector<o2::itsmft::NoiseMap> noiseVec;
//  noiseVec.emplace_back(mNoiseMap);

//  LOG(INFO) << "Sending noise Map to merger.";

//  o2::header::DataOrigin detOrig = o2::header::gDataOriginMFT;
//  outputs().snapshot(Output{detOrig, "NOISEMAP", 0}, noiseVec);
}

void PartialNoiseCalibratorSpec::endOfStream(o2::framework::EndOfStreamContext& ec)
{
//  sendOutput(ec.outputs());
}

DataProcessorSpec getPartialNoiseCalibratorSpec(bool useDigits)
{
  o2::header::DataOrigin detOrig = o2::header::gDataOriginMFT;
  std::vector<InputSpec> inputs;
  if (useDigits) {
    inputs.emplace_back("digits", detOrig, "DIGITS", 0, Lifetime::Timeframe);
    inputs.emplace_back("digitsROF", detOrig, "DIGITSROF", 0, Lifetime::Timeframe);
  } else {
    inputs.emplace_back("compClusters", detOrig, "COMPCLUSTERS", 0, Lifetime::Timeframe);
    inputs.emplace_back("patterns", detOrig, "PATTERNS", 0, Lifetime::Timeframe);
    inputs.emplace_back("ROframes", detOrig, "CLUSTERSROF", 0, Lifetime::Timeframe);
  }

  using clbUtils = o2::calibration::Utils;
  std::vector<OutputSpec> outputs;
  outputs.emplace_back(detOrig, "NOISEMAP", 0);

  return DataProcessorSpec{
    "mft-partial-noise-calibrator",
    inputs,
    outputs,
    AlgorithmSpec{adaptFromTask<PartialNoiseCalibratorSpec>(useDigits)},
    Options{
      {"prob-threshold", VariantType::Float, 1.e-6f, {"Probability threshold for noisy pixels"}},
      {"hb-per-tf", VariantType::Int, 256, {"Number of HBF per TF"}}}};
}

} // namespace mft
} // namespace o2
