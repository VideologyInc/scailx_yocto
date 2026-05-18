from dataclasses import dataclass, field
from typing import Dict


@dataclass
class Element:
    """Represents an element in a GStreamer pipeline.

    Attributes:
        id (int): Unique identifier for the element
        name (str): Name of the element (e.g., "filesrc", "videoconvert")
        properties (Dict[str, str]): Dictionary containing element properties
    """

    id: int
    name: str
    properties: Dict[str, str] = field(default_factory=dict)
