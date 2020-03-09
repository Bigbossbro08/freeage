#include "FreeAge/common/building_types.hpp"

#include "FreeAge/common/logging.hpp"

QSize GetBuildingSize(BuildingType type) {
  // TODO: Load this from some data file?
  
  if (static_cast<int>(type) >= static_cast<int>(BuildingType::FirstTree) &&
      static_cast<int>(type) <= static_cast<int>(BuildingType::LastTree)) {
    return QSize(1, 1);
  }
  
  switch (type) {
  case BuildingType::TownCenter:
    return QSize(4, 4);
  case BuildingType::House:
    return QSize(2, 2);
  default:
    LOG(ERROR) << "Invalid type given: " << static_cast<int>(type);
    break;
  }
  
  return QSize(0, 0);
}

QRect GetBuildingOccupancy(BuildingType type) {
  if (type == BuildingType::TownCenter) {
    return QRect(0, 2, 2, 2);
  } else {
    QSize size = GetBuildingSize(type);
    return QRect(0, 0, size.width(), size.height());
  }
}