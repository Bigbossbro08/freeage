#include "FreeAge/client/building.hpp"

#include "FreeAge/common/logging.hpp"
#include "FreeAge/client/map.hpp"
#include "FreeAge/client/opengl.hpp"

bool ClientBuildingType::Load(BuildingType type, const std::filesystem::path& graphicsPath, const std::filesystem::path& cachePath, const Palettes& palettes) {
  this->type = type;
  
  if (!LoadSpriteAndTexture(
      (graphicsPath / GetFilename().toStdString()).c_str(),
      (cachePath / GetFilename().toStdString()).c_str(),
      GL_CLAMP,
      GL_NEAREST,
      GL_NEAREST,
      &sprite,
      &texture,
      &shadowTexture,
      palettes)) {
    return false;
  }
  
  maxCenterY = 0;
  for (int frame = 0; frame < sprite.NumFrames(); ++ frame) {
    maxCenterY = std::max(maxCenterY, sprite.frame(frame).graphic.centerY);
  }
  
  return true;
}

QSize ClientBuildingType::GetSize() const {
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

QString ClientBuildingType::GetFilename() const {
  // TODO: Load this from some data file?
  
  switch (type) {
  case BuildingType::TownCenter:       return QStringLiteral("b_dark_town_center_age1_x1.smx");
  case BuildingType::TownCenterBack:   return QStringLiteral("b_dark_town_center_age1_back_x1.smx");
  case BuildingType::TownCenterCenter: return QStringLiteral("b_dark_town_center_age1_center_x1.smx");
  case BuildingType::TownCenterFront:  return QStringLiteral("b_dark_town_center_age1_front_x1.smx");
  case BuildingType::TownCenterMain:   return QStringLiteral("b_dark_town_center_age1_main_x1.smx");
  case BuildingType::House:            return QStringLiteral("b_dark_house_age1_x1.smx");
  case BuildingType::TreeOak:          return QStringLiteral("n_tree_oak_x1.smx");
  case BuildingType::NumBuildings:
    LOG(ERROR) << "Invalid type given: BuildingType::NumBuildings";
    break;
  }
  
  return QString();
}

bool ClientBuildingType::UsesRandomSpriteFrame() const {
  return (static_cast<int>(type) >= static_cast<int>(BuildingType::FirstTree) &&
          static_cast<int>(type) <= static_cast<int>(BuildingType::LastTree));
}

float ClientBuildingType::GetHealthBarHeightAboveCenter(int frameIndex) const {
  constexpr float kHealthBarOffset = 25;
  
  if (UsesRandomSpriteFrame()) {
    return sprite.frame(frameIndex).graphic.centerY + kHealthBarOffset;
  } else {
    return maxCenterY + kHealthBarOffset;
  }
}


ClientBuilding::ClientBuilding(int playerIndex, BuildingType type, int baseTileX, int baseTileY)
    : playerIndex(playerIndex),
      type(type),
      isSelected(false),
      fixedFrameIndex(-1),
      baseTileX(baseTileX),
      baseTileY(baseTileY) {}

QPointF ClientBuilding::GetCenterProjectedCoord(
    Map* map,
    const std::vector<ClientBuildingType>& buildingTypes) {
  const ClientBuildingType& buildingType = buildingTypes[static_cast<int>(type)];
  
  QSize size = buildingType.GetSize();
  QPointF centerMapCoord = QPointF(baseTileX + 0.5f * size.width(), baseTileY + 0.5f * size.height());
  return map->MapCoordToProjectedCoord(centerMapCoord);
}

QRectF ClientBuilding::GetRectInProjectedCoords(
    Map* map,
    const std::vector<ClientBuildingType>& buildingTypes,
    double elapsedSeconds,
    bool shadow,
    bool outline) {
  const ClientBuildingType& buildingType = buildingTypes[static_cast<int>(type)];
  
  const Sprite& sprite = buildingType.GetSprite();
  QPointF centerProjectedCoord = GetCenterProjectedCoord(map, buildingTypes);
  int frameIndex = GetFrameIndex(buildingType, elapsedSeconds);
  
  const Sprite::Frame::Layer& layer = shadow ? sprite.frame(frameIndex).shadow : sprite.frame(frameIndex).graphic;
  bool isGraphic = !shadow && !outline;
  return QRectF(
      centerProjectedCoord.x() - layer.centerX + (isGraphic ? 1 : 0),
      centerProjectedCoord.y() - layer.centerY + (isGraphic ? 1 : 0),
      layer.imageWidth + (isGraphic ? -2 : 0),
      layer.imageHeight + (isGraphic ? -2 : 0));
}

void ClientBuilding::Render(
    Map* map,
    const std::vector<ClientBuildingType>& buildingTypes,
    const std::vector<QRgb>& playerColors,
    SpriteShader* spriteShader,
    GLuint pointBuffer,
    float* viewMatrix,
    float zoom,
    int widgetWidth,
    int widgetHeight,
    double elapsedSeconds,
    bool shadow,
    bool outline) {
  const ClientBuildingType& buildingType = buildingTypes[static_cast<int>(type)];
  
  const Texture& texture = shadow ? buildingType.GetShadowTexture() : buildingType.GetTexture();
  QPointF centerProjectedCoord = GetCenterProjectedCoord(map, buildingTypes);
  int frameIndex = GetFrameIndex(buildingType, elapsedSeconds);
  
  if (type == BuildingType::TownCenter) {
    // Special case for town centers: Render all of their separate parts.
    // Main
    const ClientBuildingType& helperType1 = buildingTypes[static_cast<int>(BuildingType::TownCenterMain)];
    DrawSprite(
        helperType1.GetSprite(),
        shadow ? helperType1.GetShadowTexture() : helperType1.GetTexture(),
        spriteShader, centerProjectedCoord, pointBuffer,
        viewMatrix, zoom, widgetWidth, widgetHeight, frameIndex, shadow, outline,
        playerColors, playerIndex);
    
    // Back
    const ClientBuildingType& helperType2 = buildingTypes[static_cast<int>(BuildingType::TownCenterBack)];
    DrawSprite(
        helperType2.GetSprite(),
        shadow ? helperType2.GetShadowTexture() : helperType2.GetTexture(),
        spriteShader, centerProjectedCoord, pointBuffer,
        viewMatrix, zoom, widgetWidth, widgetHeight, frameIndex, shadow, outline,
        playerColors, playerIndex);
    
    // Center
    const ClientBuildingType& helperType3 = buildingTypes[static_cast<int>(BuildingType::TownCenterCenter)];
    DrawSprite(
        helperType3.GetSprite(),
        shadow ? helperType3.GetShadowTexture() : helperType3.GetTexture(),
        spriteShader, centerProjectedCoord, pointBuffer,
        viewMatrix, zoom, widgetWidth, widgetHeight, frameIndex, shadow, outline,
        playerColors, playerIndex);
  }
  
  DrawSprite(
      buildingType.GetSprite(),
      texture,
      spriteShader,
      centerProjectedCoord,
      pointBuffer,
      viewMatrix,
      zoom,
      widgetWidth,
      widgetHeight,
      frameIndex,
      shadow,
      outline,
      playerColors,
      playerIndex);
  
  if (type == BuildingType::TownCenter) {
    // Front
    const ClientBuildingType& helperType4 = buildingTypes[static_cast<int>(BuildingType::TownCenterFront)];
    DrawSprite(
        helperType4.GetSprite(),
        shadow ? helperType4.GetShadowTexture() : helperType4.GetTexture(),
        spriteShader, centerProjectedCoord, pointBuffer,
        viewMatrix, zoom, widgetWidth, widgetHeight, frameIndex, shadow, outline,
        playerColors, playerIndex);
  }
}

int ClientBuilding::GetFrameIndex(
    const ClientBuildingType& buildingType,
    double elapsedSeconds) {
  if (buildingType.UsesRandomSpriteFrame()) {
    if (fixedFrameIndex < 0) {
      fixedFrameIndex = rand() % buildingType.GetSprite().NumFrames();
    }
    return fixedFrameIndex;
  } else {
    float framesPerSecond = 30.f;
    return static_cast<int>(framesPerSecond * elapsedSeconds + 0.5f) % buildingType.GetSprite().NumFrames();
  }
}