/**
    Copyright Notice:
    Copyright 2021 DMTF, Componolit. All rights reserved.
    License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
**/

#include "hal/base.h"
#include "hal/library/memlib.h"
#include "library/spdm_requester_lib.h"
#include "library/spdm_transport_mctp_lib.h"
#include "library/mctp_responder_lib.h"

typedef struct {
    mctp_message_header_t                  header;
    mctp_get_secured_app_request_func_t    func;
} mctp_secured_app_dispatch_struct_t;

mctp_secured_app_dispatch_struct_t m_mctp_secured_app_dispatch[] = {
   {{MCTP_MESSAGE_TYPE_PLDM}, pldm_get_response_secured_app_request },
};

/**
    Process the MCTP request and return the response.

    @param request       the MCTP request message, start from mctp_message_header_t.
    @param request_size  size in bytes of request.
    @param response      the MCTP response message, start from mctp_message_header_t.
    @param response_size size in bytes of response.

    @retval RETURN_SUCCESS The request is processed and the response is returned.
    @return ERROR          The request is not processed.
**/
return_status mctp_get_response_secured_app_request(IN void *mctp_context,
    IN void *spdm_context, IN uint32_t *session_id,
    IN void *request, IN uintn request_size,
    OUT void *response, IN OUT uintn *response_size)
{
    mctp_message_header_t *app_request;
    mctp_message_header_t *app_response;
    uintn index;
    uintn app_response_size;
    return_status status;

    app_request = request;
    app_response = response;
    if (request_size < sizeof(mctp_message_header_t)) {
        return RETURN_INVALID_PARAMETER;
    }
    ASSERT (*response_size > sizeof(mctp_message_header_t));
    app_response_size = *response_size - sizeof(mctp_message_header_t);

    for (index = 0; index < ARRAY_SIZE(m_mctp_secured_app_dispatch); index++) {
        if (app_request->message_type == m_mctp_secured_app_dispatch[index].header.message_type) {
            status = m_mctp_secured_app_dispatch[index].func (
                        mctp_context, spdm_context, session_id,
                        (uint8_t *)request + sizeof(mctp_message_header_t),
                        request_size - sizeof(mctp_message_header_t),
                        (uint8_t *)response + sizeof(mctp_message_header_t),
                        &app_response_size
                        );
            if (RETURN_ERROR(status)) {
                return status;
            }

            zero_mem (app_response, sizeof(mctp_message_header_t));
            app_response->message_type = app_request->message_type;

            *response_size = app_response_size + sizeof(mctp_message_header_t);

            return RETURN_SUCCESS;
        }
    }

    return RETURN_INVALID_PARAMETER;
}
