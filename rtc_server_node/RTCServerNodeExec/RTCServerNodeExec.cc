
#include <fstream>
#include <filesystem> 
#include <vector>
#include <iostream>
#include <string>
#include <charconv>
#include <csignal>
#include <unistd.h>

#include "../RTCServerNode/rtc_server_node_controller/RTCServerNodeController.h"

bool is_numeric(const std::string& str, int& out) {
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), out);
    return ec == std::errc();
}

bool is_valid_ip(const std::string& ip) {
    int dots = 0;
    int num = 0;
    int count = 0;

    for (char ch : ip) {
        if (ch == '.') {
            if (count == 0 || num < 0 || num > 255) return false;
            dots++;
            num = 0;
            count = 0;
        } else if (isdigit(ch)) {
            num = num * 10 + (ch - '0');
            count++;
            if (count > 3) return false;
        } else {
            return false;
        }
    }
    if (dots != 3) return false;
    if (num < 0 || num > 255) return false;
    return true;
}

int read_or_create_listen_config(const std::string& filepath, std::string& out_ip, int& out_port) {
    namespace fs = std::filesystem;

    if (fs::exists(filepath)) {
        std::ifstream file(filepath);
        if (file.is_open()) {
            std::string ip_line;
            std::string port_line;
            std::getline(file, ip_line);
            std::getline(file, port_line);
            file.close();

            if (is_valid_ip(ip_line) && is_numeric(port_line, out_port)) {
                out_ip = ip_line;
                return 0;
            } else {
                fs::remove(filepath); 
            }
        }
    }

    std::string ip_input;
    std::string port_input;
    while (true) {
        std::cout << "input signaling server listen ip address: ";
        std::getline(std::cin, ip_input);
        if (!is_valid_ip(ip_input)) {
            std::cout << "wrong ip address: " << ip_input << std::endl;
            continue;
        }

        std::cout << "input signaling server listen port: ";
        std::getline(std::cin, port_input);
        if (!is_numeric(port_input, out_port)) {
            std::cout << "wrong listen port: " << port_input << std::endl;
            continue;
        }

        break;
    }

    std::ofstream out(filepath);
    if (out.is_open()) {
        out << ip_input << std::endl;
        out << out_port << std::endl;
        out.close();
    } else {
        std::cerr << "Failed to write port config file.\n";
    }

    out_ip = ip_input;

    return 0;
}


static void display_tester_command() {
	printf("start - s\n");
	printf("stop - e\n");
}

std::unique_ptr<RTCServerNodeController> server_node = nullptr;
bool is_closed = false;

void signal_handler(int signo)
{
	if (signo == SIGINT) {
		if (server_node) {
			server_node->Stop();
			while (!is_closed) {
				usleep(1000 * 10);
			}
		}

		exit(0);
	}
}

static void node_state_callback(RTCServerNodeController::NodeSignalState state, std::string stateDesc) {
	if (state == RTCServerNodeController::NodeState_Signal_Closed 
		|| state == RTCServerNodeController::NodeState_Signal_Error) {
        std::cout << "node_state_callback!!!!! " << stateDesc.c_str() << std::endl;
		is_closed = true;
	}
}

int main() {
	std::signal(SIGINT, signal_handler);

	server_node = RTCServerNodeController::Create(node_state_callback);
	if (server_node == nullptr) {
		printf("create server fail!!!\n");
		return 0;
	}

    std::string ip_addr;
	int port;
	read_or_create_listen_config("port.cfg", ip_addr, port);
	std::cout << "signaling server info " << ip_addr.c_str() << " port "<< port << std::endl;

	display_tester_command();

	char cmd;
	bool is_end = false;
	bool is_start = false;
	
	while (1) {
		if (is_start) {
			cmd = getchar();
		}
		else {
			cmd = 's';
		}

		switch (cmd) {
		case 's':
			{
				if (is_start == false) {
					std::cout << "Start Server" << std::endl;
					server_node->Start(ip_addr, port);
					is_start = true;
				}
				break;
			}
		case 'e':
			{
				if (is_start) {
					std::cout << "Stop Server" << std::endl;
					server_node->Stop();
					is_start = false;
				}
                is_end = true;
				break;
			}
		}

		if (is_end) {
			while (!is_closed) {
				usleep(1000 * 10);
			}
			break;
		}
	}

	return 0;
}