# Eigenbasis
> **A high-performance C++ framework for building low-latency financial software.**

## 🚀 Getting Started


### Build Instructions

```bash
# Clone the repository
git clone https://github.com/bensaadi/eigenbasis-dev.git
cd eigenbasis-dev

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make
```

## 📖 Overview
**Eigenbasis** is a long-term project dedicated to engineering open-source trading technologies that meet state-of-the-art performance and reliability standards. 

The framework provides a suite of decoupled, highly optimized building blocks—from matching engines to order routers—allowing fintech developers to construct the next generation of financial products without reinventing the underlying infrastructure.

## 🏗 Approach
Our design philosophy prioritizes:
* **Low Latency**: Critical paths are optimized for cache locality and minimal branch misprediction.
* **Modularity**: Components are loosely coupled, allowing for independent use or seamless integration.
* **Testability**: Rigorous unit testing ensures reliability in high-stakes trading environments.

## 🧩 Modules & Status

| Module | Language | Description | Status |
| :--- | :---: | :--- | :---: |
| **book** | C++ | A modular, high-throughput Limit Order Book (LOB). | ✅ Released |
| **depth** | C++ | Aggregate depth order book with arbitrary precision. | ✅ Released |
| **margin-utils**| C++ | Utility classes for margin trading and automatic liquidation. | 🚧 Upcoming |
| **mm-quotes** | C++ | Generates orders given a stream of quotes from market makers. | 🚧 Upcoming |
| **router** | C++ | Real-time order routing to multiple external exchanges. | 🚧 Upcoming |
| **observer** | C++ | Template-based wrapper for the observer pattern (Intel TBB/Lock-free).| 🚧 Upcoming |
| **ohlc** | C++ | Incremental generation of OHLC data and indicators. | 🚧 Upcoming |
| **clearing-house**| C++ | Real-time balance settlement, netting, and fee calculation. | 🚧 Upcoming |
| **wsfix** | C++ | FAST-compressed market data streaming via WebSocket (WASM ready). | 🚧 Upcoming |
| **depth-chart** | JS/D3 | Real-time, interactive depth chart visualization. | 🚧 Upcoming |

### Contributors

This project is created and currently maintained by [L. Bensaadi](https://bensaadi.com/). If you are interested in contributing, feel free to [contact me](https://bensaadi.com/contact/).
