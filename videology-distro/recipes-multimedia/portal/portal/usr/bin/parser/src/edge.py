from dataclasses import dataclass


@dataclass
class Edge:
    """Represents an edge in a pipeline graph.

    Attributes:
        src: The source node identifier
        dst: The destination node identifier
    """

    src: int
    dst: int
