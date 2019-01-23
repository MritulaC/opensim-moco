/* -------------------------------------------------------------------------- *
 * OpenSim Moco: MocoCost.cpp                                                 *
 * -------------------------------------------------------------------------- *
 * Copyright (c) 2017 Stanford University and the Authors                     *
 *                                                                            *
 * Author(s): Christopher Dembia                                              *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may    *
 * not use this file except in compliance with the License. You may obtain a  *
 * copy of the License at http://www.apache.org/licenses/LICENSE-2.0          *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 * -------------------------------------------------------------------------- */
#include "MocoCost.h"

using namespace OpenSim;

MocoCost::MocoCost() {
    constructProperties();
    if (getName().empty()) setName("cost");
}

MocoCost::MocoCost(std::string name) {
    setName(std::move(name));
    constructProperties();
}

MocoCost::MocoCost(std::string name, double weight)
        : MocoCost(std::move(name)) {
    set_weight(weight);
}


void MocoCost::printDescription(std::ostream& stream) const {
    stream << getName() << ". " << getConcreteClassName() <<
            " weight: " << get_weight() << std::endl;
}

void MocoCost::constructProperties() {
    constructProperty_weight(1);
}