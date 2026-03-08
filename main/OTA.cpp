#include "OTA.hpp"
#include "HTTPClient.hpp"

#include <esp_https_ota.h>
#include <esp_log.h>
#include <esp_system.h>
#include <mbedtls/sha256.h>

#include <cstring>

static constexpr char TAG[] = "OTA";

OTA::OTA(std::string_view version_json_url)
    : _version_json_url(version_json_url)
{
    _current_version = "1.0.0";
}

const char* OTA::get_current_version() const
{
    return _current_version.c_str();
}

const char* OTA::status_to_string(UpdateStatus status) const
{
    switch (status)
    {
        case UpdateStatus::UpToDate:
            return "Up to date";
        case UpdateStatus::UpdateAvailable:
            return "Update available";
        case UpdateStatus::Downloading:
            return "Downloading firmware...";
        case UpdateStatus::Verifying:
            return "Verifying firmware...";
        case UpdateStatus::Rebooting:
            return "Rebooting...";
        case UpdateStatus::Failed:
            return "Failed";
        default:
            return "Unknown";
    }
}

OTA::VersionInfo OTA::parse_version_json(const json& version_json)
{
    VersionInfo info = {};

    try
    {
        if (version_json.contains("version") && version_json["version"].is_string())
        {
            info.version = version_json["version"].get<std::string>();
        }

        if (version_json.contains("url") && version_json["url"].is_string())
        {
            info.url = version_json["url"].get<std::string>();
        }

        if (version_json.contains("sha256") && version_json["sha256"].is_string())
        {
            info.sha256 = version_json["sha256"].get<std::string>();
        }
    }
    catch (...)
    {
        ESP_LOGE(TAG, "Failed to parse version.json");
    }

    return info;
}

OTA::VersionInfo OTA::fetch_version_info()
{
    VersionInfo info = {};

    try
    {
        HTTPClient client;
        json json_str = client.get_json(_version_json_url);
        info = parse_version_json(json_str);
    }
    catch (const esp_err_t& err)
    {
        ESP_LOGE(TAG, "Failed to fetch version info: %s", esp_err_to_name(err));
    }
    catch (...)
    {
        ESP_LOGE(TAG, "Failed to fetch version info: unknown error");
    }

    return info;
}

bool OTA::verify_sha256(const std::string& expected_sha256)
{
    if (expected_sha256.empty())
    {
        ESP_LOGW(TAG, "No SHA256 provided, skipping verification");
        return true;
    }

    const esp_partition_t* running = esp_ota_get_running_partition();
    if (running == nullptr)
    {
        ESP_LOGE(TAG, "Failed to get running partition");
        return false;
    }

    uint8_t hash[32];
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    mbedtls_sha256_update(&ctx, reinterpret_cast<const uint8_t*>(running->address), 4096);
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);

    char hash_str[65];
    for (int i = 0; i < 32; i++)
    {
        sprintf(hash_str + (i * 2), "%02x", hash[i]);
    }
    hash_str[64] = '\0';

    bool matches = (strncmp(hash_str, expected_sha256.c_str(), 64) == 0);

    if (!matches)
    {
        ESP_LOGE(TAG, "SHA256 mismatch!");
        ESP_LOGE(TAG, "Expected: %s", expected_sha256.c_str());
        ESP_LOGE(TAG, "Got:      %s", hash_str);
    }
    else
    {
        ESP_LOGI(TAG, "SHA256 verified successfully");
    }

    return matches;
}

esp_err_t OTA::perform_ota(const std::string& firmware_url)
{
    ESP_LOGI(TAG, "Starting OTA update from: %s", firmware_url.c_str());

    esp_http_client_config_t config = {
        .url = firmware_url.c_str(),
        .skip_cert_common_name_check = true,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };

    esp_https_ota_handle_t handle = nullptr;
    esp_err_t err = esp_https_ota_begin(&ota_config, &handle);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "OTA begin failed: %s", esp_err_to_name(err));
        return err;
    }

    err = esp_https_ota_perform(handle);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "OTA perform failed: %s", esp_err_to_name(err));
        esp_https_ota_abort(handle);
        return err;
    }

    esp_https_ota_abort(handle);

    int dl_size = esp_https_ota_get_image_len_read(handle);
    ESP_LOGI(TAG, "Total OTA image size: %d bytes", dl_size);

    return ESP_OK;
}

OTA::UpdateStatus OTA::check_and_update()
{
    _last_status = UpdateStatus::Failed;

    VersionInfo remote_info = fetch_version_info();

    if (remote_info.version.empty() || remote_info.url.empty())
    {
        ESP_LOGE(TAG, "Invalid version info from server");
        _last_status = UpdateStatus::Failed;
        return _last_status;
    }

    ESP_LOGI(TAG, "Current version: %s", _current_version.c_str());
    ESP_LOGI(TAG, "Remote version: %s", remote_info.version.c_str());

    if (_current_version == remote_info.version)
    {
        ESP_LOGI(TAG, "Already up to date");
        _last_status = UpdateStatus::UpToDate;
        return _last_status;
    }

    _last_status = UpdateStatus::UpdateAvailable;
    ESP_LOGI(TAG, "Update available! Downloading...");

    _last_status = UpdateStatus::Downloading;
    esp_err_t err = perform_ota(remote_info.url);

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "OTA failed: %s", esp_err_to_name(err));
        _last_status = UpdateStatus::Failed;
        return _last_status;
    }

    _last_status = UpdateStatus::Verifying;
    if (!remote_info.sha256.empty())
    {
        ESP_LOGI(TAG, "Verifying SHA256...");
        if (!verify_sha256(remote_info.sha256))
        {
            ESP_LOGE(TAG, "SHA256 verification failed!");
            _last_status = UpdateStatus::Failed;
            return _last_status;
        }
    }

    _last_status = UpdateStatus::Rebooting;
    ESP_LOGI(TAG, "Rebooting to apply update...");

    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();

    return _last_status;
}
