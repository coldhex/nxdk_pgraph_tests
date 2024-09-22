#include "front_face_tests.h"

#include <pbkit/pbkit.h>

#include "../test_host.h"
#include "debug_output.h"
#include "shaders/precalculated_vertex_shader.h"
#include "vertex_buffer.h"

static std::string WindingName(uint32_t winding);
static std::string CullFaceName(uint32_t cull_face);

static constexpr uint32_t kWindings[] = {
    NV097_SET_FRONT_FACE_V_CW, NV097_SET_FRONT_FACE_V_CCW,
    0,   // https://github.com/mborgerson/xemu/issues/321
    99,  // Random value to verify that HW behavior is to map all unknowns to CCW.
};

static constexpr uint32_t kCullFaces[] = {
    NV097_SET_CULL_FACE_V_FRONT,
    NV097_SET_CULL_FACE_V_BACK,
    NV097_SET_CULL_FACE_V_FRONT_AND_BACK,
};

FrontFaceTests::FrontFaceTests(TestHost& host, std::string output_dir)
    : TestSuite(host, std::move(output_dir), "Front face") {
  for (auto line_mode : {false, true}) {
    for (auto winding : kWindings) {
      for (auto cull_face : kCullFaces) {
	std::string name = MakeTestName(winding, cull_face, line_mode);

	auto test = [this, winding, cull_face, line_mode]() { this->Test(winding, cull_face, line_mode); };
	tests_[name] = test;
      }
    }
  }
}

void FrontFaceTests::Initialize() {
  TestSuite::Initialize();

  {
    auto p = pb_begin();
    p = pb_push1(p, NV20_TCL_PRIMITIVE_3D_CULL_FACE_ENABLE, true);
    pb_end(p);
  }

  auto shader = std::make_shared<PrecalculatedVertexShader>();
  host_.SetVertexShaderProgram(shader);

  CreateGeometry();
}

void FrontFaceTests::CreateGeometry() {
  auto fb_width = static_cast<float>(host_.GetFramebufferWidth());
  auto fb_height = static_cast<float>(host_.GetFramebufferHeight());

  float left = floorf(fb_width / 5.0f);
  float right = left + (fb_width - left * 2.0f);
  float top = floorf(fb_height / 12.0f);
  float bottom = top + (fb_height - top * 2.0f);
  float mid_width = left + (right - left) * 0.5f;

  uint32_t num_quads = 2;
  uint32_t num_tris = 2;
  std::shared_ptr<VertexBuffer> buffer = host_.AllocateVertexBuffer(6 * num_quads + 3 * num_tris);
  buffer->SetPositionIncludesW();

  Color ul{1.0, 0.0, 0.0, 1.0};
  Color ll{0.0, 1.0, 0.0, 1.0};
  Color lr{0.0, 0.0, 1.0, 1.0};
  Color ur{0.5, 0.5, 0.5, 1.0};

  float z = 10.0f;
  buffer->DefineBiTriCCW(0, left + 10, top + 4, mid_width - 10, bottom - 10, z, z, z, z, ul, ll, lr, ur);
  float tri_one[3] = {left + 5, top + 4, z};
  float tri_two[3] = {left + 5, bottom - 10, z};
  float tri_three[3] = {left + 5, (top + bottom - 6) / 2, z};
  float normal[3] = {0.0f, 0.0f, 1.0f};
  buffer->DefineTriangleCCW(4, tri_one, tri_two, tri_three, normal, normal, normal, ul, ll, lr);

  buffer->SetPositionIncludesW();
  auto vtx = buffer->Lock();
  vtx[0].pos[3] = INFINITY;
  vtx[1].pos[3] = 0.980578f;
  vtx[2].pos[3] = 0.0f;
  vtx[3].pos[3] = INFINITY;
  vtx[4].pos[3] = INFINITY;
  vtx[5].pos[3] = INFINITY;
  vtx[12].pos[3] = 1.0f;
  vtx[13].pos[3] = 1.0f;
  vtx[14].pos[3] = 1.0f;
  vtx[15].pos[3] = 1.0f;
  vtx[16].pos[3] = 1.0f;
  vtx[17].pos[3] = 1.0f;
  buffer->Unlock();

  Color tmp = ul;
  ul = lr;
  lr = tmp;

  tmp = ur;
  ur = ll;
  ll = tmp;

  buffer->DefineBiTri(1, mid_width + 10, top + 4, right - 10, bottom - 10, z, z, z, z, ul, ll, lr, ur);
  float tri2_one[3] = {right - 5, top + 4, z};
  float tri2_two[3] = {right - 5, bottom - 10, z};
  float tri2_three[3] = {right - 5, (top + bottom - 6) / 2, z};
  buffer->DefineTriangle(5, tri2_one, tri2_two, tri2_three, normal, normal, normal, ul, ll, lr);
}

