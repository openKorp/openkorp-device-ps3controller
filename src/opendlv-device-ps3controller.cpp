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

#include <ncurses.h>

#include "cluon-complete.hpp"
#include "opendlv-standard-message-set.hpp"

#include "Ps3Controller.h"

int32_t main(int32_t argc, char **argv) {
  int32_t retCode{0};
  auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
  if (0 == commandlineArguments.count("cid") ||
      0 == commandlineArguments.count("input") ||
      0 == commandlineArguments.count("freq") ||
      0 == commandlineArguments.count("angleconversion")) {
    std::cerr << argv[0] << " interfaces to the ps3controller for the kiwi." << std::endl;
    std::cerr << "Usage:   " << argv[0] << " --cid=<OpenDaVINCI session> --freq=<hz>--input=<js node> --angleconversion=<const>[--verbose]" << std::endl;
    std::cerr << "Example: " << argv[0] << " --cid=111 --freq=10 --input=/dev/input/js0 --angleconversion=1" << std::endl;
    retCode = 1;
  } else {
    int32_t VERBOSE{commandlineArguments.count("verbose") != 0};
    if (VERBOSE) {
      VERBOSE = std::stoi(commandlineArguments["verbose"]);
    }
    cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"])),
      [](auto){}
    };
    float const FREQ = std::stof(commandlineArguments["freq"]);
    Ps3Controller ps3controller(commandlineArguments["input"]);
    if (VERBOSE == 2) {
      initscr();
    }
    float const ANGLECONVERSION = std::stof(commandlineArguments["angleconversion"]);

    auto atFrequency{[&ps3controller, &ANGLECONVERSION, &VERBOSE, &od4]() -> bool
    {
      ps3controller.readPs3Controller();
      opendlv::proxy::PedalPositionRequest ppr = ps3controller.getPedalPositionRequest();
      opendlv::proxy::GroundSteeringRequest gsr = ps3controller.getGroundSteeringRequest();
      gsr.groundSteering(gsr.groundSteering() * ANGLECONVERSION);
      cluon::data::TimeStamp sampleTime;
      od4.send(ppr, sampleTime, 0);
      od4.send(gsr, sampleTime, 0);
      if (VERBOSE == 1) {
        std::cout 
            << ps3controller.toString() << std::endl
            << "gsr: " + std::to_string(gsr.groundSteering()) << std::endl
            << "ppr: " + std::to_string(ppr.position()) << std::endl;
      }
      if (VERBOSE == 2) {
        mvprintw(1,1,(ps3controller.toString()).c_str()); 
        mvprintw(1,50,("gsr: " + std::to_string(gsr.groundSteering())).c_str()); 
        mvprintw(10,50,("ppr: " + std::to_string(ppr.position())).c_str());
        refresh();   
      }

      return true;
    }};
        

    od4.timeTrigger(FREQ, atFrequency);

    while (od4.isRunning()) {

    }
    if (VERBOSE == 2) {
      endwin();     
    }
  }
  return retCode;
}
