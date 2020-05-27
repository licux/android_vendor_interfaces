#include <linux/videodev2.h>
#include <sys/mman.h>
#include <android/log.h>
#include <ui/GraphicBuffer.h>
#include <ui/GraphicBufferMapper.h>
#include <grallocusage/GrallocUsageConversion.h>


#include "CameraMetadata.h"
#include "MyCameraDeviceSession.h"

#define LOGTAG "MyCameraDeviceSession"
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

using ::android::hardware::hidl_handle;
using ::android::hardware::hidl_vec;
using ::android::hardware::camera::common::V1_0::Status;
using ::android::hardware::camera::device::V3_2::BufferStatus;
using ::android::hardware::camera::device::V3_2::BufferCache;
using ::android::hardware::camera::device::V3_2::CaptureResult;
using ::android::hardware::camera::device::V3_2::StreamBuffer;

const int MyCameraDeviceSession::BUFFER_MAX = 4;
const int MyCameraDeviceSession::FRAMERATE = 30;

MyCameraDeviceSession::MyCameraDeviceSession(const sp<android::hardware::camera::device::V3_2::ICameraDeviceCallback>& callback, int fd){
    this->callback = callback;
    this->fd = fd;

    isCapturing = false;
    partialResult = 1;
}

// Methods from ::android::hardware::camera::device::V3_2::ICameraDeviceSession follow.
Return<void> MyCameraDeviceSession::constructDefaultRequestSettings(::android::hardware::camera::device::V3_2::RequestTemplate type, constructDefaultRequestSettings_cb _hidl_cb) {
    LOGD("RequestTemplete:%d", type);
    Status status = Status::OK;
    ::android::hardware::camera::device::V3_2::CameraMetadata metadata;

    v4l2_capability capability;
    if(ioctl(fd, VIDIOC_QUERYCAP, &capability) < 0){
        LOGE("Error open device: unable to query device!");
        _hidl_cb(Status::OPERATION_NOT_SUPPORTED, metadata);
        return Void();
    }

    if((capability.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0){
        LOGE("Error open device: video capture not supported!");
        _hidl_cb(Status::OPERATION_NOT_SUPPORTED, metadata);
        return Void();
    }
    if((capability.capabilities & V4L2_CAP_STREAMING) == 0){
        LOGE("Error open device: streaming not supported!");
        _hidl_cb(Status::OPERATION_NOT_SUPPORTED, metadata);
        return Void();
    }

    // v4l2_format format;
    // if(ioctl(fd, VIDIOC_G_FMT, &format) < 0){
    //     LOGE("Error get format: camera format error!");
    //     _hidl_cb(Status::OPERATION_NOT_SUPPORTED, metadata);
    //     return Void();
    // }
    // LOGD("width:%d, height:%d", format.fmt.pix.width, format.fmt.pix.height);

    _hidl_cb(Status::OK, metadata);
    return Void();
}

Return<void> MyCameraDeviceSession::configureStreams(const ::android::hardware::camera::device::V3_2::StreamConfiguration& requestedConfiguration, configureStreams_cb _hidl_cb) {
    ::android::hardware::camera::device::V3_2::HalStreamConfiguration halStreamConfig;

    // Check operation mode
    if(requestedConfiguration.operationMode != ::android::hardware::camera::device::V3_2::StreamConfigurationMode::NORMAL_MODE){
        LOGE("Requested configuration is not supported! : %d", requestedConfiguration.operationMode);
        _hidl_cb(Status::OPERATION_NOT_SUPPORTED, halStreamConfig);
        return Void();
    }

    LOGD("Requested configuration streams size : %d", requestedConfiguration.streams.size());
    // Check input or output
    if(requestedConfiguration.streams[0].streamType != ::android::hardware::camera::device::V3_2::StreamType::OUTPUT){
        LOGE("Camera HAL supports output only!");
        _hidl_cb(Status::OPERATION_NOT_SUPPORTED, halStreamConfig);
        return Void();
    }

    // Do not check BufferUsageFlags

    // Do not check DataspaceFlags

    // Check rotation
    if(requestedConfiguration.streams[0].rotation != ::android::hardware::camera::device::V3_2::StreamRotation::ROTATION_0){
        LOGE("Camera HAL does not support rotation!");
        _hidl_cb(Status::OPERATION_NOT_SUPPORTED, halStreamConfig);
        return Void();
    }

    v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    width = format.fmt.pix.width = requestedConfiguration.streams[0].width;
    height = format.fmt.pix.height = requestedConfiguration.streams[0].height;
 
    switch(requestedConfiguration.streams[0].format){
    case ::android::hardware::graphics::common::V1_0::PixelFormat::RGBA_8888:
        format.fmt.pix.pixelformat = V4L2_PIX_FMT_ARGB32;
        break;
    case ::android::hardware::graphics::common::V1_0::PixelFormat::RGBX_8888:
        format.fmt.pix.pixelformat = V4L2_PIX_FMT_XRGB32;
        break;
    case ::android::hardware::graphics::common::V1_0::PixelFormat::RGB_888:
        format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;
        break;
    case ::android::hardware::graphics::common::V1_0::PixelFormat::RGB_565:
        format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB565;
        break;
    case ::android::hardware::graphics::common::V1_0::PixelFormat::BGRA_8888:
        format.fmt.pix.pixelformat = V4L2_PIX_FMT_ABGR32;
        break;
    default:
        LOGE("Specified pixel format is not supported!");
        _hidl_cb(Status::OPERATION_NOT_SUPPORTED, halStreamConfig);
        return Void();
    }
    
    LOGD("Set format.fmt.pix.pixelformat = %d", format.fmt.pix.pixelformat);
    if(ioctl(fd, VIDIOC_S_FMT, &format) < 0){
        LOGE("Error device setting!");
        _hidl_cb(Status::OPERATION_NOT_SUPPORTED, halStreamConfig);
        return Void();
    }

    // v4l2_format read_fmt;
    // if(ioctl(fd, VIDIOC_G_FMT, &read_fmt) < 0){
    //     LOGE("Error device setting acqureed!");
    //     _hidl_cb(Status::OPERATION_NOT_SUPPORTED, halStreamConfig);
    //     return Void();
    // }
    // LOGD("Get format.fmt.pix.pixelformat = %d", read_fmt.fmt.pix.pixelformat);

    LOGD("Camera device configuration finished!");

    v4l2_streamparm parm;
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_G_PARM, &parm) < 0){
        LOGE("VIDIOC_G_PARM fail!");
        _hidl_cb(Status::INTERNAL_ERROR, halStreamConfig);
        return Void();
    }

    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = FRAMERATE;
    if(ioctl(fd, VIDIOC_S_PARM, &parm) < 0){
        LOGE("VIDIOC_S_PARM fail!");
        _hidl_cb(Status::INTERNAL_ERROR, halStreamConfig);
        return Void();
    }

    if(allocateBuffer() < 0){
        LOGE("allocateBuffer() failed!");
        _hidl_cb(Status::INTERNAL_ERROR, halStreamConfig);
        return Void();
    }

    if(startCapture() < 0){
        LOGE("startCapture() failed!");
        _hidl_cb(Status::INTERNAL_ERROR, halStreamConfig);
        return Void();
    }

    _hidl_cb(Status::OK, halStreamConfig);
    return Void();
}

