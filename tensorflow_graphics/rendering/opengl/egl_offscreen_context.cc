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
#include "third_party/py/tensorflow_graphics/rendering/opengl/egl_offscreen_context.h"

#include <EGL/egl.h>

#include "third_party/GL/util/egl_util.h"
#include "third_party/py/tensorflow_graphics/rendering/opengl/gl_macros.h"

EGLOffscreenContext::EGLOffscreenContext()
    : context_(EGL_NO_CONTEXT),
      display_(EGL_NO_DISPLAY),
      pixel_buffer_surface_(EGL_NO_SURFACE) {}

bool EGLOffscreenContext::Create(const int pixel_buffer_width,
                                 const int pixel_buffer_height,
                                 const EGLenum rendering_api,
                                 const EGLint* configuration_attributes,
                                 const EGLint* context_attributes) {
  EGLBoolean success;

  // Create an EGL display at device index 0.
  display_ = CreateInitializedEGLDisplay();
  if (display_ == EGL_NO_DISPLAY) return false;

  // Initialize an EGL display connection.
  RETURN_FALSE_IF_EGL_ERROR(success =
                                eglInitialize(display_, nullptr, nullptr));
  if (success == false) return false;

  // Set the current rendering API.
  RETURN_FALSE_IF_EGL_ERROR(success = eglBindAPI(rendering_api));
  if (success == false) return false;

  // Build a frame buffer configuration.
  EGLConfig frame_buffer_configuration;
  EGLint returned_num_configs;
  const EGLint requested_num_configs = 1;

  RETURN_FALSE_IF_EGL_ERROR(
      success = eglChooseConfig(display_, configuration_attributes,
                                &frame_buffer_configuration,
                                requested_num_configs, &returned_num_configs));
  if (returned_num_configs != requested_num_configs || !success) return false;

  // Create a pixel buffer surface.
  EGLint pixel_buffer_attributes[] = {
      EGL_WIDTH, pixel_buffer_width, EGL_HEIGHT, pixel_buffer_height, EGL_NONE,
  };

  RETURN_FALSE_IF_EGL_ERROR(
      pixel_buffer_surface_ = eglCreatePbufferSurface(
          display_, frame_buffer_configuration, pixel_buffer_attributes));
  if (pixel_buffer_surface_ == EGL_NO_SURFACE) return false;

  // Create the EGL rendering context.
  RETURN_FALSE_IF_EGL_ERROR(
      context_ = eglCreateContext(display_, frame_buffer_configuration,
                                  EGL_NO_CONTEXT, context_attributes));
  if (context_ == EGL_NO_CONTEXT) return false;
  return true;
}

EGLOffscreenContext::~EGLOffscreenContext() {
  EGLBoolean success;

  if (display_ != EGL_NO_DISPLAY && context_ != EGL_NO_CONTEXT) {
    success = eglDestroyContext(display_, context_);
    if (success == false)
      std::cerr
          << "EGLOffscreenContext::~EGLOffscreenContext: an error occured "
             "in eglDestroyContext."
          << std::endl;
  }

  if (display_ != EGL_NO_DISPLAY && pixel_buffer_surface_ != EGL_NO_SURFACE) {
    success = eglDestroySurface(display_, pixel_buffer_surface_);
    if (success == false)
      std::cerr
          << "EGLOffscreenContext::~EGLOffscreenContext: an error occured "
             "in eglDestroySurface."
          << std::endl;
  }

  if (display_ != EGL_NO_DISPLAY) {
    success = TerminateInitializedEGLDisplay(display_);
    if (success == false)
      std::cerr
          << "EGLOffscreenContext::~EGLOffscreenContext: an error occured "
             "in TerminateInitializedEGLDisplay."
          << std::endl;
  }
}

bool EGLOffscreenContext::MakeCurrent() const {
  RETURN_FALSE_IF_EGL_ERROR(eglMakeCurrent(display_, pixel_buffer_surface_,
                                           pixel_buffer_surface_, context_));
  return true;
}

bool EGLOffscreenContext::Release() {
  if (context_ != EGL_NO_CONTEXT && context_ == eglGetCurrentContext())
    RETURN_FALSE_IF_EGL_ERROR(eglMakeCurrent(display_, EGL_NO_SURFACE,
                                             EGL_NO_SURFACE, EGL_NO_CONTEXT));
  return true;
}
