const char graphcfg[] = R"(
input_stream: "IMAGE:input_video"

#output_stream: "DETECTIONS:palm_detections"
#output_stream: "NORM_RECT:hand_rect"
output_stream: "LANDMARKS:hand_landmarks"
#output_stream: "NORM_RECT:hand_rect_for_next_frame"
#output_stream: "PRESENCE:hand_presence"

node {
  calculator: "ImageTransformationCalculator"
  input_stream: "IMAGE_GPU:input_video"
  output_stream: "IMAGE_GPU:transformed_input_video"
  output_stream: "LETTERBOX_PADDING:letterbox_padding"
  node_options: {
    [type.googleapis.com/mediapipe.ImageTransformationCalculatorOptions] {
      output_width: 256
      output_height: 256
      scale_mode: FIT
    }
  }
}

node {
  calculator: "TfLiteCustomOpResolverCalculator"
  output_side_packet: "opresolver"
  node_options: {
    [type.googleapis.com/mediapipe.TfLiteCustomOpResolverCalculatorOptions] {
      use_gpu: true
    }
  }
}

node {
  calculator: "TfLiteConverterCalculator"
  input_stream: "IMAGE_GPU:transformed_input_video"
  output_stream: "TENSORS_GPU:detection_image_tensor"
}

node {
  calculator: "TfLiteInferenceCalculator"
  input_stream: "TENSORS_GPU:detection_image_tensor"
  output_stream: "TENSORS:detection_tensors"
  input_side_packet: "CUSTOM_OP_RESOLVER:opresolver"
  node_options: {
    [type.googleapis.com/mediapipe.TfLiteInferenceCalculatorOptions] {
      model_path: "palm_detection.tflite"
      use_gpu: true
    }
  }
}

node {
  calculator: "SsdAnchorsCalculator"
  output_side_packet: "anchors"
  node_options: {
    [type.googleapis.com/mediapipe.SsdAnchorsCalculatorOptions] {
      num_layers: 5
      min_scale: 0.1171875
      max_scale: 0.75
      input_size_height: 256
      input_size_width: 256
      anchor_offset_x: 0.5
      anchor_offset_y: 0.5
      strides: 8
      strides: 16
      strides: 32
      strides: 32
      strides: 32
      aspect_ratios: 1.0
      fixed_anchor_size: true
    }
  }
}

node {
  calculator: "TfLiteTensorsToDetectionsCalculator"
  input_stream: "TENSORS:detection_tensors"
  input_side_packet: "ANCHORS:anchors"
  output_stream: "DETECTIONS:detections"
  node_options: {
    [type.googleapis.com/mediapipe.TfLiteTensorsToDetectionsCalculatorOptions] {
      num_classes: 1
      num_boxes: 2944
      num_coords: 18
      box_coord_offset: 0
      keypoint_coord_offset: 4
      num_keypoints: 7
      num_values_per_keypoint: 2
      sigmoid_score: true
      score_clipping_thresh: 100.0
      reverse_output_order: true

      x_scale: 256.0
      y_scale: 256.0
      h_scale: 256.0
      w_scale: 256.0
      min_score_thresh: 0.7
    }
  }
}

node {
  calculator: "NonMaxSuppressionCalculator"
  input_stream: "detections"
  output_stream: "filtered_detections"
  node_options: {
    [type.googleapis.com/mediapipe.NonMaxSuppressionCalculatorOptions] {
      min_suppression_threshold: 0.3
      overlap_type: INTERSECTION_OVER_UNION
      algorithm: WEIGHTED
      return_empty_detections: true
    }
  }
}

node {
  calculator: "DetectionLabelIdToTextCalculator"
  input_stream: "filtered_detections"
  output_stream: "labeled_detections"
  node_options: {
    [type.googleapis.com/mediapipe.DetectionLabelIdToTextCalculatorOptions] {
      label_map_path: "palm_detection_labelmap.txt"
    }
  }
}

node {
  calculator: "DetectionLetterboxRemovalCalculator"
  input_stream: "DETECTIONS:labeled_detections"
  input_stream: "LETTERBOX_PADDING:letterbox_padding"
  output_stream: "DETECTIONS:palm_detections"
}

node {
  calculator: "ImagePropertiesCalculator"
  input_stream: "IMAGE_GPU:input_video"
  output_stream: "SIZE:image_size"
}

node {
  calculator: "DetectionsToRectsCalculator"
  input_stream: "DETECTIONS:palm_detections"
  input_stream: "IMAGE_SIZE:image_size"
  output_stream: "NORM_RECT:palm_rect"
  node_options: {
    [type.googleapis.com/mediapipe.DetectionsToRectsCalculatorOptions] {
      rotation_vector_start_keypoint_index: 0  # Center of wrist.
      rotation_vector_end_keypoint_index: 2  # MCP of middle finger.
      rotation_vector_target_angle_degrees: 90
      output_zero_rect_for_empty_detections: true
    }
  }
}

