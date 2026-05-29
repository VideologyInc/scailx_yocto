import argparse
import json

from src.parser import parse_pipeline
from src.pipeline import Pipeline


def convert(pipeline: Pipeline) -> list[dict]:
    """Convert a pipeline object into a list of dictionary representations.

    This function transforms a Pipeline object into a format suitable for JSON
    serialization, with each element represented as a dictionary containing
    its properties and connections.

    Args:
        pipeline (Pipeline): The pipeline object to convert, containing elements
            and edges information.

    Returns:
        list[dict]: A list of dictionaries representing the pipeline elements
            and their connections. Each dictionary has 'id' and 'data' keys where
            data contains 'type', 'settings', and 'outputIds'.
    """
    result = []

    for element in pipeline.elements:
        settings = {}
        for prop, value in element.properties.items():
            if prop in ("name", "_pad_props"):
                continue
            settings[prop] = value

        for pad_name, values in element.properties.get("_pad_props", {}).items():
            for prop, value in values.items():
                settings[f"{pad_name}::{prop}"] = value

        result.append(
            {
                "id": element.properties["name"],
                "data": {"type": element.name, "settings": settings, "outputIds": []},
            }
        )

    for edge in pipeline.edges:
        result[edge.src]["data"]["outputIds"].append(result[edge.dst]["id"])

    return result


def parse(output_file: str, pipeline: str):
    """Parse a GStreamer pipeline and write the result to a JSON file.

    Args:
        output_file (str): The path to the output JSON file.
        pipeline (str): The GStreamer pipeline string to parse.
    """
    pipeline = parse_pipeline(pipeline)
    result = convert(pipeline)
    json_data = json.dumps(result, indent=4)
    with open(output_file, mode="w", encoding="utf-8") as json_file:
        json_file.write(json_data)


def main():
    """Main function to parse command line arguments and execute the pipeline parsing."""
    parser = argparse.ArgumentParser(description="GStreamer pipeline parser")
    parser.add_argument(
        "filename", help="File name to write out pipeline parsing results"
    )
    parser.add_argument("pipeline", help="GStreamer pipeline string")
    args = parser.parse_args()

    parse(args.filename, args.pipeline)


if __name__ == "__main__":
    main()
