#include "FreeAge/client/command_button.hpp"

#include "FreeAge/client/game_controller.hpp"
#include "FreeAge/client/unit.hpp"

void CommandButton::SetInvisible() {
  type = Type::Invisible;
}

void CommandButton::SetProduceUnit(UnitType type) {
  this->type = Type::ProduceUnit;
  unitProductionType = type;
}

void CommandButton::Render(
    float x, float y, float size, float iconInset,
    const Texture& iconOverlayNormalTexture,
    UIShader* uiShader, int widgetWidth, int widgetHeight, GLuint pointBuffer) {
  if (type == Type::Invisible) {
    return;
  } else if (type == Type::ProduceUnit) {
    RenderUIGraphic(
        x + iconInset,
        y + iconInset,
        size - 2 * iconInset,
        size - 2 * iconInset,
        *ClientUnitType::GetUnitTypes()[static_cast<int>(unitProductionType)].GetIconTexture(),
        uiShader, widgetWidth, widgetHeight, pointBuffer);
    RenderUIGraphic(
        x,
        y,
        size,
        size,
        iconOverlayNormalTexture,
        uiShader, widgetWidth, widgetHeight, pointBuffer);
  }
  
  buttonRect = QRectF(x, y, size, size);
}

void CommandButton::Pressed(const std::vector<u32>& selection, GameController* gameController) {
  if (type == Type::Invisible) {
    LOG(ERROR) << "An invisible button has been pressed.";
    return;
  } else if (type == Type::ProduceUnit) {
    gameController->ProduceUnit(selection, unitProductionType);
  }
}