#ifndef VENDOR_MASAKI_HARDWARE_CAMERA_DEVICE_V3_3_MYCAMERADEVICE_H
#define VENDOR_MASAKI_HARDWARE_CAMERA_DEVICE_V3_3_MYCAMERADEVICE_H

#include <vendor/masaki/hardware/camera/device/3.3/IMyCameraDevice.h>
#include <vendor/masaki/hardware/camera/device/3.3/IMyCameraDeviceSession.h>
#include <MyCameraDeviceSession.h>
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
using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct MyCameraDevice : public IMyCameraDevice {
    // Methods from ::android::hardware::camera::device::V3_2::ICameraDevice follow.
    Return<void> getResourceCost(getResourceCost_cb _hidl_cb) override;
    Return<void> getCameraCharacteristics(getCameraCharacteristics_cb _hidl_cb) override;
    Return<::android::hardware::camera::common::V1_0::Status> setTorchMode(::android::hardware::camera::common::V1_0::TorchMode mode) override;
    Return<void> open(const sp<::android::hardware::camera::device::V3_2::ICameraDeviceCallback>& callback, open_cb _hidl_cb) override;
    Return<void> dumpState(const hidl_handle& fd) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

private:
    virtual sp<android::hardware::camera::device::V3_2::ICameraDeviceSession> createSession(const sp<android::hardware::camera::device::V3_2::ICameraDeviceCallback>&, int);

    /* camera device file descriptor */
    int fd; 
};


// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IMyCameraDevice* HIDL_FETCH_IMyCameraDevice(const char* name);

}  // namespace implementation
}  // namespace V3_3
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace masaki
}  // namespace vendor

#endif  // VENDOR_MASAKI_HARDWARE_CAMERA_DEVICE_V3_3_MYCAMERADEVICE_H
