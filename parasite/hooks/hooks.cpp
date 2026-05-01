#include "hooks.hpp"

#include <iostream>

void hooks::Init( )
{
	//Init MH
	MH_Initialize( );

	//Vulkan
	{
		ENABLE_API_HOOK( vkQueueSubmit, L"vulkan-1.dll", "vkQueueSubmit" );
		ENABLE_API_HOOK( vkGetDeviceQueue, L"vulkan-1.dll", "vkGetDeviceQueue" )
		ENABLE_API_HOOK( vkAcquireNextImageKHR, L"vulkan-1.dll", "vkAcquireNextImageKHR" )
		//ENABLE_API_HOOK( vkQueuePresentKHR, L"vulkan-1.dll", "vkQueuePresentKHR" )
		//ENABLE_API_HOOK( vkBeginCommandBuffer, L"vulkan-1.dll", "vkBeginCommandBuffer" )
		//ENABLE_API_HOOK( vkEndCommandBuffer, L"vulkan-1.dll", "vkEndCommandBuffer" )
		ENABLE_API_HOOK( vkCmdBeginRenderPass, L"vulkan-1.dll", "vkCmdBeginRenderPass" )
		//ENABLE_API_HOOK( vkCmdBindPipeline, L"vulkan-1.dll", "vkCmdBindPipeline" )
		//ENABLE_API_HOOK( vkCmdBindDescriptorSets, L"vulkan-1.dll", "vkCmdBindDescriptorSets" )
		//ENABLE_API_HOOK( vkCmdDraw, L"vulkan-1.dll", "vkCmdDraw" )
		//ENABLE_API_HOOK( vkGetPhysicalDeviceProperties, L"vulkan-1.dll", "vkGetPhysicalDeviceProperties" )
		ENABLE_API_HOOK( vkGetSwapchainImagesKHR, L"vulkan-1.dll", "vkGetSwapchainImagesKHR" )
		ENABLE_API_HOOK( vkCmdEndRenderPass, L"vulkan-1.dll", "vkCmdEndRenderPass" )
	}
}

void hooks::Uninit( )
{
	MH_DisableHook( MH_ALL_HOOKS );
	MH_Uninitialize( );
}