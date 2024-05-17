#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>

typedef  uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef  int32_t i32;
typedef    float f32;
typedef   double f64;
typedef unsigned long long int u64;

#define FALSE false
#define TRUE  true

#define VK_OK(result) \
  if (VK_SUCCESS != (result)) { fprintf(stderr, "Vulkan failed at %u %s\n", __LINE__, __FILE__); exit(-1); }

// Types
// -----

// Local Types
typedef u8  Tag;  // Tag  ::= 3-bit (rounded up to u8)
typedef u32 Val;  // Val  ::= 29-bit (rounded up to u32)
typedef u32 Port; // Port ::= Tag + Val (fits a u32)
typedef u64 Pair; // Pair ::= Port + Port (fits a u64)

// Rules
typedef u8 Rule; // Rule ::= 3-bit (rounded up to 8)

// Numbs
typedef u32 Numb; // Numb ::= 29-bit (rounded up to u32)

// Tags
#define VAR 0x0 // variable
#define REF 0x1 // reference
#define ERA 0x2 // eraser
#define NUM 0x3 // number
#define CON 0x4 // constructor
#define DUP 0x5 // duplicator
#define OPR 0x6 // operator
#define SWI 0x7 // switch

// Interaction Rule Values
#define LINK 0x0
#define CALL 0x1
#define VOID 0x2
#define ERAS 0x3
#define ANNI 0x4
#define COMM 0x5
#define OPER 0x6
#define SWIT 0x7

// Numbers
#define SYM 0x0
#define U24 0x1
#define I24 0x2
#define F24 0x3
#define ADD 0x4
#define SUB 0x5
#define MUL 0x6
#define DIV 0x7
#define REM 0x8
#define EQ  0x9
#define NEQ 0xA
#define LT  0xB
#define GT  0xC
#define AND 0xD
#define OR  0xE
#define XOR 0xF

// Constants
#define FREE 0x00000000
#define ROOT 0xFFFFFFF8
#define NONE 0xFFFFFFFF

// Cache Padding
#define CACHE_PAD 64

// Top-Level Definition
typedef struct Def {
  char name[32];
  bool safe;
  u32  rbag_len;
  u32  node_len;
  u32  vars_len;
  Port root;
  Pair rbag_buf[32];
  Pair node_buf[32];
} Def;

// Book of Definitions
typedef struct Book {
  u32 defs_len;
  Def defs_buf[256];
} Book;

VkInstance vulkan_create() {
  VkApplicationInfo create_info;
  create_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  create_info.pApplicationName = "HVM";
  create_info.applicationVersion = VK_MAKE_VERSION(2, 0, 8);
  create_info.pEngineName = "nopers";
  create_info.engineVersion = VK_MAKE_VERSION(2, 0, 8);
  create_info.apiVersion = VK_API_VERSION_1_3;

  VkInstanceCreateInfo instance_info;
  instance_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  instance_info.pApplicationInfo = &create_info;
  instance_info.enabledExtensionCount = 0;
  instance_info.enabledLayerCount = 0;

  VkInstance instance;
  VK_OK(vkCreateInstance(&instance_info, NULL, &instance))
  return instance;
}

void vulkan_destroy(VkInstance instance, VkDevice device) {
  vkDestroyDevice(device, NULL);
  
  vkDestroyInstance(instance, NULL);
}

VkDevice vulkan_grab_device(VkInstance instance) {
  u32 physical_device_count = 0;
  VK_OK(vkEnumeratePhysicalDevices(instance, &physical_device_count, NULL));

  VkPhysicalDevice* const devices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physical_device_count);

  VK_OK(vkEnumeratePhysicalDevices(instance, &physical_device_count, devices));

  for (u32 i = 0; i < physical_device_count; i++) {
    //should make this run on everything it can eventually. for now- just get the first device that'll listen.
    VkPhysicalDevice dev = devices[i];
    u32 queue_family_props_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_props_count, NULL);

    VkQueueFamilyProperties* const props = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * queue_family_props_count);

    vkGetPhysicalDeviceQueueFamilyProperties(dev, &queue_family_props_count, props);
    i32 valid_index = -1;

    for (u32 j = 0; j < queue_family_props_count; j++) {
      if (VK_QUEUE_COMPUTE_BIT & props[j].queueFlags) {
	valid_index = (i32)j;
      }
    }

    if (valid_index == -1) {
      continue;
    }

    const float queue_prio = 1.0f;
    VkDeviceQueueCreateInfo queue_create_info;
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.pNext = NULL;
    queue_create_info.flags = 0;
    queue_create_info.queueFamilyIndex = (u32)valid_index;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queue_prio;

    VkDeviceCreateInfo device_create_info;
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = NULL;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = NULL;
    device_create_info.enabledExtensionCount = 0;
    device_create_info.ppEnabledExtensionNames = NULL;
    device_create_info.pEnabledFeatures = NULL;

    VkDevice device;
    VK_OK(vkCreateDevice(dev, &device_create_info, NULL, &device));
    return device;
  }
  fprintf(stderr, "No usable compute device found!");
  exit(-1);
}

void hvm_vk(u32* book_buffer, bool run_io) {
  VkInstance instance = vulkan_create();
  VkDevice device = vulkan_grab_device(instance);
  vulkan_destroy(instance, device);
}
