#include <vector>
#include <filesystem>
#include "my_classification_postprocess.hpp"
#include "common/tensors.hpp"
#include "common/math.hpp"
#include "json_config.hpp"
#include "rapidjson/document.h"
#include "xtensor/xarray.hpp"

MyClassificationParams *init(const std::string config_path)
{
    MyClassificationParams *params = new MyClassificationParams();

    if (!std::filesystem::exists(config_path))
    {
        std::cerr << "Config file doesn't exist, using default parameters" << std::endl;
        return params;
    }
    else
    {
        char config_buffer[4096];
        const char *json_schema = R""""({
            "$schema": "http://json-schema.org/draft-04/schema#",
            "tensor_name": "string",
            "labels": "array",
        })"""";

        std::FILE *fp = fopen(config_path.c_str(), "r");
        if (fp == nullptr)
        {
            throw std::runtime_error("JSON config file is not valid");
        }

        rapidjson::FileReadStream stream(fp, config_buffer, sizeof(config_buffer));
        bool valid = common::validate_json_with_schema(stream, json_schema);
        if (valid)
        {
            rapidjson::Document doc_config_json;
            doc_config_json.ParseStream(stream);

            // parse labels
            auto labels = doc_config_json["labels"].GetArray();
            uint i = 0;
            for (auto &v : labels)
            {
                params->labels.insert(std::pair<std::uint8_t, std::string>(i, v.GetString()));
                i++;
            }

            // set the params
            if (doc_config_json.HasMember("tensor_name")) {
                params->tensor_name = doc_config_json["tensor_name"].GetString();
            }
        }

        fclose(fp);
    }

    return params;
}

void filter(HailoROIPtr roi, void *params_void_ptr)
{
    MyClassificationParams *params = reinterpret_cast<MyClassificationParams *>(params_void_ptr);
    const int k = 1;

    if (!roi->has_tensors())
    {
        return;
    }

    // Extract the relevant output tensor.
    HailoTensorPtr scores = roi->get_tensor(params->tensor_name);

    // Convert the tensor to xarray.
    xt::xarray<uint8_t> xscores = common::get_xtensor(scores);

    // Find the topk scores.
    xt::xarray<int> top_k_scores = common::top_k(xscores, k);

    // Extrats the label of the top score.
    int index = top_k_scores[0];
    std::string label = params->labels[index];

    float confidence = scores->fix_scale(xscores(index));

    // Update the tensor with the classification result.
    hailo_common::add_classification(roi, scores->name(), label, confidence, index);

    std::cout << "name:" << scores->name() << ",label:" << label << ",score:" << confidence << std::endl;
}