#include <iostream>
#include "disp_bbox.hpp"

void filter(HailoROIPtr roi)
{
    std::vector<HailoDetection> detections;

    for (int i =0; i < 10; i++)
    {
        HailoDetection detection = HailoDetection(HailoBBox(0.1 * i, 0.1 * i, 0.1, 0.1), "person", i * 0.1);
        detections.push_back(detection);
    }

    hailo_common::add_detections(roi, detections);
}