node {
  calculator: "RectTransformationCalculator"
  input_stream: "NORM_RECT:palm_rect"
  input_stream: "IMAGE_SIZE:image_size"
  output_stream: "hand_rect"
  node_options: {
    [type.googleapis.com/mediapipe.RectTransformationCalculatorOptions] {
      scale_x: 2.6
      scale_y: 2.6
      shift_y: -0.5
      square_long: true
    }
  }
}


node {
  calculator: "ImageCroppingCalculator"
  input_stream: "IMAGE_GPU:input_video"
  input_stream: "NORM_RECT:hand_rect"
  output_stream: "IMAGE_GPU:hand_image"
}

node: {
  calculator: "ImageTransformationCalculator"
  input_stream: "IMAGE_GPU:hand_image"
  output_stream: "IMAGE_GPU:transformed_hand_image"
  output_stream: "LETTERBOX_PADDING:letterbox_padding"
  node_options: {
    [type.googleapis.com/mediapipe.ImageTransformationCalculatorOptions] {
      output_width: 256
      output_height: 256
      scale_mode: FIT
    }
  }
}

node {
  calculator: "TfLiteConverterCalculator"
  input_stream: "IMAGE_GPU:transformed_hand_image"
  output_stream: "TENSORS_GPU:tracking_image_tensor"
}

node {
  calculator: "TfLiteInferenceCalculator"
  input_stream: "TENSORS_GPU:tracking_image_tensor"
  output_stream: "TENSORS:output_tensors"
  node_options: {
    [type.googleapis.com/mediapipe.TfLiteInferenceCalculatorOptions] {
      model_path: "hand_landmark.tflite"
      use_gpu: true
    }
  }
}

node {
  calculator: "SplitTfLiteTensorVectorCalculator"
  input_stream: "output_tensors"
  output_stream: "landmark_tensors"
  output_stream: "hand_flag_tensor"
  node_options: {
    [type.googleapis.com/mediapipe.SplitVectorCalculatorOptions] {
      ranges: { begin: 0 end: 1 }
      ranges: { begin: 1 end: 2 }
    }
  }
}

node {
  calculator: "TfLiteTensorsToFloatsCalculator"
  input_stream: "TENSORS:hand_flag_tensor"
  output_stream: "FLOAT:hand_presence_score"
}

node {
  calculator: "ThresholdingCalculator"
  input_stream: "FLOAT:hand_presence_score"
  output_stream: "FLAG:hand_presence"
  node_options: {
    [type.googleapis.com/mediapipe.ThresholdingCalculatorOptions] {
      threshold: 0.1
    }
  }
}

node {
  calculator: "TfLiteTensorsToLandmarksCalculator"
  input_stream: "TENSORS:landmark_tensors"
  output_stream: "NORM_LANDMARKS:landmarks"
  node_options: {
    [type.googleapis.com/mediapipe.TfLiteTensorsToLandmarksCalculatorOptions] {
      num_landmarks: 21
      input_image_width: 256
      input_image_height: 256
    }
  }
}

node {
  calculator: "LandmarkLetterboxRemovalCalculator"
  input_stream: "LANDMARKS:landmarks"
  input_stream: "LETTERBOX_PADDING:letterbox_padding"
  output_stream: "LANDMARKS:scaled_landmarks"
}

node {
  calculator: "LandmarkProjectionCalculator"
  input_stream: "NORM_LANDMARKS:scaled_landmarks"
  input_stream: "NORM_RECT:hand_rect"
  output_stream: "NORM_LANDMARKS:hand_landmarks"
}

node {
  calculator: "LandmarksToDetectionCalculator"
  input_stream: "NORM_LANDMARKS:hand_landmarks"
  output_stream: "DETECTION:hand_detection"
}

node {
  calculator: "DetectionsToRectsCalculator"
  input_stream: "DETECTION:hand_detection"
  input_stream: "IMAGE_SIZE:image_size"
  output_stream: "NORM_RECT:hand_rect_from_landmarks"
  node_options: {
    [type.googleapis.com/mediapipe.DetectionsToRectsCalculatorOptions] {
      rotation_vector_start_keypoint_index: 0  # Center of wrist.
      rotation_vector_end_keypoint_index: 9  # MCP of middle finger.
      rotation_vector_target_angle_degrees: 90
    }
  }
}

node {
  calculator: "RectTransformationCalculator"
  input_stream: "NORM_RECT:hand_rect_from_landmarks"
  input_stream: "IMAGE_SIZE:image_size"
  output_stream: "hand_rect_for_next_frame"
  node_options: {
    [type.googleapis.com/mediapipe.RectTransformationCalculatorOptions] {
      scale_x: 1.6
      scale_y: 1.6
      square_long: true
    }
  }
}
)";
