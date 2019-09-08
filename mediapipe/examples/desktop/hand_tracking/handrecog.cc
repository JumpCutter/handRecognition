// Copyright 2019 The MediaPipe Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// A simple example to print out "Hello World!" from a MediaPipe graph.

#include <iostream>

#include "mediapipe/framework/calculator_graph.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/port/file_helpers.h"
#include "mediapipe/examples/desktop/hand_tracking/graphconfig.h"

namespace mediapipe {
::mediapipe::Status PrintHelloWorld() {
    CalculatorGraphConfig config = ParseTextProtoOrDie<CalculatorGraphConfig>(graphcfg);

    CalculatorGraph graph;
    RETURN_IF_ERROR(graph.Initialize(config));
    std::cout << "inited" << std::endl;
    /*
    ASSIGN_OR_RETURN(OutputStreamPoller poller,
                     graph.AddOutputStreamPoller("out"));
    RETURN_IF_ERROR(graph.StartRun({}));
    // Give 10 input packets that contains the same std::string "Hello World!".
    for (int i = 0; i < 10; ++i) {
      RETURN_IF_ERROR(graph.AddPacketToInputStream(
          "in", MakePacket<std::string>("Hello World!").At(Timestamp(i))));
    }
    // Close the input stream "in".
    RETURN_IF_ERROR(graph.CloseInputStream("in"));
    mediapipe::Packet packet;
    // Get the output packets std::string.
    while (poller.Next(&packet)) {
        std::cout << packet.Get<std::string>() << std::endl;
    }*/
    return graph.WaitUntilDone();
}
}  // namespace mediapipe

int main(int argc, char** argv) {
  google::InitGoogleLogging(argv[0]);
  CHECK(mediapipe::PrintHelloWorld().ok());
  return 0;
}
