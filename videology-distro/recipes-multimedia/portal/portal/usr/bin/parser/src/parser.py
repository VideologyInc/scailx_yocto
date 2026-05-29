from lark import Lark, Transformer
from src.edge import Edge
from src.element import Element
from src.pipeline import Pipeline


pipeline_grammar = r"""
?start: pipeline
pipeline: statement+
statement: chain
chain: item (BANG item)*
?item: padref | caps | element
element: IDENT property*
property: PADPROP "=" VALUE   -> pad_property
        | PROP "=" VALUE      -> property
padref: PADREF
caps: CAPS
VALUE: /[^\s!]+/
PADPROP: /[a-zA-Z_][a-zA-Z0-9_-]*::[a-zA-Z_][a-zA-Z0-9_-]*/
PADREF: /[a-zA-Z_][a-zA-Z0-9_-]*\.([a-zA-Z_][a-zA-Z0-9_-]*)*/
PROP: /[a-zA-Z_][a-zA-Z0-9_-]*(?=\s*=)/
IDENT: /[a-zA-Z_][a-zA-Z0-9_-]*/
CAPS: /[a-zA-Z0-9_+.-]+\/[a-zA-Z0-9_+.-]+(\s*,\s*[a-zA-Z0-9_-]+\s*=\s*(\{[^}]*\}|\[[^\]]*\]|"[^"]*"|[^,\s!]+))*(\s*;\s*[a-zA-Z0-9_+.-]+\/[a-zA-Z0-9_+.-]+(\s*,\s*[a-zA-Z0-9_-]+\s*=\s*(\{[^}]*\}|\[[^\]]*\]|"[^"]*"|[^,\s!]+))*)*/
BANG: "!"
%import common.WS
%ignore WS
"""


class PipelineTransformer(Transformer):
    """Transforms Lark parse tree into a Pipeline object.

    This class is responsible for converting the parsed GStreamer pipeline
    grammar into a structured Pipeline object with elements and edges.
    """

    def __init__(self):
        """Initialize the PipelineTransformer with empty collections."""
        self.elements = []
        self.edges = []
        self.named = {}
        self.current_id = 0

    def pipeline(self, items):
        """Transform pipeline grammar into a list of chain nodes.

        Args:
            items: List of pipeline items

        Returns:
            Pipeline: Transformed pipeline object
        """
        chain_nodes = []

        for chain in items:
            local = []
            for item in chain:
                if item == "!":
                    local.append(item)
                    continue

                if item["type"] in ("element", "caps"):
                    node_id = self.current_id
                    self.current_id += 1

                    if item["type"] == "element":
                        el = Element(
                            id=node_id,
                            name=item["name"],
                            properties=item["props"],
                        )
                    else:
                        el = Element(
                            id=node_id,
                            name="capsfilter",
                            properties=item["props"],
                        )

                    self.elements.append(el)
                    local.append(("node", node_id, item))
                else:
                    local.append(item)

            chain_nodes.append(local)

        for el in self.elements:
            if "name" in el.properties:
                self.named[el.properties["name"]] = el.id

        for chain in chain_nodes:
            prev_node = None
            pending_link = False

            for item in chain:
                if item == "!":
                    pending_link = True
                    continue

                if isinstance(item, tuple) and item[0] == "node":
                    node_id = item[1]

                    if prev_node is not None and pending_link:
                        self.edges.append(Edge(src=prev_node, dst=node_id))

                    prev_node = node_id
                    pending_link = False

                elif item["type"] == "padref":
                    name = item["name"]

                    if name not in self.named:
                        raise ValueError(f"Unknown reference: {name}")

                    target = self.named[name]

                    if pending_link:
                        if prev_node is not None:
                            self.edges.append(Edge(src=prev_node, dst=target))
                        prev_node = target
                    else:
                        prev_node = target

                    pending_link = False

        return Pipeline(self.elements, self.edges)

    def statement(self, items):
        """Transform a statement into its first item.

        Args:
            items: List of statement items

        Returns:
            Any: First item in the statement
        """
        return items[0]

    def chain(self, items):
        """Transform a chain into its items.

        Args:
            items: List of chain items

        Returns:
            List: The list of items
        """
        return items

    def element(self, items):
        """Transform an element into a dictionary representation.

        Args:
            items: List of element items including name and properties

        Returns:
            dict: Dictionary representing the element
        """
        name = items[0].value
        props = {}
        pad_props = {}

        for prop in items[1:]:
            if prop is None:
                continue

            if prop["type"] == "property":
                value = prop["value"]
                if "+" in value:
                    value = value.split("+")
                elif "/" in value:
                    value = value.split("/")
                    if len(value) == 2:
                        try:
                            int(value[0])
                            int(value[1])
                        except ValueError:
                            value = prop["value"]
                    else:
                        value = prop["value"]
                props[prop["key"]] = value

            elif prop["type"] == "pad_property":
                pad = prop["pad"]
                if pad not in pad_props:
                    pad_props[pad] = {}
                pad_props[pad][prop["key"]] = prop["value"]

        if pad_props:
            props["_pad_props"] = pad_props

        return {"type": "element", "name": name, "props": props}

    def property(self, items):
        """Transform a property into a dictionary representation.

        Args:
            items: List containing property key and value

        Returns:
            dict: Dictionary representing the property
        """
        return {
            "type": "property",
            "key": items[0].value,
            "value": items[1].value.strip('"'),
        }

    def pad_property(self, items):
        """Transform a pad property into a dictionary representation.

        Args:
            items: List containing pad property key and value

        Returns:
            dict: Dictionary representing the pad property
        """
        full = items[0].value
        pad, key = full.split("::", 1)
        return {
            "type": "pad_property",
            "pad": pad,
            "key": key,
            "value": items[1].value,
        }

    def caps(self, items):
        """Transform caps into a dictionary representation.

        Args:
            items: List containing caps string

        Returns:
            dict: Dictionary representing the caps
        """
        return {
            "type": "caps",
            "props": {"caps": items[0].value},
        }

    def padref(self, items):
        """Transform a pad reference into a dictionary representation.

        Args:
            items: List containing pad reference string

        Returns:
            dict: Dictionary representing the pad reference
        """
        value = items[0].value
        element_name, pad_name = value.split(".", 1)
        return {
            "type": "padref",
            "name": element_name,
            "pad": pad_name,
        }

    def BANG(self, _):
        """Transform BANG token.

        Args:
            _: The BANG token (unused)

        Returns:
            str: The literal "!" character
        """
        return "!"


