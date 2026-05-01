#include "../../hooks.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_vulkan.h>

#include <vector>
#include <cstdio>

#define VKCHECK(x) if ((x) != VK_SUCCESS) { printf("VK ERROR %d\n", x); }

namespace hooks
{
    static bool g_ImGuiInitialized = false;

    static VkInstance g_Instance = VK_NULL_HANDLE;
    static VkDevice g_Device = VK_NULL_HANDLE;
    static VkPhysicalDevice g_PhysicalDevice = VK_NULL_HANDLE;
    static VkQueue g_Queue = VK_NULL_HANDLE;
    static uint32_t g_QueueFamily = 0;

    static VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;
    static VkRenderPass g_RenderPass = VK_NULL_HANDLE;

    static std::vector<VkImage> g_Images;
    static std::vector<VkImageView> g_ImageViews;
    static std::vector<VkCommandPool> g_CommandPools;
    static std::vector<VkCommandBuffer> g_CommandBuffers;

    static uint32_t g_ImageCount = 0;
    static uint32_t g_Width = 1024, g_Height = 768;
    static VkFormat g_Format = VK_FORMAT_B8G8R8A8_UNORM;

    // Track current render pass state
    static VkCommandBuffer g_CurrentCmdBuffer = VK_NULL_HANDLE;
    static VkRenderPass g_CurrentRenderPass = VK_NULL_HANDLE;
    static bool g_InRenderPass = false;

    static VkRenderPassCreateInfo g_RenderPassInfo = {};
    static bool g_RenderPassInfoCaptured = false;

    //============================================================
    // Descriptor Pool
    //============================================================
    void CreateDescriptorPool( )
    {
        if ( g_DescriptorPool != VK_NULL_HANDLE )
            return;

        printf( "[ImGui] Creating descriptor pool\n" );

        VkDescriptorPoolSize pool_sizes[ ] = {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
        };

        VkDescriptorPoolCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
        info.maxSets = 1000 * IM_ARRAYSIZE( pool_sizes );
        info.poolSizeCount = ( uint32_t )IM_ARRAYSIZE( pool_sizes );
        info.pPoolSizes = pool_sizes;

        VkResult res = vkCreateDescriptorPool( g_Device, &info, nullptr, &g_DescriptorPool );
        if ( res != VK_SUCCESS )
        {
            printf( "[ImGui] ERROR: Failed to create descriptor pool: %d\n", res );
            return;
        }

        printf( "[ImGui] Descriptor pool created\n" );
    }

    //============================================================
    // Render Pass (captures app's render pass)
    //============================================================
    static VkResult CreateRenderPass( )
    {
        if ( g_RenderPass != VK_NULL_HANDLE )
            return VK_SUCCESS;
        VkAttachmentDescription attachment{};
        attachment.format = VK_FORMAT_B8G8R8A8_UNORM;
        attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachment.initialLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_ref{};
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_ref;

        VkRenderPassCreateInfo rp_info{};
        rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rp_info.attachmentCount = 1;
        rp_info.pAttachments = &attachment;
        rp_info.subpassCount = 1;
        rp_info.pSubpasses = &subpass;

        VkResult res = vkCreateRenderPass( g_Device, &rp_info, nullptr, &g_RenderPass );
        return res;
    }

