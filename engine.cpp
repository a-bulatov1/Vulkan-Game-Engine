#include <memory>
#if defined(__INTELLISENSE__) || !defined(USE_CPP20_MODULES)
#	include <vulkan/vulkan_raii.hpp>
#else
import vulkan_hpp;
#endif
#include <GLFW/glfw3.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

const uint32_t WIDTH  = 800;
const uint32_t HEIGHT = 600;

class Engine {
  public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

  private:
	GLFWwindow *window = nullptr;

	vk::raii::Context  context;
	vk::raii::Instance instance = nullptr;

	void initWindow() {
		
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan() {
		createInstance();
	}

	void createInstance() {
		constexpr vk::ApplicationInfo appInfo{
			.pApplicationName   = "VBGE",
			.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 ),
			.pEngineName        = "No Engine",
			.engineVersion      = VK_MAKE_VERSION( 1, 0, 0 ),
			.apiVersion         = vk::ApiVersion14};

		// Get the required instance extensions from GLFW.
		uint32_t glfwExtensionCount = 0;
		auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// Check if the required GLFW extensions are supported by the Vulkan implementation.
		auto extensionProperties = context.enumerateInstanceExtensionProperties();
		for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
			if (std::ranges::none_of(extensionProperties,
				[glfwExtension = glfwExtensions[i]](auto const& extensionProperty)
				{ return strcmp(extensionProperty.extensionName, glfwExtension) == 0; })) {
				throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
			}
		}
		// A more readable way to write the return statement would be:
		// return std::string(extensionProperty.extensionName) == glfwExtension;
		// although that allocates some space for the string, making it ever so slightly slower

		vk::InstanceCreateInfo createInfo{
			.pApplicationInfo = &appInfo,
			.enabledExtensionCount = glfwExtensionCount,
			.ppEnabledExtensionNames = glfwExtensions};

		instance = vk::raii::Instance(context, createInfo);
		
		// Display available extensions in the terminal output.
		auto extensions = context.enumerateInstanceExtensionProperties();

		std::cout << "available extensions:\n";
		for (const auto& extension : extensions) {
			std::cout << '\t' << extension.extensionName << '\n';
		}
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
		glfwDestroyWindow(window);

		glfwTerminate();
	}
};

int main() {
	try {
		Engine app;
		app.run();
	}
	catch (const std::exception &e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}