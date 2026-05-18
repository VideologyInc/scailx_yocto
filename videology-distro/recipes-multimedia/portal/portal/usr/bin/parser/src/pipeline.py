from dataclasses import dataclass
from typing import List
from src.edge import Edge
from src.element import Element


@dataclass
class Pipeline:
    """Represents a GStreamer pipeline structure.

    Attributes:
        elements (List[Element]): List of elements in the pipeline
        edges (List[Edge]): List of connections between pipeline elements
    """
    elements: List[Element]
    edges: List[Edge]
