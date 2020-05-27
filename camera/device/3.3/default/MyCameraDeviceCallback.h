#ifndef VENDOR_MASAKI_HARDWARE_CAMERA_DEVICE_V3_3_MYCAMERADEVICECALLBACK_H
#define VENDOR_MASAKI_HARDWARE_CAMERA_DEVICE_V3_3_MYCAMERADEVICECALLBACK_H

#include <vendor/masaki/hardware/camera/device/3.3/IMyCameraDeviceCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace masaki {
namespace hardware {
namespace camera {
namespace device {
namespace V3_3 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct MyCameraDeviceCallback : public IMyCameraDeviceCallback {
    // Methods from ::android::hardware::camera::device::V3_2::ICameraDeviceCallback follow.
    Return<void> processCaptureResult(const hidl_vec<::android::hardware::camera::device::V3_2::CaptureResult>& results) override;
    Return<void> notify(const hidl_vec<::android::hardware::camera::device::V3_2::NotifyMsg>& msgs) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IMyCameraDeviceCallback* HIDL_FETCH_IMyCameraDeviceCallback(const char* name);

}  // namespace implementation
}  // namespace V3_3
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace masaki
}  // namespace vendor

#endif  // VENDOR_MASAKI_HARDWARE_CAMERA_DEVICE_V3_3_MYCAMERADEVICECALLBACK_H
