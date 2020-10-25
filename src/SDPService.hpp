#ifndef SDP_SERVICE_HPP
#define SDP_SERVICE_HPP

#include <string>

#include <unistd.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>
#include <bluetooth/rfcomm.h>

#define BDADDR_ANY_INITIALIZER {{0, 0, 0, 0, 0, 0}}
#define BDADDR_ALL_INITIALIZER {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}}
#define BDADDR_LOCAL_INITIALIZER {{0, 0, 0, 0xff, 0xff, 0xff}}

class SDPService {
    public:
        SDPService() = default;
        ~SDPService() = default;

        SDPService(SDPService&& sdpService) = delete;
        SDPService(const SDPService& sdpService) = delete;
        SDPService operator &=(SDPService&& sdpService) = delete;
        SDPService operator &=(const SDPService& sdpService) = delete;

        sdp_session_t* initializeService(const std::string& serviceName, const std::string& serviceDescription, 
                const std::string& serviceProvider);
        bool registerService(sdp_session_t* session, uint8_t rfcommChannel);

        static constexpr const bdaddr_t bdAny = BDADDR_ANY_INITIALIZER;
        static constexpr const bdaddr_t bdLocal = BDADDR_LOCAL_INITIALIZER;
    private:
        void setServiceId();
        void setServiceClass();
        void setBluetoothProfileInfo();
        void setL2CAPInformation();
        void setRFCOMMChannel(uint8_t rfcommChannel);
        void setServiceInfo();
        bool connectToSDPServer(sdp_session_t** session);
        void cleanUp();

    private:
       
        // SDP Service Information
        std::string m_serviceName; 
        std::string m_serviceDescription;
        std::string m_serviceProvider;

        // UUIDs for sdp service
        uuid_t m_rootUUID;
        uuid_t m_l2capUUID;
        uuid_t m_rfcommUUID;
        uuid_t m_svcUUID;
        uuid_t m_svcClassUUID;
        
        sdp_list_t *m_l2capList;
        sdp_list_t *m_rfcommList;
        sdp_list_t *m_rootList;
        sdp_list_t *m_protoList;
        sdp_list_t *m_accessProtoList;
        sdp_list_t *m_svcClassList;
        sdp_list_t *m_profileList;

        sdp_data_t *m_channel;
        sdp_profile_desc_t m_profile;
        sdp_record_t m_record;
};
#endif
