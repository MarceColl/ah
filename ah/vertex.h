#pragma once

#include "ah.h"
#include <cglm/cglm.h>
#include <vulkan/vulkan_core.h>

typedef struct vertex {
    vec2 pos;
    vec3 color;
} vertex_t;

typedef struct vertex_input_attribute_description {
    uint32_t num_attributes;
    VkVertexInputAttributeDescription attr_desc[6];
} vertex_input_attribute_description_t;

VkVertexInputBindingDescription get_vertex_binding_description();
vertex_input_attribute_description_t get_vertex_attribute_descriptions();
