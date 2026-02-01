# Logger for Data Integrity and Authenticity

With the rapid growth of Digital Transformation, data pollution has become a critical threat to decision-making and AI reliability. Our Secure Logger addresses this by establishing data integrity and authenticity at the point of capture, ensuring a high-trust pipeline for cloud-based data analytics.


## 🛠 Technical Decisions & Justifications

### 1. C++ over Python
- **Decision**: Used C++ API for both Hailo NPU and OpenCV.
- **Why**: Python's Global Interpreter Lock (GIL) and interpreter overhead are critical bottlenecks for real-time processing. C++ provides deterministic performance and fine-grained control over hardware resources, ensuring a stable 30+ FPS.

### 2. libsodium over OpenSSL
- **Decision**: Ed25519 digital signatures via libsodium.
- **Why**: OpenSSL is overly complex and prone to configuration errors. Libsodium offers a modern, high-speed, and "misuse-resistant" API, which is ideal for securing edge-to-server communications without heavy overhead.

### 3. nlohmann/json over jsoncpp
- **Decision**: nlohmann/json (Modern C++ JSON).
- **Why**: Its header-only nature simplifies the cross-compilation process for ARM-based devices. The intuitive, STL-like syntax also improves code readability and reduces development time.