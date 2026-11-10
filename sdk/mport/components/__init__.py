"""Ported components (docs/vol5_toolchain_compiler_sdk/mport_mutable_sdk.md §7). REGISTRY: name -> factory."""
from braids_saw_swarm import SawSwarm

REGISTRY = {
    'saw_swarm': SawSwarm,
}
