#ifndef OPTIQ_MODEL_INTERFACE
#define OPTIQ_MODEL_INTERFACE



struct model_interface {
    void (*optiq_model_generate_data)(request *requests, topology *topology);
}

#endif
