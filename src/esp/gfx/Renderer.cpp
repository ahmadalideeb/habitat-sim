// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include "Renderer.h"

#include <Corrade/Containers/StridedArrayView.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Framebuffer.h>
#include <Magnum/GL/PixelFormat.h>
#include <Magnum/GL/Renderbuffer.h>
#include <Magnum/GL/RenderbufferFormat.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Image.h>
#include <Magnum/PixelFormat.h>

#include "esp/gfx/DepthUnprojection.h"
#include "esp/gfx/RenderTarget.h"
#include "esp/gfx/magnum.h"
#include "esp/sensor/VisualSensor.h"

namespace Mn = Magnum;

namespace esp {
namespace gfx {

struct Renderer::Impl {
  explicit Impl(Flags flags) : depthShader_{nullptr}, flags_{flags} {
    Mn::GL::Renderer::enable(Mn::GL::Renderer::Feature::DepthTest);
    Mn::GL::Renderer::enable(Mn::GL::Renderer::Feature::FaceCulling);
  }
  ~Impl() { LOG(INFO) << "Deconstructing Renderer"; }

  void draw(RenderCamera& camera,
            scene::SceneGraph& sceneGraph,
            RenderCamera::Flags flags) {
    for (auto& it : sceneGraph.getDrawableGroups()) {
      // TODO: remove || true
      if (it.second.prepareForDraw(camera) || true) {
        camera.draw(it.second, flags);
      }
    }
  }

  void draw(sensor::VisualSensor& visualSensor,
            scene::SceneGraph& sceneGraph,
            RenderCamera::Flags flags) {
    draw(*visualSensor.getRenderCamera(), sceneGraph, flags);
  }

  void bindRenderTarget(sensor::VisualSensor& sensor) {
    auto depthUnprojection = sensor.depthUnprojection();
    if (!depthUnprojection) {
      throw std::runtime_error(
          "Sensor does not have a depthUnprojection matrix");
    }

    if (!depthShader_) {
      depthShader_ = std::make_unique<DepthShader>(
          DepthShader::Flag::UnprojectExistingDepth);
    }

    RenderTarget::Flags renderTargetFlags_ = {};
    if (flags_ & Flag::NoTextures) {
      renderTargetFlags_ |= RenderTarget::Flag::NoTextures;
    }

    switch (sensor.specification()->sensorType) {
      case sensor::SensorType::Color:
        renderTargetFlags_ |= (RenderTarget::Flag::NoObjectIdBuffer |
                               RenderTarget::Flag::NoDepthTexture);
        break;

      case sensor::SensorType::Depth:
        renderTargetFlags_ |= (RenderTarget::Flag::NoRgbaBuffer |
                               RenderTarget::Flag::NoObjectIdBuffer);
        break;

      case sensor::SensorType::Semantic:
        renderTargetFlags_ |= (RenderTarget::Flag::NoRgbaBuffer |
                               RenderTarget::Flag::NoDepthTexture);
        break;

      default:
        // I need this default, since sensor type list is long, and without
        // default clang-tidy will complain
        break;
    }

    sensor.bindRenderTarget(RenderTarget::create_unique(
        sensor.framebufferSize(), *depthUnprojection, depthShader_.get(),
        renderTargetFlags_));
  }

 private:
  std::unique_ptr<DepthShader> depthShader_;
  const Flags flags_;
};

Renderer::Renderer(Flags flags)
    : pimpl_(spimpl::make_unique_impl<Impl>(flags)) {}

void Renderer::draw(RenderCamera& camera,
                    scene::SceneGraph& sceneGraph,
                    RenderCamera::Flags flags) {
  pimpl_->draw(camera, sceneGraph, flags);
}

void Renderer::draw(sensor::VisualSensor& visualSensor,
                    scene::SceneGraph& sceneGraph,
                    RenderCamera::Flags flags) {
  pimpl_->draw(visualSensor, sceneGraph, flags);
}

void Renderer::bindRenderTarget(sensor::VisualSensor& sensor) {
  pimpl_->bindRenderTarget(sensor);
}

}  // namespace gfx
}  // namespace esp
