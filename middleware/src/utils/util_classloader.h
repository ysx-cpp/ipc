#pragma once
#include <dlfcn.h>
#include <string>
#include <memory>
#include "ipc_string.h"

namespace ipc {
namespace util {

template<typename T>
class ClassLoader {

public:
    using InterfaceCreate = T* (*)();
    using InterfaceDestroy = void (*)(T*);

    ClassLoader(const std::string& library_path) {
        assert(library_path != "");
        m_handle = dlopen(library_path.c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (m_handle == nullptr) {
            DEBUG_ERROR("%s", dlerror());
        }
    }

    ~ClassLoader() {
        if (m_handle != nullptr) {
            dlclose(m_handle);
            m_handle = nullptr;
        }
    }

    std::shared_ptr<T> load(const std::string& class_name) {
        if (m_handle == nullptr) {
            return nullptr;
        }

        // export symbol name
        std::string create_interface = "ipc_class_loader_create";
        std::string destory_interface = "ipc_class_loader_destory";
        if (class_name != "") {
            create_interface = string_sprintf("ipc_class_loader_create_%s", class_name.c_str());
            destory_interface = string_sprintf("ipc_class_loader_destory_%s", class_name.c_str());
        }

        // export by ipc_REGISTER_CLASS()
        auto create_func = reinterpret_cast<InterfaceCreate>(dlsym(m_handle, create_interface.c_str()));
        auto destory_func = reinterpret_cast<InterfaceDestroy>(dlsym(m_handle, destory_interface.c_str()));

        if (create_func==nullptr || destory_func==nullptr) {
            dlclose(m_handle);
            DEBUG_ERROR("%s", dlerror());
            return nullptr;
        }

        return std::shared_ptr<T>(
            create_func(), // alloc shared_ptr
            [destory_func](T *ptr){destory_func(ptr);} // free shared_ptr
        );
    }

private:
    void* m_handle;
};

// export create/destory func example
// must defined in ipc_task.h for hiden implement
#define ipc_REGISTER_CLASS(class_name) \
extern "C" class_name* ipc_class_loader_create_##class_name() { \
    return new class_name(); \
} \
extern "C" void ipc_class_loader_destory_##class_name(class_name* ptr) { \
    delete ptr; \
}

} // namespace util
} // namespace ipc
