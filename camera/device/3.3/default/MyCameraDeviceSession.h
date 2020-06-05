#ifndef VENDOR_MASAKI_HARDWARE_CAMERA_DEVICE_V3_3_MYCAMERADEVICESESSION_H
#define VENDOR_MASAKI_HARDWARE_CAMERA_DEVICE_V3_3_MYCAMERADEVICESESSION_H

#include <vendor/masaki/hardware/camera/device/3.3/IMyCameraDevice.h>
#include <vendor/masaki/hardware/camera/device/3.3/IMyCameraDeviceSession.h>
#include <vendor/masaki/hardware/camera/device/3.3/IMyCameraDeviceCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <list>
#include "utils/Thread.h"

// #include "HandleImporter.h"

namespace vendor {
namespace masaki {
namespace hardware {
namespace camera {
namespace device {
namespace V3_3 {
namespace implementation {

// using ::android::hardware::camera::common::V1_0::helper::HandleImporter;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct MyCameraDeviceSession : public IMyCameraDeviceSession {

    MyCameraDeviceSession(const sp<android::hardware::camera::device::V3_2::ICameraDeviceCallback>&, int fd);
    // Methods from ::android::hardware::camera::device::V3_2::ICameraDeviceSession follow.
    Return<void> constructDefaultRequestSettings(::android::hardware::camera::device::V3_2::RequestTemplate type, constructDefaultRequestSettings_cb _hidl_cb) override;
    Return<void> configureStreams(const ::android::hardware::camera::device::V3_2::StreamConfiguration& requestedConfiguration, configureStreams_cb _hidl_cb) override;
    Return<void> processCaptureRequest(const hidl_vec<::android::hardware::camera::device::V3_2::CaptureRequest>& requests, const hidl_vec<::android::hardware::camera::device::V3_2::BufferCache>& cachesToRemove, processCaptureRequest_cb _hidl_cb) override;
    Return<void> getCaptureRequestMetadataQueue(getCaptureRequestMetadataQueue_cb _hidl_cb) override;
    Return<void> getCaptureResultMetadataQueue(getCaptureResultMetadataQueue_cb _hidl_cb) override;
    Return<::android::hardware::camera::common::V1_0::Status> flush() override;
    Return<void> close() override;

    // Methods from ::android::hardware::camera::device::V3_3::ICameraDeviceSession follow.
    Return<void> configureStreams_3_3(const ::android::hardware::camera::device::V3_2::StreamConfiguration& requestedConfiguration, configureStreams_3_3_cb _hidl_cb) override;

    void initializeOutputThread();
    void closeOutputThread();

    class OutputThread : public android::Thread{
    public:
        OutputThread(IMyCameraDeviceCallback::ICameraDeviceCallback* callback);
        ~OutputThread();
        ::android::hardware::camera::common::V1_0::Status request(uint32_t frameNumber, const android::hardware::hidl_handle* handle, ::android::hardware::camera::device::V3_2::CameraMetadata metadata, bool flush);

        virtual bool threadLoop() override;
    
    private:
        std::vector<::android::hardware::camera::device::V3_2::CaptureResult> requestList;
        std::vector<hidl_vec<::android::hardware::camera::device::V3_2::CaptureResult>> resultList;
        android::hardware::camera::device::V3_2::ICameraDeviceCallback* callback;

        uint32_t partialResult;
        std::mutex mutex;
    };

    // Methods from ::android::hidl::base::V1_0::IBase follow.
private:
    int allocateBuffer();
    int releaseBuffer();
    int startCapture();
    int stopCapture();

    int acquireFrame(void *addr);
    int notifyCaptureResult(uint32_t frameNumber,  const android::hardware::hidl_handle* handle);

    const static int BUFFER_MAX;

    const static int FRAMERATE;

    // static HandleImporter sHandleImporter;

    /* callback to client */
    sp<android::hardware::camera::device::V3_2::ICameraDeviceCallback> callback;

    ::android::hardware::camera::device::V3_2::CameraMetadata  metadata;

    sp<OutputThread> outputThread;

    /* camera device file descriptor */
    int fd;
    int width;
    int height;
    /* camera data buffer */
    void ** buffer;
    /* the number of queued buffer */
    int queuedBuffer;
    /* the number of dequeued buffer */
    int dequeuedBuffer;
    /* start or stop capturing */
    bool isCapturing;
};

// FIXME: most likely delete, this is only for passthrough implementations
// extern "C" IMyCameraDeviceSession* HIDL_FETCH_IMyCameraDeviceSession(const char* name);

}  // namespace implementation
}  // namespace V3_3
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace masaki
}  // namespace vendor

#endif  // VENDOR_MASAKI_HARDWARE_CAMERA_DEVICE_V3_3_MYCAMERADEVICESESSION_H
