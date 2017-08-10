#include "utilities.hpp"

#include <iostream>

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCall(VkDebugReportFlagsEXT vkFlags, VkDebugReportObjectTypeEXT objType,
          uint64_t obj, size_t location, int32_t code, const char *layerPrefix,
          const char *msg, void *userData) {
	vk::DebugReportFlagsEXT flags;
	if (vkFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		flags |= vk::DebugReportFlagBitsEXT::eInformation;
	if (vkFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		flags |= vk::DebugReportFlagBitsEXT::eWarning;
	if (vkFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		flags |= vk::DebugReportFlagBitsEXT::ePerformanceWarning;
	if (vkFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		flags |= vk::DebugReportFlagBitsEXT::eError;
	if (vkFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		flags |= vk::DebugReportFlagBitsEXT::eDebug;
	std::cerr << "VALIDATION: " << to_string(flags) << " " << msg << std::endl;

	return VK_FALSE;
}
static PFN_vkCreateDebugReportCallbackEXT pfn_vkCreateDebugReportCallbackEXT;
VkResult vkCreateDebugReportCallbackEXT(
    VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
    const VkAllocationCallbacks *pAllocator,
    VkDebugReportCallbackEXT *pCallback) {
	return pfn_vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator,
	                                          pCallback);
}
static PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT;
void vkDestroyDebugReportCallbackEXT(VkInstance instance,
                                     VkDebugReportCallbackEXT callback,
                                     const VkAllocationCallbacks *pAllocator) {
	pfn_vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}
vk::UniqueDebugReportCallbackEXT createDebugCallback(vk::Instance instance) {
	pfn_vkCreateDebugReportCallbackEXT =
	    reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
	        instance.getProcAddr("vkCreateDebugReportCallbackEXT"));
	pfn_vkDestroyDebugReportCallbackEXT =
	    reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(
	        instance.getProcAddr("vkDestroyDebugReportCallbackEXT"));
	return instance.createDebugReportCallbackEXTUnique(
	    {vk::DebugReportFlagBitsEXT::eDebug | vk::DebugReportFlagBitsEXT::eError |
	         vk::DebugReportFlagBitsEXT::ePerformanceWarning |
	         vk::DebugReportFlagBitsEXT::eWarning,
	     debugCall});
}
void printInfo(vk::Instance instance, vk::SurfaceKHR surface) {
	using std::cout;
	using std::endl;
	cout << "EXTENTIONS:" << endl;
	for (auto &ep : vk::enumerateInstanceExtensionProperties())
		cout << '\t' << ep.extensionName << ' ' << ep.specVersion << endl;
	cout << "LAYERS:" << endl;
	for (auto &lp : vk::enumerateInstanceLayerProperties()) {
		cout << '\t' << lp.layerName << ' ' << lp.specVersion << ' '
		     << lp.implementationVersion << endl
		     << "\t\t" << lp.description << endl;

		cout << "\t\tLAYER EXTENTIONS:" << endl;
		for (auto &ep :
		     vk::enumerateInstanceExtensionProperties(std::string(lp.layerName)))
			cout << "\t\t\t" << ep.extensionName << ' ' << ep.specVersion << endl;
	}
	auto devices = instance.enumeratePhysicalDevices();
	cout << "DEVICES:" << endl;
	for (auto &d : devices) {
		cout << '\t' << d.getProperties().deviceName << " "
		     << to_string(d.getProperties().deviceType) << " "
		     << d.getProperties().driverVersion << endl;
		;
		cout << "\tDEVICE EXTENTIONS:" << endl;
		for (auto &ep : d.enumerateDeviceExtensionProperties())
			cout << "\t\t" << ep.extensionName << ' ' << ep.specVersion << endl;
		cout << "\tDEVICE LAYERS:" << endl;
		for (auto &lp : d.enumerateDeviceLayerProperties()) {
			cout << "\t\t" << lp.layerName << ' ' << lp.specVersion << ' '
			     << lp.implementationVersion << endl
			     << "\t\t\t" << lp.description << endl;

			cout << "\t\t\tLAYER EXTENTIONS:" << endl;
			for (auto &ep :
			     d.enumerateDeviceExtensionProperties(std::string(lp.layerName)))
				cout << "\t\t\t\t" << ep.extensionName << ' ' << ep.specVersion << endl;
		}
		cout << "\tDEVICE MEMORY TYPES:" << endl;
		auto props = d.getMemoryProperties();
		auto mts = props.memoryTypes;
		for (size_t i = 0; i < props.memoryTypeCount; i++)
			cout << "\t\theap id = " << mts[i].heapIndex << ' '
			     << to_string(mts[i].propertyFlags) << endl;
		cout << "\tDEVICE MEMORY HEAPS:" << endl;
		auto mhs = props.memoryHeaps;
		for (size_t i = 0; i < props.memoryHeapCount; i++)
			cout << "\t\t" << mhs[i].size << endl;
		cout << "\tDEVICE QUEUES:" << endl;
		for (auto &p : d.getQueueFamilyProperties())
			cout << "\t\t" << p.queueCount << ' ' << to_string(p.queueFlags) << endl;
		cout << "\tWINDOW SURFACE FORMATS:" << endl;
		for (auto &f : d.getSurfaceFormatsKHR(surface)) {
			cout << "\t\t" << to_string(f.format) << " " << to_string(f.colorSpace)
			     << endl;
		}
		cout << "\t\tCOMPOSITE:"
		     << to_string(
		            d.getSurfaceCapabilitiesKHR(surface).supportedCompositeAlpha)
		     << endl;
		// cout << "\tDISPLAYS:" << endl;
		// for (auto &disp : d.getDisplayPropertiesKHR()) {
		// 	cout << "\t\t" << disp.displayName << " " << disp.physicalDimensions.width
		// 	     << "x" << disp.physicalDimensions.height << " "
		// 	     << disp.physicalResolution.width << "x"
		// 	     << disp.physicalResolution.height << endl;
		// 	cout << "\t\tPLANES:" << endl;
		// 	for (auto &mode : d.getDisplayModePropertiesKHR(disp.display)) {
		// 		cout << "\t\t\t" << mode.parameters.refreshRate << " "
		// 		     << mode.parameters.visibleRegion.width << "x"
		// 		     << mode.parameters.visibleRegion.height << endl;
		// 	}
		// }
		// cout << "\tPLANES:" << endl;
		// for (auto &plane : d.getDisplayPlanePropertiesKHR()) {
		// 	cout << "\t\t" << plane.currentStackIndex << endl;
		// }
	}
}
