#include "j2k_dec_comprimato.h"

Status
comprimato_set_property
(J2kDecHandle         /**< [in/out] Decoder instance handle */
, const Property*     /**< [in] Property to write */
)
{
    return STATUS_ERROR;
}

static
J2kDecApi comprimato_plugin_api =
{
    "comprimato"
    , comprimato_get_info
    , comprimato_get_size
    , comprimato_init
    , comprimato_close
    , comprimato_process
    , comprimato_set_property
    , comprimato_get_property
    , comprimato_get_message
};

DLB_EXPORT
J2kDecApi* j2kDecGetApi()
{
    return &comprimato_plugin_api;
}

DLB_EXPORT
int j2kDecGetApiVersion(void)
{
    return J2K_DEC_API_VERSION;
}
