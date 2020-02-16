#pragma once

#include <QRectF>
#include <QRgb>

#include "FreeAge/opengl.h"

class HealthBarShader;

void RenderHealthBar(
    const QRectF& projectedCoordsRect,
    float objectCenterProjectedCoordY,
    float fillAmount,
    const QRgb& color,
    HealthBarShader* healthBarShader,
    GLuint pointBuffer,
    float* viewMatrix,
    float zoom,
    int widgetWidth,
    int widgetHeight);
