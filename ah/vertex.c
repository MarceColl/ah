#include "vertex.h"


VkVertexInputBindingDescription get_vertex_binding_description() {
    VkVertexInputBindingDescription binding_description = {};

    binding_description.binding = 0;
    binding_description.stride = sizeof(vertex_t);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return binding_description;
}

vertex_input_attribute_description_t get_vertex_attribute_descriptions() {
    vertex_input_attribute_description_t attribute_descriptions = {};
    attribute_descriptions.num_attributes = 2;

    attribute_descriptions.attr_desc[0].binding = 0;
    attribute_descriptions.attr_desc[0].location = 0;
    attribute_descriptions.attr_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_descriptions.attr_desc[0].offset = offsetof(vertex_t, pos);

    attribute_descriptions.attr_desc[1].binding = 0;
    attribute_descriptions.attr_desc[1].location = 1;
    attribute_descriptions.attr_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_descriptions.attr_desc[1].offset = offsetof(vertex_t, color);
    return attribute_descriptions;
}
