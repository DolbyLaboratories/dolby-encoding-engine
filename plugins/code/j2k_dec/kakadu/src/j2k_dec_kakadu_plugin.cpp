#include "j2k_dec_kakadu.h"

static
Status
kakadu_set_property
    (J2kDecHandle          /**< [in/out] Decoder instance handle */
    , const Property*        /**< [in] Property to write */
    )
{
    return STATUS_ERROR;
}

static
J2kDecApi kakadu_plugin_api =
{
    "kakadu"
    ,kakadu_get_info
    ,kakadu_get_size
    ,kakadu_init
    ,kakadu_close
    ,kakadu_process
    ,kakadu_set_property
    ,kakadu_get_property
    ,kakadu_get_message
};

DLB_EXPORT
J2kDecApi* j2kDecGetApi()
{
    return &kakadu_plugin_api;
}

DLB_EXPORT
int j2kDecGetApiVersion(void)
{
    return J2K_DEC_API_VERSION;
}
