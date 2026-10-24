"""Ported components (docs/MUTABLE_SDK.md §7). REGISTRY: name -> factory."""
from braids_saw_swarm import SawSwarm

REGISTRY = {
    'saw_swarm': SawSwarm,
}
