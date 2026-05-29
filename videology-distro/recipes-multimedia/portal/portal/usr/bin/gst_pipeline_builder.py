import json
import argparse

gstreamer_component_map = {}


class Element:
    def __init__(self, name, settings, caps=""):
        self.name = name
        self.settings = settings
        self.caps = caps

class Tee:
    numTee = 0
    def __init__ (self):
        self.num = Tee.numTee
        self.name = f"t{self.num}"
        self.element = Element("tee", f" name={self.name}")
        Tee.numTee += 1

def isElementInMapByName(name):
    return name in gstreamer_component_map.get("components", {})

def findNameInMap(name):
    if isElementInMapByName(name):
        map_element = gstreamer_component_map["components"].get(name, {})
        if "name" in map_element:
            name = map_element["name"]
    return name

def getSettingsFromPipelineJson(settingsJson):
    settings_str = ""
    for setting_key, setting_value in settingsJson.items():
        settings_str += f" {setting_key}={setting_value} "

    return settings_str


def findSettingsInMap(name):
    settings_str = ""
    if isElementInMapByName(name):
        map_element = gstreamer_component_map["components"].get(name, {})
        if "settings" in map_element:
            settings_str += getSettingsFromPipelineJson(map_element["settings"])

    return settings_str

def format_caps(component_caps, caps_mapping):
    caps_str = ["video/x-raw"]
    for cap_key, cap_value in component_caps.items():
        cap_setting = caps_mapping.get(cap_key)
        if cap_setting:
            if isinstance(cap_setting, dict):
                caps_str.append(cap_setting.get(cap_value, ""))
            else:
                caps_str.append(cap_setting.format(**component_caps))
        else:
            caps_str.append(f"{cap_key}={cap_value}")
    return ",".join(caps_str)

def createCaps(caps):
    capsStr = ""
    caps_str = format_caps(caps, gstreamer_component_map.get("caps", {}))
    capsStr += f" ! {caps_str} ! videoconvert " if caps_str else ""
    return capsStr

def createElementFromJson(jsonElement):
    name = findNameInMap(jsonElement['type'])
    settings = findSettingsInMap(jsonElement['type'])

    if "settings" in jsonElement['data']:
        settings += getSettingsFromPipelineJson(jsonElement['data']['settings'])
    caps = ""
    if "caps" in jsonElement["data"]:
        caps = createCaps(jsonElement['data']['caps'])

    return Element(name, settings, caps)

def findAdditionalElemInMap(name):
    result = []
    if isElementInMapByName(name):
        map_element = gstreamer_component_map["components"].get(name, {})
        if "additional_gst_plugins" in map_element:
            for elem in map_element["additional_gst_plugins"]:
                name = elem["name"]
                settings = ""
                if "settings" in elem:
                    settings = getSettingsFromPipelineJson(elem["settings"])
                result += [Element(name, settings)]
    return result


def create_pipeline(components, id="0"):
    pipelines = []
    global numTee

    def get_next_component(id_next, pipeline):
        input_component = components[id_next]
        pipeline += [createElementFromJson(input_component)]
        pipeline += findAdditionalElemInMap(input_component['type'])
        outputs = input_component["data"]["outputIds"]

        if not outputs:
            pipelines.append(pipeline)
        elif len(outputs) == 1:
            get_next_component(outputs[0], pipeline)
        else:
            tee = Tee()
            pipeline += [tee.element]

            pipelines.append(pipeline)

            for out_id in outputs:
                # First create branch started with tee
                get_next_component(out_id, [Element(tee.name + ".", "")])

    get_next_component(id, [])
    return pipelines

def element_to_str(object):
    s = ""
    s += object.name
    s += object.settings
    s += object.caps
    return s

def pipeline_to_str(pipelines):
    arrays_of_str_plugings = []
    # Convert objects to array of plugins string
    for pipeline in pipelines:
        pip_str = []
        for ob in pipeline:
            pip_str.append(element_to_str(ob))
        arrays_of_str_plugings.append(pip_str)

    # Convert array of strings to a pipeline string
    part_of_pip_arr = []
    for pip_part in arrays_of_str_plugings:
        part_of_pip_arr.append( " ! ".join(pip_part))
    return " ".join(part_of_pip_arr)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="GStreamer pipeline builder from JSON configuration")
    parser.add_argument('config_file', type=str, help='Path to the JSON configuration file')
    parser.add_argument('map_file', type=str, help='Path to the JSON mapping file')

    args = parser.parse_args()

    # Load configurations
    with open(args.config_file, 'r') as file:
        configuration = json.load(file)

    with open(args.map_file, 'r') as file:
        gstreamer_component_map = json.load(file)

    # Create pipeline

    pipeline = create_pipeline(configuration["components"], configuration["inputId"])
    pipeline_str = pipeline_to_str(pipeline)
    print(pipeline_str)

    with open ("pipeline.txt", 'w') as file:
        file.write(pipeline_str)