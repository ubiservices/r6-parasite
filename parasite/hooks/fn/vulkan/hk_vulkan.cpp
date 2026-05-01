#include "../../hooks.hpp"

#include <imgui/imgui.h>

#include <vector>
#include <cstdio>
#include <cstring>

namespace hooks
{
    static bool g_Initialized = false;
    static VkDevice g_Device = VK_NULL_HANDLE;
    static VkQueue g_Queue = VK_NULL_HANDLE;
    static uint32_t g_QueueFamily = 0;
    static uint32_t g_Width = 0, g_Height = 0;

    static VkRenderPass g_RenderPass = VK_NULL_HANDLE;
    static VkPipeline g_Pipeline = VK_NULL_HANDLE;
    static VkPipelineLayout g_PipelineLayout = VK_NULL_HANDLE;
    static VkShaderModule g_VertShader = VK_NULL_HANDLE;
    static VkShaderModule g_FragShader = VK_NULL_HANDLE;

    static VkBuffer g_VertexBuffer = VK_NULL_HANDLE;
    static VkBuffer g_IndexBuffer = VK_NULL_HANDLE;
    static VkDeviceMemory g_VertexMemory = VK_NULL_HANDLE;
    static VkDeviceMemory g_IndexMemory = VK_NULL_HANDLE;

    static VkFormat g_SwapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;

    struct Vertex
    {
        float x, y;
        float u, v;
        uint32_t color;
    };

    // ImGui's SPIR-V shaders
    static const uint32_t __glsl_shader_vert_spv[ ] =
    {
        0x07230203,0x00010000,0x0008000b,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
        0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
        0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
        0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
        0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
        0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
        0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
        0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
        0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
        0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
        0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
        0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
        0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
        0x0000001e,0x00000001,0x00030047,0x00000019,0x00000002,0x00050048,0x00000019,0x00000000,
        0x0000000b,0x00000000,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00030047,0x0000001e,
        0x00000002,0x00050048,0x0000001e,0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,
        0x00000001,0x00000023,0x00000008,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
        0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
        0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
        0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
        0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
        0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
        0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
        0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
        0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
        0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
        0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
        0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
        0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
        0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
        0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
        0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
        0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
        0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
        0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
        0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
        0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
        0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
        0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
        0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
        0x0000002d,0x0000002c,0x000100fd,0x00010038
    };

    static const uint32_t __glsl_shader_frag_spv[ ] =
    {
        0x07230203,0x00010000,0x0008000b,0x00000023,0x00000000,0x00020011,0x00000001,0x0006000b,
        0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
        0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
        0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
        0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
        0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
        0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000015,0x7865545f,0x65727574,
        0x00000000,0x00050005,0x00000019,0x6d61535f,0x72656c70,0x00000000,0x00040047,0x00000009,
        0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,0x00000000,0x00040047,0x00000015,
        0x00000021,0x00000000,0x00040047,0x00000015,0x00000022,0x00000000,0x00040047,0x00000019,
        0x00000021,0x00000000,0x00040047,0x00000019,0x00000022,0x00000001,0x00020013,0x00000002,
        0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,
        0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,0x00000007,0x0004003b,0x00000008,
        0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,0x00000002,0x0004001e,0x0000000b,
        0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,0x0000000b,0x0004003b,0x0000000c,
        0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,0x00000001,0x0004002b,0x0000000e,
        0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,0x00000007,0x00090019,0x00000013,
        0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,0x00000001,0x00000000,0x00040020,
        0x00000014,0x00000000,0x00000013,0x0004003b,0x00000014,0x00000015,0x00000000,0x0002001a,
        0x00000017,0x00040020,0x00000018,0x00000000,0x00000017,0x0004003b,0x00000018,0x00000019,
        0x00000000,0x0003001b,0x0000001b,0x00000013,0x0004002b,0x0000000e,0x0000001d,0x00000001,
        0x00040020,0x0000001e,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,0x00000000,
        0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,0x0000000f,
        0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000013,0x00000016,0x00000015,
        0x0004003d,0x00000017,0x0000001a,0x00000019,0x00050056,0x0000001b,0x0000001c,0x00000016,
        0x0000001a,0x00050041,0x0000001e,0x0000001f,0x0000000d,0x0000001d,0x0004003d,0x0000000a,
        0x00000020,0x0000001f,0x00050057,0x00000007,0x00000021,0x0000001c,0x00000020,0x00050085,
        0x00000007,0x00000022,0x00000012,0x00000021,0x0003003e,0x00000009,0x00000022,0x000100fd,
        0x00010038
    };

