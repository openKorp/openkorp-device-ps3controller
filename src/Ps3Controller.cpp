/**
 * ps3controller - Using a Ps3 controller to accelerate, brake, and steer a Drone.
 * Copyright (C) 2017 Björnborg Nguyen
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdint.h>
#include <iostream>
#include <cmath>
#include <unistd.h>
#include <string>
#include <sstream>
#include <memory>

#include "openkorp-message-set.hpp"

#include "Ps3Controller.h"

#if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
#include <linux/joystick.h>
#include <fcntl.h>
#endif


Ps3Controller::Ps3Controller(std::string a_jsnode)
    : m_ps3controllerDevice(0)
    , m_axes() 
    , m_buttons()
    , m_numAxes(0)
    , m_stateReq()
    , m_numButtons(0) 
{
  std::string const Ps3CONTROLLER_DEVICE_NODE = a_jsnode;

  std::cout << " Trying to open ps3controller " 
      << Ps3CONTROLLER_DEVICE_NODE << std::endl;
  
  // Setup ps3controller control.
  #if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
  int16_t name_of_ps3controller[80];

  m_ps3controllerDevice = open(Ps3CONTROLLER_DEVICE_NODE.c_str(), O_RDONLY);
  if (m_ps3controllerDevice == -1) {
    std::cerr << "Could not open ps3controller " 
        << Ps3CONTROLLER_DEVICE_NODE << std::endl;
    exit(1);
  }

  ioctl(m_ps3controllerDevice, (int)JSIOCGAXES, &m_numAxes);
  ioctl(m_ps3controllerDevice, (int)JSIOCGBUTTONS, &m_numButtons);
  ioctl(m_ps3controllerDevice, JSIOCGNAME(80), &name_of_ps3controller);

  m_axes = std::unique_ptr<int32_t[]>((int32_t *)calloc(m_numAxes, sizeof(int32_t)));
  m_buttons = std::unique_ptr<int32_t[]>((int32_t *)calloc(m_numButtons, sizeof(int32_t)));

  std::cout << "Ps3Controller found " << name_of_ps3controller
      << ", number of axes: " << m_numAxes
      << ", number of buttons: " << m_numButtons << std::endl;
  
  // Use non blocking reading.
  fcntl(m_ps3controllerDevice, F_SETFL, O_NONBLOCK);
  
  #else
  std::cout << "This code will not work on this computer architecture." 
      << std::endl;
  #endif
}


Ps3Controller::~Ps3Controller() {
  close(m_ps3controllerDevice);
}



std::string Ps3Controller::toString()
{
  std::stringstream ss;
  ss << "Button\n";
  for (int32_t i = 0; i < m_numButtons; i++) {
    ss << i << ": " << m_buttons[i] << "\n";
  } 
  ss << "Axes\n";
  for (int32_t i = 0; i < m_numAxes; i++) {
    ss << i << ": " << m_axes[i] << "\n";
  }
  return ss.str();
}

void Ps3Controller::readPs3Controller()
{

  #if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
  struct js_event js; 
  while (read(m_ps3controllerDevice, &js, sizeof(struct js_event)) > 0) {
    int32_t jsNumber = (int32_t)js.number;
    int32_t jsValue = (int32_t)js.value;
    switch (js.type) {
      case JS_EVENT_AXIS:
        m_axes[jsNumber] = jsValue;
        break;
      case JS_EVENT_BUTTON:
        m_buttons[jsNumber] = jsValue;
        break;
      case JS_EVENT_INIT:
        break;
      default:
        break;
    }
  updateStateReq();
  }

  /* EAGAIN is returned when the queue is empty */
  if (errno != EAGAIN) {
      std::cerr << "An error occurred in the Ps3 joystick event handling" << std::endl;
  }

  #else
  #endif

}

void Ps3Controller::updateStateReq()
{
  if (m_stateReq.baseThrust() >= 0){
    // BaseThrust
    float decrement = -m_axes[1] / (32767.0f * 160.0f);
    // std::cout << "decrement: " << decrement << std::endl;
    m_stateReq.baseThrust(m_stateReq.baseThrust() + decrement);
    if (m_stateReq.baseThrust() < 0) {
      m_stateReq.baseThrust(0);
    } else if (m_stateReq.baseThrust() > 1) {
      m_stateReq.baseThrust(1);
    }
    //Yaw speed
    m_stateReq.yawSpeed((m_axes[0]/32767.0f)*(0.05f)); //The maximum on the joystick coresponds to pi/36 radians of roll
    // Roll
    m_stateReq.roll((m_axes[2]/32767.0f)*(static_cast<float>(M_PI)/36.0f));
    // Pitch
    m_stateReq.pitch((m_axes[3]/32767.0f)*(static_cast<float>(M_PI)/36.0f));

    float pitchTrimInput = 2*3.14f/7200.0f*(m_buttons[14] - m_buttons[12]);
    float rollTrimInput = 2*3.14f/7200.0f*(m_buttons[13] - m_buttons[15]);
    m_stateReq.pitchTrim(m_stateReq.pitchTrim() + pitchTrimInput);
    m_stateReq.rollTrim(m_stateReq.rollTrim() + rollTrimInput);
  }
}


openkorp::logic::StateRequest Ps3Controller::getStateRequest()
{
  return m_stateReq;
}
