#ifndef NXDK_PGRAPH_TESTS_INF_TESTS_H
#define NXDK_PGRAPH_TESTS_INF_TESTS_H

#include "test_suite.h"

class TestHost;
class VertexBuffer;

class WParamTests : public TestSuite {
 public:
  WParamTests(TestHost &host, std::string output_dir);

  void Initialize() override;
  void Deinitialize() override;

 private:
  void CreateGeometryWGaps();
  void TestWGaps();

  void CreateGeometryPositiveWTriangleStrip();
  void TestPositiveWTriangleStrip();

  void CreateGeometryNegativeWTriangleStrip();
  void TestNegativeWTriangleStrip();

  void TestFixedFunctionZeroW(bool draw_quad);
  void TestFixedFunctionZeroInfW(bool draw_quad, float w_multiplier);
  void TestProgZeroInfW(bool draw_quad, float w_multiplier);
  void TestRccZeroInfW(float w_multiplier);

 private:
  std::shared_ptr<VertexBuffer> triangle_strip_;
  std::shared_ptr<VertexBuffer> triangles_;
};

#endif  // NXDK_PGRAPH_TESTS_INF_TESTS_H
