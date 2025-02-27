/**
    Copyright Notice:
    Copyright 2021 DMTF, Componolit. All rights reserved.
    License: BSD 3-Clause License. For full text see link: https://github.com/DMTF/libspdm/blob/main/LICENSE.md
**/

#include "hal/base.h"
#include "hal/library/memlib.h"
#include "library/spdm_requester_lib.h"
#include "library/spdm_transport_pcidoe_lib.h"
#include "library/pci_doe_responder_lib.h"

/* Vendor message*/

#pragma pack(1)

/* DOE Discovery request*/

typedef struct {
    pci_doe_data_object_header_t doe_header;
    pci_doe_discovery_request_t doe_discovery_request;
} doe_discovery_request_mine_t;


/* DOE Discovery response*/

typedef struct {
    pci_doe_data_object_header_t doe_header;
    pci_doe_discovery_response_t doe_discovery_response;
} doe_discovery_response_mine_t;

#pragma pack()

pci_doe_data_object_protocol_t m_data_object_protocol[] = {
    {PCI_DOE_VENDOR_ID_PCISIG, PCI_DOE_DATA_OBJECT_TYPE_DOE_DISCOVERY},
    {PCI_DOE_VENDOR_ID_PCISIG, PCI_DOE_DATA_OBJECT_TYPE_SPDM},
    {PCI_DOE_VENDOR_ID_PCISIG, PCI_DOE_DATA_OBJECT_TYPE_SECURED_SPDM},
};

/**
    Process the DOE request and return the response.

    @param request       the PCI_DOE request message, start from pci_doe_data_object_header_t.
    @param request_size  size in bytes of request.
    @param response      the PCI_DOE response message, start from pci_doe_data_object_header_t.
    @param response_size size in bytes of response.

    @retval RETURN_SUCCESS The request is processed and the response is returned.
    @return ERROR          The request is not processed.
**/
return_status pci_doe_get_response_discovery (IN void *pci_doe_context,
    IN void *request, IN uintn request_size,
    IN OUT void *response, IN OUT uintn *response_size)
{
    doe_discovery_request_mine_t *doe_request;
    doe_discovery_response_mine_t doe_response;
    uint8_t index;

    doe_request = (void *)request;
    if (request_size != sizeof(doe_discovery_request_mine_t)) {
        return RETURN_INVALID_PARAMETER;
    }
    if (doe_request->doe_header.length != sizeof(*doe_request) / sizeof(uint32_t)) {
        return RETURN_INVALID_PARAMETER;
    }
    index = doe_request->doe_discovery_request.index;
    if (index >= ARRAY_SIZE(m_data_object_protocol)) {
        return RETURN_INVALID_PARAMETER;
    }

    zero_mem (&doe_response, sizeof(doe_response));
    doe_response.doe_header.vendor_id = PCI_DOE_VENDOR_ID_PCISIG;
    doe_response.doe_header.data_object_type = PCI_DOE_DATA_OBJECT_TYPE_DOE_DISCOVERY;
    doe_response.doe_header.length = sizeof(doe_discovery_response_mine_t) / sizeof(uint32_t);
    doe_response.doe_discovery_response.vendor_id = m_data_object_protocol[index].vendor_id;
    doe_response.doe_discovery_response.data_object_type = m_data_object_protocol[index].data_object_type;
    if (index + 1 == ARRAY_SIZE(m_data_object_protocol)) {
        doe_response.doe_discovery_response.next_index = 0;
    } else {
        doe_response.doe_discovery_response.next_index = index + 1;
    }

    if (*response_size < sizeof(doe_response)) {
        *response_size = sizeof(doe_response);
        return RETURN_BUFFER_TOO_SMALL;
    }
    *response_size = sizeof(doe_response);
    copy_mem (response, (uint8_t *)&doe_response, sizeof(doe_response));

    return RETURN_SUCCESS;
}
