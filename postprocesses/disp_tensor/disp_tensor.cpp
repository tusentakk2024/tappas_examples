#include <iostream>
#include "disp_tensor.hpp"

void filter(HailoROIPtr roi)
{
    std::vector<HailoTensorPtr> tensors = roi->get_tensors();
    std::cout << "tensor num:" << tensors.size() << std::endl;
    for (std::size_t i = 0; i < tensors.size(); i++)
    {
        HailoTensorPtr tensor = tensors[i];
        std::cout <<
            "index:" << i + 1 <<
            " name:" << tensor->name() <<
            " width:" << tensor->width() <<
            " height:" << tensor->height() <<
            " features:" << tensor->features() <<
            std::endl;
    }
}