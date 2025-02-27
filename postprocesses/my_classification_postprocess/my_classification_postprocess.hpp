#pragma once
#include "hailo_objects.hpp"
#include "hailo_common.hpp"

__BEGIN_DECLS

class MyClassificationParams
{
public:
    std::string tensor_name;
    std::map<std::uint8_t, std::string> labels;
};

void filter(HailoROIPtr roi, void *params_void_ptr);
MyClassificationParams *init(const std::string config_path);
__END_DECLS