def adjust_pipeline(pipeline: Pipeline) -> Pipeline:
    """Adjust pipeline by generating names for elements without them.

    This function ensures that all elements in a pipeline have unique names
    by generating names for elements that don't have them, using a naming
    convention based on element type and counter.

    Args:
        pipeline (Pipeline): The input pipeline to adjust

    Returns:
        Pipeline: The adjusted pipeline with all elements having names
    """
    # Set to keep track of used names
    used_names = set()
    # Counter for each element type
    type_counters = {}

    # First pass: collect all existing names
    for element in pipeline.elements:
        if "name" in element.properties:
            used_names.add(element.properties["name"])

    # Second pass: generate names for elements without them
    for element in pipeline.elements:
        if "name" not in element.properties or not element.properties["name"]:
            # Get element type
            element_type = element.name

            # Initialize counter for this type if not exists
            if element_type not in type_counters:
                type_counters[element_type] = 0

            # Generate unique name
            counter = type_counters[element_type]
            generated_name = f"{element_type}{counter}"

            # Keep incrementing counter until we find a free name
            while generated_name in used_names:
                counter += 1
                generated_name = f"{element_type}{counter}"

            # Add name to used names and element properties
            used_names.add(generated_name)
            type_counters[element_type] = counter + 1
            element.properties["name"] = generated_name

    return pipeline


def parse_pipeline(text: str) -> Pipeline:
    """Parse a GStreamer pipeline string into a structured Pipeline object.

    This function uses the Lark parser with the defined grammar to parse
    a GStreamer pipeline string and transform it into a Pipeline object.

    Args:
        text (str): The GStreamer pipeline string to parse

    Returns:
        Pipeline: The parsed pipeline object
    """
    parser = Lark(pipeline_grammar, parser="lalr")
    tree = parser.parse(text)
    pipeline = PipelineTransformer().transform(tree)
    return adjust_pipeline(pipeline)
