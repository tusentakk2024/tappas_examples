#include <iostream>
#include <regex>
#include "hailo/hailort.h"
#include "common/labels/coco_eighty.hpp"
#include "my_yolo_postprocess.hpp"

static const hailo_format_order_t HAILO_NMS_BY_CLASS_VALUE = (hailo_format_order_t)22;

void filter(HailoROIPtr roi)
{
    std::vector<HailoTensorPtr> tensors = roi->get_tensors();
    std::vector<HailoDetection> detections;
    auto label = common::coco_eighty;

    for (auto tensor : tensors)
    {
        std::cout << "name:" << tensor->name() << std::endl;

        if (std::regex_search(tensor->name(), std::regex("nms_postprocess"))) 
        {
            auto vstream_info = tensor->vstream_info();
            uint32_t max_bboxes_per_class = vstream_info.nms_shape.max_bboxes_per_class;
            uint32_t num_of_classes = vstream_info.nms_shape.number_of_classes;
            size_t buffer_offset = 0;
            uint8_t *buffer = tensor->data();

            if ((HAILO_FORMAT_ORDER_HAILO_NMS != vstream_info.format.order) && (HAILO_NMS_BY_CLASS_VALUE != vstream_info.format.order))
            {
                continue;
            }

            for (size_t class_id = 0; class_id < num_of_classes; class_id++)
            {
                float32_t bbox_count = 0;
                memcpy(&bbox_count, buffer + buffer_offset, sizeof(bbox_count));
                buffer_offset += sizeof(bbox_count);    

                for (size_t bbox_index = 0; bbox_index < static_cast<uint32_t>(bbox_count); bbox_index++)
                {
                    hailo_bbox_float32_t *bbox = (hailo_bbox_float32_t *)(&buffer[buffer_offset]);
                    float32_t x = bbox->x_min;
                    float32_t y = bbox->y_min;
                    float32_t w = bbox->x_max - bbox->x_min;
                    float32_t h = bbox->y_max - bbox->y_min;
                    float confidence = CLAMP(bbox->score, 0.0f, 1.0f);

                    auto detection = HailoDetection(HailoBBox(x, y, w, h), class_id + 1, label[class_id + 1], confidence);
                    detections.push_back(detection);

                    buffer_offset += sizeof(hailo_bbox_float32_t);

                    std::cout << "id:" << class_id;
                    std::cout << ",x:" << x;
                    std::cout << ",y:" << y;
                    std::cout << ",w:" << w;
                    std::cout << ",h:" << h;
                    std::cout << ",score:" << confidence << std::endl;
                }
            }
        }
    }

    hailo_common::add_detections(roi, detections);
}