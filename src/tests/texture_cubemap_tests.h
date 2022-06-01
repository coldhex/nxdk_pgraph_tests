#ifndef NXDK_PGRAPH_TESTS_TEXTURE_CUBEMAP_TESTS_H
#define NXDK_PGRAPH_TESTS_TEXTURE_CUBEMAP_TESTS_H

#include <string>

#include "test_suite.h"
#include "texture_format.h"

class TestHost;

// Tests cubemap texture behavior.
class TextureCubemapTests : public TestSuite {
 public:
  TextureCubemapTests(TestHost &host, std::string output_dir);

  void Initialize() override;

 private:
  void TestCubemap();
  void TestDotSTRCubemap(const std::string &name, uint32_t dot_rgb_mapping);

  void Draw(float x, float y, float z, float r_x, float r_y, float r_z) const;
};

#endif  // NXDK_PGRAPH_TESTS_TEXTURE_CUBEMAP_TESTS_H
