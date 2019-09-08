# Copyright 2019 The MediaPipe Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

licenses(["notice"])  # Apache 2.0

package(default_visibility = ["//visibility:public"])

load(
    "//mediapipe/framework/tool:mediapipe_graph.bzl",
    "mediapipe_binary_graph",
    "mediapipe_simple_subgraph",
)

mediapipe_simple_subgraph(
    name = "full_hand_detection",
    graph = "fullhand.pbtxt",
    register_as = "HandPositionSubgraph",
    deps = [
        "//mediapipe/framework:calculator_graph",
        ":hand_detection_gpu",
        ":hand_landmark_gpu",
    ],
)

cc_binary(
    name = "handrecog",
    srcs = ["handrecog.cc", "graphconfig.h"],
    deps = [
        "//mediapipe/calculators/core:pass_through_calculator",
        "//mediapipe/framework:calculator_graph",
        "//mediapipe/framework/port:logging",
        "//mediapipe/framework/port:parse_text_proto",
        "//mediapipe/framework/port:status",
    
        "//mediapipe/calculators/tflite:ssd_anchors_calculator",
        "//mediapipe/calculators/tflite:tflite_custom_op_resolver_calculator",
        "//mediapipe/calculators/tflite:tflite_tensors_to_detections_calculator",
        "//mediapipe/calculators/util:detection_label_id_to_text_calculator",
        "//mediapipe/calculators/util:detection_letterbox_removal_calculator",
        "//mediapipe/calculators/util:non_max_suppression_calculator",
    
        "//mediapipe/calculators/core:split_vector_calculator",
        "//mediapipe/calculators/image:image_cropping_calculator",
        "//mediapipe/calculators/image:image_properties_calculator",
        "//mediapipe/calculators/image:image_transformation_calculator",
        "//mediapipe/calculators/tflite:tflite_converter_calculator",
        "//mediapipe/calculators/tflite:tflite_inference_calculator",
        "//mediapipe/calculators/tflite:tflite_tensors_to_floats_calculator",
        "//mediapipe/calculators/tflite:tflite_tensors_to_landmarks_calculator",
        "//mediapipe/calculators/util:detections_to_rects_calculator",
        "//mediapipe/calculators/util:landmark_letterbox_removal_calculator",
        "//mediapipe/calculators/util:landmark_projection_calculator",
        "//mediapipe/calculators/util:landmarks_to_detection_calculator",
        "//mediapipe/calculators/util:rect_transformation_calculator",
        "//mediapipe/calculators/util:thresholding_calculator",
    ],
)


mediapipe_simple_subgraph(
    name = "hand_detection_gpu",
    graph = "hand_detection_gpu.pbtxt",
    register_as = "HandDetectionSubgraph",
    deps = [
        "//mediapipe/calculators/image:image_properties_calculator",
        "//mediapipe/calculators/image:image_transformation_calculator",
        "//mediapipe/calculators/tflite:ssd_anchors_calculator",
        "//mediapipe/calculators/tflite:tflite_converter_calculator",
        "//mediapipe/calculators/tflite:tflite_custom_op_resolver_calculator",
        "//mediapipe/calculators/tflite:tflite_inference_calculator",
        "//mediapipe/calculators/tflite:tflite_tensors_to_detections_calculator",
        "//mediapipe/calculators/util:detection_label_id_to_text_calculator",
        "//mediapipe/calculators/util:detection_letterbox_removal_calculator",
        "//mediapipe/calculators/util:detections_to_rects_calculator",
        "//mediapipe/calculators/util:non_max_suppression_calculator",
        "//mediapipe/calculators/util:rect_transformation_calculator",
    ],
)

mediapipe_simple_subgraph(
    name = "hand_landmark_gpu",
    graph = "hand_landmark_gpu.pbtxt",
    register_as = "HandLandmarkSubgraph",
    deps = [
        "//mediapipe/calculators/core:split_vector_calculator",
        "//mediapipe/calculators/image:image_cropping_calculator",
        "//mediapipe/calculators/image:image_properties_calculator",
        "//mediapipe/calculators/image:image_transformation_calculator",
        "//mediapipe/calculators/tflite:tflite_converter_calculator",
        "//mediapipe/calculators/tflite:tflite_inference_calculator",
        "//mediapipe/calculators/tflite:tflite_tensors_to_floats_calculator",
        "//mediapipe/calculators/tflite:tflite_tensors_to_landmarks_calculator",
        "//mediapipe/calculators/util:detections_to_rects_calculator",
        "//mediapipe/calculators/util:landmark_letterbox_removal_calculator",
        "//mediapipe/calculators/util:landmark_projection_calculator",
        "//mediapipe/calculators/util:landmarks_to_detection_calculator",
        "//mediapipe/calculators/util:rect_transformation_calculator",
        "//mediapipe/calculators/util:thresholding_calculator",
    ],
)
