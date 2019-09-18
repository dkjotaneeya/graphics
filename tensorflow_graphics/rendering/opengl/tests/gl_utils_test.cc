#Copyright 2018 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#include "third_party/py/tensorflow_graphics/rendering/opengl/gl_utils.h"

#include <memory>
#include <string>

#include "testing/base/public/gmock.h"
#include "testing/base/public/gunit.h"
#include "third_party/GL/gl/include/GLES3/gl32.h"
#include "third_party/py/tensorflow_graphics/rendering/opengl/egl_offscreen_context.h"

namespace {

const std::string kEmptyShaderCode =
    "#version 430\n"
    "void main() { }\n";

TEST(GLUtilsTest, TestCompileInvalidShader) {
  EGLOffscreenContext context;
  std::unique_ptr<gl_utils::Program> program;
  const std::string kInvalidShaderCode =
      "#version 430\n"
      "void main() { syntax_error }\n";

  EXPECT_TRUE(context.Create(1, 1));
  EXPECT_TRUE(context.MakeCurrent());

  std::vector<std::pair<std::string, GLenum>> shaders;
  shaders.push_back(
      std::pair<std::string, GLenum>(kInvalidShaderCode, GL_VERTEX_SHADER));
  EXPECT_FALSE(gl_utils::Program::Create(shaders, &program));
  EXPECT_TRUE(context.Release());
}

TEST(GLUtilsTest, TestCreateProgram) {
  EGLOffscreenContext context;
  std::unique_ptr<gl_utils::Program> program;

  EXPECT_TRUE(context.Create(1, 1));
  EXPECT_TRUE(context.MakeCurrent());

  std::vector<std::pair<std::string, GLenum>> shaders;
  shaders.push_back(
      std::pair<std::string, GLenum>(kEmptyShaderCode, GL_VERTEX_SHADER));
  EXPECT_TRUE(gl_utils::Program::Create(shaders, &program));
  EXPECT_NE(0, program->GetHandle());
  EXPECT_TRUE(context.Release());
}

template <typename T>
class RenderTargetsInterfaceTest : public ::testing::Test {};
using valid_render_target_types = ::testing::Types<float, uint8>;
TYPED_TEST_CASE(RenderTargetsInterfaceTest, valid_render_target_types);

TYPED_TEST(RenderTargetsInterfaceTest, TestRenderClear) {
  EGLOffscreenContext context;
  const float kRed = 0.1;
  const float kGreen = 0.2;
  const float kBlue = 0.3;
  const float kAlpha = 1.0;
  const int kWidth = 10;
  const int kHeight = 5;
  float max_gl_value = 255.0f;

  if (typeid(TypeParam) == typeid(float)) max_gl_value = 1.0f;

  EXPECT_TRUE(context.Create(kWidth, kHeight));
  EXPECT_TRUE(context.MakeCurrent());

  std::unique_ptr<gl_utils::RenderTargets<TypeParam>> render_targets;
  EXPECT_TRUE(gl_utils::RenderTargets<TypeParam>::Create(kWidth, kHeight,
                                                         &render_targets));
  glClearColor(kRed, kGreen, kBlue, kAlpha);
  glClear(GL_COLOR_BUFFER_BIT);
  ASSERT_EQ(glGetError(), GL_NO_ERROR);
  std::vector<TypeParam> pixels(kWidth * kHeight * 4);
  EXPECT_TRUE(render_targets->ReadPixels(&pixels));

  for (int index = 0; index < kWidth * kHeight; ++index) {
    ASSERT_EQ(pixels[index * 4], TypeParam(kRed * max_gl_value));
    ASSERT_EQ(pixels[index * 4 + 1], TypeParam(kGreen * max_gl_value));
    ASSERT_EQ(pixels[index * 4 + 2], TypeParam(kBlue * max_gl_value));
    ASSERT_EQ(pixels[index * 4 + 3], TypeParam(kAlpha * max_gl_value));
  }
  EXPECT_TRUE(context.Release());
}

}  // namespace