Return<void> MyCameraDeviceSession::processCaptureRequest(const hidl_vec<::android::hardware::camera::device::V3_2::CaptureRequest>& requests, const hidl_vec<::android::hardware::camera::device::V3_2::BufferCache>& cachesToRemove, processCaptureRequest_cb _hidl_cb) {
    uint32_t numRequestProcessed = 0;

    if(requests[0].fmqSettingsSize == 0){
        metadata = requests[0].settings;
    }

    void* addr;
    buffer_handle_t handle;
    // android::GraphicBufferMapper::get().importBuffer(requests[0].outputBuffers[0].buffer, width, height, 1, android::PIXEL_FORMAT_RGB_888, android_convertGralloc1To0Usage(android::GraphicBuffer::USAGE_SW_WRITE_OFTEN, android::GraphicBuffer::USAGE_SW_READ_OFTEN),
    //     width, handle)
    android::GraphicBufferMapper::get().importBuffer(requests[0].outputBuffers[0].buffer, &handle);
    android::GraphicBufferMapper::get().lock(handle, android_convertGralloc1To0Usage(android::GraphicBuffer::USAGE_SW_WRITE_OFTEN, android::GraphicBuffer::USAGE_SW_READ_OFTEN), 
                                                android::Rect(width, height), &addr);

    if(acquireFrame(addr) < 0){
        LOGE("acquireFrame failed!");
        _hidl_cb(Status::INTERNAL_ERROR, numRequestProcessed);
        return Void();
    }
    numRequestProcessed++;

    android::GraphicBufferMapper::get().unlock(handle);
    android::GraphicBufferMapper::get().freeBuffer(handle);

    notifyCaptureResult(requests[0].frameNumber, &requests[0].outputBuffers[0].buffer);

    _hidl_cb(Status::OK, numRequestProcessed);
    return Void();
}

