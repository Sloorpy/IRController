#pragma once

#include <string>
#include <string_view>

#include <esp_err.h>
#include <esp_ota_ops.h>
#include <json.hpp>

#include "Macros.hpp"

using json = nlohmann::json;

class OTA
{
public:
    enum class UpdateStatus
    {
        UpToDate,
        UpdateAvailable,
        Downloading,
        Verifying,
        Rebooting,
        Failed
    };

    struct VersionInfo
    {
        std::string version;
        std::string url;
        std::string sha256;
    };

    explicit OTA(std::string_view version_json_url);
    ~OTA() = default;

    DELETE_COPY_MOVE(OTA)

    UpdateStatus check_and_update();
    VersionInfo fetch_version_info();
    const char* status_to_string(UpdateStatus status) const;
    const char* get_current_version() const;

private:
    VersionInfo parse_version_json(const json& json_str);
    bool verify_sha256(const std::string& expected_sha256);
    esp_err_t perform_ota(const std::string& firmware_url);

    std::string _version_json_url;
    std::string _current_version;
    UpdateStatus _last_status = UpdateStatus::UpToDate;
};
