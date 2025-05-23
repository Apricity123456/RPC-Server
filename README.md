RPC-Server

A C++ RPC-based server with authentication and file management functionalities.

Features

RPC-based communication using Protobuf & SRPC. User authentication with token-based security. File storage & management with upload/download support. Frontend UI for user interactions. Integration with AliyunOSS (optional).

Setup & Installation

Install Dependencies Ensure you have the following installed:

C++17 or later CMake (>= 3.16) Protobuf & SRPC gRPC (optional, if needed) For Ubuntu/Debian:

sudo apt update sudo apt install -y g++ cmake protobuf-compiler libprotobuf-dev For macOS (Homebrew):

brew install protobuf cmake For Windows (vcpkg):

vcpkg install protobuf:x64-windows Build & Compile Clone the repository:

git clone https://github.com/Apricity123456/RPC-Server.git cd RPC-Server Create a build directory & configure the project:

mkdir -p build && cd build cmake .. Compile the project:

make -j$(nproc) # For Linux/macOS cmake --build . # For Windows Usage Start the Server After compiling, run:

./bin/rpc_server Start the Client

./bin/rpc_client Access the Web Interface Open your browser and visit:

http://localhost:8080 API Endpoints User Signup

POST /user/signup { "username": "testuser", "password": "securepassword" } User Signin

POST /user/signin { "username": "testuser", "password": "securepassword" } Upload File

POST /file/upload Headers: { Authorization: "Bearer " } Download File

GET /file/download?filename=test.txt&token= License This project is licensed under the MIT License.