Return<void> MyCameraDeviceSession::getCaptureRequestMetadataQueue(getCaptureRequestMetadataQueue_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Return<void> MyCameraDeviceSession::getCaptureResultMetadataQueue(getCaptureResultMetadataQueue_cb _hidl_cb) {
    // TODO implement
    return Void();
}

Return<::android::hardware::camera::common::V1_0::Status> MyCameraDeviceSession::flush() {
    // TODO implement
    return ::android::hardware::camera::common::V1_0::Status {};
}

Return<void> MyCameraDeviceSession::close() {
    // TODO implement
    return Void();
}


// Methods from ::android::hardware::camera::device::V3_3::ICameraDeviceSession follow.
Return<void> MyCameraDeviceSession::configureStreams_3_3(const ::android::hardware::camera::device::V3_2::StreamConfiguration& requestedConfiguration, configureStreams_3_3_cb _hidl_cb) {
    // TODO implement
    return Void();
}

int MyCameraDeviceSession::allocateBuffer(){
    v4l2_requestbuffers requestBuffer;
    requestBuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    requestBuffer.memory = V4L2_MEMORY_MMAP;
    requestBuffer.count = BUFFER_MAX;

    if(ioctl(fd, VIDIOC_REQBUFS, &requestBuffer) < 0){
        LOGE("VIDIOC_REQBUFS failed: %s", strerror(errno));
        return -1;
    }

    buffer = new void*[BUFFER_MAX];
    queuedBuffer = 0;
    dequeuedBuffer = 0;

    for(int i = 0; i < BUFFER_MAX; i++){
        v4l2_buffer buf;
        
        memset(&buf, 0, sizeof(struct v4l2_buffer));
        buf.index = i;
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if(ioctl(fd, VIDIOC_QUERYBUF, &buf) < 0){
            LOGE("Unable to query buffer (%s)", strerror(errno));
            return -1;
        }

        buffer[i] = mmap(0, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        LOGD("image buffer length: %d", buf.length);

        if(buffer[i] == MAP_FAILED){
            LOGE("Unable to map buffer (%s)", strerror(errno));
            return -1;
        }

        if(ioctl(fd, VIDIOC_QBUF, &buf) < 0){
            LOGE("VIDIOC_QBUF failed!");
            return -1;
        }
        queuedBuffer++;
    }
    LOGD("Finish to allocate buffers : %d", queuedBuffer);
    return 0;
}

int MyCameraDeviceSession::releaseBuffer(){
    int dequeue = queuedBuffer - dequeuedBuffer;

    v4l2_buffer buf;
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    for(int i = 0; i < dequeue - 1; i++){
        if(ioctl(fd, VIDIOC_DQBUF, &buf) < 0){
            LOGE("releaseBuffer(): dequeue failed");
            return -1;
        }
    }

    queuedBuffer = dequeuedBuffer = 0;

    for(int i = 0; i < BUFFER_MAX; i++){
        if(munmap(buffer[i], buf.length) < 0){
            LOGE("releasebuffer(): unmap failed");
            return -1;
        }
    }
    return 0;
}
    
int MyCameraDeviceSession::startCapture(){
    if(isCapturing == true){
        return -1;
    }

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_STREAMON, &type) < 0){
        LOGE("startCapture(): unable tao start capture: %s", strerror(errno));
        return -1;
    }

    isCapturing = true;
    LOGD("Finish to start capturing");

    return 0;
}
    
int MyCameraDeviceSession::stopCapture(){
    if(isCapturing == false){
        return -1;
    }

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if(ioctl(fd, VIDIOC_STREAMOFF, &type) < 0){
        LOGE("stopCapture(): unable to stop capture: %s", strerror(errno));
        return -1;
    }

    isCapturing = false;
    LOGD("Finish to stop capturing");

    return 0;
}

int MyCameraDeviceSession::acquireFrame(void *addr){
    v4l2_buffer buf;

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if(ioctl(fd, VIDIOC_DQBUF, &buf) < 0){
        LOGE("acquireFrame() dequeue failed!");
        return -1;
    }

    dequeuedBuffer++;

    memcpy(addr, buffer[buf.index], buf.length);

    if(ioctl(fd, VIDIOC_QBUF, &buf) < 0){
        LOGE("acquireFrame() queue failed!");
        return -1;
    }

    queuedBuffer++;

    return 0;
}

int MyCameraDeviceSession::notifyCaptureResult(uint32_t frameNumber, const hidl_handle* handle){
    // struct CaptureResult {
    //     uint32_t frameNumber;
    //     uint64_t fmqResultSize;
    //     CameraMetadata result;
    //     vec<StreamBuffer> outputBuffers;
    //     StreamBuffer inputBuffer;
    //     uint32_t partialResult;
    // };
    // struct StreamBuffer {
    //     int32_t streamId;
    //     uint64_t bufferId;
    //     handle buffer;
    //     BufferStatus status;
    //     handle acquireFence;
    //     handle releaseFence;
    // };
    StreamBuffer outputBuffer = {0, 0, *handle, BufferStatus::OK, nullptr, nullptr};
    hidl_vec<StreamBuffer> outputBuffers = { outputBuffer };
    StreamBuffer emptyInputBuffer = {-1, 0, nullptr, BufferStatus::ERROR, nullptr, nullptr};
    CaptureResult result  = {frameNumber, 0, metadata, outputBuffers, emptyInputBuffer, partialResult++};
    hidl_vec<CaptureResult> results = { result };

    callback->processCaptureResult(results);

    return 0;
}
// Methods from ::android::hidl::base::V1_0::IBase follow.

//IMyCameraDeviceSession* HIDL_FETCH_IMyCameraDeviceSession(const char* /* name */) {
//    return new MyCameraDeviceSession();
//}

}  // namespace implementation
}  // namespace V3_3
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace masaki
}  // namespace vendor
