#include "SDPService.hpp"

#include <iostream>
#include <ios>

sdp_session_t* SDPService::initializeService(const std::string& serviceName, const std::string& serviceDescription, 
    const std::string& serviceProvider) {
    sdp_session_t *session = nullptr;
    if(!serviceName.empty() && !serviceDescription.empty() && !serviceProvider.empty()) {
        m_serviceName = serviceName;
        m_serviceDescription = serviceDescription;
        m_serviceProvider = serviceProvider;

        m_l2capList = 0;
        m_rfcommList = 0;
        m_rootList = 0;
        m_protoList = 0;
        m_accessProtoList = 0;
        m_svcClassList = 0;
        m_profileList = 0;

        m_channel = 0;
        m_record = {0};
        session = 0;
    } else {
        // TODO: Log missing information for sdp session
    }

    return session;
}

bool SDPService::registerService(sdp_session_t* session, uint8_t channel) {
    bool serviceRegistered {false};

    // Verify the rfcomm channel is between 0 and 30
    if(channel >= 0 && channel < 31) {
        setServiceId();
        setServiceClass();
        setBluetoothProfileInfo();
        
        // Makes service publicly browsable
        sdp_uuid16_create(&m_rootUUID, PUBLIC_BROWSE_GROUP);
        m_rootList = sdp_list_append(0, &m_rootUUID);
        sdp_set_browse_groups(&m_record, m_rootList);

        setL2CAPInformation(); 
        setRFCOMMChannel(channel);

        m_accessProtoList = sdp_list_append(0, m_protoList);
        sdp_set_access_protos(&m_record, m_accessProtoList);

        setServiceInfo();
        if(connectToSDPServer(&session)) {
            sdp_record_register(session, &m_record, 0);
            serviceRegistered = true;
        }

        cleanUp();
    }

    return serviceRegistered;
}

void SDPService::setServiceId() {
    uint32_t svcUUIDInt[] = { 0x01110000, 0x001000000, 0x80000080, 0xFB349B5F };
    sdp_uuid128_create(&m_svcUUID, &svcUUIDInt);
    sdp_set_service_id(&m_record, m_svcUUID);

    // Logs uuid being registered
    char str[256] = "";
    sdp_uuid2strn(&m_svcUUID, str, 256);
    std::cout << __FILE__ << ":" << __LINE__
        << " INFO: Registering UUID: " << str << "\n";
}

void SDPService::setServiceClass() {
    sdp_uuid16_create(&m_svcClassUUID, SERIAL_PORT_SVCLASS_ID);
    m_svcClassList = sdp_list_append(0, &m_svcClassUUID);
    sdp_set_service_classes(&m_record, m_svcClassList);
}

void SDPService::setBluetoothProfileInfo() {
    sdp_uuid16_create(&m_profile.uuid, SERIAL_PORT_PROFILE_ID);
    m_profile.version = 0x0100;
    m_profileList = sdp_list_append(0, &m_profile);
    sdp_set_profile_descs(&m_record, m_profileList);
}

void SDPService::setL2CAPInformation() {
    sdp_uuid16_create(&m_l2capUUID, L2CAP_UUID);
    m_l2capList = sdp_list_append(0, &m_l2capUUID);
    m_protoList = sdp_list_append(0, m_l2capList);
}

void SDPService::setRFCOMMChannel(uint8_t rfcommChannel) {
    sdp_uuid16_create(&m_rfcommUUID, RFCOMM_UUID);
    m_channel = sdp_data_alloc(SDP_UINT8, &rfcommChannel);
    m_rfcommList = sdp_list_append(0, &m_rfcommUUID);
    sdp_list_append(m_rfcommList, m_channel);
    sdp_list_append(m_protoList, m_rfcommList);
}

void SDPService::setServiceInfo() {
    sdp_set_info_attr(&m_record, m_serviceName.c_str(), m_serviceProvider.c_str(), m_serviceDescription.c_str());
}

bool SDPService::connectToSDPServer(sdp_session_t** session) {
    bool connectedToSDPServer {false};

    *session = sdp_connect(&bdAny, &bdLocal, 0);
    connectedToSDPServer = *session != nullptr;

    std::cout << __FILE__ << ":" << __LINE__ 
        << " INFO: Status of connection: " << std::boolalpha 
        << connectedToSDPServer << "\n";

    return connectedToSDPServer;
}

void SDPService::cleanUp() {
    sdp_data_free(m_channel);
    sdp_list_free(m_l2capList, 0);
    sdp_list_free(m_rfcommList, 0);
    sdp_list_free(m_rootList, 0);
    sdp_list_free(m_accessProtoList, 0);
    sdp_list_free(m_svcClassList, 0);
    sdp_list_free(m_protoList, 0);
    sdp_list_free(m_profileList, 0);
}
