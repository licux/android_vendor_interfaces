#include <fcntl.h>
#include <android/log.h>

#include "MyCameraDevice.h"

#define LOGTAG "MyCameraDevice"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOGTAG, __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOGTAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOGTAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOGTAG, __VA_ARGS__))

namespace vendor {
namespace masaki {
namespace hardware {
namespace camera {
namespace device {
namespace V3_3 {
namespace implementation {

using ::android::hardware::camera::common::V1_0::Status;

// Methods from ::android::hardware::camera::device::V3_2::ICameraDevice follow.
Return<void> MyCameraDevice::getResourceCost(getResourceCost_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Return<void> MyCameraDevice::getCameraCharacteristics(getCameraCharacteristics_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Return<::android::hardware::camera::common::V1_0::Status> MyCameraDevice::setTorchMode(::android::hardware::camera::common::V1_0::TorchMode mode) {
    // TODO implement
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<void> MyCameraDevice::open(const sp<::android::hardware::camera::device::V3_2::ICameraDeviceCallback>& callback, open_cb _hidl_cb) {
    LOGD("MyCameraDevice::open()");

    fd = ::open("/dev/video0", O_RDWR);
    if(fd < 0){
        LOGE("Camera Device Open Error!");
        _hidl_cb(Status::CAMERA_DISCONNECTED, nullptr);
    }

    sp<::android::hardware::camera::device::V3_2::ICameraDeviceSession> session;
    session = createSession(callback, fd);
    _hidl_cb(Status::OK, session);
    return Void();
}

Return<void> MyCameraDevice::dumpState(const hidl_handle& fd) {
    // TODO implement
    return Void();
}


sp<::android::hardware::camera::device::V3_2::ICameraDeviceSession> MyCameraDevice::createSession(const sp<android::hardware::camera::device::V3_2::ICameraDeviceCallback>& callback, int fd){
    return new MyCameraDeviceSession(callback, fd);
}
// Methods from ::android::hidl::base::V1_0::IBase follow.

//IMyCameraDevice* HIDL_FETCH_IMyCameraDevice(const char* /* name */) {
//    return new MyCameraDevice();
//}

}  // namespace implementation
}  // namespace V3_3
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace masaki
}  // namespace vendor
