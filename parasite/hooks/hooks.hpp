#pragma once
#include <minhook/minhook.h>
#include <cstdint>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#define RVA( address ) ((uint64_t)GetModuleHandleA(0) + address)
#define DECL_HOOK(x, ret, ...) extern ret hk_##x(__VA_ARGS__); inline ret(*o_##x)(__VA_ARGS__);
#define ENABLE_HOOK(x, address) MH_CreateHook((LPVOID)address, hk_##x, (LPVOID*)&o_##x); MH_EnableHook((LPVOID)address);
#define ENABLE_API_HOOK(x, mod, proc) MH_CreateHookApi(##mod, ##proc, hk_##x, (LPVOID*)&o_##x ); MH_EnableHook(MH_ALL_HOOKS);

namespace hooks
{
	DECL_HOOK( vkQueueSubmit, VkResult, VkQueue, uint32_t, const VkSubmitInfo*, VkFence );
	DECL_HOOK( vkGetDeviceQueue, void, VkDevice, uint32_t, uint32_t, VkQueue* );
	DECL_HOOK( vkAcquireNextImageKHR, VkResult, VkDevice, VkSwapchainKHR, uint64_t, VkSemaphore, VkFence, uint32_t* );
	DECL_HOOK( vkQueuePresentKHR, VkResult, VkQueue, const VkPresentInfoKHR* );
	DECL_HOOK( vkBeginCommandBuffer, VkResult, VkCommandBuffer, const VkCommandBufferBeginInfo* );
	DECL_HOOK( vkEndCommandBuffer, VkResult, VkCommandBuffer );
	DECL_HOOK( vkCmdBeginRenderPass, void, VkCommandBuffer, const VkRenderPassBeginInfo*, VkSubpassContents );
	DECL_HOOK( vkCmdBindPipeline, void, VkCommandBuffer, VkPipelineBindPoint, VkPipeline );
	DECL_HOOK( vkCmdBindDescriptorSets, void, VkCommandBuffer, VkPipelineBindPoint, VkPipelineLayout, uint32_t, uint32_t, const VkDescriptorSet*, uint32_t, const uint32_t* );
	DECL_HOOK( vkCmdDraw, void, VkCommandBuffer, uint32_t, uint32_t, uint32_t, uint32_t );
	DECL_HOOK( vkGetPhysicalDeviceProperties, void, VkPhysicalDevice, VkPhysicalDeviceProperties* );
	DECL_HOOK( vkGetSwapchainImagesKHR, VkResult, VkDevice, VkSwapchainKHR, uint32_t*, VkImage* );
	DECL_HOOK( vkCmdEndRenderPass, void, VkCommandBuffer );

	extern void Init( );

	extern void Uninit( );
}