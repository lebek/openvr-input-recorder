// A simple program that computes the square root of a number
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <openvr.h>
#include <vrinputemulator.h>
#include "generated/ovr_device.pb.h"
#include "generated/recording.pb.h"
#include <fstream>

int main (int argc, char *argv[])
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();

	vr::HmdError err;
	vr::IVRSystem *p = vr::VR_Init(&err, vr::VRApplication_Background);
	if (err != vr::HmdError::VRInitError_None) {
		printf("HmdError");
		return 1;
	}

	Recording recording;

	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
		auto deviceClass = vr::VRSystem()->GetTrackedDeviceClass(i);
		if (deviceClass == vr::TrackedDeviceClass_Invalid) continue;

		auto device = recording.add_devices();
		device->set_device_class(deviceClass);
		if (deviceClass == vr::TrackedDeviceClass_Controller) {
			device->set_controller_role(vr::VRSystem()->GetControllerRoleForTrackedDeviceIndex(i));
		}

		for (int p = 1; p < 20000; p++) {
			vr::ETrackedPropertyError error;
			vr::TrackedDeviceIndex_t deviceId = i;
				
			char buffer[1024] = { '\0' };
			vr::VRSystem()->GetStringTrackedDeviceProperty(deviceId, (vr::ETrackedDeviceProperty)p, buffer, 1024, &error);
			if (error == vr::TrackedProp_Success) {
				auto prop = device->add_properties();
				prop->set_type(OVRDeviceProperty_Type::OVRDeviceProperty_Type_String);
				prop->set_string_value(buffer);
				prop->set_identifier(p);
				continue;
			}

			auto valueInt32 = vr::VRSystem()->GetInt32TrackedDeviceProperty(deviceId, (vr::ETrackedDeviceProperty)p, &error);
			if (error == vr::TrackedProp_Success) {
				auto prop = device->add_properties();
				prop->set_type(OVRDeviceProperty_Type::OVRDeviceProperty_Type_Int32);
				prop->set_int32_value(valueInt32);
				prop->set_identifier(p);
				continue;
			}

			auto valueUint64 = vr::VRSystem()->GetUint64TrackedDeviceProperty(deviceId, (vr::ETrackedDeviceProperty)p, &error);
			if (error == vr::TrackedProp_Success) {
				auto prop = device->add_properties();
				prop->set_type(OVRDeviceProperty_Type::OVRDeviceProperty_Type_Uint64);
				prop->set_uint64_value(valueUint64);
				prop->set_identifier(p);
				continue;
			}

			auto valueBool = vr::VRSystem()->GetBoolTrackedDeviceProperty(deviceId, (vr::ETrackedDeviceProperty)p, &error);
			if (error == vr::TrackedProp_Success) {
				auto prop = device->add_properties();
				prop->set_type(OVRDeviceProperty_Type::OVRDeviceProperty_Type_Bool);
				prop->set_bool_value(valueBool);
				prop->set_identifier(p);
				continue;
			}

			auto valueFloat = vr::VRSystem()->GetFloatTrackedDeviceProperty(deviceId, (vr::ETrackedDeviceProperty)p, &error);
			if (error == vr::TrackedProp_Success) {
				auto prop = device->add_properties();
				prop->set_type(OVRDeviceProperty_Type::OVRDeviceProperty_Type_Float);
				prop->set_float_value(valueFloat);
				prop->set_identifier(p);
				continue;
			}

			auto valueMatrix34 = vr::VRSystem()->GetMatrix34TrackedDeviceProperty(deviceId, (vr::ETrackedDeviceProperty)p, &error);
			if (error == vr::TrackedProp_Success) {
				auto prop = device->add_properties();
				prop->set_type(OVRDeviceProperty_Type::OVRDeviceProperty_Type_Matrix34);

				for (int i = 0; i < 3; ++i) {
					for (int j = 0; j < 4; ++j) {
						prop->add_matrix34_value(valueMatrix34.m[i][j]);
					}
				}

				prop->set_identifier(p);
				continue;
			}
		}
	}

	std::fstream output(argv[1], std::ios::out | std::ios::trunc | std::ios::binary);
	if (!recording.SerializeToOstream(&output)) {
		printf("Failed to write address book.");
		return -1;
	}

  return 0;
}
