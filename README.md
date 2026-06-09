# c-ternary Fleet Integration Demo

A minimal C99 demo showing agent-to-agent communication using **c-ternary** vector operations and I2I wire-format packing.

## Architecture

```
┌─────────────────────────────────────────────────────┐
│  Agent A stance        Agent B stance               │
│  (8 trit vector)       (8 trit vector)              │
│                                                      │
│  ┌──────┐              ┌──────┐                     │
│  │-.0.0 │              │++.0- │                     │
│  │-.+0- │              │0.+-+ │                     │
│  └──────┘              └──────┘                     │
│       \                  /                           │
│        ▼                ▼                            │
│  ┌────────────────────────────────────────────┐      │
│  │  Consensus (AND)   Disagreement (XOR)      │      │
│  │  -0000+--          ++0-00-+                │      │
│  └────────────────────────────────────────────┘      │
│         │                                            │
│         ▼                                            │
│  ┌─────────────────────────┐                         │
│  │ ct_pack_trits()         │  I2I wire format        │
│  │ [version][count][payload]──► [ hex bytes ]       │
│  └─────────────────────────┘                         │
│         │                                            │
│         ▼                                            │
│  ┌─────────────────────────┐                         │
│  │ ct_unpack_trits()       │  Round-trip verify     │
│  └─────────────────────────┘                         │
└─────────────────────────────────────────────────────┘
```

## Building

```bash
# Requires c-ternary.h in ../construct-coordination/
make
```

## Running

```bash
make run
# or
./fleet_agent
```

## What It Demonstrates

| Operation | Description | Fleet Meaning |
|-----------|-------------|---------------|
| `ct_and_vec` | Element-wise AND | **Consensus** — pessimistic agreement between agents |
| `ct_xor` | Element-wise XOR | **Disagreement** — where stances diverge |
| `ct_dot` | Scalar dot product | **Trust** — normalized alignment score in [0,1] |
| `ct_pack_trits` | I2I wire encoding | **Transport** — pack trits into byte wire format for fleet messaging |
| `ct_unpack_trits` | I2I wire decoding | **Round-trip** — verify serialization integrity |
| `ct_distance` | Hamming distance | **Divergence** — how many positions differ |
| `ct_norm` | L1 norm (non-zero count) | **Conviction** — how many stances are non-neutral |
| `ct_sum` | Sum of trit values | **Net polarity** — overall direction of agent |

## Output

The program prints each agent's stance as a trit vector, computes consensus and disagreement, shows the I2I wire hex, demonstrates a round-trip unpack, and reports fleet coordination metrics.

## Dependencies

- **c-ternary.h v2.1+** — single-header C99 library (MIT)
- GCC or compatible C99 compiler
- No dynamic allocations beyond stack + one `malloc` for the I2I buffer

## Related Projects

- [c-ternary](https://github.com/SuperInstance/c-ternary) — Core ternary logic library
- [ternary-fleet-packing](https://github.com/SuperInstance/ternary-fleet-packing) — Python reference implementation of I2I packing
