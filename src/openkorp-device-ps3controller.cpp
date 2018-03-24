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

int32_t main(int32_t argc, char **argv) {
  int32_t retCode{0};
  auto commandlineArguments = cluon::getCommandlineArguments(argc, argv);
  if (0 == commandlineArguments.count("cid")) {
    std::cerr << argv[0] << " interfaces to the motors of the Kiwi platform." << std::endl;
    std::cerr << "Usage:   " << argv[0] << " --cid=<OpenDaVINCI session> [--verbose]" << std::endl;
    std::cerr << "Example: " << argv[0] << " --cid=111" << std::endl;
    retCode = 1;
  } else {
    cluon::OD4Session od4{static_cast<uint16_t>(std::stoi(commandlineArguments["cid"])),
      [](auto){}
    };
    Ps3Controller ps3controller();

    while (od4.isRunning()) {
      std::this_thread::sleep_for(std::chrono::duration<double>(1.0));
    }
  }
  return retCode;
}
