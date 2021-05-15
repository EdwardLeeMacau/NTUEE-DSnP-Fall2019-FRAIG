# Functional Reduced And-Inverter Gate

Author: Edward Lee, b05901119@ntu.edu.tw

## Outline

- And-Inverter Gate Reader
- Functional-Reduced
    - `SWEEP`: Unused Gate Removel
    - `OPTIMIZATION`: Remove Constant Propagation / Constant 0
    - `STRASH`: Remove indentical AIG
    - `SIMULATE`: Find out FEC Groups by SIMULATION
    - `FRAIG`: Perform circuit reduction based on FEC groups segment by `SIMULATE`
    - `WRITE`: Outputt the circuit
    - `MITER`:

## Setup

```
make mac
make clean
make
```

## Data Structure

```cpp
class CirGate
{

}
```

```cpp
class CirMgr
{

}
```

## Algorithm

Hash Function
$$
\text{Hash}(a, b) = \frac{\max(a, b) * (\max(a, b) + 1)}{2} + \min(a, b)
$$

### Algorithm 1: `STRASH`

```cpp
void
CirMgr::Strash()
{

}
```

### Algorithm 2: `SIMULATION`

```cpp
void
CirMgr::Simulation()
{
}
```

```cpp
void
CirMgr::Fraig()
{

}
```

## Support Environment

Linux16 / Linux18 / Mac

## Reference

- Ric. NTUEE Data Structure and Programming, Fall 2019
- Hash Function:
- ./Description.pdf
- ./FraigProject.pdf