void FrontFaceTests::Test(uint32_t front_face, uint32_t cull_face, bool line_mode) {
  host_.PrepareDraw();

  // To verify that the HW is simply preserving a previously set value, force it to a known valid, but different value
  // before setting the value under test.
  // Note that the setup steps done by host_ will set the front face to CCW, so CW is preferred to differentiate the
  // behavior from simply running tests in sequence.
  if (front_face != NV097_SET_FRONT_FACE_V_CW) {
    auto p = pb_begin();
    p = pb_push1(p, NV097_SET_FRONT_FACE, NV097_SET_FRONT_FACE_V_CW);
    pb_end(p);
  } else {
    auto p = pb_begin();
    p = pb_push1(p, NV097_SET_FRONT_FACE, NV097_SET_FRONT_FACE_V_CCW);
    pb_end(p);
  }

  while (pb_busy()) {
  }

  auto p = pb_begin();
  p = pb_push1(p, NV097_SET_FRONT_FACE, front_face);
  p = pb_push1(p, NV097_SET_CULL_FACE, cull_face);
  if (line_mode) {
    p = pb_push1(p, NV097_SET_FRONT_POLYGON_MODE, NV097_SET_FRONT_POLYGON_MODE_V_LINE);
    p = pb_push1(p, NV097_SET_BACK_POLYGON_MODE, NV097_SET_FRONT_POLYGON_MODE_V_LINE);
  }
  pb_end(p);
  host_.DrawArrays();

  if (line_mode) {
    p = pb_begin();
    p = pb_push1(p, NV097_SET_FRONT_POLYGON_MODE, NV097_SET_FRONT_POLYGON_MODE_V_FILL);
    p = pb_push1(p, NV097_SET_BACK_POLYGON_MODE, NV097_SET_FRONT_POLYGON_MODE_V_FILL);
    pb_end(p);
  }

  pb_print(line_mode ? "Line mode\n" : "Fill mode\n");
  std::string winding_name = WindingName(front_face);
  pb_print("FF: %s\n", winding_name.c_str());
  std::string cull_face_name = CullFaceName(cull_face);
  pb_print("CF: %s\n", cull_face_name.c_str());
  pb_printat(8, 19, (char*)"CCW");
  pb_printat(8, 38, (char*)"CW");
  pb_draw_text_screen();

  std::string name = MakeTestName(front_face, cull_face, line_mode);
  host_.FinishDraw(allow_saving_, output_dir_, name);
}

std::string FrontFaceTests::MakeTestName(uint32_t front_face, uint32_t cull_face, bool line_mode) {
  std::string ret = "FrontFace_";
  ret += line_mode ? "LM_" : "FM_";
  ret += WindingName(front_face);
  ret += "_CF_";
  ret += CullFaceName(cull_face);
  return ret;
}

static std::string WindingName(uint32_t winding) {
  if (winding == NV097_SET_FRONT_FACE_V_CW) {
    return "CW";
  } else if (winding == NV097_SET_FRONT_FACE_V_CCW) {
    return "CCW";
  }

  char buf[16] = {0};
  snprintf(buf, 15, "%X", winding);
  return buf;
}

static std::string CullFaceName(uint32_t cull_face) {
  if (cull_face == NV097_SET_CULL_FACE_V_FRONT) {
    return "F";
  } else if (cull_face == NV097_SET_CULL_FACE_V_BACK) {
    return "B";
  } else if (cull_face == NV097_SET_CULL_FACE_V_FRONT_AND_BACK) {
    return "FaB";
  }

  char buf[16] = {0};
  snprintf(buf, 15, "%X", cull_face);
  return buf;
}
