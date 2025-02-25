RPC-Server
A C++ RPC-based server with authentication and file management functionalities.

Features
RPC-based communication using Protobuf & SRPC.
User authentication with token-based security.
File storage & management with upload/download support.
Frontend UI for user interactions.
Integration with AliyunOSS (optional).
Project Structure

Setup & Installation
Install Dependencies
Ensure you have the following installed:

C++17 or later
CMake (>= 3.16)
Protobuf & SRPC
gRPC (optional, if needed)
For Ubuntu/Debian:


sudo apt update
sudo apt install -y g++ cmake protobuf-compiler libprotobuf-dev
For macOS (Homebrew):


brew install protobuf cmake
For Windows (vcpkg):


vcpkg install protobuf:x64-windows
Build & Compile
Clone the repository:


git clone https://github.com/Apricity123456/RPC-Server.git
cd RPC-Server
Create a build directory & configure the project:


mkdir -p build && cd build
cmake ..
Compile the project:


make -j$(nproc)  # For Linux/macOS
cmake --build .  # For Windows
Usage
Start the Server
After compiling, run:

./bin/rpc_server
Start the Client

./bin/rpc_client
Access the Web Interface
Open your browser and visit:


http://localhost:8080
API Endpoints
User Signup

POST /user/signup
{
    "username": "testuser",
    "password": "securepassword"
}
User Signin

POST /user/signin
{
    "username": "testuser",
    "password": "securepassword"
}
Upload File

POST /file/upload
Headers: { Authorization: "Bearer <token>" }
Download File

GET /file/download?filename=test.txt&token=<token>
License
This project is licensed under the MIT License.

Contributing
Fork the repository.
Create a new branch (git checkout -b feature-branch).
Make your changes and commit (git commit -m "Add new feature").
Push to the branch (git push origin feature-branch).
Open a Pull Request.
Contact
For any issues or contributions, feel free to open an issue or reach out to:

Email: yangshangyu98@gmail.com

