//
// Date       : 12/04/2026
// Project    : world_exporter
// Author     : -Ry
//

#include <format>
#include <string>
#include <vector>
#include <iostream>

#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#define LOG_INFO(...) (std::cout << std::format("[INFO] {}\n", std::format(__VA_ARGS__)))

glm::vec3 QuaternionToEulerBlender(const glm::quat& q) {
    // Convert quaternion to rotation matrix
    glm::mat3 m = glm::mat3_cast(q);

    float sy = -m[2][0];  // -Zx
    float cy = glm::sqrt(1.0F - sy * sy);

    float x, y, z;  // Blender's X=Pitch, Y=Yaw, Z=Roll

    if (cy > 1e-6f) {
        x = std::atan2(m[2][1], m[2][2]);
        y = std::asin(sy);
        z = std::atan2(m[1][0], m[0][0]);
    } else {
        // Gimbal lock
        x = std::atan2(-m[1][2], m[1][1]);
        y = std::asin(sy);
        z = 0;
    }

    return glm::degrees(glm::vec3(x, y, z));
}


glm::quat swizzle(const glm::quat& in) {
    return glm::quat{in.w, -in.x, in.z, -in.y};
}

glm::quat swizzle2(const glm::quat& in) {
    return glm::quat{in.w, -in.x, -in.z, in.y};
}

void print_as_blender_xyz(const glm::quat& in) {
    glm::quat q = swizzle(in);
    glm::vec3 e = QuaternionToEulerBlender(q);
    glm::vec3 r = glm::degrees(glm::eulerAngles(in));
    LOG_INFO("{:.3f},{:.3f},{:.3f}", e.x, e.y, e.z);
}

void print_quat(const glm::quat& q) {
    LOG_INFO("{:.3f},{:.3f},{:.3f},{:.3f},", q.w, q.x, q.y, q.z);
}

glm::quat make_quat(float* data) {
    return glm::normalize(glm::quat{data[3], data[0], data[1], data[2]});
}

int main() {
    // expected: 44.868,87.803,98.591
    // float data[] = {
    //     0.6774628162384033,
    //     0.6652110815048218,
    //     0.23373188078403473,
    //     0.20954208076000214
    // };

    float data[] = {
        -0.2810690402984619,
        -0.6793162822723389,
        -0.5782889127731323,
        0.3537110984325409
    };

    glm::quat l = make_quat(data);
    glm::quat r = make_quat(data);
    glm::quat q = glm::normalize(l);

    LOG_INFO("Original:");
    print_as_blender_xyz(q);

    {
        glm::quat rz = glm::angleAxis(glm::pi<float>(), glm::vec3{0,0,1});
        q = rz * q;
        q = glm::normalize(q);
        glm::vec3 e = QuaternionToEulerBlender(swizzle(q));
        e.z = -e.z;
        std::swap(e.y, e.z);
        q = glm::quat{glm::radians(e)};
        print_as_blender_xyz(q);
    }
}