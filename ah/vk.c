#include "vk.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <threads.h>
#include <time.h>
#include <vulkan/vulkan_core.h>
#include "ah.h"
#include "errors.h"
#include "helpers.h"

const char* validation_layers[] = {
    "VK_LAYER_KHRONOS_validation"
};
const int32_t validation_layers_count = 1;

const char* extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
const int32_t extensions_count = 1;

void populate_queue_families(vulkan_state_t *vk_state);

AH_RESULT ah_vk_init(vulkan_state_t *vk_state) {
    if (ah_vk_create_instance(vk_state) != AH_SUCCESS) {
        print_error("init_vulkan/create_instance");
        return AH_FAILURE;
    }

    if (ah_vk_create_surface(vk_state) != AH_SUCCESS) {
        print_error("init_vulkan/create_surface");
        return AH_FAILURE;
    }

    if (ah_vk_pick_physical_device(vk_state) != AH_SUCCESS) {
        print_error("init_vulkan/pick_physical_device");
        return AH_FAILURE;
    }

    populate_queue_families(vk_state);

    if (ah_vk_create_logical_device(vk_state) != AH_SUCCESS) {
        print_error("init_vulkan/create_logical_device");
        return AH_FAILURE;
    }

    if (ah_vk_create_swapchain(vk_state) != AH_SUCCESS) {
        print_error("init_vulkan/create_swapchain");
        return AH_FAILURE;
    }

    if (ah_vk_create_image_views(vk_state) != AH_SUCCESS) {
        print_error("init_vulkan/create_image_views");
        return AH_FAILURE;
    }

    if (ah_vk_create_render_pass(vk_state) != AH_SUCCESS) {
        print_error("init_vulkan/create_render_pass");
        return AH_FAILURE;
    }

    if (ah_vk_create_graphics_pipeline(vk_state) != AH_SUCCESS) {
        print_error("init_vulkan/create_graphics_pipeline");
        return AH_FAILURE;
    }

    if (ah_vk_create_framebuffers(vk_state) != AH_SUCCESS) {
        print_error("init_vulkan/create_framebuffers");
        return AH_FAILURE;
    }

    if (ah_vk_create_command_pool(vk_state) != AH_SUCCESS) {
        print_error("init_vulkan/create_command_pool");
        return AH_FAILURE;
    }

    if (ah_vk_create_command_buffer(vk_state) != AH_SUCCESS) {
        print_error("init_vulkan/create_command_buffer");
        return AH_FAILURE;
    }

    if (ah_vk_create_sync_objects(vk_state) != AH_SUCCESS) {
        print_error("init_vulkan/create_sync_objects");
        return AH_FAILURE;
    }

    return AH_SUCCESS;
}

bool is_physical_device_suitable(VkPhysicalDevice device) {
    return true;
}

bool check_validation_layer_support() {
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    VkLayerProperties *layers = (VkLayerProperties*)malloc(sizeof(VkLayerProperties)*layer_count);
    vkEnumerateInstanceLayerProperties(&layer_count, layers);

    for (int i = 0; i < validation_layers_count; i++) {
        bool layer_found = false;

        for (int j = 0; j < layer_count; j++) {
            if (strcmp(validation_layers[i], layers[j].layerName)) {
                layer_found = true;
                break;
            }
        }

        if (!layer_found) {
            return false;
        }
    }

    return true;
}

void populate_queue_families(vulkan_state_t *vk_state) {
    vk_state->queue_family_indices.has_graphics_family = false;

    uint32_t queue_family_count = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(vk_state->physical_device, &queue_family_count, NULL);
    VkQueueFamilyProperties *queue_families = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queue_family_count);

    vkGetPhysicalDeviceQueueFamilyProperties(vk_state->physical_device, &queue_family_count, queue_families);

    printf("# Families: %d\n", queue_family_count);

    for (int i = 0; i < queue_family_count; i++) {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            vk_state->queue_family_indices.has_graphics_family = true;
            vk_state->queue_family_indices.graphics_family = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(vk_state->physical_device, i, vk_state->surface, &present_support);

        if (present_support) {
            vk_state->queue_family_indices.has_present_family = true;
            vk_state->queue_family_indices.present_family = i;
        }
    }
}

