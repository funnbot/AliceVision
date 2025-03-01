// This file is part of the AliceVision project.
// Copyright (c) 2018 AliceVision contributors.
// This Source Code Form is subject to the terms of the Mozilla Public License,
// v. 2.0. If a copy of the MPL was not distributed with this file,
// You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <aliceVision/types.hpp>
#include <aliceVision/geometry/Pose3.hpp>

namespace aliceVision {
namespace sfmData {

class CameraPose
{
  public:
    /**
     * @brief CameraPose default constructor
     */
    CameraPose() = default;

    /**
     * @brief CameraPose constructor
     * @param[in] transform The camera initial 3d transformation
     * @param[in] locked If enable the camera pose is locked
     */
    explicit CameraPose(const geometry::Pose3& transform, bool locked = false)
      : _transform(transform),
        _locked(locked)
    {}

    /**
     * @brief Get the 3d transformation of the camera
     * @return 3d transformation
     */
    inline const geometry::Pose3& getTransform() const { return _transform; }

    /**
     * @brief Get the lock state of the camera
     * @return true if the camera pose is locked
     */
    inline bool isLocked() const { return _locked; }

    /**
     * @brief operator ==
     */
    inline bool operator==(const CameraPose& other) const { return (_transform == other._transform && _locked == other._locked); }

    /**
     * @brief Set the 3d transformation of the camera
     * @param[in] 3d transformation
     */
    inline void setTransform(const geometry::Pose3& transform) { _transform = transform; }

    /**
     * @brief lock the camera pose
     */
    inline void lock() { _locked = true; }

    /**
     * @brief unlock the camera pose
     */
    inline void unlock() { _locked = false; }

    void initializeState()
    {
        if (_locked)
        {
            _state = EEstimatorParameterState::CONSTANT;
        }
        else
        {
            _state = EEstimatorParameterState::REFINED;
        }
    }

    EEstimatorParameterState getState() const { return _state; }

    void setState(EEstimatorParameterState state) { _state = state; }

    bool isRotationOnly() const
    {
        return _rotationOnly;
    }   

    /**
     * Set the flag for partial state
     * Partial flag set to on means the camera translation is not known
    */
    void setRotationOnly(bool partial)
    {
        _rotationOnly = partial;
    }

  private:
    /// camera 3d transformation
    geometry::Pose3 _transform;
    /// camera lock
    bool _locked = false;
    /// Only rotation is solved
    bool _rotationOnly = false;
    /// Estimator state
    EEstimatorParameterState _state = EEstimatorParameterState::REFINED;
};

}  // namespace sfmData
}  // namespace aliceVision
