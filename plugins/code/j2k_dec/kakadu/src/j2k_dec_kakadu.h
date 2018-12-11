#include "j2k_dec_api.h"

size_t
kakadu_get_info
    (const PropertyInfo** info);

size_t
kakadu_get_size();

Status
kakadu_init
    (J2kDecHandle               handle          /**< [in/out] Decoder instance handle */
    ,const J2kDecInitParams*    init_params     /**< [in] Properties to init decoder instance */
    );

Status
kakadu_close
    (J2kDecHandle handle
    );

Status
kakadu_process
    (J2kDecHandle           handle  /**< [in/out] Decoder instance handle */
    ,const J2kDecInput*     in      /**< [in] Encoded input */
    ,J2kDecOutput*          out     /**< [out] Decoded output */
    );

Status
kakadu_get_property
    (J2kDecHandle                /**< [in/out] Decoder instance handle */
    , Property*                  /**< [in/out] Property to read */
    );

const char*
kakadu_get_message
    (J2kDecHandle handle        /**< [in/out] Decoder instance handle */
    );
