#pragma once

#include <core/common.h>
#include <core/scene.h>

NORI_NAMESPACE_BEGIN

void render(Scene *scene, const std::string &filename, int samplingBudget);

NORI_NAMESPACE_END