#include "image_transformer_api.h"

#include <string>

static const PropertyInfo img_transformer_info[] = {
    { "string_param", PROPERTY_TYPE_STRING, "Dummy string parameter.", NULL, NULL, 0, 1, ACCESS_TYPE_USER},
    { "int_param", PROPERTY_TYPE_INTEGER, "Dummy int parameter.", "-1", "0:10", 0, 1, ACCESS_TYPE_USER},
    { "count_frames", PROPERTY_TYPE_BOOLEAN, "Count the number of frames processed by the plugin.", "false", "true:1:false:0", 0, 1, ACCESS_TYPE_USER}
};

static size_t img_transformer_dummy_get_info(const PropertyInfo** info) {
    *info = img_transformer_info;
    return sizeof(img_transformer_info) / sizeof(PropertyInfo);
}

struct img_transformer_dummy_data_t {
    std::string stringParam;
    int intParam{-1};
    int frameCount{0};
    bool countFrames{false};
    std::string msg;
};

/* This structure can contain only pointers and simple types */
struct img_transformer_dummy_t{
    img_transformer_dummy_data_t* data;
};

static size_t img_transformer_dummy_get_size() {
    return sizeof(img_transformer_dummy_t);
}

static Status img_transformer_dummy_init(ImgTransformerHandle handle, const ImgTransformerInitParams* init_params) {
    img_transformer_dummy_t* state = (img_transformer_dummy_t*)handle;
    state->data = new img_transformer_dummy_data_t;
    auto invalidValue = [&](const std::string& option, const std::string& value, const std::string& expectedValues) {
        return "Invalid '"+option+"' option value: '"+value+"'. Expected value: "+expectedValues+".";
    };
    for (int i = 0; i < (int)init_params->count; i++){
        std::string name(init_params->properties[i].name);
        std::string value(init_params->properties[i].value);
        if (name == "string_param") {
            state->data->stringParam = value;
        } else if (name == "int_param") {
            state->data->intParam = std::atoi(value.c_str());
        } else if(name == "count_frames") {
            if(value == "true" || value == "1")
                state->data->countFrames = true;
            else if (value == "false" || value == "0")
                state->data->countFrames = false;
            else {
                state->data->msg = invalidValue(name, value, "true:1:false:0");
                return STATUS_ERROR;
            }
        } else {
            state->data->msg = "Could not recognise option '" + name +"'.";
            return STATUS_ERROR;
        }
    }
    return STATUS_OK;
}

static Status img_transformer_dummy_close(ImgTransformerHandle handle) {
    img_transformer_dummy_t* state = (img_transformer_dummy_t*)handle;
    if (state && state->data) {
        delete state->data;
        state->data = nullptr;
    }
    return STATUS_OK;
}

static Status img_transformer_dummy_process(ImgTransformerHandle handle, ImgTransformerFrame*) {
    img_transformer_dummy_t* state = (img_transformer_dummy_t*)handle;
    state->data->msg = "";
    if(state->data->intParam >= 0)
        state->data->msg += "\nImage transformer dummy int param: " + std::to_string(state->data->intParam);
    if(!state->data->stringParam.empty())
        state->data->msg += "\nImage transformer dummy string param: " + state->data->stringParam;
    if(state->data->countFrames)
        state->data->msg += "\nImage transformer frame count: " + std::to_string(++state->data->frameCount);

    return STATUS_OK;
}

static const char* img_transformer_dummy_get_message(ImgTransformerHandle handle) {
    img_transformer_dummy_t* state = (img_transformer_dummy_t*)handle;
    if (state && state->data)
        return state->data->msg.empty() ? NULL : state->data->msg.c_str();
    else
        return NULL;
}

static ImgTransformerApi img_transformer_dummy_plugin_api = {"dummy",
                                           img_transformer_dummy_get_info,
                                           img_transformer_dummy_get_size,
                                           img_transformer_dummy_init,
                                           img_transformer_dummy_close,
                                           img_transformer_dummy_process,
                                           img_transformer_dummy_get_message};

DLB_EXPORT
ImgTransformerApi* imgTransformerGetApi() {
    return &img_transformer_dummy_plugin_api;
}

DLB_EXPORT
int imgTransformerGetApiVersion(void) {
    return IMG_TRANSFORMER_API_VERSION;
}
