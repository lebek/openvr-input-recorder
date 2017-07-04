#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>
#include <fstream>
#include <signal.h>
#include "generated/ovr_device.pb.h"
#include "generated/recording.pb.h"
#include <openvr.h>
#include <vrinputemulator.h>

float lerp(float a, float b, float alpha) {
	return (1 - alpha)*a + alpha*b;
}

vr::HmdQuaternion_t get_rotation(vr::HmdMatrix34_t matrix) {
	vr::HmdQuaternion_t q;

	q.w = sqrt(fmax(0, 1 + matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = sqrt(fmax(0, 1 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.y = sqrt(fmax(0, 1 - matrix.m[0][0] + matrix.m[1][1] - matrix.m[2][2])) / 2;
	q.z = sqrt(fmax(0, 1 - matrix.m[0][0] - matrix.m[1][1] + matrix.m[2][2])) / 2;
	q.x = copysign(q.x, matrix.m[2][1] - matrix.m[1][2]);
	q.y = copysign(q.y, matrix.m[0][2] - matrix.m[2][0]);
	q.z = copysign(q.z, matrix.m[1][0] - matrix.m[0][1]);
	return q;
}

vr::HmdVector3_t get_position(vr::HmdMatrix34_t matrix) {
	vr::HmdVector3_t vector;

	vector.v[0] = matrix.m[0][3];
	vector.v[1] = matrix.m[1][3];
	vector.v[2] = matrix.m[2][3];

	return vector;
}

void record(int argc, char *argv[]) {
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();

	vr::HmdError err;
	vr::IVRSystem *p = vr::VR_Init(&err, vr::VRApplication_Background);
	if (err != vr::HmdError::VRInitError_None) {
		throw std::runtime_error("HmdError");
	}

	/**** Find devices to record ****/

	Recording recording;
	std::map<int, OVRTimeline *> device_to_timeline;
	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
		auto deviceClass = vr::VRSystem()->GetTrackedDeviceClass(i);
		if (deviceClass == vr::TrackedDeviceClass_Invalid) continue;

		/* Ignore virtual devices */
		std::string prefix = "virtual-";
		char buffer[1024] = { '\0' };
		vr::ETrackedPropertyError error;
		vr::VRSystem()->GetStringTrackedDeviceProperty(i, (vr::ETrackedDeviceProperty)1002, buffer, 1024, &error);
		if (error != vr::TrackedProp_Success || ((std::string)buffer).compare(0, prefix.length(), prefix) == 0) {
			std::cout << "Skipping device: " << i << std::endl;
			continue;
		}

		std::cout << "Serial: " << buffer << std::endl;

		auto device = recording.add_devices();
		device->set_id(i);
		device->set_device_class(deviceClass);
		auto timeline = recording.add_timeline();
		timeline->set_device_id(i);
		device_to_timeline.insert(std::pair<int, OVRTimeline *>(i, timeline));

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
	auto devices = recording.devices();

	std::cout << "Press S to stop recording (max 120 seconds)" << std::endl;

	/**** Record ****/
	auto startTime = std::chrono::system_clock::now();
	vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	while (1) {
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
		auto now = std::chrono::system_clock::now();
		auto timeMillis = std::chrono::duration_cast <std::chrono::milliseconds>(now - startTime).count();
		if (GetAsyncKeyState('S') & 0x8000 || timeMillis > 120 * 1000) break;

		std::map<int, OVRTimeline *>::iterator it;
		for (it = device_to_timeline.begin(); it != device_to_timeline.end(); ++it) {
			auto device = devices.Get(it->first);
			auto timeline = it->second;

			vr::VRSystem()->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseRawAndUncalibrated, 0, m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount);

			auto sample = timeline->add_samples();
			sample->set_time(timeMillis);
			auto position = get_position(m_rTrackedDevicePose[it->first].mDeviceToAbsoluteTracking);
			auto quaternion = get_rotation(m_rTrackedDevicePose[it->first].mDeviceToAbsoluteTracking);

			sample->add_position(position.v[0]);
			sample->add_position(position.v[1]);
			sample->add_position(position.v[2]);

			sample->add_rotation((float)quaternion.w);
			sample->add_rotation((float)quaternion.x);
			sample->add_rotation((float)quaternion.y);
			sample->add_rotation((float)quaternion.z);

			if (device.device_class() == vr::TrackedDeviceClass_Controller) {
				vr::VRControllerState_t controllerState;
				vr::VRSystem()->GetControllerState(it->first, &controllerState, sizeof(controllerState));
				sample->add_axis(controllerState.rAxis[0].x);
				sample->add_axis(controllerState.rAxis[0].y);
				sample->add_axis(controllerState.rAxis[1].x);
				sample->add_axis(controllerState.rAxis[1].y);
				sample->add_axis(controllerState.rAxis[2].x);
				sample->add_axis(controllerState.rAxis[2].y);
				sample->add_axis(controllerState.rAxis[3].x);
				sample->add_axis(controllerState.rAxis[3].y);
				sample->add_axis(controllerState.rAxis[4].x);
				sample->add_axis(controllerState.rAxis[4].y);

				sample->set_button_pressed(controllerState.ulButtonPressed);
				sample->set_button_touched(controllerState.ulButtonTouched);
			}
		}
	}

	inputEmulator.disconnect();

	std::fstream output(argv[2], std::ios::out | std::ios::trunc | std::ios::binary);
	if (!recording.SerializeToOstream(&output)) {
		throw std::runtime_error("Error: Failed to write recording.");
	}
}

bool get_serial_number(OVRDevice device, std::string &serial) {
	auto properties = device.properties();
	
	for (auto it = properties.begin(); it != properties.end(); ++it) {
		if (it->identifier() == 1002) {
			serial = it->string_value();
			return false;
		}
	}

	return true;
}

int get_connected_device(std::string serial_number) {
	vr::ETrackedPropertyError error;
	for (int i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i) {
		char buffer[1024] = { '\0' };
		vr::VRSystem()->GetStringTrackedDeviceProperty(i, (vr::ETrackedDeviceProperty)1002, buffer, 1024, &error);
		if (error == vr::TrackedProp_Success && serial_number == buffer) {
			return i;
		}
	}

	return -1;
}

/* Get or create a virtual device for the given serial, returns the virtual id */
int get_virtual_device(vrinputemulator::VRInputEmulator& inputEmulator, std::string serial) {
	for (int i = 0; i < (int)inputEmulator.getVirtualDeviceCount(); i++) {
		auto info = inputEmulator.getVirtualDeviceInfo(i);
		if (info.deviceSerial == serial) return info.virtualDeviceId;
	}

	return inputEmulator.addVirtualDevice(vrinputemulator::VirtualDeviceType::TrackedController, serial, true);
}

int get_interval(double t, OVRTimeline& timeline) {
	if (t < timeline.samples(0).time()) return 0;

	for (int i = 0; i < timeline.samples().size() - 1; i++) {
		auto sample = timeline.samples(i);
		if (sample.time() > t) {
			return i - 1;
		}
	}

	return timeline.samples().size() - 2;
}

void replay(int argc, char *argv[]) {
	Recording recording;
	std::fstream input(argv[2], std::ios::in | std::ios::binary);

	if (!input) {
		throw std::runtime_error("Error: File not found.");
	}
	else if (!recording.ParseFromIstream(&input)) {
		throw std::runtime_error("Error: Failed to parse recording.");
	}

	int speed = 1;
	if (argc > 3) speed = atoi(argv[3]);

	vr::HmdError err;
	vr::IVRSystem *p = vr::VR_Init(&err, vr::VRApplication_Background);
	if (err != vr::HmdError::VRInitError_None) {
		throw std::runtime_error("HmdError");
	}

	/* Setup virtual devices for each recorded device, and redirect them to original device (if present) */
	auto devices = recording.devices();
	vrinputemulator::VRInputEmulator inputEmulator;
	inputEmulator.connect();
	std::map<int, int> device_to_virtual;
	for (auto it = devices.begin(); it != devices.end(); ++it) {
		if (it->device_class() != vr::TrackedDeviceClass_Controller && it->device_class() != vr::TrackedDeviceClass_HMD) {
			std::cout << "Not controller or HMD" << std::endl;
			continue;
		}

		std::string serial = "";
		if (get_serial_number(*it, serial)) {
			std::cout << "No serial" << std::endl;
			continue;
		}

		// find connected device
		int ovr_id = get_connected_device(serial);
		if (ovr_id == -1) {
			std::cout << serial << " not connected" << std::endl;
			continue;
		}

		std::cout << "Found " << serial << " @ " << ovr_id << std::endl;

		char buffer[256];
		sprintf_s(buffer, sizeof(buffer), "virtual-%s", serial.c_str());
		auto virtual_id = get_virtual_device(inputEmulator, buffer);

		auto properties = it->properties();
		for (auto it2 = properties.begin(); it2 != properties.end(); ++it2) {
			/* Skip serial */
			if (it2->identifier() == 1002) continue;

			if (it2->type() == OVRDeviceProperty_Type::OVRDeviceProperty_Type_String) {
				inputEmulator.setVirtualDeviceProperty(virtual_id, (vr::ETrackedDeviceProperty)it2->identifier(), it2->string_value());
			}
			else if (it2->type() == OVRDeviceProperty_Type::OVRDeviceProperty_Type_Bool) {
				inputEmulator.setVirtualDeviceProperty(virtual_id, (vr::ETrackedDeviceProperty)it2->identifier(), it2->bool_value());
			} 
			else if (it2->type() == OVRDeviceProperty_Type::OVRDeviceProperty_Type_Int32) {
				if (it2->identifier() == 1029) {
					inputEmulator.setVirtualDeviceProperty(virtual_id, (vr::ETrackedDeviceProperty)it2->identifier(), vr::TrackedDeviceClass_Controller);
				}
				else {
					inputEmulator.setVirtualDeviceProperty(virtual_id, (vr::ETrackedDeviceProperty)it2->identifier(), it2->int32_value());
				}
			} 
			else if (it2->type() == OVRDeviceProperty_Type::OVRDeviceProperty_Type_Uint64) {
				inputEmulator.setVirtualDeviceProperty(virtual_id, (vr::ETrackedDeviceProperty)it2->identifier(), it2->uint64_value());
			} 
			else if (it2->type() == OVRDeviceProperty_Type::OVRDeviceProperty_Type_Float) {
				inputEmulator.setVirtualDeviceProperty(virtual_id, (vr::ETrackedDeviceProperty)it2->identifier(), it2->float_value());
			}
			else if (it2->type() == OVRDeviceProperty_Type::OVRDeviceProperty_Type_Matrix34) {
				//inputEmulator.setVirtualDeviceProperty(virtual_id, (vr::ETrackedDeviceProperty)it2->identifier(), it2->matrix34_value());
			}
		}
		inputEmulator.publishVirtualDevice(virtual_id);
		
		auto info = inputEmulator.getVirtualDeviceInfo(virtual_id);

		// Sometimes takes a few ticks for OVR to register the new device... we wait
		while (info.openvrDeviceId > vr::k_unMaxTrackedDeviceCount) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			info = inputEmulator.getVirtualDeviceInfo(virtual_id);
		}

		std::cout << "Forwarding: " << info.openvrDeviceId << " -> " << ovr_id << std::endl;
		inputEmulator.setDeviceNormalMode(info.openvrDeviceId);
		inputEmulator.setDeviceNormalMode(ovr_id);
		inputEmulator.setDeviceRedictMode(info.openvrDeviceId, ovr_id);
		device_to_virtual.insert(std::pair<int, int>(it->id(), virtual_id));
	}

	std::cout << "Press S to stop playback" << std::endl;

	/* Replay  */
	auto timelines = recording.timeline();
	auto startTime = std::chrono::system_clock::now();
	auto last = std::chrono::system_clock::now();
	while (1) {
		if (GetAsyncKeyState('S') & 0x8000) break;
		auto now = std::chrono::system_clock::now();
		auto deltaMillis = std::chrono::duration_cast <std::chrono::milliseconds>(now - last).count();
		auto t = std::chrono::duration_cast <std::chrono::milliseconds>(now - startTime).count() * speed;
		//std::cout << "Delta millis: " << deltaMillis << std::endl;
		last = now;

		/*if (deltaMillis < 5) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5 - (int)deltaMillis));
		}*/

		//std::cout << t << std::endl;
		for (auto it = timelines.begin(); it != timelines.end(); ++it) {
			auto it_virtual_id = device_to_virtual.find(it->device_id());
			if (it_virtual_id == device_to_virtual.end()) continue;
			auto virtual_id = it_virtual_id->second;
			
			// pop sample
			
			int sample_idx = get_interval(t, *it);
			if (sample_idx == it->samples_size() - 2) goto theEnd;
			//auto a = std::chrono::system_clock::now();
			auto sample = it->samples(sample_idx);
			auto sample2 = it->samples(sample_idx+1);
			float alpha = (float)(t - sample.time()) / (float)(sample2.time() - sample.time());
			//std::cout << sample.time()  << " " << alpha << " " << sample2.time() << std::endl;
			//std::cout << sample.position(0) << sample.position(1) << sample.position(2) << std::endl;
			auto pose = inputEmulator.getVirtualDevicePose(virtual_id);
			pose.vecPosition[0] = lerp(sample.position(0), sample2.position(0), alpha);
			pose.vecPosition[1] = lerp(sample.position(1), sample2.position(1), alpha);
			pose.vecPosition[2] = lerp(sample.position(2), sample2.position(2), alpha);
			pose.qRotation.w = lerp(sample.rotation(0), sample2.rotation(0), alpha);
			pose.qRotation.x = lerp(sample.rotation(1), sample2.rotation(1), alpha);
			pose.qRotation.y = lerp(sample.rotation(2), sample2.rotation(2), alpha);
			pose.qRotation.z = lerp(sample.rotation(3), sample2.rotation(3), alpha);
			pose.poseIsValid = true;
			pose.deviceIsConnected = true;
			pose.result = vr::TrackingResult_Running_OK;
			inputEmulator.setVirtualDevicePose(virtual_id, pose, false);
			if (sample.axis_size() > 0) {
				//auto state = inputEmulator.getVirtualControllerState(virtual_id);
				vr::VRControllerState_t state;
				state.ulButtonPressed = sample.button_pressed();
				state.ulButtonTouched = sample.button_touched();
				state.rAxis[0].x = sample.axis(0);
				state.rAxis[0].y = sample.axis(1);
				state.rAxis[1].x = sample.axis(2);
				state.rAxis[1].y = sample.axis(3);
				state.rAxis[2].x = sample.axis(4);
				state.rAxis[2].y = sample.axis(5);
				state.rAxis[3].x = sample.axis(6);
				state.rAxis[3].y = sample.axis(7);
				state.rAxis[4].x = sample.axis(8);
				state.rAxis[4].y = sample.axis(9);
				inputEmulator.setVirtualControllerState(virtual_id, state, false);
			}
			//auto b = std::chrono::system_clock::now();
			//std::cout << std::chrono::duration_cast <std::chrono::milliseconds>(b - a).count() << std::endl;
		}
	}
theEnd:

	for (auto it = device_to_virtual.begin(); it != device_to_virtual.end(); ++it) {
		inputEmulator.setDeviceNormalMode(it->first);
		auto info = inputEmulator.getVirtualDeviceInfo(it->second);
		inputEmulator.setDeviceFakeDisconnectedMode(info.openvrDeviceId);
	}

	inputEmulator.disconnect();
	
	return;
}

int main (int argc, char *argv[])
{
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	int retval = 0;

	if (argc <= 1) {
		std::cout << "Error: No Arguments given." << std::endl;
		exit(1);
	}

	try {
		if (std::strcmp(argv[1], "record") == 0) {
			record(argc, argv);
		} else if (std::strcmp(argv[1], "replay") == 0) {
			replay(argc, argv);
		}
		else {
			throw std::runtime_error("Error: Unknown command.");
		}
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		retval = 3;
	}

	return retval;
}
