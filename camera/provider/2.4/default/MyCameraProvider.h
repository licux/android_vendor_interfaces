#ifndef VENDOR_MASAKI_HARDWARE_CAMERA_PROVIDER_V2_4_MYCAMERAPROVIDER_H
#define VENDOR_MASAKI_HARDWARE_CAMERA_PROVIDER_V2_4_MYCAMERAPROVIDER_H

#include <regex>
#include <mutex>

#include <vendor/masaki/hardware/camera/provider/2.4/IMyCameraProvider.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <utils/Mutex.h>
#include <utils/Thread.h>

namespace vendor {
namespace masaki {
namespace hardware {
namespace camera {
namespace provider {
namespace V2_4 {
namespace implementation {

using ::android::hardware::camera::provider::V2_4::ICameraProviderCallback;

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct MyCameraProvider : public IMyCameraProvider {
    MyCameraProvider();
    ~MyCameraProvider();

    // Methods from ::android::hardware::camera::provider::V2_4::ICameraProvider follow.
    Return<::android::hardware::camera::common::V1_0::Status> setCallback(const sp<::android::hardware::camera::provider::V2_4::ICameraProviderCallback>& callback) override;
    Return<void> getVendorTags(getVendorTags_cb _hidl_cb) override;
    Return<void> getCameraIdList(getCameraIdList_cb _hidl_cb) override;
    Return<void> isSetTorchModeSupported(isSetTorchModeSupported_cb _hidl_cb) override;
    Return<void> getCameraDeviceInterface_V1_x(const hidl_string& cameraDeviceName, getCameraDeviceInterface_V1_x_cb _hidl_cb) override;
    Return<void> getCameraDeviceInterface_V3_x(const hidl_string& cameraDeviceName, getCameraDeviceInterface_V3_x_cb _hidl_cb) override;

private:
    bool matchDeviceName(const hidl_string& deviceName, std::string* deviceVersion, std::string* cameraId);

    android::Mutex mLock;
    sp<ICameraProviderCallback> mCallback = nullptr;

    const std::regex deviceNameRE;
};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IMyCameraProvider* HIDL_FETCH_IMyCameraProvider(const char* name);

}  // namespace implementation
}  // namespace V2_4
}  // namespace provider
}  // namespace camera
}  // namespace hardware
}  // namespace masaki
}  // namespace vendor

#endif  // VENDOR_MASAKI_HARDWARE_CAMERA_PROVIDER_V2_4_MYCAMERAPROVIDER_H
