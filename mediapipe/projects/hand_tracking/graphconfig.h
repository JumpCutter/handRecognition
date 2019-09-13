const char graphcfg[] = R"(
# MediaPipe graph that performs hand tracking with TensorFlow Lite on GPU.
# Used in the examples in
# mediapipie/examples/android/src/java/com/mediapipe/apps/handtrackinggpu and
# mediapipie/examples/ios/handtrackinggpu.

# Images coming into and out of the graph.
input_stream: "input_image"

output_stream: "LANDMARKS:hand_landmarks"
#output_stream: "NORM_RECT:hand_rect"
#output_stream: "DETECTIONS:palm_detections"
#output_stream: "IMAGE:output_image"

# Throttles the images flowing downstream for flow control. It passes through
# the very first incoming image unaltered, and waits for downstream nodes
# (calculators and subgraphs) in the graph to finish their tasks before it
# passes through another image. All images that come in while waiting are
# dropped, limiting the number of in-flight images in most part of the graph to
# 1. This prevents the downstream nodes from queuing up incoming images and data
# excessively, which leads to increased latency and memory usage, unwanted in
# real-time mobile applications. It also eliminates unnecessarily computation,
# e.g., the output produced by a node may get dropped downstream if the
# subsequent nodes are still busy processing previous inputs.
#node {
#  calculator: "FlowLimiterCalculator"
#  input_stream: "input_image"
#  input_stream: "FINISHED:hand_rect"
#  input_stream_info: {
#    tag_index: "FINISHED"
#    back_edge: true
#  }
#  output_stream: "input_image"
#}

# Caches a hand-presence decision fed back from HandLandmarkSubgraph, and upon
# the arrival of the next input image sends out the cached decision with the
# timestamp replaced by that of the input image, essentially generating a packet
# that carries the previous hand-presence decision. Note that upon the arrival
# of the very first input image, an empty packet is sent out to jump start the
# feedback loop.
node {
  calculator: "PreviousLoopbackCalculator"
  input_stream: "MAIN:input_image"
  input_stream: "LOOP:hand_presence"
  input_stream_info: {
    tag_index: "LOOP"
    back_edge: true
  }
  output_stream: "PREV_LOOP:prev_hand_presence"
}

# Drops the incoming image if HandLandmarkSubgraph was able to identify hand
# presence in the previous image. Otherwise, passes the incoming image through
# to trigger a new round of hand detection in HandDetectionSubgraph.
node {
  calculator: "GateCalculator"
  input_stream: "input_image"
  input_stream: "DISALLOW:prev_hand_presence"
  output_stream: "hand_detection_input_image"

  node_options: {
    [type.googleapis.com/mediapipe.GateCalculatorOptions] {
      empty_packets_as_allow: true
    }
  }
}

# Subgraph that detections hands (see hand_detection_gpu.pbtxt).
node {
  calculator: "HandDetectionSubgraph"
  input_stream: "hand_detection_input_image"
  output_stream: "DETECTIONS:palm_detections"
  output_stream: "NORM_RECT:hand_rect_from_palm_detections"
}

# Subgraph that localizes hand landmarks (see hand_landmark_gpu.pbtxt).
node {
  calculator: "HandLandmarkSubgraph"
  input_stream: "IMAGE:input_image"
  input_stream: "NORM_RECT:hand_rect"
  output_stream: "LANDMARKS:hand_landmarks"
  output_stream: "NORM_RECT:hand_rect_from_landmarks"
  output_stream: "PRESENCE:hand_presence"
}

# Caches a hand rectangle fed back from HandLandmarkSubgraph, and upon the
# arrival of the next input image sends out the cached rectangle with the
# timestamp replaced by that of the input image, essentially generating a packet
# that carries the previous hand rectangle. Note that upon the arrival of the
# very first input image, an empty packet is sent out to jump start the
# feedback loop.
node {
  calculator: "PreviousLoopbackCalculator"
  input_stream: "MAIN:input_image"
  input_stream: "LOOP:hand_rect_from_landmarks"
  input_stream_info: {
    tag_index: "LOOP"
    back_edge: true
  }
  output_stream: "PREV_LOOP:prev_hand_rect_from_landmarks"
}

# Merges a stream of hand rectangles generated by HandDetectionSubgraph and that
# generated by HandLandmarkSubgraph into a single output stream by selecting
# between one of the two streams. The formal is selected if the incoming packet
# is not empty, i.e., hand detection is performed on the current image by
# HandDetectionSubgraph (because HandLandmarkSubgraph could not identify hand
# presence in the previous image). Otherwise, the latter is selected, which is
# never empty because HandLandmarkSubgraphs processes all images (that went
# through FlowLimiterCaculator).
node {
  calculator: "MergeCalculator"
  input_stream: "hand_rect_from_palm_detections"
  input_stream: "prev_hand_rect_from_landmarks"
  output_stream: "hand_rect"
}

# Subgraph that renders annotations and overlays them on top of the input
# images (see renderer_gpu.pbtxt).
node {
  calculator: "RendererSubgraph"
  input_stream: "IMAGE:input_image"
  input_stream: "LANDMARKS:hand_landmarks"
  input_stream: "NORM_RECT:hand_rect"
  input_stream: "DETECTIONS:palm_detections"
  output_stream: "IMAGE:output_image"
}
)";