    VkRenderPass CreateRenderPass( VkDevice device, VkFormat format )
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = format;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorRef{};
        colorRef.attachment = 0;
        colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorRef;

        VkRenderPassCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        info.attachmentCount = 1;
        info.pAttachments = &colorAttachment;
        info.subpassCount = 1;
        info.pSubpasses = &subpass;

        VkRenderPass rp;
        vkCreateRenderPass( device, &info, nullptr, &rp );
        return rp;
    }

    void CreatePipeline( )
    {
        if ( g_Pipeline != VK_NULL_HANDLE )
            return;

        g_RenderPass = CreateRenderPass( g_Device, g_SwapchainFormat );

        printf( "[Render] Creating pipeline\n" );

        // Create shader modules from ImGui's SPIR-V
        VkShaderModuleCreateInfo vertInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof( __glsl_shader_vert_spv ),
            .pCode = __glsl_shader_vert_spv,
        };

        VkResult res = vkCreateShaderModule( g_Device, &vertInfo, nullptr, &g_VertShader );
        if ( res != VK_SUCCESS )
        {
            printf( "[Render] ERROR: vkCreateShaderModule (vert) failed: %d\n", res );
            return;
        }

        VkShaderModuleCreateInfo fragInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof( __glsl_shader_frag_spv ),
            .pCode = __glsl_shader_frag_spv,
        };

        res = vkCreateShaderModule( g_Device, &fragInfo, nullptr, &g_FragShader );
        if ( res != VK_SUCCESS )
        {
            printf( "[Render] ERROR: vkCreateShaderModule (frag) failed: %d\n", res );
            vkDestroyShaderModule( g_Device, g_VertShader, nullptr );
            return;
        }

        VkPipelineShaderStageCreateInfo shaderStages[ 2 ] = {
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_VERTEX_BIT,
                .module = g_VertShader,
                .pName = "main",
            },
            {
                .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                .module = g_FragShader,
                .pName = "main",
            },
        };

        VkVertexInputBindingDescription bindingDesc = {
            .binding = 0,
            .stride = sizeof( Vertex ),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };

        VkVertexInputAttributeDescription attrDesc[ 3 ] = {
            {
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof( Vertex, x ),
            },
            {
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof( Vertex, u ),
            },
            {
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R8G8B8A8_UNORM,
                .offset = offsetof( Vertex, color ),
            },
        };

        VkPipelineVertexInputStateCreateInfo vertexInput = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = 1,
            .pVertexBindingDescriptions = &bindingDesc,
            .vertexAttributeDescriptionCount = 3,
            .pVertexAttributeDescriptions = attrDesc,
        };

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        };

        VkViewport viewport = {
            .x = 0, .y = 0,
            .width = ( float )g_Width,
            .height = ( float )g_Height,
            .minDepth = 0, .maxDepth = 1,
        };

        VkRect2D scissor = {
            .offset = {0, 0},
            .extent = {g_Width, g_Height},
        };

        VkPipelineViewportStateCreateInfo viewportState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport,
            .scissorCount = 1,
            .pScissors = &scissor,
        };

        VkPipelineRasterizationStateCreateInfo rasterization = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_NONE,
            .lineWidth = 1.0f,
        };

        VkPipelineMultisampleStateCreateInfo multisample = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        };

        VkPipelineColorBlendAttachmentState colorBlend = {
            .blendEnable = VK_TRUE,
            .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
            .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
            .colorBlendOp = VK_BLEND_OP_ADD,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        };

        VkPipelineColorBlendStateCreateInfo colorBlendState = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &colorBlend,
        };

        VkPipelineLayoutCreateInfo layoutInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        };
        vkCreatePipelineLayout( g_Device, &layoutInfo, nullptr, &g_PipelineLayout );

        VkGraphicsPipelineCreateInfo pipelineInfo = {
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = 2,
            .pStages = shaderStages,
            .pVertexInputState = &vertexInput,
            .pInputAssemblyState = &inputAssembly,
            .pViewportState = &viewportState,
            .pRasterizationState = &rasterization,
            .pMultisampleState = &multisample,
            .pColorBlendState = &colorBlendState,
            .layout = g_PipelineLayout,
            .renderPass = g_RenderPass,
            .subpass = 0,
        };

        res = vkCreateGraphicsPipelines( g_Device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &g_Pipeline );
        if ( res != VK_SUCCESS )
        {
            printf( "[Render] ERROR: vkCreateGraphicsPipelines failed: %d\n", res );
            return;
        }

        printf( "[Render] Pipeline created successfully\n" );
    }

    void Render( VkCommandBuffer cmdBuffer )
    {
        if ( g_Pipeline == VK_NULL_HANDLE )
            return;

        vkCmdBindPipeline( cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_Pipeline );

        VkViewport vp{};
        vp.x = 0;
        vp.y = 0;
        vp.width = ( float )g_Width;
        vp.height = ( float )g_Height;
        vp.minDepth = 0.0f;
        vp.maxDepth = 1.0f;

        VkRect2D sc{};
        sc.offset = { 0, 0 };
        sc.extent = { g_Width, g_Height };

        vkCmdSetViewport( cmdBuffer, 0, 1, &vp );
        vkCmdSetScissor( cmdBuffer, 0, 1, &sc );

        vkCmdDraw( cmdBuffer, 3, 1, 0, 0 );
    }

    void hk_vkCmdBeginRenderPass(
        VkCommandBuffer cmd,
        const VkRenderPassBeginInfo* info,
        VkSubpassContents contents )
    {
        if ( info )
        {
            g_Width = info->renderArea.extent.width;
            g_Height = info->renderArea.extent.height;
            //g_RenderPass = info->renderPass;

            if ( !g_Initialized && g_Device != VK_NULL_HANDLE )
            {
                CreatePipeline( );
                g_Initialized = true;
            }
        }

        o_vkCmdBeginRenderPass( cmd, info, contents );
    }

    void hk_vkCmdEndRenderPass( VkCommandBuffer commandBuffer )
    {
        if ( g_Initialized && g_Pipeline != VK_NULL_HANDLE )
        {
            Render( commandBuffer );
        }

        o_vkCmdEndRenderPass( commandBuffer );
    }

    VkResult hk_vkAcquireNextImageKHR(
        VkDevice device,
        VkSwapchainKHR swapchain,
        uint64_t timeout,
        VkSemaphore semaphore,
        VkFence fence,
        uint32_t* pIndex )
    {
        if ( !g_Device )
            g_Device = device;

        return o_vkAcquireNextImageKHR( device, swapchain, timeout, semaphore, fence, pIndex );
    }

    void hk_vkGetDeviceQueue( VkDevice device, uint32_t family, uint32_t index, VkQueue* pQueue )
    {
        o_vkGetDeviceQueue( device, family, index, pQueue );
        g_Device = device;
        g_Queue = *pQueue;
        g_QueueFamily = family;
    }

    VkResult hk_vkQueueSubmit( VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence )
    {
        return o_vkQueueSubmit( queue, submitCount, pSubmits, fence );
    }

    VkResult hk_vkGetSwapchainImagesKHR( VkDevice device, VkSwapchainKHR swapchain, uint32_t* pCount, VkImage* pImages )
    {
        return o_vkGetSwapchainImagesKHR( device, swapchain, pCount, pImages );
    }

    VkResult hk_vkQueuePresentKHR( VkQueue queue, const VkPresentInfoKHR* pPresentInfo )
    {
        return o_vkQueuePresentKHR( queue, pPresentInfo );
    }

    VkResult hk_vkEndCommandBuffer( VkCommandBuffer commandBuffer )
    {
        return o_vkEndCommandBuffer( commandBuffer );
    }

    void hk_vkCmdBindPipeline( VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline )
    {
        o_vkCmdBindPipeline( commandBuffer, pipelineBindPoint, pipeline );
    }

    void hk_vkCmdBindDescriptorSets( VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint, VkPipelineLayout pipelineLayout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet* pDescriptorSets, uint32_t dynamicOffsetCount, const uint32_t* pDynamicOffsets )
    {
        o_vkCmdBindDescriptorSets( commandBuffer, pipelineBindPoint, pipelineLayout, firstSet, descriptorSetCount, pDescriptorSets, dynamicOffsetCount, pDynamicOffsets );
    }

    void hk_vkCmdDraw( VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance )
    {
        o_vkCmdDraw( commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance );
    }
}