/*
 * Copyright (C) 2018 Ola Benderius
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"
#include "Ps3Controller.h"
#include <ncurses.h>
#include "openkorp-message-set.hpp"

int32_t main(int32_t argc, char **argv) {
  int32_t retCode{0};
  auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
  if (0 == commandlineArguments.count("cid") ||
      0 == commandlineArguments.count("input") ||
      0 == commandlineArguments.count("freq")) {
    std::cerr << argv[0] << " interfaces to the ps3controller for the Drone." << std::endl;
    std::cerr << "Usage:   " << argv[0] << " --cid=<OpenDaVINCI session> [--verbose] --input=<js node>" << std::endl;
    std::cerr << "Example: " << argv[0] << " --cid=111 --input=/dev/input/js0" << std::endl;
    retCode = 1;
    Ps3Controller ps3controller();
  } else {
    int32_t VERBOSE{commandlineArguments.count("verbose") != 0};
    cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"])),
      [](auto){}
    };
    if (VERBOSE) {
      VERBOSE = std::stoi(commandlineArguments["verbose"]);
    }
    float const FREQ = std::stof(commandlineArguments["freq"]);
    Ps3Controller ps3controller(commandlineArguments["input"]);
    if (VERBOSE == 2) {
      initscr();
    }

    auto atFrequency{[&ps3controller, &VERBOSE, &od4]() -> bool
    {
      ps3controller.readPs3Controller();
      openkorp::logic::StateRequest ps3StateReq = ps3controller.getStateRequest();
      double baseThrust = ps3StateReq.baseThrust();
      double yawSpeed  = ps3StateReq.yawSpeed();
      double roll  = ps3StateReq.roll();
      double pitch  = ps3StateReq.pitch();
      double pitchTrim  = ps3StateReq.pitchTrim();
      double rollTrim  = ps3StateReq.rollTrim();

      if (VERBOSE == 1) {
        std::cout 
            << ps3controller.toString() << std::endl
            << "baseThrust: " + std::to_string(baseThrust) << std::endl
            << "yawSpeed: " + std::to_string(yawSpeed) << std::endl
            << "roll: " + std::to_string(roll) << std::endl
            << "pitch: " + std::to_string(pitch) << std::endl
            << "pitchTrim: " + std::to_string(pitchTrim) << std::endl
            << "rollTrim: " + std::to_string(rollTrim) << std::endl;
      }
      if (VERBOSE == 2) {
        mvprintw(1,1,(ps3controller.toString()).c_str()); 
        mvprintw(1,40,(std::to_string(baseThrust)).c_str()); 
        mvprintw(20,40,(std::to_string(rollTrim)).c_str()); 
        refresh();   
      }
      cluon::data::TimeStamp sampleTime = cluon::time::now();

      od4.send(ps3StateReq, sampleTime, 0);
      return true;
    }};

    od4.timeTrigger(FREQ, atFrequency);

    while (od4.isRunning()) {
      std::this_thread::sleep_for(std::chrono::duration<double>(1.0));
    }
  }
  return retCode;
}
