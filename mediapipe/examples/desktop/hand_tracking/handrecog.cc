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
#include <cstdio>
#include <cmath>

#define PI 3.14159265
#define THRESHOLD PI/4
#define FRAMETHRESHOLD 2

#include "mediapipe/framework/port/opencv_imgcodecs_inc.h"
#include "mediapipe/framework/port/opencv_video_inc.h"
#include "mediapipe/framework/calculator_graph.h"
#include "mediapipe/framework/port/logging.h"
#include "mediapipe/framework/port/parse_text_proto.h"
#include "mediapipe/framework/port/status.h"
#include "mediapipe/framework/formats/image_frame.h"
#include "mediapipe/framework/formats/landmark.pb.h"
#include "mediapipe/framework/formats/rect.pb.h"
#include "mediapipe/framework/formats/image_frame_opencv.h"
#include "mediapipe/framework/subgraph.h"

#include "mediapipe/examples/desktop/hand_tracking/graphconfig.h"

namespace mediapipe {

static const char detection_graph[] =
#include "mediapipe/examples/desktop/hand_tracking/hand_detection.inc"
    ;  // NOLINT(whitespace/semicolon)

static const char landmark_graph[] =
#include "mediapipe/examples/desktop/hand_tracking/hand_landmark.inc"
    ;  // NOLINT(whitespace/semicolon)

static const char renderer_graph[] =
#include "mediapipe/examples/desktop/hand_tracking/hand_landmark.inc"
    ;  // NOLINT(whitespace/semicolon)

class HandDetectionSubgraph : public Subgraph {
 public:
  ::mediapipe::StatusOr<CalculatorGraphConfig> GetConfig(
      const SubgraphOptions& /*options*/) {
    CalculatorGraphConfig config;
    // Note: this is a binary protobuf serialization, and may include NUL
    // bytes. The trailing NUL added to the std::string literal should be excluded.
    if (config.ParseFromArray(detection_graph, sizeof(detection_graph) - 1)) {
      return config;
    } else {
      return ::mediapipe::InternalError("Could not parse subgraph.");
    }
  }
};

class HandLandmarkSubgraph : public Subgraph {
 public:
  ::mediapipe::StatusOr<CalculatorGraphConfig> GetConfig(
      const SubgraphOptions& /*options*/) {
    CalculatorGraphConfig config;
    // Note: this is a binary protobuf serialization, and may include NUL
    // bytes. The trailing NUL added to the std::string literal should be excluded.
    if (config.ParseFromArray(landmark_graph, sizeof(landmark_graph) - 1)) {
      return config;
    } else {
      return ::mediapipe::InternalError("Could not parse subgraph.");
    }
  }
};

class RendererSubgraph : public Subgraph {
 public:
  ::mediapipe::StatusOr<CalculatorGraphConfig> GetConfig(
      const SubgraphOptions& /*options*/) {
    CalculatorGraphConfig config;
    // Note: this is a binary protobuf serialization, and may include NUL
    // bytes. The trailing NUL added to the std::string literal should be excluded.
    if (config.ParseFromArray(renderer_graph, sizeof(renderer_graph) - 1)) {
      return config;
    } else {
      return ::mediapipe::InternalError("Could not parse subgraph.");
    }
  }
};
//REGISTER_MEDIAPIPE_GRAPH(HandLandmarkSubgraph);
//REGISTER_MEDIAPIPE_GRAPH(HandDetectionSubgraph);
double mod(double x, double y){
    return fmod(fmod(x, y)+y, y);
}

double getAngle(NormalizedLandmark start, NormalizedLandmark end){
    double dx = end.x() - start.x();
    double dy = end.y() - start.y();
    return atan2(dy, dx);
}


bool isExtended(NormalizedLandmark s1, NormalizedLandmark e1, NormalizedLandmark s2, NormalizedLandmark e2){
    double ang1 = getAngle(s1, e1);
    double ang2 = getAngle(s2, e2);
    double diff = mod(ang1-ang2, 2*PI);
    return diff < THRESHOLD || diff > 2*PI-THRESHOLD;
}

bool isFolded(NormalizedLandmark s1, NormalizedLandmark e1, NormalizedLandmark s2, NormalizedLandmark e2){
    return isExtended(s1, e1, e2, s2);
}

bool isFingerExtended(std::vector<NormalizedLandmark> ls, int finger){
    return isExtended(ls[finger*4+1], ls[finger*4+2], ls[finger*4+3], ls[finger*4+4]);
}
bool isFingerFolded(std::vector<NormalizedLandmark> ls, int finger){
    return isFolded(ls[finger*4+1], ls[finger*4+2], ls[finger*4+3], ls[finger*4+4]);
}
std::string exOrFd(std::vector<NormalizedLandmark> ls, int finger){
    bool extended = isFingerExtended(ls, finger);
    bool folded = isFingerFolded(ls, finger);
    return extended ? " extended" : (folded ? " folded" : " neutral");
}


int getGesture(std::vector<NormalizedLandmark> ls){
    bool isGesture = isFingerExtended(ls, 0) && \
                     isFingerFolded(ls, 1) && \
                     isFingerFolded(ls, 2) && \
                     isFingerFolded(ls, 3) && \
                     isFingerFolded(ls, 4);

    if (!isGesture) return 0;
    // Check that thumb is pointing up and out of hand
    if(!isExtended(ls[3], ls[4], ls[17], ls[5])) return 0;
    double thumbAngle = getAngle(ls[3], ls[4]);
    thumbAngle = mod(thumbAngle, 2*PI);
    if(PI/2-THRESHOLD < thumbAngle && thumbAngle < PI/2+THRESHOLD) return 2;
    if(3*PI/2-THRESHOLD < thumbAngle && thumbAngle < 3*PI/2+THRESHOLD) return 1;
    return 0;
                     
}

::mediapipe::Status DetectHandGestures(char* filename) {
    CalculatorGraphConfig config = ParseTextProtoOrDie<CalculatorGraphConfig>(graphcfg);
    CalculatorGraph graph;
    RETURN_IF_ERROR(graph.Initialize(config));
    ASSIGN_OR_RETURN(OutputStreamPoller lpoller,
                      graph.AddOutputStreamPoller(
                         "hand_landmarks"));
    //ASSIGN_OR_RETURN(OutputStreamPoller presencepoller,
    //                  graph.AddOutputStreamPoller(
    //                     "hand_presence"));
    //ASSIGN_OR_RETURN(OutputStreamPoller rectpoller,
    //                  graph.AddOutputStreamPoller(
    //                     "hand_rect"));
    ASSIGN_OR_RETURN(OutputStreamPoller poller,
                      graph.AddOutputStreamPoller(
                         "output_image"));
    RETURN_IF_ERROR(graph.StartRun({}));
    
    cv::VideoCapture video = cv::VideoCapture(filename);
    int i=0, j=0;
    int smoothedstate=0, laststate=0, stateframes=0;
    mediapipe::Packet packet;
    const char format[] = "output/frame_%03d.png";
    while(++i){
        cv::Mat inmatrix;// = cv::imread("test.png");
        if(!video.read(inmatrix)) break;
        auto image_frame =
        absl::make_unique<mediapipe::ImageFrame>(mediapipe::ImageFormat::SRGB,
            inmatrix.cols, inmatrix.rows,
            ImageFrame::kDefaultAlignmentBoundary);
        cv::Mat input_frame_mat = mediapipe::formats::MatView(image_frame.get());
        inmatrix.copyTo(input_frame_mat);
        RETURN_IF_ERROR(graph.AddPacketToInputStream(
            "input_image", Adopt(image_frame.release()).At(Timestamp(i))));
        
        if(lpoller.Next(&packet)){
            std::vector<NormalizedLandmark> landmarks = packet.Get<std::vector<NormalizedLandmark>>();
            int time =  packet.Timestamp().Value();
            int gesture = getGesture(landmarks);
            if(laststate==gesture){
                stateframes++;
            } else {
                laststate = gesture;
                stateframes = 0;
            }
            if(laststate != smoothedstate && stateframes >= FRAMETHRESHOLD){
                switch(smoothedstate){
                    case 1: std::cout << time << " end up" << std::endl; break;
                    case 2: std::cout << time << " end dn" << std::endl; break;
                }
                switch(laststate){
                    case 1: std::cout << time << " start up" << std::endl; break;
                    case 2: std::cout << time << " start dn" << std::endl; break;
                }
                smoothedstate = laststate; 
            }
            switch (gesture){
                case 1: std::cerr << time << "up" << std::endl; break;
                case 2: std::cerr << time << "dn" << std::endl; break;
                //default: std::cerr << time << "no" << std::endl; break;
            }
            j++;
        }
        if(poller.Next(&packet)){
            ImageFrame img = packet.Get<ImageFrame>();
            int time = packet.Timestamp().Value();
            char fname[256];
            sprintf(fname, format, time);
            cv::Mat mat = formats::MatView(&img);
            cv::imwrite(fname, mat);
        }

    }
    RETURN_IF_ERROR(graph.CloseInputStream("input_image"));
    /*while (j<i && poller.Next(&packet)) {
        /*std::vector<NormalizedLandmark> landmarks = packet.Get<std::vector<NormalizedLandmark>>();
        std::cout << packet.Timestamp().Value() << ' ';
        int gesture = getGesture(landmarks);
        switch(gesture){
            case 1: std::cout << "thumbs up" << std::endl; break;
            case 2: std::cout << "thumbs down" << std::endl; break;
            default: std::cout << "neutral" << std::endl;
        }
        //NormalizedRect r = rectpacket.Get<NormalizedRect>();
        //Timestamp t = rectpacket.Timestamp();
        //printf("%ld %f %f %f %f\n", t.Value(), r.x_center(), r.y_center(), r.width(), r.height());
        //std::cout << "packet gotten" << std::endl;
        //bool ispresent = presencepacket.Get<bool>();
        //printf("%s\n", ispresent?"found hand":"no hand");
        j++;
        printf("%d\n", i);
    }*/
    return graph.WaitUntilDone();
}
}  // namespace mediapipe

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    if(argc < 2){
        std::cerr << "No file provided" << std::endl;
        return 1;
    }

    CHECK(mediapipe::DetectHandGestures(argv[1]).ok());
    return 0;
}
