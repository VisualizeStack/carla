// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Iterator.h"
#include "carla/ListView.h"
#include "carla/NonCopyable.h"
#include "carla/road/InformationSet.h"
#include "carla/road/Junction.h"
#include "carla/road/LaneSection.h"
#include "carla/road/LaneSectionMap.h"
#include "carla/road/RoadElementSet.h"
#include "carla/road/RoadTypes.h"
#include "carla/road/element/Geometry.h"
#include "carla/road/element/RoadInfo.h"
#include "carla/road/signal/Signal.h"
#include "carla/road/signal/SignalReference.h"

#include <unordered_map>
#include <vector>

namespace carla {
namespace road {

  class MapData;
  class Elevation;
  class MapBuilder;

  class Road : private MovableNonCopyable {
  public:

    Road() = default;
    /// @todo: remove the move constructors
    Road(Road&&) = default;
    Road &operator=(Road&&) = default;

    const MapData *GetMap() const;

    RoadId GetId() const;

    std::string GetName() const;

    double GetLength() const;

    bool IsJunction() const;

    JuncId GetJunction() const;

    Lane &GetLaneByDistance(double s, LaneId lane_id);

    const Lane &GetLaneByDistance(double s, LaneId lane_id) const;

    RoadId GetSuccessor() const;

    RoadId GetPredecessor() const;

    Lane &GetLaneById(SectionId section_id, LaneId lane_id);

    const Lane &GetLaneById(SectionId section_id, LaneId lane_id) const;

    Lane *GetNextLane(const double s, const LaneId lane_id);

    Lane *GetPrevLane(const double s, const LaneId lane_id);

    // get the start and end section with a lan id
    LaneSection *GetStartSection(LaneId id);
    LaneSection *GetEndSection(LaneId id);

    std::vector<Road *> GetNexts() const;

    std::vector<Road *> GetPrevs() const;

    const geom::CubicPolynomial &GetElevationOn(const double s) const;

    carla::road::signal::Signal* GetSignal(const SignId id);

    carla::road::signal::SignalReference* GetSignalRef(const SignRefId id);

    std::unordered_map<SignId, signal::Signal>* getSignals();

    std::unordered_map<SignId, signal::SignalReference>* getSignalReferences();

    /// Returns a directed point on the center of the road (lane 0),
    /// with the corresponding laneOffset and elevation records applied,
    /// on distance "s".
    /// - @ param s distance regarding the road to compute the point
    element::DirectedPoint GetDirectedPointIn (const double s) const;

    /// Returns a pair containing:
    /// - @b first:  distance to the nearest point on the center in
    ///              this road segment from the begining of it (s).
    /// - @b second: Euclidean distance from the nearest point in
    ///              this road segment to p.
    ///   @param loc point to calculate the distance
    const std::pair<double, double> GetNearestPoint(
        const geom::Location &loc) const;

    /// Returns a pointer to the nearest lane, given s relative to Road and
    /// a location
    ///   @param dist distance from the begining of the road to the point you
    ///          want to calculate the distance
    ///   @param loc point to calculate the distance
    const std::pair<const Lane *, double> GetNearestLane(
        const double s,
        const geom::Location &loc,
        uint32_t type = static_cast<uint32_t>(Lane::LaneType::Any)) const;

    template <typename T>
    const T *GetInfo (const double s) const {
      return _info.GetInfo<T>(s);
    }

    auto GetLaneSections() const {
      return MakeListView(
          iterator::make_map_values_const_iterator(_lane_sections.begin()),
          iterator::make_map_values_const_iterator(_lane_sections.end()));
    }

  private:

    template <typename MultiMapT>
    static auto GetLessEqualRange(MultiMapT &map, double s) {
      if (map.find(s) == map.end()) {
        auto it = map.lower_bound(s);
        if (it == map.begin()) {
          return std::make_pair(map.end(), map.end());
        }
        s = (--it)->first;
      }
      return std::make_pair(map.lower_bound(s), map.upper_bound(s));
    }

  public:

    auto GetLaneSectionsAt(const double s) {
      auto pair = GetLessEqualRange(_lane_sections, s);
      return MakeListView(
          iterator::make_map_values_iterator(pair.first),
          iterator::make_map_values_iterator(pair.second));
    }

    auto GetLaneSectionsAt(const double s) const {
      auto pair = GetLessEqualRange(_lane_sections, s);
      return MakeListView(
          iterator::make_map_values_const_iterator(pair.first),
          iterator::make_map_values_const_iterator(pair.second));
    }

    LaneSection &GetLaneSectionById(SectionId id) {
      return _lane_sections.GetById(id);
    }

    const LaneSection &GetLaneSectionById(SectionId id) const {
      return _lane_sections.GetById(id);
    }

    /// @todo Give a better name to this function.
    double UpperBound(double s) const {
      /// @todo can this fail with multiple lane sections sharing s?
      auto it = _lane_sections.upper_bound(s);
      return it != _lane_sections.end() ? it->first : _length;
    }

    /// Get all lanes at a given s
    std::map<LaneId, const Lane *> GetLanesAt(const double s) const;

  private:

    friend MapBuilder;

    MapData *_map_data { nullptr };

    RoadId _id { 0 };

    std::string _name;

    double _length { 0.0 };

    bool _is_junction { false };

    JuncId _junction_id { -1 };

    LaneSectionMap _lane_sections;

    RoadId _successor { 0 };

    RoadId _predecessor { 0 };

    // std::multimap<double, LaneSection> _lane_sections;

    InformationSet _info;

    std::vector<Road *> _nexts;

    std::vector<Road *> _prevs;

    std::unordered_map<SignId, signal::Signal> _signals;

    std::unordered_map<SignRefId, signal::SignalReference> _sign_ref;
  };

} // road
} // carla