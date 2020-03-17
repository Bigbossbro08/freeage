#include "FreeAge/server/unit.hpp"

#include "FreeAge/server/building.hpp"

ServerUnit::ServerUnit(int playerIndex, UnitType type, const QPointF& mapCoord)
    : ServerObject(ObjectType::Unit, playerIndex),
      type(type),
      mapCoord(mapCoord),
      currentAction(UnitAction::Idle) {}

void ServerUnit::SetTarget(u32 targetObjectId, ServerObject* targetObject) {
  InteractionType interaction = GetInteractionType(this, targetObject);
  
  if (interaction == InteractionType::Construct) {
    type = IsMaleVillager(type) ? UnitType::MaleVillagerBuilder : UnitType::FemaleVillagerBuilder;
    SetTargetInternal(targetObjectId, targetObject);
    return;
  } else if (interaction == InteractionType::CollectBerries) {
    type = IsMaleVillager(type) ? UnitType::MaleVillagerForager : UnitType::FemaleVillagerForager;
    SetTargetInternal(targetObjectId, targetObject);
    return;
  } else if (interaction == InteractionType::CollectWood) {
    type = IsMaleVillager(type) ? UnitType::MaleVillagerLumberjack : UnitType::FemaleVillagerLumberjack;
    SetTargetInternal(targetObjectId, targetObject);
    return;
  } else if (interaction == InteractionType::CollectGold) {
    type = IsMaleVillager(type) ? UnitType::MaleVillagerGoldMiner : UnitType::FemaleVillagerGoldMiner;
    SetTargetInternal(targetObjectId, targetObject);
    return;
  } else if (interaction == InteractionType::CollectStone) {
    type = IsMaleVillager(type) ? UnitType::MaleVillagerStoneMiner : UnitType::FemaleVillagerStoneMiner;
    SetTargetInternal(targetObjectId, targetObject);
    return;
  }
}

void ServerUnit::RemoveTarget() {
  targetObjectId = kInvalidObjectId;
}

void ServerUnit::SetMoveToTarget(const QPointF& mapCoord) {
  // The path will be computed on the next game state update.
  hasPath = false;
  
  moveToTarget = mapCoord;
  hasMoveToTarget = true;
  
  targetObjectId = kInvalidObjectId;
}

void ServerUnit::SetTargetInternal(u32 targetObjectId, ServerObject* targetObject) {
  // The path will be computed on the next game state update.
  hasPath = false;
  
  // TODO: For now, we simply make the unit move to the center of the target object.
  //       Actually, it should only move such as to touch the target object in any way (for melee interactions),
  //       or move to a location from where it can reach it (for ranged attacks).
  // TODO: In addition, if the target is a unit, the commanded unit needs to update its path
  //       if the target unit moves.
  if (targetObject->isBuilding()) {
    ServerBuilding* targetBuilding = static_cast<ServerBuilding*>(targetObject);
    QSize buildingSize = GetBuildingSize(targetBuilding->GetBuildingType());
    moveToTarget = targetBuilding->GetBaseTile() + 0.5f * QPointF(buildingSize.width(), buildingSize.height());
  } else if (targetObject->isUnit()) {
    ServerUnit* targetUnit = static_cast<ServerUnit*>(targetObject);
    moveToTarget = targetUnit->GetMapCoord();
  }
  hasMoveToTarget = true;
  
  this->targetObjectId = targetObjectId;
}
