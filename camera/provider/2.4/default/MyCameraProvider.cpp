
#include "MyCameraProvider.h"
#include "MyCameraDevice.h"

namespace vendor {
namespace masaki {
namespace hardware {
namespace camera {
namespace provider {
namespace V2_4 {
namespace implementation {

using ::android::hardware::camera::common::V1_0::CameraDeviceStatus;
using ::android::hardware::camera::common::V1_0::Status;
using ::android::hardware::camera::common::V1_0::VendorTagSection;
using ::android::hardware::camera::provider::V2_4::ICameraProvider;
using ::android::hardware::camera::provider::V2_4::ICameraProviderCallback;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;
using ::android::sp;
using ::android::Mutex;

MyCameraProvider::MyCameraProvider()
 : deviceNameRE("device@([0-9]+\\.[0-9]+)/mycamera/(.+)"){
}

MyCameraProvider::~MyCameraProvider(){

}

// Methods from ::android::hardware::camera::provider::V2_4::ICameraProvider follow.
Return<::android::hardware::camera::common::V1_0::Status> MyCameraProvider::setCallback(const sp<::android::hardware::camera::provider::V2_4::ICameraProviderCallback>& callback) {
    android::Mutex::Autolock lock(mLock);
    mCallback = callback;

    return ::android::hardware::camera::common::V1_0::Status::OK;
}

Return<void> MyCameraProvider::getVendorTags(getVendorTags_cb _hidl_cb) {
    hidl_vec<VendorTagSection> sections;

    _hidl_cb(Status::OK, sections);
    return Void();
}

Return<void> MyCameraProvider::getCameraIdList(getCameraIdList_cb _hidl_cb) {
   hidl_vec<hidl_string> deviceList;
   deviceList.resize(1);
   deviceList[0] =  "device@3.2/mycamera/0";
    _hidl_cb(Status::OK, deviceList);

    return Void();
}

Return<void> MyCameraProvider::isSetTorchModeSupported(isSetTorchModeSupported_cb _hidl_cb) {
    _hidl_cb(Status::OK, true);
    return Void();
}

Return<void> MyCameraProvider::getCameraDeviceInterface_V1_x(const hidl_string& cameraDeviceName, getCameraDeviceInterface_V1_x_cb _hidl_cb) {
    _hidl_cb(Status::OPERATION_NOT_SUPPORTED, nullptr);
    return Void();
}

Return<void> MyCameraProvider::getCameraDeviceInterface_V3_x(const hidl_string& cameraDeviceName, getCameraDeviceInterface_V3_x_cb _hidl_cb) {
    
    std::string device_name;

    sp<android::hardware::camera::device::V3_2::ICameraDevice> device;
    device = new ::vendor::masaki::hardware::camera::device::V3_3::implementation::MyCameraDevice();

    _hidl_cb(Status::OK, device);
    return Void();
}


bool MyCameraProvider::matchDeviceName(const hidl_string& deviceName, std::string *deviceVersion, std::string *cameraId){
    std::string tmp(deviceName.c_str());
    std::smatch sm;
    if(std::regex_match(tmp, sm, deviceNameRE)){
        if(deviceVersion != nullptr){
            *deviceVersion = sm[1];
        }
        if(cameraId != nullptr){
            *cameraId = sm[2];
        }
        return true;
    }
    return false;
}


//IMyCameraProvider* HIDL_FETCH_IMyCameraProvider(const char* /* name */) {
//    return new MyCameraProvider();
//}

}  // namespace implementation
}  // namespace V2_4
}  // namespace provider
}  // namespace camera
}  // namespace hardware
}  // namespace masaki
}  // namespace vendor