void populate_swapchain_support(vulkan_state_t *vk_state) {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_state->physical_device, vk_state->surface, &vk_state->swapchain_support.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(vk_state->physical_device, vk_state->surface, &vk_state->swapchain_support.num_formats, NULL);
    if (vk_state->swapchain_support.num_formats != 0) {
        vk_state->swapchain_support.formats = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR)*vk_state->swapchain_support.num_formats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(
            vk_state->physical_device,
            vk_state->surface,
            &vk_state->swapchain_support.num_formats,
            vk_state->swapchain_support.formats
        );
    }

    vkGetPhysicalDeviceSurfacePresentModesKHR(vk_state->physical_device, vk_state->surface, &vk_state->swapchain_support.num_present_modes, NULL);
    if (vk_state->swapchain_support.num_present_modes != 0) {
        vk_state->swapchain_support.present_modes = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR)*vk_state->swapchain_support.num_present_modes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            vk_state->physical_device,
            vk_state->surface,
            &vk_state->swapchain_support.num_present_modes,
            vk_state->swapchain_support.present_modes
        );
    }
}

AH_RESULT ah_vk_create_instance(vulkan_state_t *vk_state) {
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "Atom-Heart";
    app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    app_info.apiVersion = VK_API_VERSION_1_3;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    uint32_t glfw_extension_count = 0;
    const char** glfw_extensions;

    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

    create_info.enabledExtensionCount = glfw_extension_count;
    create_info.ppEnabledExtensionNames = glfw_extensions;

    if (!check_validation_layer_support()) {
        set_error("Required validation layers not found");
        return AH_FAILURE;
    }

    create_info.enabledLayerCount = validation_layers_count;
    create_info.ppEnabledLayerNames = validation_layers;

    if (vkCreateInstance(&create_info, NULL, &vk_state->instance) != VK_SUCCESS) {
        set_error("Failed creating instance");
        return AH_FAILURE;
    }

    uint32_t extension_count = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);

    VkExtensionProperties *extensions = (VkExtensionProperties*)malloc(sizeof(VkExtensionProperties) * extension_count);
    vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extensions);

    for (int i = 0; i < extension_count; i++) {
        printf("\t%s\n", extensions[i].extensionName);
    }

    free(extensions);

    return AH_SUCCESS;
}


/// Pick a Vulkan physical device
AH_RESULT ah_vk_pick_physical_device(vulkan_state_t *vk_state) {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(vk_state->instance, &device_count, NULL);

    if (device_count == 0) {
        set_error("Failed to find GPUs with Vulkan support!");
        return AH_FAILURE;
    }

    VkPhysicalDevice *devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice)*device_count);
    vkEnumeratePhysicalDevices(vk_state->instance, &device_count, devices);

    for (int i = 0; i < device_count; i++) {
        if (is_physical_device_suitable(devices[i])) {
            vk_state->physical_device = devices[i];
            break;
        }
    }

    if (vk_state->physical_device == VK_NULL_HANDLE) {
        set_error("Failed to find a suitable GPU");
        return AH_FAILURE;
    }

    return AH_SUCCESS;
}


AH_RESULT ah_vk_create_logical_device(vulkan_state_t *vk_state) {
    VkDeviceQueueCreateInfo queue_create_infos[2] = {};

    float queue_priority = 1.0f;

    queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[0].queueFamilyIndex = vk_state->queue_family_indices.graphics_family;
    queue_create_infos[0].queueCount = 1;
    queue_create_infos[0].pQueuePriorities = &queue_priority;
    queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_infos[1].queueFamilyIndex = vk_state->queue_family_indices.present_family;
    queue_create_infos[1].queueCount = 1;
    queue_create_infos[1].pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures device_features = {};

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = queue_create_infos;
    create_info.queueCreateInfoCount = 2;
    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount = extensions_count;
    create_info.ppEnabledExtensionNames = extensions;
    create_info.enabledLayerCount = validation_layers_count;
    create_info.ppEnabledLayerNames = validation_layers;

    if (vkCreateDevice(vk_state->physical_device, &create_info, NULL, &vk_state->device) != VK_SUCCESS) {
        set_error("Could not create logical device");
        return AH_FAILURE;
    }

    vkGetDeviceQueue(
        vk_state->device,
        vk_state->queue_family_indices.graphics_family,
        0,
        &vk_state->graphics_queue
    );

    vkGetDeviceQueue(
        vk_state->device,
        vk_state->queue_family_indices.present_family,
        0,
        &vk_state->present_queue
    );

    return AH_SUCCESS;
}

AH_RESULT ah_vk_create_surface(vulkan_state_t *vk_state) {
    if (glfwCreateWindowSurface(vk_state->instance, vk_state->window, NULL, &vk_state->surface) != VK_SUCCESS) {
        set_error("Error creating surface");
        return AH_FAILURE;
    }

    return AH_SUCCESS;
}

