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
// #include <cmath>
#include <unistd.h>
#include <string>
#include <sstream>
#include <memory>

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
  }

  /* EAGAIN is returned when the queue is empty */
  if (errno != EAGAIN) {
      std::cerr << "An error occurred in the Ps3 joystick event handling" << std::endl;
  }

  #else
  #endif

}

opendlv::proxy::GroundSteeringRequest Ps3Controller::getGroundSteeringRequest()
{
  float val{-(m_axes[0] / MAX_AXES_VALUE)};
  opendlv::proxy::GroundSteeringRequest gsr;
  gsr.groundSteering(val);
  return gsr;
}

opendlv::proxy::PedalPositionRequest Ps3Controller::getPedalPositionRequest()
{
  float val{-(m_axes[3] / MAX_AXES_VALUE)};
  opendlv::proxy::PedalPositionRequest ppr;
  ppr.position(val);
  return ppr;
}
