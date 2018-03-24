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
#include <thread>
#include <unistd.h>
#include <string>

#include "Ps3Controller.h"

#if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
#include <linux/joystick.h>
#include <fcntl.h>
#endif


Ps3Controller::Ps3Controller()
    , m_ps3controllerDevice(0)
    , m_axes() 
    , m_buttons()
    , m_numAxes(0)
    , m_numButtons(0) 
    , m_stateReq(-1.0,0.0,0.0,0.0,0.0,0.0)
    , pitchTrim()
    , rollTrim()
{
  std::stringstream info;
  odcore::base::KeyValueConfiguration kv = getKeyValueConfiguration();

  std::string const Ps3CONTROLLER_DEVICE_NODE = 
      kv.getValue<std::string>(getName() + ".ps3controllerdevicenode");

  std::cout << "[" << getName() << "] Trying to open ps3controller " 
      << Ps3CONTROLLER_DEVICE_NODE << std::endl;
  
  // Setup ps3controller control.
  #if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
  // int16_t num_of_axes = 0;
  // int16_t num_of_buttons = 0;
  int16_t name_of_ps3controller[80];

  m_ps3controllerDevice = open(Ps3CONTROLLER_DEVICE_NODE.c_str(), O_RDONLY);
  if (m_ps3controllerDevice == -1) {
    std::cerr << "[" << getName() << "] Could not open ps3controller " 
        << Ps3CONTROLLER_DEVICE_NODE << std::endl;
    exit(1);
  }

  ioctl(m_ps3controllerDevice, JSIOCGAXES, &m_numAxes);
  ioctl(m_ps3controllerDevice, JSIOCGBUTTONS, &m_numButtons);
  ioctl(m_ps3controllerDevice, JSIOCGNAME(80), &name_of_ps3controller);

  m_axes = std::unique_ptr<int32_t[]>((int32_t *)calloc(m_numAxes, sizeof(int32_t)));
  m_buttons = std::unique_ptr<int32_t[]>((int32_t *)calloc(m_numButtons, sizeof(int32_t)));

  std::cout << "[" << getName() 
      << "] Ps3Controller found " << name_of_ps3controller
      << ", number of axes: " << m_numAxes
      << ", number of buttons: " << m_numButtons << std::endl;
  
  // Use non blocking reading.
  fcntl(m_ps3controllerDevice, F_SETFL, O_NONBLOCK);
  
  #else
  std::cout << "[" << getName() 
      << "] This code will not work on this computer architecture." 
      << std::endl;
  #endif
}


Ps3Controller::~Ps3Controller() {
  close(m_ps3controllerDevice);
  m_stateReq = opendlv::logic::StateRequest(-1.0,0.0,0.0,0.0,0.0,0.0);
  sendStateRequest();
}



odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode Ps3Controller::body()
{
  /* Lessons learned from live test:
  * - Care must be taken when activating the brakes, since braking too much 
  * is *very* uncomfortable -and potentially dangerous-, on the other hand a 
  * smaller maximum value for the deceleration could be dangerous as well, since it
  * could impair the possiblity to perform an emergency brake while using the controller.
  */

  while (getModuleStateAndWaitForRemainingTimeInTimeslice() 
      == odcore::data::dmcp::ModuleStateMessage::RUNNING) {
    readPs3Controller();
    updateTrim();
    updateStateReq();
    sendStateRequest();
    // std::cout << "[" << getName() << "] Looped " << std::endl;
    if(getVerbosity() > 1){
      std::cout << "Button\n";
      for (int32_t i = 0; i < m_numButtons; i++) {
        std::cout << i << ": " << m_buttons[i] << "\n";
      } 
      std::cout << "Axes\n";
      for (int32_t i = 0; i < m_numAxes; i++) {
        std::cout << i << ": " << m_axes[i] << "\n";
      } 
    }
  }
  return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
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
        if (m_buttons[3] == 1){ //Pressing the start-button to activate engines
          if(m_stateReq.getBaseThrust() < 0){
            m_stateReq.setBaseThrust(0);
          } else {
            m_stateReq.setBaseThrust(-1);
          }
        }
        
        break;
      case JS_EVENT_INIT:
        break;
      default:
        break;
    }
  }

  /* EAGAIN is returned when the queue is empty */
  if (errno != EAGAIN) {
      std::cerr << "[" << getName() 
          << "] An error occurred in the Ps3 joystick event handling" << std::endl;
  }

  #else
  #endif

}

void Ps3Controller::updateStateReq()
{
  if (m_stateReq.getBaseThrust() >= 0){
    // BaseThrust
    double decrement = -static_cast<double>(m_axes[1] / (32767.0 * 160.0));
    // std::cout << "decrement: " << decrement << std::endl;
    m_stateReq.setBaseThrust(m_stateReq.getBaseThrust() + decrement);
    if (m_stateReq.getBaseThrust() < 0) {
      m_stateReq.setBaseThrust(0);
    } else if (m_stateReq.getBaseThrust() > 1) {
      m_stateReq.setBaseThrust(1);
    }
    //Yaw speed
    m_stateReq.setYawSpeed((m_axes[0]/32767.0)*(0.05)); //The maximum on the joystick coresponds to pi/36 radians of roll
    // Roll
    m_stateReq.setRoll((m_axes[2]/32767.0)*(M_PI/36));
    // Pitch
    m_stateReq.setPitch((m_axes[3]/32767.0)*(M_PI/36));
  }


}

void Ps3Controller::updateTrim()
{
  double pitchTrimInput = 2*3.14/7200.0*(m_buttons[14] - m_buttons[12]);
  double pitchRollInput = 2*3.14/7200.0*(m_buttons[13] - m_buttons[15]);

  m_stateReq.setPitchTrim(pitchTrimInput);
  m_stateReq.setRollTrim(pitchRollInput);

}

void Ps3Controller::sendStateRequest()
{
  odcore::data::Container c(m_stateReq);
  getConference().send(c);
  if (isVerbose()) {
    std::cout << "Sending: " << m_stateReq.toString() << std::endl;
  }
}