VkSurfaceFormatKHR choose_swap_surface_format(vulkan_state_t *vk_state) {
    for (int i = 0; i < vk_state->swapchain_support.num_formats; i++) {
        VkSurfaceFormatKHR format = vk_state->swapchain_support.formats[i];
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return vk_state->swapchain_support.formats[0];
}

VkPresentModeKHR choose_swap_present_mode(vulkan_state_t *vk_state) {
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D choose_swap_extent(vulkan_state_t *vk_state) {
    return vk_state->swapchain_support.capabilities.currentExtent;
}

AH_RESULT ah_vk_create_swapchain(vulkan_state_t *vk_state) {
    populate_swapchain_support(vk_state);

    VkSurfaceFormatKHR surface_format = choose_swap_surface_format(vk_state);
    VkPresentModeKHR present_mode = choose_swap_present_mode(vk_state);
    VkExtent2D extent = choose_swap_extent(vk_state);
    uint32_t image_count = vk_state->swapchain_support.capabilities.minImageCount + 1;

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = vk_state->surface;
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queue_family_indices[2] = {
        vk_state->queue_family_indices.graphics_family,
        vk_state->queue_family_indices.present_family
    };

    if (vk_state->queue_family_indices.graphics_family != vk_state->queue_family_indices.present_family) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        set_error("Can only use concurrent sharing mode");
        return AH_FAILURE;
    }

    create_info.preTransform = vk_state->swapchain_support.capabilities.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(vk_state->device, &create_info, NULL, &vk_state->swapchain) != VK_SUCCESS) {
        set_error("Error creating swapchain");
        return AH_FAILURE;
    }

    vkGetSwapchainImagesKHR(vk_state->device, vk_state->swapchain, &vk_state->num_swapchain_images, NULL);
    vk_state->swapchain_images = (VkImage*)malloc(sizeof(VkImage)*vk_state->num_swapchain_images);
    vkGetSwapchainImagesKHR(vk_state->device, vk_state->swapchain, &vk_state->num_swapchain_images, vk_state->swapchain_images);
    vk_state->swapchain_extent = extent;
    vk_state->swapchain_image_format = surface_format.format;

    return AH_SUCCESS;
}

AH_RESULT ah_vk_create_image_views(vulkan_state_t *vk_state) {
    vk_state->swapchain_image_views = (VkImageView*)malloc(sizeof(VkImageView)*vk_state->num_swapchain_images);

    for (int i = 0; i < vk_state->num_swapchain_images; i++) {
        VkImageViewCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        create_info.image = vk_state->swapchain_images[i];
        create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        create_info.format = vk_state->swapchain_image_format;
        create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        create_info.subresourceRange.baseMipLevel = 0;
        create_info.subresourceRange.levelCount = 1;
        create_info.subresourceRange.baseArrayLayer = 0;
        create_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vk_state->device, &create_info, NULL, &vk_state->swapchain_image_views[i]) != VK_SUCCESS) {
            set_error("error creating image view");
            return AH_FAILURE;
        }
    }

    return AH_SUCCESS;
}

AH_RESULT create_shader_module(vulkan_state_t *vk_state, buffer_t *spv_buffer, VkShaderModule *module) {
    VkShaderModuleCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = spv_buffer->size;
    create_info.pCode = (uint32_t*)&spv_buffer->data;

    if (vkCreateShaderModule(vk_state->device, &create_info, NULL, module) != VK_SUCCESS) {
        set_error("Error creating shadow module");
        return AH_FAILURE;
    }

    return AH_SUCCESS;
}

AH_RESULT ah_vk_create_render_pass(vulkan_state_t *vk_state) {
    VkAttachmentDescription color_attachment = {};
    color_attachment.format = vk_state->swapchain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (vkCreateRenderPass(vk_state->device, &render_pass_info, NULL, &vk_state->render_pass) != VK_SUCCESS) {
        set_error("Error creating render pass");
        return AH_FAILURE;
    }

    return AH_SUCCESS;
}

