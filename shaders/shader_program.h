#ifndef NXDK_PGRAPH_TESTS_SHADER_PROGRAM_H
#define NXDK_PGRAPH_TESTS_SHADER_PROGRAM_H

#include <cstdint>

class ShaderProgram {
 public:
  ShaderProgram(bool enable_texture = true) : enable_texture_(enable_texture) {}

  virtual void Activate() = 0;
  virtual void PrepareDraw() = 0;

  bool RequiresTextureStage() { return enable_texture_; }

 protected:
  void LoadShaderProgram(const uint32_t *shader, uint32_t shader_size);

  bool enable_texture_;
};

#endif  // NXDK_PGRAPH_TESTS_SHADER_PROGRAM_H
