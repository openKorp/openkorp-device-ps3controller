#include <openkorp-message-set.hpp>

/*
 * THIS IS AN AUTO-GENERATED FILE. DO NOT MODIFY AS CHANGES MIGHT BE OVERWRITTEN!
 */
namespace openkorp { namespace proxy {

int32_t PwmMotorRequest::ID() {
    return 1000;
}

const std::string PwmMotorRequest::ShortName() {
    return "PwmMotorRequest";
}
const std::string PwmMotorRequest::LongName() {
    return "openkorp.proxy.PwmMotorRequest";
}

PwmMotorRequest& PwmMotorRequest::name(const std::string &v) noexcept {
    m_name = v;
    return *this;
}
std::string PwmMotorRequest::name() const noexcept {
    return m_name;
}

PwmMotorRequest& PwmMotorRequest::power(const double &v) noexcept {
    m_power = v;
    return *this;
}
double PwmMotorRequest::power() const noexcept {
    return m_power;
}

}}

#include <openkorp-message-set.hpp>

/*
 * THIS IS AN AUTO-GENERATED FILE. DO NOT MODIFY AS CHANGES MIGHT BE OVERWRITTEN!
 */
namespace openkorp { namespace logic {

int32_t State::ID() {
    return 1010;
}

const std::string State::ShortName() {
    return "State";
}
const std::string State::LongName() {
    return "openkorp.logic.State";
}

State& State::roll(const double &v) noexcept {
    m_roll = v;
    return *this;
}
double State::roll() const noexcept {
    return m_roll;
}

State& State::pitch(const double &v) noexcept {
    m_pitch = v;
    return *this;
}
double State::pitch() const noexcept {
    return m_pitch;
}

State& State::rollSpeed(const double &v) noexcept {
    m_rollSpeed = v;
    return *this;
}
double State::rollSpeed() const noexcept {
    return m_rollSpeed;
}

State& State::pitchSpeed(const double &v) noexcept {
    m_pitchSpeed = v;
    return *this;
}
double State::pitchSpeed() const noexcept {
    return m_pitchSpeed;
}

}}

#include <openkorp-message-set.hpp>

/*
 * THIS IS AN AUTO-GENERATED FILE. DO NOT MODIFY AS CHANGES MIGHT BE OVERWRITTEN!
 */
namespace openkorp { namespace logic {

int32_t StateRequest::ID() {
    return 1011;
}

const std::string StateRequest::ShortName() {
    return "StateRequest";
}
const std::string StateRequest::LongName() {
    return "openkorp.logic.StateRequest";
}

StateRequest& StateRequest::baseThrust(const double &v) noexcept {
    m_baseThrust = v;
    return *this;
}
double StateRequest::baseThrust() const noexcept {
    return m_baseThrust;
}

StateRequest& StateRequest::roll(const double &v) noexcept {
    m_roll = v;
    return *this;
}
double StateRequest::roll() const noexcept {
    return m_roll;
}

StateRequest& StateRequest::pitch(const double &v) noexcept {
    m_pitch = v;
    return *this;
}
double StateRequest::pitch() const noexcept {
    return m_pitch;
}

StateRequest& StateRequest::yawSpeed(const double &v) noexcept {
    m_yawSpeed = v;
    return *this;
}
double StateRequest::yawSpeed() const noexcept {
    return m_yawSpeed;
}

StateRequest& StateRequest::rollTrim(const double &v) noexcept {
    m_rollTrim = v;
    return *this;
}
double StateRequest::rollTrim() const noexcept {
    return m_rollTrim;
}

StateRequest& StateRequest::pitchTrim(const double &v) noexcept {
    m_pitchTrim = v;
    return *this;
}
double StateRequest::pitchTrim() const noexcept {
    return m_pitchTrim;
}

}}

