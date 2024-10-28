#pragma once

#include "ah.h"
#include <stdbool.h>
#include <vulkan/vulkan_core.h>

typedef struct vulkan_swapchain_support_details {
    VkSurfaceCapabilitiesKHR capabilities;
    uint32_t num_formats;
    VkSurfaceFormatKHR *formats;
    uint32_t num_present_modes;
    VkPresentModeKHR *present_modes;
} vulkan_swapchain_support_details_t;

typedef struct vulkan_queue_family_indices {
    bool has_graphics_family;
    uint32_t graphics_family;
    bool has_present_family;
    uint32_t present_family;
} vulkan_queue_family_indices_t;

typedef struct vulkan_state {
    GLFWwindow *window;
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue graphics_queue;
    VkQueue present_queue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    uint32_t num_swapchain_images;
    VkImage *swapchain_images;
    VkImageView *swapchain_image_views;
    VkFramebuffer *swapchain_framebuffers;
    VkFormat swapchain_image_format;
    VkExtent2D swapchain_extent;
    VkRenderPass render_pass;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    VkCommandPool command_pool;
    VkCommandBuffer command_buffer;
    VkBuffer vertex_buffer;
    VkDeviceMemory vertex_buffer_memory;

    VkSemaphore image_available_sempahore;
    VkSemaphore render_finished_semaphore;
    VkFence in_flight_fence;

    vulkan_queue_family_indices_t queue_family_indices;
    vulkan_swapchain_support_details_t swapchain_support;
} vulkan_state_t;

void ah_init_vulkan_state(vulkan_state_t *vk_state);
AH_RESULT ah_vk_init(vulkan_state_t *vk_state);
AH_RESULT ah_vk_create_instance(vulkan_state_t *vk_state);
AH_RESULT ah_vk_create_surface(vulkan_state_t *vk_state);
AH_RESULT ah_vk_pick_physical_device(vulkan_state_t *vk_state);
AH_RESULT ah_vk_create_logical_device(vulkan_state_t *vk_state);
AH_RESULT ah_vk_create_image_views(vulkan_state_t *vk_state);
AH_RESULT ah_vk_create_graphics_pipeline(vulkan_state_t *vk_state);
AH_RESULT ah_vk_create_swapchain(vulkan_state_t *vk_state);
AH_RESULT ah_vk_create_render_pass(vulkan_state_t *vk_state);
AH_RESULT ah_vk_create_framebuffers(vulkan_state_t *vk_state);
AH_RESULT ah_vk_create_command_pool(vulkan_state_t *vk_state);
AH_RESULT ah_vk_create_command_buffer(vulkan_state_t *vk_state);
AH_RESULT ah_vk_create_sync_objects(vulkan_state_t *vk_state);
AH_RESULT ah_vk_create_vertex_buffer(vulkan_state_t *vk_state);

AH_RESULT ah_vk_record_command_buffer(vulkan_state_t *vk_state, VkCommandBuffer command_buffer, uint32_t image_index, uint32_t index);
