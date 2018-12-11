#include "j2k_dec_api.h"

size_t
comprimato_get_info
(const PropertyInfo** info);

size_t
comprimato_get_size();

Status
comprimato_init
(J2kDecHandle               handle          /**< [in/out] Decoder instance handle */
, const J2kDecInitParams*   init_params     /**< [in] Properties to init decoder instance */
);

Status
comprimato_close
(J2kDecHandle handle
);

Status
comprimato_process
(J2kDecHandle            handle  /**< [in/out] Decoder instance handle */
, const J2kDecInput*     in      /**< [in] Encoded input */
, J2kDecOutput*          out     /**< [out] Decoded output */
);

Status
comprimato_set_property
(J2kDecHandle         /**< [in/out] Decoder instance handle */
, const Property*     /**< [in] Property to write */
);

Status
comprimato_get_property
(J2kDecHandle                /**< [in/out] Decoder instance handle */
, Property*                  /**< [in/out] Property to read */
);

const char*
comprimato_get_message
(J2kDecHandle handle        /**< [in/out] Decoder instance handle */
);




