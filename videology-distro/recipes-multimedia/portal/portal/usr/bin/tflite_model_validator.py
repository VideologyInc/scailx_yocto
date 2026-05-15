import tflite_runtime.interpreter as tf
import numpy as np
import argparse
import json

# Function to validate TFLite model file
def validate_yolov5_tflite_model(model_path):
    # Check file extension
    if not model_path.endswith('.tflite'):
        return False, f"Invalid file extension: {model_path}. Expected .tflite file."
    
    try:
        # Load TFLite model
        interpreter = tf.Interpreter(model_path=model_path)
        interpreter.allocate_tensors()
        # print("Model loaded successfully.")
    except Exception as e:
        return False, f"Error loading model: {e}"
    
    try:
        # Get model input and output details
        input_details = interpreter.get_input_details()
        output_details = interpreter.get_output_details()
        
        # print("Model input details:")
        # print(input_details)
        
        # print("Model output details:")
        # print(output_details)
        
        # Perform a simple inference with a dummy image
        input_shape = input_details[0]['shape']
        dummy_input = np.zeros(input_shape, dtype=np.uint8)
        interpreter.set_tensor(input_details[0]['index'], dummy_input)
        
        interpreter.invoke()
        
        # Retrieve and print the inference result
        output_data = interpreter.get_tensor(output_details[0]['index'])
        
        # print("Inference result:")
        # print(output_data)
        
        return True, ""
    except Exception as e:
        return False, f"Error during inference: {e}"

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Validation tflite models")
    parser.add_argument('path', type=str, help='Path to the model')

    args = parser.parse_args()

    is_valid, error = validate_yolov5_tflite_model(args.path)
    json_string = json.dumps({"validation": is_valid, "error": error})
    print(json_string)

