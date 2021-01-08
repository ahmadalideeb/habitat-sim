// Copyright (c) Facebook, Inc. and its affiliates.
// This source code is licensed under the MIT license found in the
// LICENSE file in the root directory of this source tree.

#include "MeshVisualizerDrawable.h"
#include "Magnum/GL/Renderer.h"
#include "esp/scene/SceneNode.h"

namespace Mn = Magnum;

namespace esp {
namespace gfx {

MeshVisualizerDrawable::MeshVisualizerDrawable(
    scene::SceneNode& node,
    Magnum::Shaders::MeshVisualizer3D& shader,
    Magnum::GL::Mesh& mesh,
    DrawableGroup* group)
    : Drawable{node, mesh, group}, shader_(shader) {}

void MeshVisualizerDrawable::draw(const Magnum::Matrix4& transformationMatrix,
                                  Magnum::SceneGraph::Camera3D& camera) {
  Cr::Utility::Debug{} << "drawing MeshVisualizerDrawable";

  Mn::GL::Renderer::enable(Mn::GL::Renderer::Feature::Blending);
  Mn::GL::Renderer::enable(Mn::GL::Renderer::Feature::PolygonOffsetFill);
  Mn::GL::Renderer::setPolygonOffset(-5.0f, -5.0f);

  shader_.setViewportSize(Mn::Vector2{camera.viewport()});
  shader_.setProjectionMatrix(camera.projectionMatrix())
      .setTransformationMatrix(transformationMatrix);

  shader_.draw(mesh_);

  Mn::GL::Renderer::setPolygonOffset(0.0f, 0.0f);
  Mn::GL::Renderer::disable(Mn::GL::Renderer::Feature::PolygonOffsetFill);
  Mn::GL::Renderer::disable(Mn::GL::Renderer::Feature::Blending);
}
}  // namespace gfx
}  // namespace esp