    //============================================================
    // Frame Resources
    //============================================================
    void CreateFrameResources( )
    {
        g_CommandPools.resize( g_ImageCount );
        g_CommandBuffers.resize( g_ImageCount );

        for ( uint32_t i = 0; i < g_ImageCount; i++ )
        {
            VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            poolInfo.queueFamilyIndex = g_QueueFamily;
            poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

            VKCHECK( vkCreateCommandPool( g_Device, &poolInfo, nullptr, &g_CommandPools[ i ] ) );

            VkCommandBufferAllocateInfo alloc{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            alloc.commandPool = g_CommandPools[ i ];
            alloc.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            alloc.commandBufferCount = 1;

            VKCHECK( vkAllocateCommandBuffers( g_Device, &alloc, &g_CommandBuffers[ i ] ) );
        }
    }

    static VkInstance GetInstance( )
    {
        auto UnkClass = *reinterpret_cast< std::uint64_t* >(
            ( std::uint64_t )GetModuleHandleA( 0 ) + 0x7898170 );

        UnkClass = *reinterpret_cast< std::uint64_t* >( UnkClass + 0x6FE0 );

        return *reinterpret_cast< VkInstance* >( UnkClass + 0x8 );
    }

    //============================================================
    // Init ImGui
    //============================================================
    void InitImGui( )
    {
        if ( g_ImGuiInitialized )
            return;

        // WAIT for render pass to be captured first
        if ( !g_RenderPass )
        {
            printf( "[ImGui] Waiting for render pass...\n" );
            return;
        }

        printf( "[ImGui] Init starting with app render pass: %p\n", g_RenderPass );

        IMGUI_CHECKVERSION( );
        ImGui::CreateContext( );
        ImGui::StyleColorsDark( );

        CreateDescriptorPool( );

        ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
        pipelineInfo.RenderPass = g_RenderPass;  // Use APP's render pass
        pipelineInfo.Subpass = 0;
        pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        ImGui_ImplVulkan_InitInfo init{};
        init.Instance = g_Instance;
        init.PhysicalDevice = g_PhysicalDevice;
        init.Device = g_Device;
        init.QueueFamily = g_QueueFamily;
        init.Queue = g_Queue;
        init.DescriptorPool = g_DescriptorPool;
        init.MinImageCount = 2;
        init.ImageCount = 2;
        init.PipelineCache = VK_NULL_HANDLE;
        init.PipelineInfoMain = pipelineInfo;
        init.UseDynamicRendering = false;

        printf( "[ImGui] Calling ImGui_ImplVulkan_Init\n" );

        // Wrap in try-catch or add error handling
        if ( !ImGui_ImplVulkan_Init( &init ) )
        {
            printf( "[ImGui] ERROR: ImGui_ImplVulkan_Init failed\n" );
            return;
        }

        printf( "[ImGui] ImGui init succeeded\n" );
        g_ImGuiInitialized = true;
    }

    //============================================================
    // HOOKS
    //============================================================

    void hk_vkGetPhysicalDeviceProperties( VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties )
    {
        o_vkGetPhysicalDeviceProperties( physicalDevice, pProperties );
        if ( !g_PhysicalDevice )
        {
            g_PhysicalDevice = physicalDevice;
            uint32_t count = 0;
            vkGetPhysicalDeviceQueueFamilyProperties( g_PhysicalDevice, &count, nullptr );

            std::vector<VkQueueFamilyProperties> props( count );
            vkGetPhysicalDeviceQueueFamilyProperties( g_PhysicalDevice, &count, props.data( ) );

            for ( uint32_t i = 0; i < count; i++ )
            {
                if ( props[ i ].queueFlags & VK_QUEUE_GRAPHICS_BIT )
                {
                    g_QueueFamily = i;
                    break;
                }
            }

            printf( "g_PhysicalDevice %p | QueueFamily %i \n", g_PhysicalDevice, g_QueueFamily );
        }
    }

    void hk_vkGetDeviceQueue( VkDevice device, uint32_t family, uint32_t index, VkQueue* pQueue )
    {
        o_vkGetDeviceQueue( device, family, index, pQueue );

        g_Device = device;
        g_Queue = *pQueue;
        g_QueueFamily = family;
    }

    VkResult hk_vkGetSwapchainImagesKHR(
        VkDevice device,
        VkSwapchainKHR swapchain,
        uint32_t* pCount,
        VkImage* pImages )
    {
        VkResult res = o_vkGetSwapchainImagesKHR( device, swapchain, pCount, pImages );

        if ( pImages && *pCount )
        {
            g_ImageCount = *pCount;
            g_Images.assign( pImages, pImages + *pCount );

            g_ImageViews.resize( g_ImageCount );

            for ( uint32_t i = 0; i < g_ImageCount; i++ )
            {
                VkImageViewCreateInfo view{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
                view.image = g_Images[ i ];
                view.viewType = VK_IMAGE_VIEW_TYPE_2D;
                view.format = g_Format;
                view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                view.subresourceRange.levelCount = 1;
                view.subresourceRange.layerCount = 1;

                vkCreateImageView( g_Device, &view, nullptr, &g_ImageViews[ i ] );
            }
        }

        return res;
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
            g_CurrentRenderPass = info->renderPass;

            // Capture render pass on first main render pass
            if ( !g_RenderPassInfoCaptured && g_Width >= 1024 && g_Height >= 768 )
            {
                g_RenderPass = info->renderPass;
                g_RenderPassInfoCaptured = true;
                printf( "[ImGui] Captured app render pass: %p (%dx%d)\n",
                    info->renderPass, g_Width, g_Height );
            }
        }

        g_CurrentCmdBuffer = cmd;
        g_InRenderPass = true;

        o_vkCmdBeginRenderPass( cmd, info, contents );
    }

    void hk_vkCmdEndRenderPass( VkCommandBuffer commandBuffer )
    {
        // Render ImGui into the main render pass before ending
        if ( g_ImGuiInitialized && g_InRenderPass && g_Width >= 1024 && g_Height >= 768 )
        {
            ImDrawData* drawData = ImGui::GetDrawData( );
            if ( drawData && drawData->CmdListsCount > 0 )
            {
                printf( "[ImGui] Rendering ImGui\n" );
                ImGui_ImplVulkan_RenderDrawData( drawData, commandBuffer );
            }
        }

        g_InRenderPass = false;
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

        VkResult res = o_vkAcquireNextImageKHR( device, swapchain, timeout, semaphore, fence, pIndex );

        if ( res == VK_SUCCESS )
        {
            // Try init on every frame until it succeeds
            if ( !g_ImGuiInitialized && g_RenderPass )
                InitImGui( );

            if ( g_ImGuiInitialized )
            {
                ImGui_ImplVulkan_NewFrame( );
                ImGui::NewFrame( );

                ImGui::Begin( "Overlay" );
                ImGui::Text( "Hello World" );
                ImGui::End( );

                ImGui::Render( );
            }
        }

        return res;
    }

    VkResult hk_vkQueueSubmit( VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence )
    {
        if ( !g_Queue )
            g_Queue = queue;

        return o_vkQueueSubmit( queue, submitCount, pSubmits, fence );
    }
}   