AH_RESULT ah_vk_create_graphics_pipeline(vulkan_state_t *vk_state) {
    buffer_t *vert_shader_code = read_file("./shader_vert.spv");
    buffer_t *frag_shader_code = read_file("./shader_frag.spv");

    printf("VERT: %ld bytes\n", vert_shader_code->size);
    printf("FRAG: %ld bytes\n", frag_shader_code->size);

    VkShaderModule vert_shader_module;
    VkShaderModule frag_shader_module;

    create_shader_module(vk_state, vert_shader_code, &vert_shader_module);
    create_shader_module(vk_state, frag_shader_code, &frag_shader_module);

    VkPipelineShaderStageCreateInfo vert_shader_stage_info = {};
    vert_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vert_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vert_shader_stage_info.module = vert_shader_module;
    vert_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo frag_shader_stage_info = {};
    frag_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    frag_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    frag_shader_stage_info.module = frag_shader_module;
    frag_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {
        vert_shader_stage_info,
        frag_shader_stage_info
    };

    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state = {};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates = dynamic_states;

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.pVertexBindingDescriptions = NULL;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = NULL;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vk_state->swapchain_extent.width;
    viewport.height = (float)vk_state->swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = vk_state->swapchain_extent;

    VkPipelineViewportStateCreateInfo viewport_state = {};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = NULL;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending = {};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 0;
    pipeline_layout_info.pSetLayouts = NULL;

    if (vkCreatePipelineLayout(vk_state->device, &pipeline_layout_info, NULL, &vk_state->pipeline_layout) != VK_SUCCESS) {
        set_error("Error creating pipeline layout");
        return AH_FAILURE;
    }

    VkGraphicsPipelineCreateInfo pipeline_info = {};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = NULL;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = vk_state->pipeline_layout;
    pipeline_info.renderPass = vk_state->render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(vk_state->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &vk_state->pipeline) != VK_SUCCESS) {
        set_error("Error creating graphics pipeline");
        return AH_FAILURE;
    }

    vkDestroyShaderModule(vk_state->device, vert_shader_module, NULL);
    vkDestroyShaderModule(vk_state->device, frag_shader_module, NULL);

    return AH_SUCCESS;
}

AH_RESULT ah_vk_create_framebuffers(vulkan_state_t *vk_state) {
    vk_state->swapchain_framebuffers = malloc(sizeof(VkFramebuffer)*vk_state->num_swapchain_images);

    for (int i = 0; i < vk_state->num_swapchain_images; i++) {
        VkImageView attachments[] = {
            vk_state->swapchain_image_views[i]
        };

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = vk_state->render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = vk_state->swapchain_extent.width;
        framebuffer_info.height = vk_state->swapchain_extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(vk_state->device, &framebuffer_info, NULL, &vk_state->swapchain_framebuffers[i]) != VK_SUCCESS) {
            set_error("Error creating framebuffer");
            return AH_FAILURE;
        }
    }

    return AH_SUCCESS;
}

AH_RESULT ah_vk_create_command_pool(vulkan_state_t *vk_state) {
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = vk_state->queue_family_indices.graphics_family;

    if (vkCreateCommandPool(vk_state->device, &pool_info, NULL, &vk_state->command_pool) != VK_SUCCESS) {
        set_error("Error creating command pool");
        return AH_FAILURE;
    }

    return AH_SUCCESS;
}

AH_RESULT ah_vk_create_command_buffer(vulkan_state_t *vk_state) {
    VkCommandBufferAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = vk_state->command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(vk_state->device, &alloc_info, &vk_state->command_buffer) != VK_SUCCESS) {
        set_error("Error creating command buffer");
        return AH_FAILURE;
    }

    return AH_SUCCESS;
}

AH_RESULT ah_vk_record_command_buffer(vulkan_state_t *vk_state, VkCommandBuffer command_buffer, uint32_t image_index, uint32_t index) {
    VkCommandBufferBeginInfo begin_info = {};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = NULL;

    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
        set_error("Failed to begin recording command buffer");
        return AH_FAILURE;
    }

    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = vk_state->render_pass;
    render_pass_info.framebuffer = vk_state->swapchain_framebuffers[image_index];
    render_pass_info.renderArea.offset.x = 0;
    render_pass_info.renderArea.offset.y = 0;
    render_pass_info.renderArea.extent = vk_state->swapchain_extent;

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_state->pipeline);

    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)vk_state->swapchain_extent.width;
    viewport.height = (float)vk_state->swapchain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = vk_state->swapchain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdDraw(command_buffer, 3, 1, index % 4, 0);

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS) {
        set_error("Couldn't end command buffer");
        return AH_FAILURE;
    }

    return AH_SUCCESS;
}


AH_RESULT ah_vk_create_sync_objects(vulkan_state_t *vk_state) {
    VkSemaphoreCreateInfo semaphore_info = {};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(vk_state->device, &semaphore_info, NULL, &vk_state->image_available_sempahore) != VK_SUCCESS ||
        vkCreateSemaphore(vk_state->device, &semaphore_info, NULL, &vk_state->render_finished_semaphore) != VK_SUCCESS ||
        vkCreateFence(vk_state->device, &fence_info, NULL, &vk_state->in_flight_fence) != VK_SUCCESS) {
       set_error("Failed to create sync objects") ;
       return AH_FAILURE;
    }

    return AH_SUCCESS;
}
