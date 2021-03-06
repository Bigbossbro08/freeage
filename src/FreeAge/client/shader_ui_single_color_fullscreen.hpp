// Copyright 2020 The FreeAge authors
// This file is part of FreeAge, licensed under the new BSD license.
// See the COPYING file in the project root for the license text.

#pragma once

#include <memory>

#include <QOpenGLFunctions_3_2_Core>

#include "FreeAge/client/shader_program.hpp"
#include "FreeAge/client/texture.hpp"

/// Shader for rendering user interface (UI) elements.
class UISingleColorFullscreenShader {
 public:
  UISingleColorFullscreenShader();
  ~UISingleColorFullscreenShader();
  
  inline ShaderProgram* GetProgram() { return program.get(); }
  
  inline GLint GetColorLocation() const { return color_location; }
  
 private:
  std::shared_ptr<ShaderProgram> program;
  
  GLint color_location;
};
