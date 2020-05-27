#include <android/log.h>
#include <ui/GraphicBuffer.h>
#include <ui/GraphicBufferMapper.h>
#include <grallocusage/GrallocUsageConversion.h>


#include "CameraMetadata.h"
#include "MyCameraDeviceCallback.h"

#include "png.h"

#define LOGTAG "MyCameraDeviceCallback"
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

// Methods from ::android::hardware::camera::device::V3_2::ICameraDeviceCallback follow.
Return<void> MyCameraDeviceCallback::processCaptureResult(const hidl_vec<::android::hardware::camera::device::V3_2::CaptureResult>& results) {
    LOGD("processCaptureResult is called!");
    
    int width = 1280;
    int height = 720;

    png_structp png;
    png_infop info;
    png_bytepp datap;
    png_byte type;

    png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info = png_create_info_struct(png);

    type = PNG_COLOR_TYPE_RGB;

    FILE *file = fopen("/data/local/tmp/test.png", "wb");
    if(file == NULL){
        LOGE("File was not opened!");
        return Void();
    }

    unsigned char *addr;
    buffer_handle_t handle;
    // android::GraphicBufferMapper::get().importBuffer(requests[0].outputBuffers[0].buffer, width, height, 1, android::PIXEL_FORMAT_RGB_888, android_convertGralloc1To0Usage(android::GraphicBuffer::USAGE_SW_WRITE_OFTEN, android::GraphicBuffer::USAGE_SW_READ_OFTEN),
    //     width, handle)
    android::GraphicBufferMapper::get().importBuffer(results[0].outputBuffers[0].buffer, &handle);
    android::GraphicBufferMapper::get().lock(handle, android_convertGralloc1To0Usage(android::GraphicBuffer::USAGE_SW_WRITE_OFTEN, android::GraphicBuffer::USAGE_SW_READ_OFTEN), 
                                                android::Rect(width, height), (void**)&addr);


    png_init_io(png, file);
    png_set_IHDR(png, info, width, height, 8, type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    datap = (png_bytep*)png_malloc(png, sizeof(png_bytep) * height);

    png_set_rows(png, info, datap);

    unsigned char *image = new unsigned char[width * height * 3];
    for(int i = 0; i < width * height * 2 / 4; i++){
        int y = addr[i] << 8;

        int u = addr[i * 4 + 1] - 128;
        int v = addr[i * 4 + 3] - 128;

        int r = (y + (359 * v)) >> 8;
        int g = (y - (88 * u) - (183 * v)) >> 8;
        int b = (y + (454 * u)) >> 8;
        image[i * 6] = r; 
        image[i * 6 + 1] = g;
        image[i * 6 + 2] = b;

        y = addr[i * 4 + 2] << 8;

        r = (y + (359 * v)) >> 8;
        g = (y - (88 * u) - (183 * v)) >> 8;
        b = (y + (454 * u)) >> 8;
        image[i * 6 + 3] = r; 
        image[i * 6 + 4] = g;
        image[i * 6 + 5] = b;
    }
    // for(int i = 0; i < width * height; i += 2){
    //     unsigned short value = ((unsigned short)addr[i * 2 + 1] << 8) + addr[i * 2];
    //     unsigned char r = (value & 0xf800) >> 11;
    //     unsigned char g = (value & 0x07e0) >> 5;
    //     unsigned char b = value & 0x001f;
    //     r *= 8;
    //     g *= 4;
    //     b *= 8;
    //     image[i * 3] = r ;
    //     image[i * 3 + 1] = g;
    //     image[i * 3 + 2] = b;
    // }
    for(int i = 0; i < height; i++){
        datap[i] = (png_bytep)png_malloc(png, width * /* ch */ 3 );
        // memcpy(datap[i], &addr[i * width * /* ch */ 3], width * /* ch */ 3);
        memcpy(datap[i], &image[i * width * /* ch */ 3], width * /* ch */ 3);
    }
    delete[] image;
    png_write_png(png, info, PNG_TRANSFORM_IDENTITY, NULL);
    LOGD("png write finish!");

    for(int i = 0; i < height; i++){
        png_free(png, datap[i]);
    }
    png_free(png, datap);

    png_destroy_write_struct(&png, &info);
    fclose(file);

    return Void();
}

Return<void> MyCameraDeviceCallback::notify(const hidl_vec<::android::hardware::camera::device::V3_2::NotifyMsg>& msgs) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

//IMyCameraDeviceCallback* HIDL_FETCH_IMyCameraDeviceCallback(const char* /* name */) {
//    return new MyCameraDeviceCallback();
//}

}  // namespace implementation
}  // namespace V3_3
}  // namespace device
}  // namespace camera
}  // namespace hardware
}  // namespace masaki
}  // namespace vendor
