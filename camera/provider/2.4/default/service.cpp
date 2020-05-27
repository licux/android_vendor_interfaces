#include <android/log.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <android/hardware/camera/provider/2.4/ICameraProvider.h>
#include "MyCameraProvider.h"

#define LOGTAG "MyCameraProvider"
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, LOGTAG, __VA_ARGS__))
#define LOGD(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, LOGTAG, __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, LOGTAG, __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, LOGTAG, __VA_ARGS__))

using ::android::hardware::configureRpcThreadpool;
using ::android::hardware::joinRpcThreadpool;
using ::android::sp;


int main()
{
    sp<android::hardware::camera::provider::V2_4::ICameraProvider> cp = new vendor::masaki::hardware::camera::provider::V2_4::implementation::MyCameraProvider();
    configureRpcThreadpool(6, true);
    if(cp->registerAsService("mycamera") != android::OK){
        LOGE("Could register service!");
        return 1;
    }
    joinRpcThreadpool();

    return 1;
}
