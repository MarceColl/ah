#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <vulkan/vulkan_core.h>

#include "ah.h"
#include "vk.h"
#include "errors.h"


#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #include <glm/vec4.hpp>
// #include <glm/mat4x4.hpp>

void init_window(vulkan_state_t *vk_state) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    vk_state->window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Vulkan", NULL, NULL);
}

AH_RESULT draw_frame(vulkan_state_t *vk_state, uint32_t index) {
    vkWaitForFences(vk_state->device, 1, &vk_state->in_flight_fence, VK_TRUE, UINT64_MAX);
    vkResetFences(vk_state->device, 1, &vk_state->in_flight_fence);
    uint32_t image_index;
    vkAcquireNextImageKHR(
        vk_state->device,
        vk_state->swapchain,
        UINT64_MAX,
        vk_state->image_available_sempahore,
        VK_NULL_HANDLE,
        &image_index
    );
    vkResetCommandBuffer(vk_state->command_buffer, 0);
    ah_vk_record_command_buffer(vk_state, vk_state->command_buffer, image_index, index);
    VkSubmitInfo submit_info = {};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {vk_state->image_available_sempahore};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &vk_state->command_buffer;

    VkSemaphore signal_semaphores[] = {vk_state->render_finished_semaphore};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (vkQueueSubmit(vk_state->graphics_queue, 1, &submit_info, vk_state->in_flight_fence) != VK_SUCCESS) {
        set_error("Error submitting queue");
        return AH_FAILURE;
    }

    VkPresentInfoKHR present_info = {};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swapchains[] = {vk_state->swapchain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swapchains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = NULL;

    if (vkQueuePresentKHR(vk_state->present_queue, &present_info) != VK_SUCCESS) {
        set_error("Error presenting");
        return AH_FAILURE;
    }

    return AH_SUCCESS;
}

void main_loop(vulkan_state_t *vk_state) {
    uint32_t index = 0;
    uint32_t counter = 0;

    while(!glfwWindowShouldClose(vk_state->window)) {
        glfwPollEvents();
        if (counter > 20) {
            counter = 0;
            index += 1;
        }
        counter += 1;

        if (draw_frame(vk_state, index) != AH_SUCCESS) {
            print_error("main_loop/draw_frame");
            break;
        }
    }
}

void cleanup(vulkan_state_t *vk_state) {
    vkDestroySemaphore(vk_state->device, vk_state->render_finished_semaphore, NULL);
    vkDestroySemaphore(vk_state->device, vk_state->image_available_sempahore, NULL);
    vkDestroyFence(vk_state->device, vk_state->in_flight_fence, NULL);

    vkDestroyCommandPool(vk_state->device, vk_state->command_pool, NULL);

    for (int i = 0; i < vk_state->num_swapchain_images; i++) {
        vkDestroyFramebuffer(vk_state->device, vk_state->swapchain_framebuffers[i], NULL);
    }

    vkDestroyPipeline(vk_state->device, vk_state->pipeline, NULL);
    vkDestroyPipelineLayout(vk_state->device, vk_state->pipeline_layout, NULL);
    vkDestroyRenderPass(vk_state->device, vk_state->render_pass, NULL);

    for (int i = 0; i < vk_state->num_swapchain_images; i++) {
        vkDestroyImageView(vk_state->device, vk_state->swapchain_image_views[i], NULL);
    }

    vkDestroySwapchainKHR(vk_state->device, vk_state->swapchain, NULL);
    vkDestroyDevice(vk_state->device, NULL);
    vkDestroySurfaceKHR(vk_state->instance, vk_state->surface, NULL);
    vkDestroyInstance(vk_state->instance, NULL);
    glfwDestroyWindow(vk_state->window);
    glfwTerminate();
}

void ah_init_vulkan_state(vulkan_state_t *vk_state) {
    vk_state->window = NULL;
    vk_state->instance = VK_NULL_HANDLE;
    vk_state->physical_device = VK_NULL_HANDLE;
    vk_state->device = VK_NULL_HANDLE;
    vk_state->surface = VK_NULL_HANDLE;
}

int main() {
    vulkan_state_t vk_state;
    ah_init_vulkan_state(&vk_state);

    init_window(&vk_state);
    ah_vk_init(&vk_state);
    main_loop(&vk_state);
    cleanup(&vk_state);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

    printf("%d extensions supported\n", extensionCount);



    return 0;
}
